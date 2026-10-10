#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include <vector>
#include <memory>

// Hardware-style vintage LookAndFeel for rotary knobs and switches
class MoogKnobLookAndFeel : public juce::LookAndFeel_V4 {
public:
    MoogKnobLookAndFeel() {
        setColour(juce::Slider::thumbColourId, juce::Colours::silver);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffe5983b)); // Moog amber
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff222226));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;
};

class SubGyattyAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      public juce::Timer
{
public:
    explicit SubGyattyAudioProcessorEditor(SubGyattyAudioProcessor&);
    ~SubGyattyAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct LabeledKnob {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<SliderAttachment> attachment;
    };

    void setupKnob(LabeledKnob& knob, const juce::String& paramId, const juce::String& labelText, bool isLarge = false);

    SubGyattyAudioProcessor& processor_;
    MoogKnobLookAndFeel lookAndFeel_;

    // On-screen playable virtual keyboard
    juce::MidiKeyboardComponent keyboardComponent_;

    // Section 1: Oscillators
    LabeledKnob osc1OctaveKnob_;
    LabeledKnob osc1WaveKnob_;
    LabeledKnob osc2OctaveKnob_;
    LabeledKnob osc2WaveKnob_;
    LabeledKnob osc2DetuneKnob_;
    juce::ToggleButton osc2SyncButton_{"HARD SYNC"};
    std::unique_ptr<ButtonAttachment> osc2SyncAttachment_;

    // Section 2: Mixer
    LabeledKnob osc1LevelKnob_;
    LabeledKnob osc2LevelKnob_;
    LabeledKnob subOscLevelKnob_;
    LabeledKnob noiseLevelKnob_;

    // Section 3: Ladder Filter & Multidrive
    LabeledKnob filterCutoffKnob_;
    LabeledKnob filterResonanceKnob_;
    LabeledKnob filterMultidriveKnob_;
    LabeledKnob filterEnvAmountKnob_;
    LabeledKnob filterKbTrackKnob_;
    juce::ComboBox filterSlopeBox_;
    std::unique_ptr<ComboBoxAttachment> filterSlopeAttachment_;
    juce::Label filterSlopeLabel_;

    // Section 4: Envelopes (Filter & Amp)
    LabeledKnob filterAttackKnob_;
    LabeledKnob filterDecayKnob_;
    LabeledKnob filterSustainKnob_;
    LabeledKnob filterReleaseKnob_;

    LabeledKnob ampAttackKnob_;
    LabeledKnob ampDecayKnob_;
    LabeledKnob ampSustainKnob_;
    LabeledKnob ampReleaseKnob_;

    // Section 5: Modulation & Glide
    LabeledKnob lfoRateKnob_;
    LabeledKnob lfoAmountKnob_;
    juce::ComboBox lfoShapeBox_;
    std::unique_ptr<ComboBoxAttachment> lfoShapeAttachment_;
    juce::Label lfoShapeLabel_;

    LabeledKnob glideTimeKnob_;
    juce::ToggleButton legatoButton_{"LEGATO"};
    std::unique_ptr<ButtonAttachment> legatoAttachment_;

    // Master
    LabeledKnob masterVolumeKnob_;

    float midiLedBrightness_{0.0f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SubGyattyAudioProcessorEditor)
};
