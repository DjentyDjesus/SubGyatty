#include "PluginProcessor.h"
#include "PluginEditor.h"

SubGyattyAudioProcessor::SubGyattyAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "Parameters", createParameterLayout())
{
    cacheParameterPointers();
}

void SubGyattyAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    synth_.setSampleRate(static_cast<float>(sampleRate));
    synth_.reset();
}

void SubGyattyAudioProcessor::releaseResources()
{
}

bool SubGyattyAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void SubGyattyAudioProcessor::cacheParameterPointers() noexcept
{
    osc1OctaveParam_      = apvts_.getRawParameterValue("osc1_octave");
    osc1WaveParam_        = apvts_.getRawParameterValue("osc1_wave");
    osc1LevelParam_       = apvts_.getRawParameterValue("osc1_level");

    osc2OctaveParam_      = apvts_.getRawParameterValue("osc2_octave");
    osc2WaveParam_        = apvts_.getRawParameterValue("osc2_wave");
    osc2DetuneParam_      = apvts_.getRawParameterValue("osc2_detune");
    osc2LevelParam_       = apvts_.getRawParameterValue("osc2_level");
    osc2SyncParam_        = apvts_.getRawParameterValue("osc2_sync");

    subOscLevelParam_     = apvts_.getRawParameterValue("sub_osc_level");
    noiseLevelParam_      = apvts_.getRawParameterValue("noise_level");

    filterCutoffParam_    = apvts_.getRawParameterValue("filter_cutoff");
    filterResonanceParam_ = apvts_.getRawParameterValue("filter_resonance");
    filterMultidriveParam_= apvts_.getRawParameterValue("filter_multidrive");
    filterSlopeParam_     = apvts_.getRawParameterValue("filter_slope");
    filterEnvAmountParam_ = apvts_.getRawParameterValue("filter_env_amount");
    filterKbTrackParam_   = apvts_.getRawParameterValue("filter_kb_track");

    filterAttackParam_    = apvts_.getRawParameterValue("filter_attack");
    filterDecayParam_     = apvts_.getRawParameterValue("filter_decay");
    filterSustainParam_   = apvts_.getRawParameterValue("filter_sustain");
    filterReleaseParam_   = apvts_.getRawParameterValue("filter_release");

    ampAttackParam_       = apvts_.getRawParameterValue("amp_attack");
    ampDecayParam_        = apvts_.getRawParameterValue("amp_decay");
    ampSustainParam_      = apvts_.getRawParameterValue("amp_sustain");
    ampReleaseParam_      = apvts_.getRawParameterValue("amp_release");

    lfoRateParam_         = apvts_.getRawParameterValue("lfo_rate");
    lfoWaveParam_         = apvts_.getRawParameterValue("lfo_wave");
    lfoAmountParam_       = apvts_.getRawParameterValue("lfo_amount");

    glideTimeParam_       = apvts_.getRawParameterValue("glide_time");
    legatoParam_          = apvts_.getRawParameterValue("legato");
    masterVolumeParam_    = apvts_.getRawParameterValue("master_volume");
}

