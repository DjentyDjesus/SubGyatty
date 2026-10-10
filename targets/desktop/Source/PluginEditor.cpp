#include "PluginEditor.h"

// --- Custom Vintage LookAndFeel Implementation ---

void MoogKnobLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPosProportional, float rotaryStartAngle,
                                           float rotaryEndAngle, juce::Slider& /*slider*/)
{
    const float radius = static_cast<float>(juce::jmin(width / 2, height / 2)) - 4.0f;
    const float centreX = static_cast<float>(x + width / 2);
    const float centreY = static_cast<float>(y + height / 2);
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // 1. Outer background arc track
    const float trackWidth = 3.5f;
    juce::Path trackPath;
    trackPath.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff252528));
    g.strokePath(trackPath, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Active value arc (Moog vintage amber glow)
    if (sliderPosProportional > 0.001f)
    {
        juce::Path valuePath;
        valuePath.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(juce::Colour(0xffe5983b));
        g.strokePath(valuePath, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Knob Body (Outer fluted black skirt)
    const float knobRadius = radius - 7.0f;
    if (knobRadius <= 2.0f) return;

    // Outer drop shadow
    g.setColour(juce::Colour(0x60000000));
    g.fillEllipse(centreX - knobRadius - 1.0f, centreY - knobRadius + 1.0f, (knobRadius + 1.0f) * 2.0f, (knobRadius + 1.0f) * 2.0f);

    // Dark charcoal knob skirt
    juce::ColourGradient skirtGrad(juce::Colour(0xff323236), centreX - knobRadius * 0.5f, centreY - knobRadius * 0.5f,
                                   juce::Colour(0xff121214), centreX + knobRadius, centreY + knobRadius, true);
    g.setGradientFill(skirtGrad);
    g.fillEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    // Bevel rim
    g.setColour(juce::Colour(0xff444448));
    g.drawEllipse(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);

    // 4. Inner metallic cap (brushed aluminum)
    const float capRadius = knobRadius * 0.65f;
    juce::ColourGradient capGrad(juce::Colour(0xffdcdce0), centreX - capRadius * 0.3f, centreY - capRadius * 0.3f,
                                 juce::Colour(0xff7a7a82), centreX + capRadius * 0.5f, centreY + capRadius * 0.5f, true);
    g.setGradientFill(capGrad);
    g.fillEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);
    g.setColour(juce::Colour(0xff55555c));
    g.drawEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);

    // 5. Indicator notch pointer
    juce::Path pointer;
    const float pointerLength = knobRadius - 2.0f;
    const float pointerThickness = 2.4f;
    pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength - (capRadius * 0.3f));
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(juce::Colours::white);
    g.fillPath(pointer);
}

void MoogKnobLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                           bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    const bool isToggled = button.getToggleState();

    // Button body
    g.setColour(isToggled ? juce::Colour(0xff2a2318) : juce::Colour(0xff1a1a1c));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border
    g.setColour(isToggled ? juce::Colour(0xffe5983b) : (shouldDrawButtonAsHighlighted ? juce::Colour(0xff66666c) : juce::Colour(0xff333338)));
    g.drawRoundedRectangle(bounds, 4.0f, isToggled ? 1.5f : 1.0f);

    // LED indicator pip
    const float ledX = bounds.getX() + 8.0f;
    const float ledY = bounds.getCentreY();
    const float ledR = 3.5f;

    if (isToggled)
    {
        g.setColour(juce::Colour(0x80ffaa33));
        g.fillEllipse(ledX - ledR - 2.0f, ledY - ledR - 2.0f, (ledR + 2.0f) * 2.0f, (ledR + 2.0f) * 2.0f);
        g.setColour(juce::Colour(0xffffbb44));
        g.fillEllipse(ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);
    }
    else
    {
        g.setColour(juce::Colour(0xff3a3a40));
        g.fillEllipse(ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);
    }

    // Text label
    g.setColour(isToggled ? juce::Colours::white : juce::Colour(0xffa0a0a8));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), bounds.withTrimmedLeft(20.0f).toNearestInt(), juce::Justification::centredLeft, true);
}

void MoogKnobLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                      int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/, juce::ComboBox& /*box*/)
{
    auto bounds = juce::Rectangle<float>(0, 0, static_cast<float>(width), static_cast<float>(height)).reduced(1.0f);
    g.setColour(juce::Colour(0xff18181a));
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(juce::Colour(0xff44444c));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    // Arrow indicator
    juce::Path arrow;
    const float right = static_cast<float>(width) - 12.0f;
    const float midY = static_cast<float>(height) * 0.5f;
    arrow.addTriangle(right - 4.0f, midY - 2.5f, right + 4.0f, midY - 2.5f, right, midY + 3.0f);
    g.setColour(juce::Colour(0xffe5983b));
    g.fillPath(arrow);
}

