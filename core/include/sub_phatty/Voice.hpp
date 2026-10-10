#pragma once

#include "Common.hpp"
#include "Oscillator.hpp"
#include "SubOscillator.hpp"
#include "NoiseGenerator.hpp"
#include "Mixer.hpp"
#include "LadderFilter.hpp"
#include "Multidrive.hpp"
#include "Envelope.hpp"
#include "LFO.hpp"

namespace sub_phatty {

enum class NotePriority : uint8_t {
    Last = 0,
    Lowest = 1,
    Highest = 2
};

class Voice {
public:
    static constexpr size_t MAX_HELD_NOTES = 16;

    Voice() noexcept {
        reset();
    }

    void setSampleRate(float sampleRate) noexcept {
        sampleRate_ = (sampleRate > 1000.0f) ? sampleRate : DEFAULT_SAMPLE_RATE;
        invSampleRate_ = 1.0f / sampleRate_;

        osc1_.setSampleRate(sampleRate_);
        osc2_.setSampleRate(sampleRate_);
        subOsc_.setSampleRate(sampleRate_);
        mixer_.setSampleRate(sampleRate_);
        filter_.setSampleRate(sampleRate_);
        multidrive_.setSampleRate(sampleRate_);
        filterEnv_.setSampleRate(sampleRate_);
        ampEnv_.setSampleRate(sampleRate_);
        lfo_.setSampleRate(sampleRate_);

        updateGlideCoeff();
    }

    void reset() noexcept {
        numHeldNotes_ = 0;
        activeNote_ = 60;
        targetFreq_ = 261.63f;
        currentFreq_ = 261.63f;
        velocity_ = 0.0f;
        pitchBendSemitones_ = 0.0f;

        osc1_.reset();
        osc2_.reset();
        subOsc_.reset();
        noise_.reset();
        mixer_.reset();
        filter_.reset();
        multidrive_.reset();
        filterEnv_.reset();
        ampEnv_.reset();
        lfo_.reset();
    }

    // Note Priority Configuration
    void setNotePriority(NotePriority priority) noexcept {
        notePriority_ = priority;
    }

    // Legato Mode: when enabled, overlapping notes glide without retriggering envelopes
    void setLegato(bool enabled) noexcept {
        legato_ = enabled;
    }

    // Glide / Portamento time in seconds [0.0, 5.0]
    void setGlideTime(float seconds) noexcept {
        glideTime_ = clamp(seconds, 0.0f, 5.0f);
        updateGlideCoeff();
    }

    // Glide mode: true = always glide, false = glide only when playing legato
    void setGlideAlways(bool always) noexcept {
        glideAlways_ = always;
    }

    // Filter Envelope Amount (bipolar [-1.0, 1.0] or in octaves)
    void setFilterEnvAmount(float amount) noexcept {
        filterEnvAmount_ = clamp(amount, -1.0f, 1.0f);
    }

    // Filter Key Tracking [0.0, 1.0] (1.0 = 100% 1V/Oct)
    void setFilterKeyTracking(float tracking) noexcept {
        filterKeyTracking_ = clamp(tracking, 0.0f, 1.0f);
    }

    // Master Volume [0.0, 1.0]
    void setMasterVolume(float vol) noexcept {
        masterVolume_ = clamp(vol, 0.0f, 1.0f);
    }

    // Pitch Bend range in semitones (e.g. +/- 2 semitones or +/- 12)
    void setPitchBend(float semitones) noexcept {
        pitchBendSemitones_ = semitones;
    }

    // Note On handler
    void noteOn(uint8_t note, uint8_t vel) noexcept {
        if (numHeldNotes_ < MAX_HELD_NOTES) {
            heldNotes_[numHeldNotes_++] = note;
        }

        const bool wasPlaying = ampEnv_.isActive();
        resolveActiveNote();

        const float newTargetFreq = midiNoteToFrequency(static_cast<float>(activeNote_));
        targetFreq_ = newTargetFreq;

        const bool isLegatoTransition = wasPlaying && (numHeldNotes_ > 1);

        if (!isLegatoTransition || glideAlways_) {
            if (glideTime_ <= 0.0001f) {
                currentFreq_ = targetFreq_;
            }
        }

        velocity_ = static_cast<float>(vel) / 127.0f;

        const bool shouldRetrigger = !legato_ || !isLegatoTransition;
        filterEnv_.gate(true, shouldRetrigger);
        ampEnv_.gate(true, shouldRetrigger);
        lfo_.trigger();
    }

