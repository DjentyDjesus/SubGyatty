#pragma once

#include "Common.hpp"

namespace sub_phatty {

class Mixer {
public:
    Mixer() noexcept {
        dcBlocker_.reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : DEFAULT_SAMPLE_RATE;
        dcBlocker_.setSampleRate(sampleRate_);
    }

    void reset() noexcept {
        dcBlocker_.reset();
    }

    // Set levels [0.0, 1.0]
    // In Moog Sub Phatty hardware, values above 12 o'clock (>0.5) progressively overdrive
    void setOsc1Level(float level) noexcept { osc1Level_ = clamp(level, 0.0f, 1.0f); }
    void setOsc2Level(float level) noexcept { osc2Level_ = clamp(level, 0.0f, 1.0f); }
    void setSubLevel(float level) noexcept  { subLevel_  = clamp(level, 0.0f, 1.0f); }
    void setNoiseLevel(float level) noexcept{ noiseLevel_ = clamp(level, 0.0f, 1.0f); }

    [[nodiscard]] float getOsc1Level() const noexcept { return osc1Level_; }
    [[nodiscard]] float getOsc2Level() const noexcept { return osc2Level_; }
    [[nodiscard]] float getSubLevel() const noexcept  { return subLevel_; }
    [[nodiscard]] float getNoiseLevel() const noexcept { return noiseLevel_; }

    // Mix and saturate inputs
    [[nodiscard]] float process(float osc1, float osc2, float sub, float noise) noexcept {
        // Map [0, 1] knob travel: <= 0.5 is linear up to 1.0x gain, > 0.5 overdrives up to 2.5x gain
        const float g1 = calculateGain(osc1Level_);
        const float g2 = calculateGain(osc2Level_);
        const float gSub = calculateGain(subLevel_);
        const float gNoise = calculateGain(noiseLevel_);

        const float sum = (osc1 * g1) + (osc2 * g2) + (sub * gSub) + (noise * gNoise);

        // Pre-filter non-linear saturation:
        // Asymmetric warm clipping with 2nd and 3rd harmonic generation
        const float asymInput = sum + 0.1f * (sum * std::fabs(sum));
        const float saturated = fast_tanh(asymInput * 0.85f);

        // Remove any DC offset introduced by asymmetric saturation
        return dcBlocker_.process(saturated);
    }

private:
    [[nodiscard]] static inline float calculateGain(float knob) noexcept {
        if (knob <= 0.5f) {
            return knob * 2.0f; // 0.0 to 1.0
        } else {
            // Overdrive curve past 12 o'clock
            const float extra = (knob - 0.5f) * 2.0f; // 0.0 to 1.0
            return 1.0f + extra * 1.5f; // 1.0 to 2.5
        }
    }

    float sampleRate_{DEFAULT_SAMPLE_RATE};
    float osc1Level_{0.8f};
    float osc2Level_{0.0f};
    float subLevel_{0.0f};
    float noiseLevel_{0.0f};

    DCBlocker dcBlocker_;
};

} // namespace sub_phatty
