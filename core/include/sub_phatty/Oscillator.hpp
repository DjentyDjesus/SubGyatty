#pragma once

#include "Common.hpp"

namespace sub_phatty {

class Oscillator {
public:
    Oscillator() noexcept = default;

    void setSampleRate(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : DEFAULT_SAMPLE_RATE;
        invSampleRate_ = 1.0f / sampleRate_;
        updatePhaseIncrement();
    }

    void reset() noexcept {
        phase_ = 0.0f;
    }

    // Set base pitch (in Hz)
    void setFrequency(float freqHz) noexcept {
        frequency_ = clamp(freqHz, 5.0f, sampleRate_ * 0.49f);
        updatePhaseIncrement();
    }

    // Octave switch: -2 (16'), -1 (8'), 0 (4'), +1 (2')
    void setOctave(int octave) noexcept {
        octaveOffset_ = octave;
        updatePhaseIncrement();
    }

    // Fine tune / detune in semitones (e.g. +/- 7 semitones)
    void setDetuneSemitones(float semitones) noexcept {
        detuneSemitones_ = semitones;
        updatePhaseIncrement();
    }

    // Detune in cents (e.g. +/- 100 cents)
    void setDetuneCents(float cents) noexcept {
        detuneCents_ = cents;
        updatePhaseIncrement();
    }

    // Continuous wave morph: 0.0 (Tri) -> 0.333 (Saw) -> 0.666 (Square 50%) -> 1.0 (Narrow Pulse ~5%)
    void setWaveform(float wave) noexcept {
        wave_ = clamp(wave, 0.0f, 1.0f);
    }

    [[nodiscard]] float getWaveform() const noexcept {
        return wave_;
    }

    // Manual pulse width offset (shifts duty cycle in pulse mode)
    void setPulseWidth(float pw) noexcept {
        pulseWidthOffset_ = clamp(pw, -0.45f, 0.45f);
    }

    // Enable/disable hard sync to master oscillator
    void setHardSync(bool enabled) noexcept {
        hardSyncEnabled_ = enabled;
    }

    [[nodiscard]] bool isHardSyncEnabled() const noexcept {
        return hardSyncEnabled_;
    }

    // External pitch modulation (e.g. from Pitch Bend, LFO, Filter Env) in semitones
    void setPitchModulation(float semitones) noexcept {
        pitchModSemitones_ = semitones;
        updatePhaseIncrement();
    }

    // Process one sample and return anti-aliased audio output.
    // Out-parameter `masterWrapped` is set to true if this oscillator wrapped its phase
    // this sample (for slaving other oscillators in Hard Sync).
    [[nodiscard]] float process(bool* masterWrapped = nullptr, float syncFraction = 0.0f, bool syncTrigger = false) noexcept {
        // Handle incoming hard sync from a master oscillator
        if (hardSyncEnabled_ && syncTrigger) {
            // Hard sync reset: align phase to the fractional sub-sample boundary
            phase_ = clamp(syncFraction, 0.0f, 1.0f) * dt_;
        }

        // Render current waveform at current phase
        const float out = renderWaveform(phase_, dt_, wave_, pulseWidthOffset_);

        // Increment phase
        phase_ += dt_;
        bool wrapped = false;
        if (phase_ >= 1.0f) {
            phase_ -= 1.0f;
            wrapped = true;
        }

        if (masterWrapped != nullptr) {
            *masterWrapped = wrapped;
        }

        return out;
    }

    [[nodiscard]] float getPhase() const noexcept {
        return phase_;
    }

    [[nodiscard]] float getPhaseIncrement() const noexcept {
        return dt_;
    }

private:
    void updatePhaseIncrement() noexcept {
        const float totalSemitones = static_cast<float>(octaveOffset_ * 12) + detuneSemitones_ + (detuneCents_ * 0.01f) + pitchModSemitones_;
        const float effectiveFreq = frequency_ * semitonesToRatio(totalSemitones);
        const float clampedFreq = clamp(effectiveFreq, 5.0f, sampleRate_ * 0.48f);
        dt_ = clampedFreq * invSampleRate_;
    }

