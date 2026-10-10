# SubGyatty (Moog Sub Phatty Emulation): Master Architecture & Execution Plan

> **Living Technical Specification & Methodology Document**  
> *Target Platforms:* Desktop (VST3 / CLAP / Standalone GUI), Embedded Linux (Raspberry Pi Zero), Microcontroller (Teensy 4.0 / 4.1 Bare Metal).  
> *Core Mandate:* Write the DSP engine once as a pure, zero-allocation, platform-agnostic C++ library, wrapped by target-specific shells.

---

## 1. Architectural Philosophy & Non-Negotiable Invariants

To achieve identical acoustic behavior across desktop audio workstations, embedded Linux boxes, and a 600 MHz microcontroller without rewriting DSP code:

### Core Invariants
1. **Zero Runtime Allocation on Audio Paths**:
   - `no malloc`, `no new`, `no std::vector::push_back`, `no std::string` manipulation inside any audio rendering block.
   - All state variables, filter integrator states, circular buffers, and envelope states must be statically sized or allocated during engine initialization.
2. **Single-Precision Floating Point (`float`)**:
   - Standardize on `float` across all DSP code. Teensy 4.x (Cortex-M7) features a single-precision hardware FPU. Using `double` causes software-emulated slowdowns on microcontrollers.
3. **Decoupled Architecture ("Pure Core")**:
   - The core DSP (`core/`) must have **zero dependencies** on JUCE, ALSA, Teensy Audio, OS threads, file I/O, or graphics.
   - Input: MIDI events and sample buffers. Output: 32-bit floating-point audio blocks.
4. **Fast Math Approximations for Non-Linearities**:
   - The Moog ladder filter and Multidrive saturation circuits rely on hyperbolic tangent (`tanh`) curves.
   - On Desktop and embedded systems, use branchless rational approximations (e.g., Padé approximant or $f(x) = \frac{x}{1 + |x|}$ or optimized polynomial saturation) with compile-time or configurable quality switches.
5. **Sample Rate & Block Size Independence**:
   - Teensy audio processes fixed blocks of 128 samples at 44.1 kHz.
   - Desktop VST3 hosts process variable buffer sizes (32 to 2048 samples) at arbitrary rates (44.1 kHz to 96 kHz).
   - All modules must step per sample or per variable block seamlessly.

---

## 2. Signal Path & Component Specifications

```
                     ┌────────────────────────────────────────────────────────┐
                     │                     OSCILLATOR SECTION                  │
                     │  - Osc 1: Continuous Tri -> Saw -> Square -> Pulse     │
                     │  - Osc 2: Continuous Shape + Beat Detune + Hard Sync   │
                     │  - Sub-Osc: Square (-1 Octave)                         │
                     │  - Noise: Pink Noise Generator                         │
                     └───────────────────────────┬────────────────────────────┘
                                                 │
                                                 ▼
                     ┌────────────────────────────────────────────────────────┐
                     │               MIXER (Pre-Filter Saturation)             │
                     │  - Overdrive saturation past 12 o'clock                │
                     │  - Summed signal drives into the ladder filter         │
                     └───────────────────────────┬────────────────────────────┘
                                                 │
                                                 ▼
                     ┌────────────────────────────────────────────────────────┐
                     │            4-POLE ZDF TRANSISTOR LADDER FILTER         │
                     │  - 24 dB/oct slope (optional 6, 12, 18 dB/oct)         │
                     │  - Zero-Delay Feedback (ZDF) topology                  │
                     │  - Self-oscillating non-linear resonance               │
                     └───────────────────────────┬────────────────────────────┘
                                                 │
                                                 ▼
                     ┌────────────────────────────────────────────────────────┐
                     │                   MULTIDRIVE CIRCUIT                   │
                     │  - Dual-stage OTA pre-saturation + FET/Diode clipping  │
                     │  - Asymmetric harmonic generation, tight bass response │
                     └───────────────────────────┬────────────────────────────┘
                                                 │
                                                 ▼
                     ┌────────────────────────────────────────────────────────┐
                     │                     VCA & OUTPUT STAGE                 │
                     │  - Exponential Amp Envelope Modulation                 │
                     │  - Master Volume                                       │
                     └────────────────────────────────────────────────────────┘
```

### 2.1. Oscillators (`Oscillator.hpp`, `SubOscillator.hpp`, `NoiseGenerator.hpp`)
* **Continuous Wave-Morphing**:
  - Triangle $\rightarrow$ Sawtooth $\rightarrow$ Square $\rightarrow$ Narrow Pulse (pulsewidth 1% to 50%).
  - Smooth interpolation across shapes without phase discontinuity.
* **Anti-Aliasing**:
  - PolyBLEP (Polynomial Bandlimited Step) residual correction applied at waveform transitions. Low CPU overhead, highly effective on Cortex-M7.