// --- SubGyattyAudioProcessorEditor Implementation ---

SubGyattyAudioProcessorEditor::SubGyattyAudioProcessorEditor(SubGyattyAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor_(p),
      keyboardComponent_(p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel(&lookAndFeel_);

    // Section 1: Oscillators
    setupKnob(osc1OctaveKnob_, "osc1_octave", "OCTAVE");
    setupKnob(osc1WaveKnob_,   "osc1_wave",   "WAVE");
    setupKnob(osc2OctaveKnob_, "osc2_octave", "OCTAVE");
    setupKnob(osc2WaveKnob_,   "osc2_wave",   "WAVE");
    setupKnob(osc2DetuneKnob_, "osc2_detune", "DETUNE");

    addAndMakeVisible(osc2SyncButton_);
    osc2SyncAttachment_ = std::make_unique<ButtonAttachment>(processor_.getAPVTS(), "osc2_sync", osc2SyncButton_);

    // Section 2: Mixer
    setupKnob(osc1LevelKnob_,   "osc1_level",    "OSC 1");
    setupKnob(osc2LevelKnob_,   "osc2_level",    "OSC 2");
    setupKnob(subOscLevelKnob_, "sub_osc_level", "SUB OSC");
    setupKnob(noiseLevelKnob_,  "noise_level",   "NOISE");

    // Section 3: Filter & Multidrive
    setupKnob(filterCutoffKnob_,     "filter_cutoff",     "CUTOFF", true);
    setupKnob(filterResonanceKnob_,  "filter_resonance",  "RESONANCE");
    setupKnob(filterMultidriveKnob_, "filter_multidrive", "MULTIDRIVE", true);
    setupKnob(filterEnvAmountKnob_,  "filter_env_amount", "AMOUNT");
    setupKnob(filterKbTrackKnob_,    "filter_kb_track",   "KB TRACK");

    filterSlopeBox_.addItemList({"6 dB (1-Pole)", "12 dB (2-Pole)", "18 dB (3-Pole)", "24 dB (4-Pole)"}, 1);
    addAndMakeVisible(filterSlopeBox_);
    filterSlopeAttachment_ = std::make_unique<ComboBoxAttachment>(processor_.getAPVTS(), "filter_slope", filterSlopeBox_);
    filterSlopeLabel_.setText("SLOPE", juce::dontSendNotification);
    filterSlopeLabel_.setJustificationType(juce::Justification::centred);
    filterSlopeLabel_.setFont(juce::Font(10.0f, juce::Font::bold));
    filterSlopeLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff9999a0));
    addAndMakeVisible(filterSlopeLabel_);

    // Section 4: Envelopes
    setupKnob(filterAttackKnob_,  "filter_attack",  "ATTACK");
    setupKnob(filterDecayKnob_,   "filter_decay",   "DECAY");
    setupKnob(filterSustainKnob_, "filter_sustain", "SUSTAIN");
    setupKnob(filterReleaseKnob_, "filter_release", "RELEASE");

    setupKnob(ampAttackKnob_,  "amp_attack",  "ATTACK");
    setupKnob(ampDecayKnob_,   "amp_decay",   "DECAY");
    setupKnob(ampSustainKnob_, "amp_sustain", "SUSTAIN");
    setupKnob(ampReleaseKnob_, "amp_release", "RELEASE");

    // Section 5: Modulation & Glide
    setupKnob(lfoRateKnob_,   "lfo_rate",   "RATE");
    setupKnob(lfoAmountKnob_, "lfo_amount", "AMOUNT");

    lfoShapeBox_.addItemList({"Triangle", "Square", "Saw", "Ramp", "S&H"}, 1);
    addAndMakeVisible(lfoShapeBox_);
    lfoShapeAttachment_ = std::make_unique<ComboBoxAttachment>(processor_.getAPVTS(), "lfo_wave", lfoShapeBox_);
    lfoShapeLabel_.setText("LFO SHAPE", juce::dontSendNotification);
    lfoShapeLabel_.setJustificationType(juce::Justification::centred);
    lfoShapeLabel_.setFont(juce::Font(10.0f, juce::Font::bold));
    lfoShapeLabel_.setColour(juce::Label::textColourId, juce::Colour(0xff9999a0));
    addAndMakeVisible(lfoShapeLabel_);

    setupKnob(glideTimeKnob_, "glide_time", "GLIDE");
    addAndMakeVisible(legatoButton_);
    legatoAttachment_ = std::make_unique<ButtonAttachment>(processor_.getAPVTS(), "legato", legatoButton_);

    // Master
    setupKnob(masterVolumeKnob_, "master_volume", "VOLUME");

    // Bottom Virtual Keyboard
    keyboardComponent_.setKeyWidth(19.0f);
    keyboardComponent_.setAvailableRange(36, 84); // C2 to C6
    addAndMakeVisible(keyboardComponent_);

    setSize(1000, 600);
    startTimerHz(30);
}

