#pragma once

#include "Common.hpp"
#include "Parameters.hpp"
#include "Voice.hpp"
#include <array>

namespace sub_phatty {

class SynthEngine {
public:
    SynthEngine() noexcept;

    void setSampleRate(float sampleRate) noexcept;
    void reset() noexcept;

    // Parameter Interface
    void setParameter(ParamId id, float value) noexcept;
    [[nodiscard]] float getParameter(ParamId id) const noexcept;

    // MIDI Event Handlers
    void handleNoteOn(uint8_t note, uint8_t velocity) noexcept;
    void handleNoteOff(uint8_t note) noexcept;
    void handlePitchBend(int16_t bendValue) noexcept; // [-8192, 8191]
    void handleMidiCC(uint8_t controller, uint8_t value) noexcept;
    void handleAllNotesOff() noexcept;

    // Audio Rendering (Zero-allocation)
    // Single sample processing
    [[nodiscard]] float processSample() noexcept;

    // Block processing (mono to stereo output)
    void process(float* outputLeft, float* outputRight, size_t numSamples) noexcept;

    // Direct voice accessor
    [[nodiscard]] Voice& getVoice() noexcept { return voice_; }
    [[nodiscard]] const Voice& getVoice() const noexcept { return voice_; }

private:
    void applyParameter(ParamId id, float value) noexcept;
    void setupDefaultParameters() noexcept;

    float sampleRate_{DEFAULT_SAMPLE_RATE};
    Voice voice_;

    std::array<float, static_cast<size_t>(ParamId::Count)> params_{};
    float pitchBendRange_{2.0f}; // Semitones
};

} // namespace sub_phatty
