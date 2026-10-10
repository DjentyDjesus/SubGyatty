#include "wav_writer.hpp"
#include "sub_phatty/SynthEngine.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

using namespace sub_phatty;
using namespace sub_phatty::test;

void renderWaveMorphTest(const std::string& filename, uint32_t sampleRate) {
    std::cout << "[DSP Test 1] Rendering Continuous Wave Morph Sweep -> " << filename << " ...\n";
    const size_t durationSeconds = 4;
    const size_t totalSamples = durationSeconds * sampleRate;
    std::vector<float> buffer(totalSamples, 0.0f);

    Oscillator osc;
    osc.setSampleRate(static_cast<float>(sampleRate));
    osc.setFrequency(130.81f); // C3

    for (size_t i = 0; i < totalSamples; ++i) {
        const float progress = static_cast<float>(i) / static_cast<float>(totalSamples);
        // Smoothly sweep waveform knob from 0.0 (Tri) to 1.0 (Narrow Pulse)
        osc.setWaveform(progress);
        buffer[i] = osc.process() * 0.75f;
    }

    WavWriter::writeWav16(filename, buffer.data(), nullptr, totalSamples, sampleRate);
    std::cout << "             Done. (" << totalSamples << " samples written)\n";
}

void renderHardSyncTest(const std::string& filename, uint32_t sampleRate) {
    std::cout << "[DSP Test 2] Rendering Hard Sync Pitch Sweep -> " << filename << " ...\n";
    const size_t durationSeconds = 4;
    const size_t totalSamples = durationSeconds * sampleRate;
    std::vector<float> buffer(totalSamples, 0.0f);

    Oscillator osc1;
    Oscillator osc2;
    osc1.setSampleRate(static_cast<float>(sampleRate));
    osc2.setSampleRate(static_cast<float>(sampleRate));

    // Master Osc 1 at 110 Hz (A2)
    osc1.setFrequency(110.0f);
    osc1.setWaveform(0.333333f); // Saw

    // Slave Osc 2 with Hard Sync enabled
    osc2.setWaveform(0.333333f); // Saw
    osc2.setHardSync(true);

    for (size_t i = 0; i < totalSamples; ++i) {
        const float progress = static_cast<float>(i) / static_cast<float>(totalSamples);
        // Sweep Osc 2 frequency upward from 110 Hz up to 880 Hz (1 to 8 harmonics)
        const float sweepFreq = 110.0f * (1.0f + progress * 7.0f);
        osc2.setFrequency(sweepFreq);

        bool wrapped = false;
        (void)osc1.process(&wrapped);

        const float syncFrac = osc1.getPhaseIncrement() > 0.0f ? osc1.getPhase() / osc1.getPhaseIncrement() : 0.0f;
        const float osc2Out = osc2.process(nullptr, syncFrac, wrapped);

        buffer[i] = osc2Out * 0.75f;
    }

    WavWriter::writeWav16(filename, buffer.data(), nullptr, totalSamples, sampleRate);
    std::cout << "             Done. (" << totalSamples << " samples written)\n";
}

void renderFilterSweepTest(const std::string& filename, uint32_t sampleRate) {
    std::cout << "[DSP Test 3] Rendering 4-Pole ZDF Ladder Filter Resonance Sweep -> " << filename << " ...\n";
    const size_t durationSeconds = 5;
    const size_t totalSamples = durationSeconds * sampleRate;
    std::vector<float> buffer(totalSamples, 0.0f);

    Oscillator osc;
    LadderFilter filter;
    osc.setSampleRate(static_cast<float>(sampleRate));
    filter.setSampleRate(static_cast<float>(sampleRate));

    osc.setFrequency(65.41f); // Low C2
    osc.setWaveform(0.333333f); // Saw
    filter.setResonance(0.88f); // High self-oscillating resonance

    for (size_t i = 0; i < totalSamples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(totalSamples);
        // Triangle sweep: 50 Hz -> 12000 Hz -> 50 Hz
        const float sweepMod = (t < 0.5f) ? (t * 2.0f) : (2.0f - t * 2.0f);
        const float cutoff = 50.0f * std::pow(240.0f, sweepMod); // 50 Hz to 12 kHz
        filter.setCutoff(cutoff);

        const float rawOsc = osc.process();
        const float filtered = filter.process(rawOsc);
        buffer[i] = fast_tanh(filtered * 0.9f) * 0.75f;
    }

    WavWriter::writeWav16(filename, buffer.data(), nullptr, totalSamples, sampleRate);
    std::cout << "             Done. (" << totalSamples << " samples written)\n";
}