SubGyattyAudioProcessorEditor::~SubGyattyAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void SubGyattyAudioProcessorEditor::setupKnob(LabeledKnob& knob, const juce::String& paramId,
                                             const juce::String& labelText, bool isLarge)
{
    knob.slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    knob.slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(knob.slider);

    knob.label.setText(labelText, juce::dontSendNotification);
    knob.label.setJustificationType(juce::Justification::centred);
    knob.label.setFont(juce::Font(isLarge ? 11.5f : 10.0f, juce::Font::bold));
    knob.label.setColour(juce::Label::textColourId, isLarge ? juce::Colour(0xffe5983b) : juce::Colour(0xffb0b0b8));
    addAndMakeVisible(knob.label);

    knob.attachment = std::make_unique<SliderAttachment>(processor_.getAPVTS(), paramId, knob.slider);
}

void SubGyattyAudioProcessorEditor::timerCallback()
{
    if (processor_.hasMidiActivity())
    {
        midiLedBrightness_ = 1.0f;
        processor_.clearMidiActivity();
        repaint(220, 16, 24, 24);
    }
    else if (midiLedBrightness_ > 0.01f)
    {
        midiLedBrightness_ *= 0.85f;
        repaint(220, 16, 24, 24);
    }
}

void SubGyattyAudioProcessorEditor::paint(juce::Graphics& g)
{
    // 1. Vintage textured matte black chassis
    g.fillAll(juce::Colour(0xff121214));

    // Subtle brushed chassis gradient
    juce::ColourGradient bgGrad(juce::Colour(0xff19191d), 0, 0,
                                juce::Colour(0xff0d0d0f), 0, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGrad);
    g.fillRect(getLocalBounds().withTrimmedBottom(80));

    // 2. Top Header Bar
    g.setColour(juce::Colour(0xff1e1e24));
    g.fillRect(0, 0, getWidth(), 56);
    g.setColour(juce::Colour(0xff35353d));
    g.drawHorizontalLine(56, 0.0f, static_cast<float>(getWidth()));

    // Hardware Logo Badge
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.drawText("SUBGYATTY", 24, 10, 180, 24, juce::Justification::left);

    g.setColour(juce::Colour(0xffe5983b));
    g.setFont(juce::Font(9.5f, juce::Font::bold));
    g.drawText("ANALOG EMULATION SYNTHESIZER  |  ZDF 4-POLE", 25, 34, 250, 14, juce::Justification::left);

    // MIDI Activity LED
    const float ledX = 230.0f;
    const float ledY = 22.0f;
    g.setColour(juce::Colour(0xff333338));
    g.fillEllipse(ledX - 5.0f, ledY - 5.0f, 10.0f, 10.0f);
    if (midiLedBrightness_ > 0.02f)
    {
        g.setColour(juce::Colour(0xffffaa33).withAlpha(midiLedBrightness_));
        g.fillEllipse(ledX - 7.0f, ledY - 7.0f, 14.0f, 14.0f);
    }
    g.setColour(juce::Colour(0xff777780));
    g.setFont(juce::Font(8.5f, juce::Font::bold));
    g.drawText("MIDI", static_cast<int>(ledX + 8.0f), 15, 30, 14, juce::Justification::left);

    // 3. Section Division Panels
    auto drawSectionBox = [&](juce::Rectangle<int> rect, const juce::String& title)
    {
        // Dark panel background
        g.setColour(juce::Colour(0xff161619));
        g.fillRoundedRectangle(rect.toFloat(), 6.0f);

        // Border line
        g.setColour(juce::Colour(0xff2a2a30));
        g.drawRoundedRectangle(rect.toFloat(), 6.0f, 1.2f);

        // Section Title Header bar
        auto headerRect = rect.removeFromTop(24);
        g.setColour(juce::Colour(0xff202026));
        g.fillRoundedRectangle(headerRect.toFloat(), 6.0f);
        // Cover bottom corners of header to join smoothly
        g.fillRect(headerRect.removeFromBottom(4));

        g.setColour(juce::Colour(0xffe0e0e6));
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText(title, headerRect, juce::Justification::centred);
    };

    // Calculate section bounds
    const int contentY = 66;
    const int contentH = getHeight() - 66 - 90; // Leave 90px for bottom keyboard
    const int w = getWidth();

    // 5 Sections:
    // Sec 1: Oscillators (200px)
    // Sec 2: Mixer (130px)
    // Sec 3: Filter & Multidrive (230px)
    // Sec 4: Envelopes (240px)
    // Sec 5: Mod & Glide (remaining ~160px)
    const int x1 = 12;
    const int w1 = 200;
    const int x2 = x1 + w1 + 10;
    const int w2 = 135;
    const int x3 = x2 + w2 + 10;
    const int w3 = 230;
    const int x4 = x3 + w3 + 10;
    const int w4 = 225;
    const int x5 = x4 + w4 + 10;
    const int w5 = w - x5 - 12;

    drawSectionBox({x1, contentY, w1, contentH}, "OSCILLATORS");
    drawSectionBox({x2, contentY, w2, contentH}, "MIXER");
    drawSectionBox({x3, contentY, w3, contentH}, "FILTER & MULTIDRIVE");
    drawSectionBox({x4, contentY, w4, contentH}, "ENVELOPES");
    drawSectionBox({x5, contentY, w5, contentH}, "MOD & GLIDE");

    // Divider line above keyboard
    g.setColour(juce::Colour(0xff33333a));
    g.drawHorizontalLine(getHeight() - 84, 0.0f, static_cast<float>(getWidth()));
}

