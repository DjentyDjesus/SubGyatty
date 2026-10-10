#pragma once

#include "Common.hpp"

namespace sub_phatty {

enum class LFOShape : uint8_t {
    Triangle = 0,
    Square   = 1,
    Saw      = 2,
    Ramp     = 3,
    SampleAndHold = 4
};

enum class LFODestination : uint8_t {
    PitchAll = 0,
    PitchOsc2Only = 1,
    FilterCutoff  = 2,
    WaveShape     = 3
};

class LFO {
public:
    LFO() noexcept {
        reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : DEFAULT_SAMPLE_RATE;
        invSampleRate_ = 1.0f / sampleRate_;
        updatePhaseIncrement();
    }

    void reset() noexcept {
        phase_ = 0.0f;
        shCurrentValue_ = 0.0f;
        rngState_ = 0x54321098;
    }

    // Rate in Hz [0.1 Hz, 1000.0 Hz] (Audio-rate capable)
    void setRate(float rateHz) noexcept {
        rateHz_ = clamp(rateHz, 0.1f, 1000.0f);
        updatePhaseIncrement();
    }

    // Waveform shape
    void setShape(LFOShape shape) noexcept {
        shape_ = shape;
    }

    // Depth amount [0.0, 1.0]
    void setAmount(float amount) noexcept {
        amount_ = clamp(amount, 0.0f, 1.0f);
    }

    // Key sync / Retrigger on Note On
    void setKeySync(bool enabled) noexcept {
        keySync_ = enabled;
    }

    void trigger() noexcept {
        if (keySync_) {
            phase_ = 0.0f;
            updateSampleAndHold();
        }
    }

    [[nodiscard]] float getRate() const noexcept   { return rateHz_; }
    [[nodiscard]] LFOShape getShape() const noexcept{ return shape_; }
    [[nodiscard]] float getAmount() const noexcept { return amount_; }
    [[nodiscard]] bool isKeySync() const noexcept  { return keySync_; }

    // Process one sample and return current modulation value [-amount, +amount]
    [[nodiscard]] float process() noexcept {
        float raw = 0.0f;

        switch (shape_) {
            case LFOShape::Triangle:
                // Triangle: ramps from +1 to -1 and back to +1
                raw = 4.0f * std::fabs(phase_ - 0.5f) - 1.0f;
                break;

            case LFOShape::Square:
                raw = (phase_ < 0.5f) ? 1.0f : -1.0f;
                break;

            case LFOShape::Saw:
                // Falling sawtooth: +1 to -1
                raw = 1.0f - 2.0f * phase_;
                break;

            case LFOShape::Ramp:
                // Rising ramp: -1 to +1
                raw = 2.0f * phase_ - 1.0f;
                break;

            case LFOShape::SampleAndHold:
                raw = shCurrentValue_;
                break;
        }

        // Advance phase
        phase_ += dt_;
        if (phase_ >= 1.0f) {
            phase_ -= 1.0f;
            if (shape_ == LFOShape::SampleAndHold) {
                updateSampleAndHold();
            }
        }

        return raw * amount_;
    }

private:
    void updatePhaseIncrement() noexcept {
        dt_ = clamp(rateHz_ * invSampleRate_, 0.000001f, 0.49f);
    }

    void updateSampleAndHold() noexcept {
        rngState_ ^= (rngState_ << 13);
        rngState_ ^= (rngState_ >> 17);
        rngState_ ^= (rngState_ << 5);
        shCurrentValue_ = static_cast<float>(static_cast<int32_t>(rngState_)) * (1.0f / 2147483648.0f);
    }

    float sampleRate_{DEFAULT_SAMPLE_RATE};
    float invSampleRate_{1.0f / DEFAULT_SAMPLE_RATE};
    float rateHz_{2.0f};
    float amount_{0.0f};
    LFOShape shape_{LFOShape::Triangle};
    bool keySync_{false};

    float phase_{0.0f};
    float dt_{0.0001f};
    float shCurrentValue_{0.0f};
    uint32_t rngState_{0x54321098};
};

} // namespace sub_phatty