    // Note Off handler
    void noteOff(uint8_t note) noexcept {
        for (size_t i = 0; i < numHeldNotes_; ++i) {
            if (heldNotes_[i] == note) {
                for (size_t j = i; j + 1 < numHeldNotes_; ++j) {
                    heldNotes_[j] = heldNotes_[j + 1];
                }
                --numHeldNotes_;
                break;
            }
        }

        if (numHeldNotes_ == 0) {
            filterEnv_.gate(false);
            ampEnv_.gate(false);
        } else {
            resolveActiveNote();
            targetFreq_ = midiNoteToFrequency(static_cast<float>(activeNote_));
            if (glideTime_ <= 0.0001f) {
                currentFreq_ = targetFreq_;
            }
        }
    }

    void allNotesOff() noexcept {
        numHeldNotes_ = 0;
        filterEnv_.gate(false);
        ampEnv_.gate(false);
    }

    // Component accessors for configuration
    [[nodiscard]] Oscillator& getOsc1() noexcept { return osc1_; }
    [[nodiscard]] Oscillator& getOsc2() noexcept { return osc2_; }
    [[nodiscard]] SubOscillator& getSubOsc() noexcept { return subOsc_; }
    [[nodiscard]] NoiseGenerator& getNoise() noexcept { return noise_; }
    [[nodiscard]] Mixer& getMixer() noexcept { return mixer_; }
    [[nodiscard]] LadderFilter& getFilter() noexcept { return filter_; }
    [[nodiscard]] Multidrive& getMultidrive() noexcept { return multidrive_; }
    [[nodiscard]] Envelope& getFilterEnv() noexcept { return filterEnv_; }
    [[nodiscard]] Envelope& getAmpEnv() noexcept { return ampEnv_; }
    [[nodiscard]] LFO& getLFO() noexcept { return lfo_; }

    [[nodiscard]] bool isActive() const noexcept {
        return ampEnv_.isActive();
    }

    // Process a single audio sample through the monophonic voice chain
    [[nodiscard]] float processSample() noexcept {
        // 1. Portamento glide smoothing
        if (currentFreq_ != targetFreq_) {
            if (glideTime_ > 0.0001f) {
                currentFreq_ += glideCoeff_ * (targetFreq_ - currentFreq_);
                if (std::fabs(targetFreq_ - currentFreq_) < 0.05f) {
                    currentFreq_ = targetFreq_;
                }
            } else {
                currentFreq_ = targetFreq_;
            }
        }

        // 2. Evaluate modulation generators
        const float lfoMod = lfo_.process();
        const float fEnvVal = filterEnv_.process();
        const float aEnvVal = ampEnv_.process();

        // 3. Modulate and drive Oscillators
        const float baseFreq = currentFreq_;
        osc1_.setFrequency(baseFreq);
        osc2_.setFrequency(baseFreq);
        subOsc_.setBaseFrequency(baseFreq);

        // Apply pitch bend and LFO modulation to oscillators
        float osc1PitchMod = pitchBendSemitones_;
        float osc2PitchMod = pitchBendSemitones_;
        if (lfoPitchDest_ == LFODestination::PitchAll) {
            osc1PitchMod += lfoMod * 12.0f;
            osc2PitchMod += lfoMod * 12.0f;
        } else if (lfoPitchDest_ == LFODestination::PitchOsc2Only) {
            osc2PitchMod += lfoMod * 12.0f;
        }
        osc1_.setPitchModulation(osc1PitchMod);
        osc2_.setPitchModulation(osc2PitchMod);

        // Optional LFO modulation to wave morph
        if (lfoWaveDest_) {
            const float baseWave = osc1_.getWaveform();
            osc1_.setWaveform(clamp(baseWave + lfoMod * 0.5f, 0.0f, 1.0f));
        }

        // Render Osc 1 and capture phase wrap for Hard Sync
        bool osc1Wrapped = false;
        const float osc1Out = osc1_.process(&osc1Wrapped);

        // Lock Sub-Oscillator phase to Osc 1
        subOsc_.syncToOsc1(osc1_.getPhase());
        const float subOut = subOsc_.process();

        // Render Osc 2 with Hard Sync slaved to Osc 1
        const float syncFraction = osc1_.getPhaseIncrement() > 0.0f ? osc1_.getPhase() / osc1_.getPhaseIncrement() : 0.0f;
        const float osc2Out = osc2_.process(nullptr, syncFraction, osc1Wrapped);

        // Pink Noise
        const float noiseOut = noise_.process();

        // 4. Mixer Stage with Pre-Filter Saturation
        const float mixed = mixer_.process(osc1Out, osc2Out, subOut, noiseOut);

        // 5. Ladder Filter Modulation & Processing
        // Calculate modulated cutoff
        float effectiveCutoff = filterBaseCutoff_;

        // Key tracking (e.g. tracking active note relative to Middle C / note 60)
        if (filterKeyTracking_ > 0.0f) {
            const float keyDelta = static_cast<float>(activeNote_ - 60);
            effectiveCutoff *= semitonesToRatio(keyDelta * filterKeyTracking_);
        }

        // Filter Envelope modulation (up to +7 octaves / +84 semitones)
        if (std::fabs(filterEnvAmount_) > 0.001f) {
            const float envSemitones = filterEnvAmount_ * fEnvVal * 84.0f;
            effectiveCutoff *= semitonesToRatio(envSemitones);
        }

        // LFO Cutoff modulation
        if (lfoCutoffDest_) {
            const float lfoSemitones = lfoMod * 36.0f; // +/- 3 octaves
            effectiveCutoff *= semitonesToRatio(lfoSemitones);
        }

        filter_.setCutoff(effectiveCutoff);

        // Link Multidrive Stage 1 (OTA Pre-Drive) into ladder filter
        filter_.setPreDrive(multidrive_.getPreFilterDrive());

        // Process through 4-pole ZDF Ladder Filter
        const float filtered = filter_.process(mixed);

        // 6. Multidrive Stage 2 (Asymmetric FET/Diode clipping & Bass Retention)
        const float driven = multidrive_.process(filtered);

        // 7. VCA & Master Output Stage
        const float vcaGain = aEnvVal * velocity_ * masterVolume_;
        return driven * vcaGain;
    }

