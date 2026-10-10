#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "sub_phatty/SynthEngine.hpp"
#include <atomic>

class SubGyattyAudioProcessor : public juce::AudioProcessor {
public:
    SubGyattyAudioProcessor();
    ~SubGyattyAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SubGyatty"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts_; }
    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState_; }
    bool hasMidiActivity() const noexcept { return midiActivity_.load(std::memory_order_relaxed); }
    void clearMidiActivity() noexcept { midiActivity_.store(false, std::memory_order_relaxed); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void cacheParameterPointers() noexcept;
    void updateSynthParameters() noexcept;

    sub_phatty::SynthEngine synth_;
    juce::AudioProcessorValueTreeState apvts_;
    juce::MidiKeyboardState keyboardState_;
    std::atomic<bool> midiActivity_{false};

    // Cached raw parameter pointers for lock-free audio thread reads
    std::atomic<float>* osc1OctaveParam_{nullptr};
    std::atomic<float>* osc1WaveParam_{nullptr};
    std::atomic<float>* osc1LevelParam_{nullptr};

    std::atomic<float>* osc2OctaveParam_{nullptr};
    std::atomic<float>* osc2WaveParam_{nullptr};
    std::atomic<float>* osc2DetuneParam_{nullptr};
    std::atomic<float>* osc2LevelParam_{nullptr};
    std::atomic<float>* osc2SyncParam_{nullptr};

    std::atomic<float>* subOscLevelParam_{nullptr};
    std::atomic<float>* noiseLevelParam_{nullptr};

    std::atomic<float>* filterCutoffParam_{nullptr};
    std::atomic<float>* filterResonanceParam_{nullptr};
    std::atomic<float>* filterMultidriveParam_{nullptr};
    std::atomic<float>* filterSlopeParam_{nullptr};
    std::atomic<float>* filterEnvAmountParam_{nullptr};
    std::atomic<float>* filterKbTrackParam_{nullptr};

    std::atomic<float>* filterAttackParam_{nullptr};
    std::atomic<float>* filterDecayParam_{nullptr};
    std::atomic<float>* filterSustainParam_{nullptr};
    std::atomic<float>* filterReleaseParam_{nullptr};

    std::atomic<float>* ampAttackParam_{nullptr};
    std::atomic<float>* ampDecayParam_{nullptr};
    std::atomic<float>* ampSustainParam_{nullptr};
    std::atomic<float>* ampReleaseParam_{nullptr};

    std::atomic<float>* lfoRateParam_{nullptr};
    std::atomic<float>* lfoWaveParam_{nullptr};
    std::atomic<float>* lfoAmountParam_{nullptr};

    std::atomic<float>* glideTimeParam_{nullptr};
    std::atomic<float>* legatoParam_{nullptr};
    std::atomic<float>* masterVolumeParam_{nullptr};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SubGyattyAudioProcessor)
};
