#pragma once

#include "Common.hpp"

namespace sub_phatty {

enum class EnvelopeStage : uint8_t {
    Idle,
    Delay,
    Attack,
    Hold,
    Decay,
    Sustain,
    Release
};

class Envelope {
public:
    Envelope() noexcept {
        reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : DEFAULT_SAMPLE_RATE;
        invSampleRate_ = 1.0f / sampleRate_;
        updateCoefficients();
    }

    void reset() noexcept {
        stage_ = EnvelopeStage::Idle;
        currentLevel_ = 0.0f;
        stageTimer_ = 0;
        gateActive_ = false;
    }

    // Time parameters in seconds
    void setDelay(float seconds) noexcept   { delayTime_ = clamp(seconds, 0.0f, 10.0f); }
    void setAttack(float seconds) noexcept  { attackTime_ = clamp(seconds, 0.001f, 10.0f); updateCoefficients(); }
    void setHold(float seconds) noexcept    { holdTime_ = clamp(seconds, 0.0f, 10.0f); }
    void setDecay(float seconds) noexcept   { decayTime_ = clamp(seconds, 0.001f, 10.0f); updateCoefficients(); }
    void setSustain(float level) noexcept   { sustainLevel_ = clamp(level, 0.0f, 1.0f); }
    void setRelease(float seconds) noexcept { releaseTime_ = clamp(seconds, 0.001f, 10.0f); updateCoefficients(); }

    [[nodiscard]] float getDelay() const noexcept   { return delayTime_; }
    [[nodiscard]] float getAttack() const noexcept  { return attackTime_; }
    [[nodiscard]] float getHold() const noexcept    { return holdTime_; }
    [[nodiscard]] float getDecay() const noexcept   { return decayTime_; }
    [[nodiscard]] float getSustain() const noexcept { return sustainLevel_; }
    [[nodiscard]] float getRelease() const noexcept { return releaseTime_; }

    void gate(bool high, bool retrigger = true) noexcept {
        gateActive_ = high;
        if (high) {
            if (retrigger || stage_ == EnvelopeStage::Idle || stage_ == EnvelopeStage::Release) {
                if (delayTime_ > 0.0001f) {
                    stage_ = EnvelopeStage::Delay;
                    stageTimer_ = static_cast<uint32_t>(delayTime_ * sampleRate_);
                } else {
                    stage_ = EnvelopeStage::Attack;
                }
            }
        } else {
            if (stage_ != EnvelopeStage::Idle) {
                stage_ = EnvelopeStage::Release;
            }
        }
    }

    [[nodiscard]] bool isActive() const noexcept {
        return stage_ != EnvelopeStage::Idle;
    }

    [[nodiscard]] EnvelopeStage getStage() const noexcept {
        return stage_;
    }

    [[nodiscard]] float process() noexcept {
        switch (stage_) {
            case EnvelopeStage::Idle:
                currentLevel_ = 0.0f;
                break;

            case EnvelopeStage::Delay:
                currentLevel_ = 0.0f;
                if (stageTimer_ > 0) {
                    --stageTimer_;
                } else {
                    stage_ = EnvelopeStage::Attack;
                }
                break;

            case EnvelopeStage::Attack: {
                // Analog exponential charge towards target 1.5V for snappy, punchy attack
                currentLevel_ += attackCoeff_ * (1.5f - currentLevel_);
                if (currentLevel_ >= 1.0f) {
                    currentLevel_ = 1.0f;
                    if (holdTime_ > 0.0001f) {
                        stage_ = EnvelopeStage::Hold;
                        stageTimer_ = static_cast<uint32_t>(holdTime_ * sampleRate_);
                    } else {
                        stage_ = EnvelopeStage::Decay;
                    }
                }
                break;
            }

            case EnvelopeStage::Hold:
                currentLevel_ = 1.0f;
                if (stageTimer_ > 0) {
                    --stageTimer_;
                } else {
                    stage_ = EnvelopeStage::Decay;
                }
                break;

            case EnvelopeStage::Decay: {
                // Analog exponential discharge towards sustain level
                currentLevel_ += decayCoeff_ * (sustainLevel_ - currentLevel_);
                if (std::fabs(currentLevel_ - sustainLevel_) < 1e-4f) {
                    currentLevel_ = sustainLevel_;
                    stage_ = EnvelopeStage::Sustain;
                }
                break;
            }

            case EnvelopeStage::Sustain:
                currentLevel_ = sustainLevel_;
                break;

            case EnvelopeStage::Release: {
                // Analog exponential discharge towards -0.01 to ensure clean cut-off at 0.0
                currentLevel_ += releaseCoeff_ * (-0.01f - currentLevel_);
                if (currentLevel_ <= 0.0f) {
                    currentLevel_ = 0.0f;
                    stage_ = EnvelopeStage::Idle;
                }
                break;
            }
        }

        return currentLevel_;
    }

private:
    void updateCoefficients() noexcept {
        // Calculate 1-pole RC coefficients based on time to reach ~99% of target
        attackCoeff_ = 1.0f - std::exp(-1.0f / (attackTime_ * 0.4f * sampleRate_));
        decayCoeff_  = 1.0f - std::exp(-1.0f / (decayTime_ * 0.35f * sampleRate_));
        releaseCoeff_= 1.0f - std::exp(-1.0f / (releaseTime_ * 0.35f * sampleRate_));
    }

    float sampleRate_{DEFAULT_SAMPLE_RATE};
    float invSampleRate_{1.0f / DEFAULT_SAMPLE_RATE};

    EnvelopeStage stage_{EnvelopeStage::Idle};
    float currentLevel_{0.0f};
    uint32_t stageTimer_{0};
    bool gateActive_{false};

    float delayTime_{0.0f};
    float attackTime_{0.01f};
    float holdTime_{0.0f};
    float decayTime_{0.2f};
    float sustainLevel_{0.7f};
    float releaseTime_{0.15f};

    float attackCoeff_{0.01f};
    float decayCoeff_{0.01f};
    float releaseCoeff_{0.01f};
};

} // namespace sub_phatty