void SubGyattyAudioProcessor::updateSynthParameters() noexcept
{
    using namespace sub_phatty;

    if (osc1OctaveParam_)       synth_.setParameter(ParamId::Osc1Octave, osc1OctaveParam_->load(std::memory_order_relaxed));
    if (osc1WaveParam_)         synth_.setParameter(ParamId::Osc1Waveform, osc1WaveParam_->load(std::memory_order_relaxed));
    if (osc1LevelParam_)        synth_.setParameter(ParamId::Osc1Level, osc1LevelParam_->load(std::memory_order_relaxed));

    if (osc2OctaveParam_)       synth_.setParameter(ParamId::Osc2Octave, osc2OctaveParam_->load(std::memory_order_relaxed));
    if (osc2WaveParam_)         synth_.setParameter(ParamId::Osc2Waveform, osc2WaveParam_->load(std::memory_order_relaxed));
    if (osc2DetuneParam_)       synth_.setParameter(ParamId::Osc2Detune, osc2DetuneParam_->load(std::memory_order_relaxed));
    if (osc2LevelParam_)        synth_.setParameter(ParamId::Osc2Level, osc2LevelParam_->load(std::memory_order_relaxed));
    if (osc2SyncParam_)         synth_.setParameter(ParamId::Osc2Sync, osc2SyncParam_->load(std::memory_order_relaxed));

    if (subOscLevelParam_)      synth_.setParameter(ParamId::SubOscLevel, subOscLevelParam_->load(std::memory_order_relaxed));
    if (noiseLevelParam_)       synth_.setParameter(ParamId::NoiseLevel, noiseLevelParam_->load(std::memory_order_relaxed));

    if (filterCutoffParam_)     synth_.setParameter(ParamId::FilterCutoff, filterCutoffParam_->load(std::memory_order_relaxed));
    if (filterResonanceParam_)  synth_.setParameter(ParamId::FilterResonance, filterResonanceParam_->load(std::memory_order_relaxed));
    if (filterMultidriveParam_) synth_.setParameter(ParamId::FilterMultidrive, filterMultidriveParam_->load(std::memory_order_relaxed));
    if (filterSlopeParam_)      synth_.setParameter(ParamId::FilterSlope, filterSlopeParam_->load(std::memory_order_relaxed));
    if (filterEnvAmountParam_)  synth_.setParameter(ParamId::FilterEnvAmount, filterEnvAmountParam_->load(std::memory_order_relaxed));
    if (filterKbTrackParam_)    synth_.setParameter(ParamId::FilterKeyTracking, filterKbTrackParam_->load(std::memory_order_relaxed));

    if (filterAttackParam_)     synth_.setParameter(ParamId::FilterAttack, filterAttackParam_->load(std::memory_order_relaxed));
    if (filterDecayParam_)      synth_.setParameter(ParamId::FilterDecay, filterDecayParam_->load(std::memory_order_relaxed));
    if (filterSustainParam_)    synth_.setParameter(ParamId::FilterSustain, filterSustainParam_->load(std::memory_order_relaxed));
    if (filterReleaseParam_)    synth_.setParameter(ParamId::FilterRelease, filterReleaseParam_->load(std::memory_order_relaxed));

    if (ampAttackParam_)        synth_.setParameter(ParamId::AmpAttack, ampAttackParam_->load(std::memory_order_relaxed));
    if (ampDecayParam_)         synth_.setParameter(ParamId::AmpDecay, ampDecayParam_->load(std::memory_order_relaxed));
    if (ampSustainParam_)       synth_.setParameter(ParamId::AmpSustain, ampSustainParam_->load(std::memory_order_relaxed));
    if (ampReleaseParam_)       synth_.setParameter(ParamId::AmpRelease, ampReleaseParam_->load(std::memory_order_relaxed));

    if (lfoRateParam_)          synth_.setParameter(ParamId::LFORate, lfoRateParam_->load(std::memory_order_relaxed));
    if (lfoWaveParam_)          synth_.setParameter(ParamId::LFOShape, lfoWaveParam_->load(std::memory_order_relaxed));
    if (lfoAmountParam_)        synth_.setParameter(ParamId::LFOAmount, lfoAmountParam_->load(std::memory_order_relaxed));

    if (glideTimeParam_)        synth_.setParameter(ParamId::GlideTime, glideTimeParam_->load(std::memory_order_relaxed));
    if (legatoParam_)           synth_.setParameter(ParamId::Legato, legatoParam_->load(std::memory_order_relaxed));
    if (masterVolumeParam_)     synth_.setParameter(ParamId::MasterVolume, masterVolumeParam_->load(std::memory_order_relaxed));
}

void SubGyattyAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // Clear unused input channels
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    // Merge virtual on-screen keyboard events
    keyboardState_.processNextMidiBuffer(midiMessages, 0, numSamples, true);

    // Apply updated parameters
    updateSynthParameters();

    // Parse incoming MIDI events with zero allocation
    for (const auto metadata : midiMessages)
    {
        const auto message = metadata.getMessage();
        if (message.isNoteOn())
        {
            synth_.handleNoteOn(static_cast<uint8_t>(message.getNoteNumber()), static_cast<uint8_t>(message.getVelocity()));
            midiActivity_.store(true, std::memory_order_relaxed);
        }
        else if (message.isNoteOff())
        {
            synth_.handleNoteOff(static_cast<uint8_t>(message.getNoteNumber()));
        }
        else if (message.isPitchWheel())
        {
            // Pitch bend: range [-8192, 8191]
            synth_.handlePitchBend(static_cast<int16_t>(message.getPitchWheelValue() - 8192));
        }
        else if (message.isController())
        {
            synth_.handleMidiCC(static_cast<uint8_t>(message.getControllerNumber()), static_cast<uint8_t>(message.getControllerValue()));
        }
        else if (message.isAllNotesOff())
        {
            synth_.handleAllNotesOff();
        }
    }

    // Render audio block
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = (totalNumOutputChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    synth_.process(leftChannel, rightChannel ? rightChannel : leftChannel, static_cast<size_t>(numSamples));
}

juce::AudioProcessorEditor* SubGyattyAudioProcessor::createEditor()
{
    return new SubGyattyAudioProcessorEditor(*this);
}

void SubGyattyAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SubGyattyAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts_.state.getType()))
        apvts_.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout SubGyattyAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // --- Oscillators ---
    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"osc1_octave", 1}, "Osc 1 Octave", -2, 1, -1));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc1_wave", 1}, "Osc 1 Wave",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.333f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc1_level", 1}, "Osc 1 Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.85f));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"osc2_octave", 1}, "Osc 2 Octave", -2, 1, -1));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc2_wave", 1}, "Osc 2 Wave",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.666f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc2_detune", 1}, "Osc 2 Detune (Cents)",
        juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f), 7.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"osc2_level", 1}, "Osc 2 Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"osc2_sync", 1}, "Osc 2 Hard Sync", false));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"sub_osc_level", 1}, "Sub-Osc Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"noise_level", 1}, "Noise Level",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));

    // --- Ladder Filter & Multidrive ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_cutoff", 1}, "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.25f), 1200.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_resonance", 1}, "Filter Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.25f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_multidrive", 1}, "Multidrive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.20f));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{"filter_slope", 1}, "Filter Slope (Poles)", 1, 4, 4));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_env_amount", 1}, "Filter Env Amount",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.60f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_kb_track", 1}, "Keyboard Track",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.50f));

    // --- Filter Envelope ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_attack", 1}, "Filter Attack",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.35f), 0.010f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_decay", 1}, "Filter Decay",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.001f, 0.35f), 0.350f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_sustain", 1}, "Filter Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.30f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"filter_release", 1}, "Filter Release",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.001f, 0.35f), 0.200f));

    // --- Amp Envelope ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"amp_attack", 1}, "Amp Attack",
        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.35f), 0.005f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"amp_decay", 1}, "Amp Decay",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.001f, 0.35f), 0.250f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"amp_sustain", 1}, "Amp Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.80f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"amp_release", 1}, "Amp Release",
        juce::NormalisableRange<float>(0.01f, 10.0f, 0.001f, 0.35f), 0.150f));

    // --- Modulation & Performance ---
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lfo_rate", 1}, "LFO Rate",
        juce::NormalisableRange<float>(0.1f, 1000.0f, 0.01f, 0.25f), 3.5f));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"lfo_wave", 1}, "LFO Shape",
        juce::StringArray{"Triangle", "Square", "Saw", "Ramp", "S&H"}, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"lfo_amount", 1}, "LFO Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"glide_time", 1}, "Glide Time",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f, 0.4f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"legato", 1}, "Legato Mode", true));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"master_volume", 1}, "Master Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.85f));

    return layout;
}

// AudioProcessor creation hook for JUCE
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SubGyattyAudioProcessor();
}