void SubGyattyAudioProcessorEditor::resized()
{
    const int contentY = 96;
    const int knobH = 65;
    const int knobW = 60;

    // --- Master Volume in top bar ---
    masterVolumeKnob_.slider.setBounds(getWidth() - 85, 4, 48, 48);
    masterVolumeKnob_.label.setBounds(getWidth() - 85, 38, 48, 14);

    // --- Section 1: Oscillators (x: 12, w: 200) ---
    // Osc 1: Row 1
    osc1OctaveKnob_.slider.setBounds(25, contentY, knobW, knobH - 15);
    osc1OctaveKnob_.label.setBounds(25, contentY + knobH - 18, knobW, 14);

    osc1WaveKnob_.slider.setBounds(115, contentY, knobW, knobH - 15);
    osc1WaveKnob_.label.setBounds(115, contentY + knobH - 18, knobW, 14);

    // Osc 2: Row 2
    const int row2Y = contentY + 85;
    osc2OctaveKnob_.slider.setBounds(25, row2Y, knobW, knobH - 15);
    osc2OctaveKnob_.label.setBounds(25, row2Y + knobH - 18, knobW, 14);

    osc2WaveKnob_.slider.setBounds(115, row2Y, knobW, knobH - 15);
    osc2WaveKnob_.label.setBounds(115, row2Y + knobH - 18, knobW, 14);

    // Detune & Sync: Row 3
    const int row3Y = row2Y + 85;
    osc2DetuneKnob_.slider.setBounds(25, row3Y, knobW, knobH - 15);
    osc2DetuneKnob_.label.setBounds(25, row3Y + knobH - 18, knobW, 14);

    osc2SyncButton_.setBounds(105, row3Y + 12, 95, 26);

    // --- Section 2: Mixer (x: 222, w: 135) ---
    const int mixX = 260;
    osc1LevelKnob_.slider.setBounds(mixX, contentY, knobW, knobH - 15);
    osc1LevelKnob_.label.setBounds(mixX, contentY + knobH - 18, knobW, 14);

    osc2LevelKnob_.slider.setBounds(mixX, contentY + 70, knobW, knobH - 15);
    osc2LevelKnob_.label.setBounds(mixX, contentY + 70 + knobH - 18, knobW, 14);

    subOscLevelKnob_.slider.setBounds(mixX, contentY + 140, knobW, knobH - 15);
    subOscLevelKnob_.label.setBounds(mixX, contentY + 140 + knobH - 18, knobW, 14);

    noiseLevelKnob_.slider.setBounds(mixX, contentY + 210, knobW, knobH - 15);
    noiseLevelKnob_.label.setBounds(mixX, contentY + 210 + knobH - 18, knobW, 14);

    // --- Section 3: Filter & Multidrive (x: 367, w: 230) ---
    // Big prominent Cutoff and Multidrive knobs!
    filterCutoffKnob_.slider.setBounds(385, contentY - 5, 85, 85);
    filterCutoffKnob_.label.setBounds(385, contentY + 80, 85, 16);

    filterMultidriveKnob_.slider.setBounds(490, contentY - 5, 85, 85);
    filterMultidriveKnob_.label.setBounds(490, contentY + 80, 85, 16);

    // Resonance & Env Amount: Row 2
    const int filtRow2Y = contentY + 115;
    filterResonanceKnob_.slider.setBounds(395, filtRow2Y, knobW, knobH - 15);
    filterResonanceKnob_.label.setBounds(395, filtRow2Y + knobH - 18, knobW, 14);

    filterEnvAmountKnob_.slider.setBounds(500, filtRow2Y, knobW, knobH - 15);
    filterEnvAmountKnob_.label.setBounds(500, filtRow2Y + knobH - 18, knobW, 14);

    // KB Track & Slope: Row 3
    const int filtRow3Y = filtRow2Y + 80;
    filterKbTrackKnob_.slider.setBounds(395, filtRow3Y, knobW, knobH - 15);
    filterKbTrackKnob_.label.setBounds(395, filtRow3Y + knobH - 18, knobW, 14);

    filterSlopeLabel_.setBounds(480, filtRow3Y + 5, 100, 14);
    filterSlopeBox_.setBounds(480, filtRow3Y + 20, 100, 24);

    // --- Section 4: Envelopes (x: 607, w: 225) ---
    // Filter Env (Top row)
    const int env1X = 620;
    const int envSpacing = 52;
    filterAttackKnob_.slider.setBounds(env1X, contentY, 46, 50);
    filterAttackKnob_.label.setBounds(env1X, contentY + 48, 46, 12);

    filterDecayKnob_.slider.setBounds(env1X + envSpacing, contentY, 46, 50);
    filterDecayKnob_.label.setBounds(env1X + envSpacing, contentY + 48, 46, 12);

    filterSustainKnob_.slider.setBounds(env1X + envSpacing * 2, contentY, 46, 50);
    filterSustainKnob_.label.setBounds(env1X + envSpacing * 2, contentY + 48, 46, 12);

    filterReleaseKnob_.slider.setBounds(env1X + envSpacing * 3, contentY, 46, 50);
    filterReleaseKnob_.label.setBounds(env1X + envSpacing * 3, contentY + 48, 46, 12);

    // Amp Env (Bottom row)
    const int ampY = contentY + 120;
    ampAttackKnob_.slider.setBounds(env1X, ampY, 46, 50);
    ampAttackKnob_.label.setBounds(env1X, ampY + 48, 46, 12);

    ampDecayKnob_.slider.setBounds(env1X + envSpacing, ampY, 46, 50);
    ampDecayKnob_.label.setBounds(env1X + envSpacing, ampY + 48, 46, 12);

    ampSustainKnob_.slider.setBounds(env1X + envSpacing * 2, ampY, 46, 50);
    ampSustainKnob_.label.setBounds(env1X + envSpacing * 2, ampY + 48, 46, 12);

    ampReleaseKnob_.slider.setBounds(env1X + envSpacing * 3, ampY, 46, 50);
    ampReleaseKnob_.label.setBounds(env1X + envSpacing * 3, ampY + 48, 46, 12);

    // --- Section 5: Mod & Glide (x: 842, w: remaining) ---
    const int modX = 865;
    lfoRateKnob_.slider.setBounds(modX, contentY, knobW, knobH - 15);
    lfoRateKnob_.label.setBounds(modX, contentY + knobH - 18, knobW, 14);

    lfoAmountKnob_.slider.setBounds(modX + 65, contentY, knobW, knobH - 15);
    lfoAmountKnob_.label.setBounds(modX + 65, contentY + knobH - 18, knobW, 14);

    lfoShapeLabel_.setBounds(modX + 5, contentY + 75, 110, 14);
    lfoShapeBox_.setBounds(modX + 5, contentY + 92, 110, 24);

    const int glideY = contentY + 145;
    glideTimeKnob_.slider.setBounds(modX + 5, glideY, knobW, knobH - 15);
    glideTimeKnob_.label.setBounds(modX + 5, glideY + knobH - 18, knobW, 14);

    legatoButton_.setBounds(modX + 75, glideY + 12, 80, 26);

    // --- Bottom Virtual Keyboard ---
    keyboardComponent_.setBounds(0, getHeight() - 80, getWidth(), 80);
}
