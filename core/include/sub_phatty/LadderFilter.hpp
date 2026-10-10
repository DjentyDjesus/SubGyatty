#pragma once

#include "Common.hpp"

namespace sub_phatty {

enum class FilterSlope : uint8_t {
    Slope6dB  = 1, // 1-pole
    Slope12dB = 2, // 2-pole
    Slope18dB = 3, // 3-pole
    Slope24dB = 4  // 4-pole (Classic Moog)
};

class LadderFilter {
public:
    LadderFilter() noexcept {
        reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : DEFAULT_SAMPLE_RATE;
        // Internal 2x oversampling
        oversampledRate_ = sampleRate_ * 2.0f;
        updateCoefficients();
    }

    void reset() noexcept {
        s1_ = s2_ = s3_ = s4_ = 0.0f;
        prevInput_ = 0.0f;
    }

    // Cutoff frequency in Hz [20.0, 20000.0]
    void setCutoff(float cutoffHz) noexcept {
        cutoff_ = clamp(cutoffHz, 20.0f, sampleRate_ * 0.45f);
        updateCoefficients();
    }

    // Resonance [0.0, 1.0]. Above ~0.85, the filter reaches self-oscillation
    void setResonance(float res) noexcept {
        resonance_ = clamp(res, 0.0f, 1.0f);
        updateCoefficients();
    }

    // Slope mode (Shift parameter in Sub Phatty)
    void setSlope(FilterSlope slope) noexcept {
        slope_ = slope;
    }

    [[nodiscard]] float getCutoff() const noexcept { return cutoff_; }
    [[nodiscard]] float getResonance() const noexcept { return resonance_; }
    [[nodiscard]] FilterSlope getSlope() const noexcept { return slope_; }

    // Pre-filter drive multiplier (boosted by Multidrive stage 1)
    void setPreDrive(float drive) noexcept {
        preDrive_ = clamp(drive, 1.0f, 4.0f);
    }

    // Process one input sample with 2x oversampled ZDF ladder solver
    [[nodiscard]] float process(float input) noexcept {
        const float inDriven = input * preDrive_;

        // 2x internal oversampling for alias-free non-linearities
        // Sub-sample 1: Linear interpolation with previous sample
        const float midInput = 0.5f * (prevInput_ + inDriven);
        prevInput_ = inDriven;

        solveStep(midInput);
        return solveStep(inDriven);
    }

private:
    void updateCoefficients() noexcept {
        // Bilinear transform pre-warping at 2x oversampled rate
        const float wd = TWO_PI * cutoff_;
        const float T = 1.0f / oversampledRate_;
        const float wa = (2.0f / T) * std::tan(wd * T * 0.5f);
        g_ = clamp(wa * T * 0.5f, 0.0001f, 0.999f);

        G_ = g_ / (1.0f + g_);
        G2_ = G_ * G_;
        G3_ = G2_ * G_;
        G4_ = G3_ * G_;

        // Resonance feedback gain: 0 to ~4.1 (self-oscillation starts at 4.0)
        k_ = resonance_ * 4.15f;
    }

    float solveStep(float in) noexcept {
        // State predictions for each stage
        const float S1 = (1.0f - G_) * s1_;
        const float S2 = (1.0f - G_) * s2_;
        const float S3 = (1.0f - G_) * s3_;
        const float S4 = (1.0f - G_) * s4_;

        // Total feedback path contribution from integrator states
        const float S = G3_ * S1 + G2_ * S2 + G_ * S3 + S4;

        // Instantaneous Zero-Delay Feedback loop estimate
        // u = (in - k * S) / (1 + k * G^4)
        float u = (in - k_ * S) / (1.0f + k_ * G4_);

        // Two rapid Picard iterations resolving non-linear tanh saturation in the loop
        for (int iter = 0; iter < 2; ++iter) {
            const float y4_est = G4_ * fast_tanh(u) + S;
            u = fast_tanh(in - k_ * y4_est);
        }

        // Evaluate cascaded 1-pole stages with non-linear saturation
        const float stage0_out = fast_tanh(u);

        const float y1 = G_ * stage0_out + S1;
        s1_ = 2.0f * y1 - s1_;

        const float y2 = G_ * fast_tanh(y1) + S2;
        s2_ = 2.0f * y2 - s2_;

        const float y3 = G_ * fast_tanh(y2) + S3;
        s3_ = 2.0f * y3 - s3_;

        const float y4 = G_ * fast_tanh(y3) + S4;
        s4_ = 2.0f * y4 - s4_;

        // Tap output according to selected slope mode
        switch (slope_) {
            case FilterSlope::Slope6dB:  return y1;
            case FilterSlope::Slope12dB: return y2;
            case FilterSlope::Slope18dB: return y3;
            case FilterSlope::Slope24dB:
            default:                     return y4;
        }
    }

    float sampleRate_{DEFAULT_SAMPLE_RATE};
    float oversampledRate_{DEFAULT_SAMPLE_RATE * 2.0f};
    float cutoff_{1000.0f};
    float resonance_{0.0f};
    float preDrive_{1.0f};
    FilterSlope slope_{FilterSlope::Slope24dB};

    float g_{0.1f};
    float G_{0.1f};
    float G2_{0.01f};
    float G3_{0.001f};
    float G4_{0.0001f};
    float k_{0.0f};

    // Integrator state variables
    float s1_{0.0f};
    float s2_{0.0f};
    float s3_{0.0f};
    float s4_{0.0f};
    float prevInput_{0.0f};
};

} // namespace sub_phatty