* **Hard Sync**:
  - When Sync is active, Osc 1 resets the phase of Osc 2 to 0 whenever Osc 1 crosses zero with a positive slope.
* **Sub-Oscillator**:
  - Phase-locked square wave exactly 1 octave lower ($f / 2$) than Osc 1.
* **Noise**:
  - Digital pink noise filter (Paul Kellet / Gardner filter cascade over white noise generator).

### 2.2. Mixer Stage (`Mixer.hpp`)
* Individual level controls for Osc 1, Osc 2, Sub-Osc, and Noise.
* Non-linear summing: Signals above 50% knob travel progressively soft-clip via asymmetric polynomial saturation before reaching the filter.

### 2.3. Zero-Delay Feedback (ZDF) Ladder Filter (`LadderFilter.hpp`)
* **Topology**: 4-pole transistor ladder based on Vadim Zavalishin's Zero-Delay Feedback (ZDF) model with non-linear saturation per stage.
* **Resonance**:
  - Closed-loop feedback with resonance amount from 0 to self-oscillation.
  - Native resonance frequency tracking up to Nyquist without pitch warping.
* **Slope Modes (Shift Parameter)**:
  - 1-pole (6 dB/oct), 2-pole (12 dB/oct), 3-pole (18 dB/oct), 4-pole (24 dB/oct).

### 2.4. Multidrive Circuit (`Multidrive.hpp`)
* The signature feature of the Sub Phatty:
  1. Pre-filter OTA drive: Increases input gain into the ladder stages.
  2. Post-filter clipping: Asymmetrical diode and FET waveshaping curve adding 2nd and 3rd harmonic richness without destroying bass frequency energy.

### 2.5. Envelopes & Modulation (`Envelope.hpp`, `LFO.hpp`)
* **Envelopes**:
  - Two DAHDSR (Delay, Attack, Hold, Decay, Sustain, Release) envelope generators: Filter Envelope and Amp Envelope.
  - Analog exponential charging and discharging curves ($RC$ circuit emulation).
  - Fast snappy attacks down to 1 ms.
* **LFO**:
  - Shapes: Triangle, Square, Saw, Ramp, Sample & Hold.
  - Range: 0.1 Hz to 1.0 kHz (Audio rate modulation for metallic/FM timbres).
  - Destinations: Pitch (both or Osc 2 only), Filter Cutoff, Wave Shape.

### 2.6. Voice Controller (`Voice.hpp`)
* Monophonic voice allocation with selectable note priority (Lowest, Highest, Last).
* Legato triggering: Re-trigger envelopes or smooth glide without envelope re-attack.
* Portamento / Glide: Linear constant-rate or exponential constant-time glide.

---

## 3. Multi-Target Project Layout

```
SubGyatty/
├── CMakeLists.txt                  <-- Top-level multi-target build orchestration
├── MOOG_SUB_PHATTY_ARCHITECTURE_PLAN.md <-- This living plan
│
├── core/                           <-- SHARED C++20 ZERO-ALLOCATION DSP
│   ├── CMakeLists.txt
│   ├── include/sub_phatty/
│   │   ├── Common.hpp              <-- Fast math, sample rate constants, types
│   │   ├── Oscillator.hpp          <-- PolyBLEP Continuous Wave Morph
│   │   ├── SubOscillator.hpp       <-- -1 Octave square
│   │   ├── NoiseGenerator.hpp      <-- Pink noise
│   │   ├── Mixer.hpp               <-- Pre-filter saturation
│   │   ├── LadderFilter.hpp        <-- 4-pole ZDF Ladder Filter
│   │   ├── Multidrive.hpp          <-- Dual-stage overdrive circuit
│   │   ├── Envelope.hpp            <-- DAHDSR analog exponential curves
│   │   ├── LFO.hpp                 <-- Audio-rate capable LFO
│   │   ├── Voice.hpp               <-- Monophonic voice, priority & glide
│   │   ├── Parameters.hpp          <-- Parameter IDs & MIDI CC bindings
│   │   └── SynthEngine.hpp         <-- Main engine orchestrator
│   └── src/
│       └── SynthEngine.cpp
│
├── targets/
│   ├── desktop/                    <-- JUCE VST3 / CLAP / STANDALONE APP
│   │   ├── CMakeLists.txt
│   │   └── Source/
│   │       ├── PluginProcessor.cpp <-- Bridges JUCE AudioProcessor to SynthEngine
│   │       ├── PluginProcessor.h
│   │       ├── PluginEditor.cpp    <-- Front panel GUI (hardware look)
│   │       └── PluginEditor.h
│   │
│   ├── headless_pi/                <-- EMBEDDED LINUX (Raspberry Pi Zero)
│   │   ├── CMakeLists.txt
│   │   ├── main_alsa.cpp           <-- ALSA audio loop + ALSA RawMIDI listener
│   │   └── install_service.sh      <-- systemd auto-start service for boot-to-play
│   │
│   └── teensy/                     <-- BARE METAL TEENSY 4.0 / 4.1
│       ├── platformio.ini          <-- PlatformIO configuration (Teensyduino)
│       └── src/
│           ├── AudioStreamSubPhatty.h <-- Teensy PJRC AudioStream adapter
│           └── main.cpp            <-- USB Host MIDI listener + Audio engine
│
└── tests/
    ├── CMakeLists.txt
    ├── dsp_verification.cpp        <-- Offline render & audio quality benchmark
    └── wav_writer.hpp              <-- Header-only test audio exporter
```