    void setBaseFilterCutoff(float cutoffHz) noexcept {
        filterBaseCutoff_ = clamp(cutoffHz, 20.0f, 20000.0f);
    }

    void setLFOPitchDestination(LFODestination dest) noexcept {
        lfoPitchDest_ = dest;
    }

    void setLFOCutoffDestination(bool enabled) noexcept {
        lfoCutoffDest_ = enabled;
    }

    void setLFOWaveDestination(bool enabled) noexcept {
        lfoWaveDest_ = enabled;
    }

private:
    void resolveActiveNote() noexcept {
        if (numHeldNotes_ == 0) return;

        switch (notePriority_) {
            case NotePriority::Last:
                activeNote_ = heldNotes_[numHeldNotes_ - 1];
                break;
            case NotePriority::Lowest: {
                uint8_t minN = heldNotes_[0];
                for (size_t i = 1; i < numHeldNotes_; ++i) {
                    if (heldNotes_[i] < minN) minN = heldNotes_[i];
                }
                activeNote_ = minN;
                break;
            }
            case NotePriority::Highest: {
                uint8_t maxN = heldNotes_[0];
                for (size_t i = 1; i < numHeldNotes_; ++i) {
                    if (heldNotes_[i] > maxN) maxN = heldNotes_[i];
                }
                activeNote_ = maxN;
                break;
            }
        }
    }

    void updateGlideCoeff() noexcept {
        if (glideTime_ <= 0.0001f) {
            glideCoeff_ = 1.0f;
        } else {
            glideCoeff_ = 1.0f - std::exp(-1.0f / (glideTime_ * 0.2f * sampleRate_));
        }
    }

    float sampleRate_{DEFAULT_SAMPLE_RATE};
    float invSampleRate_{1.0f / DEFAULT_SAMPLE_RATE};

    // Note allocation & Glide
    std::array<uint8_t, MAX_HELD_NOTES> heldNotes_{};
    size_t numHeldNotes_{0};
    uint8_t activeNote_{60};
    NotePriority notePriority_{NotePriority::Last};
    bool legato_{true};
    float glideTime_{0.0f};
    bool glideAlways_{false};
    float glideCoeff_{1.0f};

    float targetFreq_{261.63f};
    float currentFreq_{261.63f};
    float velocity_{1.0f};
    float pitchBendSemitones_{0.0f};

    float filterBaseCutoff_{1000.0f};
    float filterEnvAmount_{0.5f};
    float filterKeyTracking_{0.5f};
    float masterVolume_{0.8f};

    LFODestination lfoPitchDest_{LFODestination::PitchAll};
    bool lfoCutoffDest_{false};
    bool lfoWaveDest_{false};

    // Voice Modules
    Oscillator osc1_;
    Oscillator osc2_;
    SubOscillator subOsc_;
    NoiseGenerator noise_;
    Mixer mixer_;
    LadderFilter filter_;
    Multidrive multidrive_;
    Envelope filterEnv_;
    Envelope ampEnv_;
    LFO lfo_;
};

} // namespace sub_phatty
