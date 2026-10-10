#pragma once

#include "Common.hpp"

namespace sub_phatty {

class Multidrive {
public:
    Multidrive() noexcept {
        dcBlocker_.reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : DEFAULT_SAMPLE_RATE;
        dcBlocker_.setSampleRate(sampleRate_);
    }

    void reset() noexcept {
        dcBlocker_.reset();
    }

    // Multidrive amount [0.0, 1.0]
    void setAmount(float amount) noexcept {
        amount_ = clamp(amount, 0.0f, 1.0f);
    }

    [[nodiscard]] float getAmount() const noexcept {
        return amount_;
    }

    // Stage 1: Pre-filter OTA gain multiplier [1.0 to 3.0]
    [[nodiscard]] float getPreFilterDrive() const noexcept {
        return 1.0f + amount_ * 2.0f;
    }

    // Stage 2: Post-filter asymmetric FET + diode clipping
    [[nodiscard]] float process(float filterOutput) noexcept {
        if (amount_ <= 0.001f) {
            return filterOutput;
        }

        // Post-filter drive gain [1.0 to 5.0]
        const float driveGain = 1.0f + amount_ * 4.0f;
        const float x = filterOutput * driveGain;

        // Asymmetrical transfer curve modeling FET conduction and silicon diode pair
        // Generates warm 2nd harmonics alongside aggressive 3rd harmonics
        float shaped = 0.0f;
        if (x >= 0.0f) {
            // Positive half-cycle: smooth JFET compression curve
            const float p = x * 0.9f;
            shaped = fast_tanh(p);
        } else {
            // Negative half-cycle: sharper silicon diode knee
            const float n = x * 1.25f;
            shaped = fast_tanh(n);
        }

        // Bass retention: blend in a portion of dry low frequencies to prevent bass thinning
        const float bassRetention = lerp(shaped, filterOutput, 0.25f * amount_);

        // Make-up normalization to prevent runaway output amplitude
        const float normGain = 1.0f / (1.0f + amount_ * 0.6f);
        const float out = bassRetention * normGain;

        // Remove asymmetric DC offset
        return dcBlocker_.process(out);
    }

private:
    float sampleRate_{DEFAULT_SAMPLE_RATE};
    float amount_{0.0f};
    DCBlocker dcBlocker_;
};

} // namespace sub_phatty
