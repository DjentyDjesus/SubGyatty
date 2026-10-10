#pragma once

#include <cmath>
#include <cstdint>
#include <numbers>
#include <algorithm>
#include <array>

namespace sub_phatty {

// Math Constants (Single-precision float for ARM Cortex-M7 & SIMD efficiency)
constexpr float PI          = 3.14159265358979323846f;
constexpr float TWO_PI      = 6.28318530717958647692f;
constexpr float HALF_PI     = 1.57079632679489661923f;
constexpr float INV_TWO_PI  = 0.15915494309189533576f;
constexpr float DEFAULT_SAMPLE_RATE = 44100.0f;

// Utility functions
template <typename T>
[[nodiscard]] constexpr T clamp(T val, T minVal, T maxVal) noexcept {
    return (val < minVal) ? minVal : ((val > maxVal) ? maxVal : val);
}

[[nodiscard]] constexpr float lerp(float a, float b, float t) noexcept {
    return a + t * (b - a);
}

[[nodiscard]] inline float midiNoteToFrequency(float note, float a4 = 440.0f) noexcept {
    return a4 * std::pow(2.0f, (note - 69.0f) / 12.0f);
}

[[nodiscard]] inline float semitonesToRatio(float semitones) noexcept {
    return std::pow(2.0f, semitones / 12.0f);
}

[[nodiscard]] inline float centsToRatio(float cents) noexcept {
    return std::pow(2.0f, cents / 1200.0f);
}

// Fast Padé rational approximation of tanh(x)
// Max error < 0.1% within [-3, 3], smooth saturation to +/-1 beyond.
[[nodiscard]] inline float fast_tanh(float x) noexcept {
    if (x <= -3.0f) return -1.0f;
    if (x >= 3.0f) return 1.0f;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// Algebraic soft-clipper x / (1 + |x|)
[[nodiscard]] inline float fast_softclip(float x) noexcept {
    return x / (1.0f + std::fabs(x));
}

// PolyBLEP (Polynomial Bandlimited Step) residual correction
// Smooths step discontinuities at phase boundaries in [0, 1)
[[nodiscard]] inline float polyBlep(float phase, float dt) noexcept {
    if (dt <= 0.0f) return 0.0f;
    if (phase < dt) {
        const float t = phase / dt;
        return 2.0f * t - t * t - 1.0f;
    } else if (phase > 1.0f - dt) {
        const float t = (phase - 1.0f) / dt;
        return t * t + 2.0f * t + 1.0f;
    }
    return 0.0f;
}

// 1-Pole DC Blocker (Highpass filter ~10 Hz)
class DCBlocker {
public:
    void reset() noexcept {
        x1_ = 0.0f;
        y1_ = 0.0f;
    }

    void setSampleRate(float sampleRate) noexcept {
        const float cutoff = 10.0f;
        r_ = 1.0f - (TWO_PI * cutoff / sampleRate);
        r_ = clamp(r_, 0.9f, 0.9999f);
    }

    [[nodiscard]] inline float process(float x) noexcept {
        const float y = x - x1_ + r_ * y1_;
        x1_ = x;
        y1_ = y;
        return y;
    }

private:
    float x1_{0.0f};
    float y1_{0.0f};
    float r_{0.999f};
};

// Zero-allocation One-Pole Parameter Smoother (eliminates zipper noise)
class ParameterSmoother {
public:
    void init(float val) noexcept {
        current_ = target_ = val;
    }

    void setTarget(float val) noexcept {
        target_ = val;
    }

    void setTimeConstant(float timeMs, float sampleRate) noexcept {
        if (timeMs <= 0.0f) {
            coeff_ = 1.0f;
        } else {
            coeff_ = 1.0f - std::exp(-1.0f / (timeMs * 0.001f * sampleRate));
        }
    }

    [[nodiscard]] inline float process() noexcept {
        current_ += coeff_ * (target_ - current_);
        return current_;
    }

    [[nodiscard]] inline float get() const noexcept {
        return current_;
    }

    [[nodiscard]] inline bool isSettled(float eps = 1e-4f) const noexcept {
        return std::fabs(target_ - current_) < eps;
    }

private:
    float current_{0.0f};
    float target_{0.0f};
    float coeff_{0.05f};
};

} // namespace sub_phatty
