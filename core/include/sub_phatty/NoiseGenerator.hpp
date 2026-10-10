#pragma once

#include "Common.hpp"

namespace sub_phatty {

class NoiseGenerator {
public:
    NoiseGenerator() noexcept = default;

    void reset(uint32_t seed = 0x12345678) noexcept {
        rngState_ = (seed == 0) ? 0x12345678 : seed;
        b0_ = b1_ = b2_ = b3_ = b4_ = b5_ = b6_ = 0.0f;
    }

    // Process one sample and return authentic analog pink noise (-3 dB/oct)
    [[nodiscard]] float process() noexcept {
        // Fast XorShift32 PRNG for uniform white noise [-1.0, 1.0]
        rngState_ ^= (rngState_ << 13);
        rngState_ ^= (rngState_ >> 17);
        rngState_ ^= (rngState_ << 5);
        const float white = static_cast<float>(static_cast<int32_t>(rngState_)) * (1.0f / 2147483648.0f);

        // Paul Kellet's refined 6-pole IIR filter cascade for pink spectrum
        b0_ = 0.99886f * b0_ + white * 0.0555179f;
        b1_ = 0.99332f * b1_ + white * 0.0750759f;
        b2_ = 0.96900f * b2_ + white * 0.1538520f;
        b3_ = 0.86650f * b3_ + white * 0.3104856f;
        b4_ = 0.55000f * b4_ + white * 0.5329522f;
        b5_ = -0.7616f * b5_ - white * 0.0168980f;

        const float pink = b0_ + b1_ + b2_ + b3_ + b4_ + b5_ + b6_ + white * 0.5362f;
        b6_ = white * 0.115926f;

        // Normalize nominal peak amplitude to ~[-1.0, 1.0]
        return pink * 0.115f;
    }

private:
    uint32_t rngState_{0x87654321};
    float b0_{0.0f};
    float b1_{0.0f};
    float b2_{0.0f};
    float b3_{0.0f};
    float b4_{0.0f};
    float b5_{0.0f};
    float b6_{0.0f};
};

} // namespace sub_phatty