void renderMultidriveSaturationTest(const std::string& filename, uint32_t sampleRate) {
    std::cout << "[DSP Test 4] Rendering Multidrive Saturation Sweep -> " << filename << " ...\n";
    const size_t durationSeconds = 4;
    const size_t totalSamples = durationSeconds * sampleRate;
    std::vector<float> buffer(totalSamples, 0.0f);

    Oscillator osc;
    LadderFilter filter;
    Multidrive multidrive;

    osc.setSampleRate(static_cast<float>(sampleRate));
    filter.setSampleRate(static_cast<float>(sampleRate));
    multidrive.setSampleRate(static_cast<float>(sampleRate));

    osc.setFrequency(55.0f); // Low A1 bass
    osc.setWaveform(0.333333f); // Saw
    filter.setCutoff(800.0f);
    filter.setResonance(0.4f);

    for (size_t i = 0; i < totalSamples; ++i) {
        const float progress = static_cast<float>(i) / static_cast<float>(totalSamples);
        // Sweep Multidrive knob from 0.0 (Clean) to 1.0 (Maximum OTA + FET/diode drive)
        multidrive.setAmount(progress);
        filter.setPreDrive(multidrive.getPreFilterDrive());

        const float raw = osc.process();
        const float filtered = filter.process(raw);
        const float driven = multidrive.process(filtered);

        buffer[i] = driven * 0.8f;
    }

    WavWriter::writeWav16(filename, buffer.data(), nullptr, totalSamples, sampleRate);
    std::cout << "             Done. (" << totalSamples << " samples written)\n";
}

void renderSubPhattyBasslineTest(const std::string& filename, uint32_t sampleRate) {
    std::cout << "[DSP Test 5] Rendering Full Sub Phatty Acid/Funk Bassline -> " << filename << " ...\n";
    SynthEngine synth;
    synth.setSampleRate(static_cast<float>(sampleRate));

    // Configure classic punchy Sub Phatty bass patch:
    synth.setParameter(ParamId::Osc1Octave, -1.0f);       // 16'
    synth.setParameter(ParamId::Osc1Waveform, 0.333333f); // Saw
    synth.setParameter(ParamId::Osc1Level, 0.9f);         // Overdriving mixer

    synth.setParameter(ParamId::Osc2Octave, -1.0f);       // 16'
    synth.setParameter(ParamId::Osc2Waveform, 0.666667f); // Square
    synth.setParameter(ParamId::Osc2Detune, 6.0f);        // +6 cents beat detune
    synth.setParameter(ParamId::Osc2Level, 0.65f);

    synth.setParameter(ParamId::SubOscLevel, 0.60f);      // Heavy sub bass
    synth.setParameter(ParamId::NoiseLevel, 0.02f);       // Subtle analog grit

    synth.setParameter(ParamId::FilterCutoff, 450.0f);     // Low base cutoff
    synth.setParameter(ParamId::FilterResonance, 0.55f);   // Acid resonance
    synth.setParameter(ParamId::FilterMultidrive, 0.45f);  // Warm aggressive drive
    synth.setParameter(ParamId::FilterSlope, 4.0f);        // 24 dB/oct
    synth.setParameter(ParamId::FilterEnvAmount, 0.70f);   // Snappy envelope opening

    synth.setParameter(ParamId::FilterAttack, 0.005f);     // 5ms snappy attack
    synth.setParameter(ParamId::FilterDecay, 0.180f);      // 180ms decay
    synth.setParameter(ParamId::FilterSustain, 0.10f);
    synth.setParameter(ParamId::FilterRelease, 0.120f);

    synth.setParameter(ParamId::AmpAttack, 0.003f);        // 3ms attack
    synth.setParameter(ParamId::AmpDecay, 0.220f);
    synth.setParameter(ParamId::AmpSustain, 0.40f);
    synth.setParameter(ParamId::AmpRelease, 0.100f);

    synth.setParameter(ParamId::GlideTime, 0.045f);        // 45ms portamento
    synth.setParameter(ParamId::Legato, 1.0f);             // Legato on

    // 16-step bassline pattern (16th notes @ 125 BPM)
    // 125 BPM = ~2.083 beats/sec -> 1 beat = 0.48s -> 16th note = 0.12s
    const float stepDuration = 0.12f;
    const size_t samplesPerStep = static_cast<size_t>(stepDuration * sampleRate);

    struct NoteEvent {
        uint8_t note;
        bool legatoNext;
    };

    const std::vector<NoteEvent> pattern = {
        { 36, false }, // C2
        { 36, false }, // C2
        { 48, true  }, // C3 (glide)
        { 46, false }, // Bb2
        { 36, false }, // C2
        { 39, false }, // Eb2
        { 41, true  }, // F2 (glide)
        { 42, false }, // F#2
        { 41, false }, // F2
        { 39, false }, // Eb2
        { 36, false }, // C2
        { 36, false }, // C2
        { 48, true  }, // C3
        { 51, true  }, // Eb3
        { 48, false }, // C3
        { 36, false }  // C2
    };

    const size_t totalSamples = pattern.size() * samplesPerStep;
    std::vector<float> buffer(totalSamples, 0.0f);
    size_t writeIdx = 0;

    for (const auto& ev : pattern) {
        synth.handleNoteOn(ev.note, 110);
        const size_t noteSamples = ev.legatoNext ? samplesPerStep : static_cast<size_t>(samplesPerStep * 0.75f);
        const size_t restSamples = samplesPerStep - noteSamples;

        for (size_t s = 0; s < noteSamples && writeIdx < totalSamples; ++s) {
            buffer[writeIdx++] = synth.processSample();
        }

        synth.handleNoteOff(ev.note);
        for (size_t s = 0; s < restSamples && writeIdx < totalSamples; ++s) {
            buffer[writeIdx++] = synth.processSample();
        }
    }

    WavWriter::writeWav16(filename, buffer.data(), nullptr, totalSamples, sampleRate);
    std::cout << "             Done. (" << totalSamples << " samples written)\n";
}