    // PolyBLEP bandlimited square/pulse generator
    [[nodiscard]] static inline float generatePulse(float phase, float dt, float dutyCycle) noexcept {
        const float d = clamp(dutyCycle, 0.02f, 0.98f);
        float naive = (phase < d) ? 1.0f : -1.0f;

        // Transition 1 at phase = 0 (rising edge from -1 to +1: jump of +2)
        naive += polyBlep(phase, dt);

        // Transition 2 at phase = d (falling edge from +1 to -1: jump of -2)
        float pD = phase - d;
        if (pD < 0.0f) pD += 1.0f;
        naive -= polyBlep(pD, dt);

        // Remove DC component introduced by asymmetrical pulse width
        naive -= (2.0f * d - 1.0f);
        return naive;
    }

    // PolyBLEP bandlimited sawtooth generator
    [[nodiscard]] static inline float generateSaw(float phase, float dt) noexcept {
        // Naive saw ramps from -1 to +1, then drops by 2 at phase = 0
        const float naive = 2.0f * phase - 1.0f;
        return naive - polyBlep(phase, dt);
    }

    // PolyBLAMP bandlimited triangle generator
    [[nodiscard]] static inline float generateTriangle(float phase, float dt) noexcept {
        // Triangle has slope transitions at phase = 0 and phase = 0.5
        // Naive triangle from -1 to +1
        float naive = 4.0f * std::fabs(phase - 0.5f) - 1.0f;

        // Bandlimiting derivative discontinuities via integral of PolyBLEP
        auto blamp = [](float p, float dtStep) noexcept -> float {
            if (dtStep <= 0.0f) return 0.0f;
            if (p < dtStep) {
                const float t = p / dtStep;
                return dtStep * (t * t * t / 3.0f - t * t + t);
            } else if (p > 1.0f - dtStep) {
                const float t = (p - 1.0f) / dtStep;
                return dtStep * (t * t * t / 3.0f + t * t + t);
            }
            return 0.0f;
        };

        float p05 = phase - 0.5f;
        if (p05 < 0.0f) p05 += 1.0f;

        // Slope change of -8 at phase = 0, +8 at phase = 0.5
        naive -= 8.0f * blamp(phase, dt);
        naive += 8.0f * blamp(p05, dt);

        return clamp(naive, -1.0f, 1.0f);
    }

    // Continuous morphing engine across Triangle -> Saw -> Square -> Pulse
    [[nodiscard]] static inline float renderWaveform(float phase, float dt, float morph, float pwOffset) noexcept {
        constexpr float P1 = 0.333333f;
        constexpr float P2 = 0.666667f;

        if (morph < P1) {
            // Zone 1: Triangle -> Sawtooth
            const float t = morph / P1;
            const float tri = generateTriangle(phase, dt);
            const float saw = generateSaw(phase, dt);
            return lerp(tri, saw, t);
        } else if (morph < P2) {
            // Zone 2: Sawtooth -> Square (50% duty cycle)
            const float t = (morph - P1) / (P2 - P1);
            const float saw = generateSaw(phase, dt);
            const float sqr = generatePulse(phase, dt, 0.5f + pwOffset);
            return lerp(saw, sqr, t);
        } else {
            // Zone 3: Square -> Narrow Pulse (~5%)
            const float t = (morph - P2) / (1.0f - P2);
            // Duty cycle morphs from 0.5 down to 0.05
            const float duty = lerp(0.5f, 0.05f, t) + pwOffset;
            return generatePulse(phase, dt, duty);
        }
    }

    float sampleRate_{DEFAULT_SAMPLE_RATE};
    float invSampleRate_{1.0f / DEFAULT_SAMPLE_RATE};
    float frequency_{440.0f};
    int octaveOffset_{0};
    float detuneSemitones_{0.0f};
    float detuneCents_{0.0f};
    float pitchModSemitones_{0.0f};
    float wave_{0.333333f}; // Default: Sawtooth
    float pulseWidthOffset_{0.0f};
    bool hardSyncEnabled_{false};

    float phase_{0.0f};
    float dt_{0.01f};
};

} // namespace sub_phatty