---

## 4. Multi-Phase Execution Roadmap

### Phase 1: Pure DSP Core & Offline Audio Verification
* **Goal**: Implement all mathematical DSP blocks in `core/` and verify audio quality with offline `.wav` test renders.
* **Deliverables**:
  1. `core/include/sub_phatty/` headers (Oscillator, Mixer, LadderFilter, Multidrive, Envelope, LFO, Voice, SynthEngine).
  2. `tests/dsp_verification.cpp` standalone test app that renders:
     - Oscillator continuous wave-morph sweep.
     - Hard sync pitch sweeps.
     - Ladder filter cutoff sweep with self-oscillating resonance.
     - Multidrive saturation sweeps.
     - Full monophonic bassline audio snippet.
* **Verification Criterion**: Generate clean, non-aliasing 44.1 kHz and 48 kHz `.wav` files with zero clicks and authentic Moog warmth.

### Phase 2: Desktop JUCE Wrapper (VST3 + Standalone)
* **Goal**: Wrap `SynthEngine` inside JUCE, enabling plug-and-play in any DAW or as a standalone desktop executable.
* **Deliverables**:
  1. `targets/desktop/Source/PluginProcessor.cpp` mapping JUCE `AudioBuffer<float>` and `MidiBuffer` to `sub_phatty::SynthEngine`.
  2. AudioProcessorValueTreeState (APVTS) parameter binding for automation and preset persistence.
  3. Basic GUI controls for live testing on PC.
* **Verification Criterion**: Builds VST3 and Standalone executable; playable via USB MIDI keyboard with sub-5ms latency on Windows/macOS.

### Phase 3: Headless Embedded Linux Target (Raspberry Pi Zero)
* **Goal**: Direct ALSA audio output and USB RawMIDI input on Linux with zero desktop overhead.
* **Deliverables**:
  1. `targets/headless_pi/main_alsa.cpp` using `alsa/asoundlib.h`.
  2. Low-latency multi-threaded architecture (Thread 1: ALSA audio pump; Thread 2: MIDI polling).
  3. `install_service.sh` systemd script configuring the Pi to auto-boot directly into synthesizer mode.
* **Verification Criterion**: Pi boots in ~15s, detects plugged-in USB MIDI keyboard, and outputs glitch-free audio through USB DAC or I2S HAT (PCM5102).

### Phase 4: Bare-Metal Teensy 4.0/4.1 Firmware Target
* **Goal**: Sub-half-second boot time and zero-OS plug-and-play operation.
* **Deliverables**:
  1. `targets/teensy/src/AudioStreamSubPhatty.h` subclassing PJRC `AudioStream`.
  2. Teensy `USBHost_t36` MIDI listener connecting directly to class-compliant MIDI keyboards.
  3. Hardware pin assignments for optional physical potentiometers (Cutoff, Resonance, Multidrive, Volume).
* **Verification Criterion**: Compiles cleanly with GCC ARM Embedded (PlatformIO); runs at under 25% CPU on Cortex-M7 @ 600 MHz with 128-sample buffer.

### Phase 5: Hardware-Authentic UI & Shift Parameters
* **Goal**: Complete front-panel styling matching the physical Sub Phatty, including the 16 hidden under-the-hood Shift functions.
* **Deliverables**:
  1. Custom hardware knobs, LED switches, and pitch/mod wheels.
  2. Shift mode entry (Bank 4 + Activate buttons) to adjust filter slopes, pitch bend ranges, and envelope modes.
  3. Preset library featuring factory Sub Phatty patches.

---

## 5. Guidelines for Downstream IDE Agent Execution

When instructed to execute any phase from this plan:
1. **Never violate the Clean Core rule**: Do not include `<juce_audio_processors/...>` or `<alsa/asoundlib.h>` inside `core/`.
2. **Always test DSP with the offline test harness** before plugging into UI.
3. **Run audio benchmarks** to guarantee that single-sample rendering takes $< 100\text{ ns}$ on modern CPUs to ensure plenty of headroom on 600 MHz microcontrollers.