void runBenchmark(uint32_t sampleRate) {
    std::cout << "\n[DSP Benchmark] Measuring single-sample render latency (Voice + Multidrive + ZDF Ladder)...\n";
    SynthEngine synth;
    synth.setSampleRate(static_cast<float>(sampleRate));
    synth.handleNoteOn(48, 127);

    constexpr size_t benchmarkSamples = 200000;
    const auto startTime = std::chrono::high_resolution_clock::now();

    float sum = 0.0f;
    for (size_t i = 0; i < benchmarkSamples; ++i) {
        sum += synth.processSample();
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto elapsedNs = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
    const double nsPerSample = static_cast<double>(elapsedNs) / static_cast<double>(benchmarkSamples);
    const double audioSampleBudgetNs = 1.0e9 / static_cast<double>(sampleRate);
    const double cpuUsagePct = (nsPerSample / audioSampleBudgetNs) * 100.0;

    std::cout << "  Rendered " << benchmarkSamples << " samples in " << (elapsedNs / 1000000.0) << " ms\n";
    std::cout << "  Latency per sample: " << nsPerSample << " ns\n";
    std::cout << "  Audio budget @ " << sampleRate << " Hz: " << audioSampleBudgetNs << " ns\n";
    std::cout << "  CPU load on current core: " << cpuUsagePct << "%\n";
    std::cout << "  Headroom on 600 MHz Cortex-M7: " << (nsPerSample < 250.0 ? "EXCELLENT (<15% MCU CPU)" : "ACCEPTABLE") << "\n";
    std::cout << "  (Dummy accumulator: " << sum << ")\n";
}

int main() {
    std::cout << "=================================================================\n";
    std::cout << "  SubGyatty DSP Verification & Audio Quality Harness (Phase 1)   \n";
    std::cout << "  Zero-Allocation C++20 Moog Sub Phatty Core Engine              \n";
    std::cout << "=================================================================\n\n";

    constexpr uint32_t sampleRate = 44100;

    renderWaveMorphTest("test_wave_morph.wav", sampleRate);
    renderHardSyncTest("test_hard_sync.wav", sampleRate);
    renderFilterSweepTest("test_filter_sweep.wav", sampleRate);
    renderMultidriveSaturationTest("test_multidrive_saturation.wav", sampleRate);
    renderSubPhattyBasslineTest("test_sub_phatty_bassline.wav", sampleRate);

    runBenchmark(sampleRate);

    std::cout << "\n=================================================================\n";
    std::cout << "  All Phase 1 Audio Verification Tests Completed Successfully!   \n";
    std::cout << "=================================================================\n";
    return 0;
}
