#pragma once

#include "Common.hpp"

namespace sub_phatty {

class SubOscillator {
public:
    SubOscillator() noexcept = default;

    void setSampleRate(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : DEFAULT_SAMPLE_RATE;
        invSampleRate_ = 1.0f / sampleRate_;
        updatePhaseIncrement();
    }

    void reset() noexcept {
        phase_ = 0.0f;
    }

    // Set base frequency to match Osc 1 (Sub-Osc will divide by 2)
    void setBaseFrequency(float osc1FreqHz) noexcept {
        osc1Freq_ = clamp(osc1FreqHz, 5.0f, sampleRate_ * 0.49f);
        updatePhaseIncrement();
    }

    // Phase-lock to Osc 1: synchronize phase whenever Osc 1 resets
    void syncToOsc1(float osc1Phase) noexcept {
        // Sub-osc completes 1 cycle for every 2 cycles of Osc 1
        // Keep fractional phase aligned
        if (osc1Phase < dtOsc1_) {
            // Check if we are in the first or second half of the sub-osc cycle
            if (phase_ > 0.5f) {
                phase_ = 0.5f + (osc1Phase * 0.5f);
            } else {
                phase_ = (osc1Phase * 0.5f);
            }
        }
    }

    [[nodiscard]] float process() noexcept {
        // Generate PolyBLEP bandlimited 50% square wave at -1 octave
        constexpr float d = 0.5f;
        float naive = (phase_ < d) ? 1.0f : -1.0f;

        // Transition 1 at phase = 0 (rising edge)
        naive += polyBlep(phase_, dt_);

        // Transition 2 at phase = 0.5 (falling edge)
        float pD = phase_ - d;
        if (pD < 0.0f) pD += 1.0f;
        naive -= polyBlep(pD, dt_);

        // Advance phase
        phase_ += dt_;
        if (phase_ >= 1.0f) {
            phase_ -= 1.0f;
        }

        return naive;
    }

private:
    void updatePhaseIncrement() noexcept {
        const float subFreq = osc1Freq_ * 0.5f; // Exactly 1 octave lower
        dtOsc1_ = osc1Freq_ * invSampleRate_;
        dt_ = clamp(subFreq * invSampleRate_, 0.0f, 0.49f);
    }

    float sampleRate_{DEFAULT_SAMPLE_RATE};
    float invSampleRate_{1.0f / DEFAULT_SAMPLE_RATE};
    float osc1Freq_{440.0f};
    float dtOsc1_{0.01f};
    float dt_{0.005f};
    float phase_{0.0f};
};

} // namespace sub_phatty
