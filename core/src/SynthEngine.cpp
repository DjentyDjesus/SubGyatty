#include "sub_phatty/SynthEngine.hpp"
#include <cmath>

namespace sub_phatty {

SynthEngine::SynthEngine() noexcept {
    setupDefaultParameters();
    setSampleRate(DEFAULT_SAMPLE_RATE);
    reset();
}

void SynthEngine::setSampleRate(float sampleRate) noexcept {
    sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : DEFAULT_SAMPLE_RATE;
    voice_.setSampleRate(sampleRate_);
}

void SynthEngine::reset() noexcept {
    voice_.reset();
}

void SynthEngine::setupDefaultParameters() noexcept {
    // Sensible classic Moog default preset:
    // Osc 1 Saw, Osc 2 Detuned Saw, Sub-Osc at moderate level, Ladder Filter at 1.5 kHz, slight Multidrive
    params_[static_cast<size_t>(ParamId::Osc1Octave)]       = 0.0f;       // 8' (middle)
    params_[static_cast<size_t>(ParamId::Osc1Waveform)]     = 0.333333f;  // Sawtooth
    params_[static_cast<size_t>(ParamId::Osc1Level)]        = 0.85f;      // 85%

    params_[static_cast<size_t>(ParamId::Osc2Octave)]       = 0.0f;       // 8'
    params_[static_cast<size_t>(ParamId::Osc2Frequency)]    = 0.0f;       // 0 semitones
    params_[static_cast<size_t>(ParamId::Osc2Detune)]       = 7.0f;       // +7 cents warm beat detune
    params_[static_cast<size_t>(ParamId::Osc2Waveform)]     = 0.333333f;  // Sawtooth
    params_[static_cast<size_t>(ParamId::Osc2Sync)]         = 0.0f;       // Sync Off
    params_[static_cast<size_t>(ParamId::Osc2Level)]        = 0.70f;      // 70%

    params_[static_cast<size_t>(ParamId::SubOscLevel)]      = 0.50f;      // 50%
    params_[static_cast<size_t>(ParamId::NoiseLevel)]       = 0.0f;       // 0%

    params_[static_cast<size_t>(ParamId::FilterCutoff)]     = 1200.0f;    // 1.2 kHz
    params_[static_cast<size_t>(ParamId::FilterResonance)]  = 0.25f;      // 25%
    params_[static_cast<size_t>(ParamId::FilterMultidrive)] = 0.20f;      // Warm pre/post drive
    params_[static_cast<size_t>(ParamId::FilterSlope)]      = 4.0f;       // 24 dB/oct (4-pole)
    params_[static_cast<size_t>(ParamId::FilterEnvAmount)]  = 0.60f;      // +60% env depth
    params_[static_cast<size_t>(ParamId::FilterKeyTracking)]= 0.50f;      // 50% key tracking

    params_[static_cast<size_t>(ParamId::FilterAttack)]     = 0.010f;     // 10 ms
    params_[static_cast<size_t>(ParamId::FilterDecay)]      = 0.350f;     // 350 ms
    params_[static_cast<size_t>(ParamId::FilterSustain)]    = 0.30f;      // 30%
    params_[static_cast<size_t>(ParamId::FilterRelease)]    = 0.200f;     // 200 ms

    params_[static_cast<size_t>(ParamId::AmpAttack)]        = 0.005f;     // 5 ms
    params_[static_cast<size_t>(ParamId::AmpDecay)]         = 0.250f;     // 250 ms
    params_[static_cast<size_t>(ParamId::AmpSustain)]       = 0.80f;      // 80%
    params_[static_cast<size_t>(ParamId::AmpRelease)]       = 0.150f;     // 150 ms

    params_[static_cast<size_t>(ParamId::LFORate)]          = 3.5f;       // 3.5 Hz
    params_[static_cast<size_t>(ParamId::LFOShape)]         = 0.0f;       // Triangle
    params_[static_cast<size_t>(ParamId::LFOAmount)]        = 0.0f;       // 0%
    params_[static_cast<size_t>(ParamId::LFOPitchDest)]     = 0.0f;       // Pitch All
    params_[static_cast<size_t>(ParamId::LFOCutoffDest)]    = 0.0f;       // Filter Off
    params_[static_cast<size_t>(ParamId::LFOWaveDest)]      = 0.0f;       // Wave Off

    params_[static_cast<size_t>(ParamId::GlideTime)]        = 0.0f;       // Off
    params_[static_cast<size_t>(ParamId::GlideMode)]        = 0.0f;       // Legato only
    params_[static_cast<size_t>(ParamId::Legato)]           = 1.0f;       // On
    params_[static_cast<size_t>(ParamId::MasterVolume)]     = 0.85f;      // 85%
    params_[static_cast<size_t>(ParamId::PitchBendRange)]   = 2.0f;       // +/- 2 semitones

    for (size_t i = 0; i < static_cast<size_t>(ParamId::Count); ++i) {
        applyParameter(static_cast<ParamId>(i), params_[i]);
    }
}

void SynthEngine::setParameter(ParamId id, float value) noexcept {
    const size_t idx = static_cast<size_t>(id);
    if (idx < params_.size()) {
        params_[idx] = value;
        applyParameter(id, value);
    }
}

float SynthEngine::getParameter(ParamId id) const noexcept {
    const size_t idx = static_cast<size_t>(id);
    return (idx < params_.size()) ? params_[idx] : 0.0f;
}

void SynthEngine::applyParameter(ParamId id, float value) noexcept {
    switch (id) {
        case ParamId::Osc1Octave:
            voice_.getOsc1().setOctave(static_cast<int>(std::round(value)));
            break;
        case ParamId::Osc1Waveform:
            voice_.getOsc1().setWaveform(value);
            break;
        case ParamId::Osc1Level:
            voice_.getMixer().setOsc1Level(value);
            break;

        case ParamId::Osc2Octave:
            voice_.getOsc2().setOctave(static_cast<int>(std::round(value)));
            break;
        case ParamId::Osc2Frequency:
            voice_.getOsc2().setDetuneSemitones(value);
            break;
        case ParamId::Osc2Detune:
            voice_.getOsc2().setDetuneCents(value);
            break;
        case ParamId::Osc2Waveform:
            voice_.getOsc2().setWaveform(value);
            break;
        case ParamId::Osc2Sync:
            voice_.getOsc2().setHardSync(value >= 0.5f);
            break;
        case ParamId::Osc2Level:
            voice_.getMixer().setOsc2Level(value);
            break;

        case ParamId::SubOscLevel:
            voice_.getMixer().setSubLevel(value);
            break;
        case ParamId::NoiseLevel:
            voice_.getMixer().setNoiseLevel(value);
            break;

        case ParamId::FilterCutoff:
            voice_.setBaseFilterCutoff(value);
            break;
        case ParamId::FilterResonance:
            voice_.getFilter().setResonance(value);
            break;
        case ParamId::FilterMultidrive:
            voice_.getMultidrive().setAmount(value);
            break;
        case ParamId::FilterSlope: {
            const int s = clamp(static_cast<int>(std::round(value)), 1, 4);
            voice_.getFilter().setSlope(static_cast<FilterSlope>(s));
            break;
        }
        case ParamId::FilterEnvAmount:
            voice_.setFilterEnvAmount(value);
            break;
        case ParamId::FilterKeyTracking:
            voice_.setFilterKeyTracking(value);
            break;

        case ParamId::FilterAttack:
            voice_.getFilterEnv().setAttack(value);
            break;
        case ParamId::FilterDecay:
            voice_.getFilterEnv().setDecay(value);
            break;
        case ParamId::FilterSustain:
            voice_.getFilterEnv().setSustain(value);
            break;
        case ParamId::FilterRelease:
            voice_.getFilterEnv().setRelease(value);
            break;

        case ParamId::AmpAttack:
            voice_.getAmpEnv().setAttack(value);
            break;
        case ParamId::AmpDecay:
            voice_.getAmpEnv().setDecay(value);
            break;
        case ParamId::AmpSustain:
            voice_.getAmpEnv().setSustain(value);
            break;
        case ParamId::AmpRelease:
            voice_.getAmpEnv().setRelease(value);
            break;

        case ParamId::LFORate:
            voice_.getLFO().setRate(value);
            break;
        case ParamId::LFOShape: {
            const int shapeInt = clamp(static_cast<int>(std::round(value)), 0, 4);
            voice_.getLFO().setShape(static_cast<LFOShape>(shapeInt));
            break;
        }
        case ParamId::LFOAmount:
            voice_.getLFO().setAmount(value);
            break;
        case ParamId::LFOPitchDest:
            voice_.setLFOPitchDestination(value >= 0.5f ? LFODestination::PitchOsc2Only : LFODestination::PitchAll);
            break;
        case ParamId::LFOCutoffDest:
            voice_.setLFOCutoffDestination(value >= 0.5f);
            break;
        case ParamId::LFOWaveDest:
            voice_.setLFOWaveDestination(value >= 0.5f);
            break;

        case ParamId::GlideTime:
            voice_.setGlideTime(value);
            break;
        case ParamId::GlideMode:
            voice_.setGlideAlways(value >= 0.5f);
            break;
        case ParamId::Legato:
            voice_.setLegato(value >= 0.5f);
            break;
        case ParamId::MasterVolume:
            voice_.setMasterVolume(value);
            break;
        case ParamId::PitchBendRange:
            pitchBendRange_ = clamp(value, 0.0f, 24.0f);
            break;

        default:
            break;
    }
}

void SynthEngine::handleNoteOn(uint8_t note, uint8_t velocity) noexcept {
    voice_.noteOn(note, velocity);
}

void SynthEngine::handleNoteOff(uint8_t note) noexcept {
    voice_.noteOff(note);
}

void SynthEngine::handlePitchBend(int16_t bendValue) noexcept {
    const float normBend = static_cast<float>(bendValue) / 8192.0f;
    voice_.setPitchBend(normBend * pitchBendRange_);
}

void SynthEngine::handleMidiCC(uint8_t controller, uint8_t value) noexcept {
    const float norm = static_cast<float>(value) / 127.0f;

    auto timeMap = [](float v) noexcept -> float {
        // Map [0, 1] exponentially from 0.001s (1ms) to 10.0s
        return 0.001f * std::pow(10000.0f, v);
    };

    switch (controller) {
        case MidiCC::ModWheel:
            setParameter(ParamId::LFOAmount, norm);
            break;
        case MidiCC::LFORate:
            // 0.1 Hz to 1000.0 Hz exponential mapping
            setParameter(ParamId::LFORate, 0.1f * std::pow(10000.0f, norm));
            break;
        case MidiCC::GlideTime:
            setParameter(ParamId::GlideTime, norm * 4.0f);
            break;
        case MidiCC::MasterVolume:
            setParameter(ParamId::MasterVolume, norm);
            break;
        case MidiCC::NoiseLevel:
            setParameter(ParamId::NoiseLevel, norm);
            break;
        case MidiCC::Osc1Waveform:
            setParameter(ParamId::Osc1Waveform, norm);
            break;
        case MidiCC::Osc1Octave: {
            // Map 0-127 into octaves: -2, -1, 0, +1
            const int oct = static_cast<int>(std::round(norm * 3.0f)) - 2;
            setParameter(ParamId::Osc1Octave, static_cast<float>(oct));
            break;
        }
        case MidiCC::Osc2Frequency:
            // +/- 7 semitones
            setParameter(ParamId::Osc2Frequency, (norm * 14.0f) - 7.0f);
            break;
        case MidiCC::Osc2Detune:
            // +/- 100 cents
            setParameter(ParamId::Osc2Detune, (norm * 200.0f) - 100.0f);
            break;
        case MidiCC::Osc2Waveform:
            setParameter(ParamId::Osc2Waveform, norm);
            break;
        case MidiCC::Osc1Level:
            setParameter(ParamId::Osc1Level, norm);
            break;
        case MidiCC::Osc2Level:
            setParameter(ParamId::Osc2Level, norm);
            break;
        case MidiCC::SubOscLevel:
            setParameter(ParamId::SubOscLevel, norm);
            break;
        case MidiCC::Multidrive:
            setParameter(ParamId::FilterMultidrive, norm);
            break;
        case MidiCC::FilterCutoff:
            // 20 Hz to 20000 Hz exponential mapping
            setParameter(ParamId::FilterCutoff, 20.0f * std::pow(1000.0f, norm));
            break;
        case MidiCC::FilterResonance:
            setParameter(ParamId::FilterResonance, norm);
            break;
        case MidiCC::FilterAttack:
            setParameter(ParamId::FilterAttack, timeMap(norm));
            break;
        case MidiCC::FilterDecay:
            setParameter(ParamId::FilterDecay, timeMap(norm));
            break;
        case MidiCC::FilterSustain:
            setParameter(ParamId::FilterSustain, norm);
            break;
        case MidiCC::FilterRelease:
            setParameter(ParamId::FilterRelease, timeMap(norm));
            break;
        case MidiCC::FilterEnvAmount:
            setParameter(ParamId::FilterEnvAmount, (norm * 2.0f) - 1.0f);
            break;
        case MidiCC::AmpAttack:
            setParameter(ParamId::AmpAttack, timeMap(norm));
            break;
        case MidiCC::AmpDecay:
            setParameter(ParamId::AmpDecay, timeMap(norm));
            break;
        case MidiCC::AmpSustain:
            setParameter(ParamId::AmpSustain, norm);
            break;
        case MidiCC::AmpRelease:
            setParameter(ParamId::AmpRelease, timeMap(norm));
            break;
        case MidiCC::GlideSwitch:
            setParameter(ParamId::GlideMode, value >= 64 ? 1.0f : 0.0f);
            break;
        case MidiCC::Osc2Sync:
            setParameter(ParamId::Osc2Sync, value >= 64 ? 1.0f : 0.0f);
            break;
        case MidiCC::AllNotesOff:
            handleAllNotesOff();
            break;
        default:
            break;
    }
}

void SynthEngine::handleAllNotesOff() noexcept {
    voice_.allNotesOff();
}

float SynthEngine::processSample() noexcept {
    return voice_.processSample();
}

void SynthEngine::process(float* outputLeft, float* outputRight, size_t numSamples) noexcept {
    if (outputLeft == nullptr || outputRight == nullptr) return;

    for (size_t i = 0; i < numSamples; ++i) {
        const float sample = voice_.processSample();
        outputLeft[i] = sample;
        outputRight[i] = sample;
    }
}

} // namespace sub_phatty
