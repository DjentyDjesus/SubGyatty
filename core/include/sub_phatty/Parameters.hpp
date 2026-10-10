#pragma once

#include <cstdint>
#include <string_view>

namespace sub_phatty {

enum class ParamId : uint16_t {
    // Oscillators
    Osc1Octave = 0,
    Osc1Waveform,
    Osc1Level,

    Osc2Octave,
    Osc2Frequency,
    Osc2Detune,
    Osc2Waveform,
    Osc2Sync,
    Osc2Level,

    SubOscLevel,
    NoiseLevel,

    // Ladder Filter
    FilterCutoff,
    FilterResonance,
    FilterMultidrive,
    FilterSlope,
    FilterEnvAmount,
    FilterKeyTracking,

    // Envelopes
    FilterAttack,
    FilterDecay,
    FilterSustain,
    FilterRelease,

    AmpAttack,
    AmpDecay,
    AmpSustain,
    AmpRelease,

    // LFO & Modulation
    LFORate,
    LFOShape,
    LFOAmount,
    LFOPitchDest,
    LFOCutoffDest,
    LFOWaveDest,

    // Performance & Playability
    GlideTime,
    GlideMode,
    Legato,
    MasterVolume,
    PitchBendRange,

    Count
};

// Moog Sub Phatty Standard MIDI Continuous Controller (CC) Mappings
namespace MidiCC {
    constexpr uint8_t ModWheel         = 1;
    constexpr uint8_t LFORate          = 3;
    constexpr uint8_t GlideTime        = 5;
    constexpr uint8_t MasterVolume     = 7;
    constexpr uint8_t NoiseLevel       = 8;
    constexpr uint8_t Osc1Waveform     = 9;
    constexpr uint8_t Osc1Octave       = 11;
    constexpr uint8_t Osc2Frequency    = 12;
    constexpr uint8_t Osc2Detune       = 13;
    constexpr uint8_t Osc2Waveform     = 14;
    constexpr uint8_t Osc1Level        = 15;
    constexpr uint8_t Osc2Level        = 16;
    constexpr uint8_t SubOscLevel      = 17;
    constexpr uint8_t Multidrive       = 18;
    constexpr uint8_t FilterCutoff     = 19;
    constexpr uint8_t FilterResonance  = 21;
    constexpr uint8_t FilterAttack     = 23;
    constexpr uint8_t FilterDecay      = 24;
    constexpr uint8_t FilterSustain    = 25;
    constexpr uint8_t FilterRelease    = 26;
    constexpr uint8_t FilterEnvAmount  = 27;
    constexpr uint8_t AmpAttack        = 28;
    constexpr uint8_t AmpDecay         = 29;
    constexpr uint8_t AmpSustain       = 30;
    constexpr uint8_t AmpRelease       = 31;
    constexpr uint8_t GlideSwitch      = 65;
    constexpr uint8_t Osc2Sync         = 77;
    constexpr uint8_t AllNotesOff      = 123;
}

struct ParameterInfo {
    ParamId id;
    std::string_view name;
    float minValue;
    float maxValue;
    float defaultValue;
    uint8_t midiCC;
};

} // namespace sub_phatty
