#include "PluginEditor.h"

namespace
{
constexpr auto teal = g3x::ui::Colours::teal;
}

G3XRCompAudioProcessorEditor::G3XRCompAudioProcessorEditor(G3XRCompAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&lookAndFeel);
    setResizable(true, true);
    setResizeLimits(720, 460, 1440, 920);
    setSize(920, 580);

    configureKnob(threshold, thresholdLabel, "THRESHOLD", " dB");
    configureKnob(ratio, ratioLabel, "RATIO", ":1");
    configureKnob(output, outputLabel, "OUTPUT", " dB");
    configureKnob(attack, attackLabel, "ATTACK", " ms");
    configureKnob(release, releaseLabel, "RELEASE", " ms");
    configureKnob(mix, mixLabel, "MIX", " %");
    configureKnob(trim, trimLabel, "TRIM", " dB");

    configureChoice(releaseMode, releaseModeLabel, "RELEASE MODE", { "Manual", "Auto" });
    configureChoice(behavior, behaviorLabel, "BEHAVIOR", { "Modern", "Vintage" });
    configureChoice(character, characterLabel, "CHARACTER", { "Clean", "Warm" });

    const std::array<juce::Component*, 7> displayComponents {
        &inputMeter, &gainMeter, &outputMeter, &inputReadout, &gainReadout, &outputReadout, &limiterReadout
    };
    for (auto* component : displayComponents)
        addAndMakeVisible(component);

    for (auto* label : { &inputReadout, &gainReadout, &outputReadout, &limiterReadout })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colour(g3x::ui::Colours::muted));
        label->setFont(juce::FontOptions { 11.0f }.withStyle("Bold"));
    }

    thresholdAttachment = std::make_unique<SliderAttachment>(p.state, "thresholdDb", threshold);
    ratioAttachment = std::make_unique<SliderAttachment>(p.state, "ratio", ratio);
    outputAttachment = std::make_unique<SliderAttachment>(p.state, "outputGainDb", output);
    attackAttachment = std::make_unique<SliderAttachment>(p.state, "attackMs", attack);
    releaseAttachment = std::make_unique<SliderAttachment>(p.state, "releaseMs", release);
    mixAttachment = std::make_unique<SliderAttachment>(p.state, "mixPercent", mix);
    trimAttachment = std::make_unique<SliderAttachment>(p.state, "trimDb", trim);
    releaseModeAttachment = std::make_unique<ComboAttachment>(p.state, "releaseMode", releaseMode);
    behaviorAttachment = std::make_unique<ComboAttachment>(p.state, "behavior", behavior);
    characterAttachment = std::make_unique<ComboAttachment>(p.state, "character", character);

    startTimerHz(45);
}

G3XRCompAudioProcessorEditor::~G3XRCompAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void G3XRCompAudioProcessorEditor::configureKnob(juce::Slider& slider, juce::Label& label,
                                                 const juce::String& title, const juce::String& suffix)
{
    addAndMakeVisible(slider);
    addAndMakeVisible(label);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 82, 24);
    slider.setTextValueSuffix(suffix);
    slider.setDoubleClickReturnValue(true, title == "RATIO" ? 1.0 : title == "MIX" ? 100.0 : 0.0);
    slider.setTitle(title);
    slider.setDescription(title + " control");
    label.setText(title, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
}

void G3XRCompAudioProcessorEditor::configureChoice(juce::ComboBox& box, juce::Label& label,
                                                   const juce::String& title,
                                                   std::initializer_list<const char*> items)
{
    addAndMakeVisible(box);
    addAndMakeVisible(label);
    int id = 1;
    for (auto* item : items)
        box.addItem(item, id++);
    box.setTitle(title);
    box.setDescription(title + " selector");
    box.setColour(juce::ComboBox::backgroundColourId, juce::Colour(g3x::ui::Colours::background));
    box.setColour(juce::ComboBox::outlineColourId, juce::Colour(teal).withAlpha(0.45f));
    box.setColour(juce::ComboBox::textColourId, juce::Colour(g3x::ui::Colours::ink));
    label.setText(title, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
}

void G3XRCompAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(g3x::ui::Colours::background));
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient glow(juce::Colour(teal).withAlpha(0.13f), bounds.getCentreX(), 0.0f,
                              juce::Colours::transparentBlack, bounds.getCentreX(), bounds.getHeight() * 0.55f, false);
    g.setGradientFill(glow);
    g.fillRect(bounds);

    g.setColour(juce::Colour(g3x::ui::Colours::ink));
    g.setFont(juce::FontOptions { 24.0f }.withStyle("Bold"));
    g.drawText("G3X", 26, 16, 80, 32, juce::Justification::centredLeft);
    g.setColour(juce::Colour(teal));
    g.drawText("RCOMP", 94, 16, 120, 32, juce::Justification::centredLeft);
    g.setColour(juce::Colour(g3x::ui::Colours::muted));
    g.setFont(juce::FontOptions { 11.0f }.withStyle("Bold"));
    g.drawText("MUSICAL DYNAMICS", getWidth() - 190, 20, 164, 24, juce::Justification::centredRight);

    auto mainPanel = juce::Rectangle<float>(20.0f, 62.0f, bounds.getWidth() - 40.0f, bounds.getHeight() - 186.0f);
    g.setColour(juce::Colour(g3x::ui::Colours::panel));
    g.fillRoundedRectangle(mainPanel, 15.0f);
    g.setColour(juce::Colour(teal).withAlpha(0.18f));
    g.drawRoundedRectangle(mainPanel, 15.0f, 1.0f);

    auto lowerPanel = juce::Rectangle<float>(20.0f, bounds.getHeight() - 108.0f,
                                             bounds.getWidth() - 40.0f, 88.0f);
    g.setColour(juce::Colour(g3x::ui::Colours::panelLight));
    g.fillRoundedRectangle(lowerPanel, 13.0f);
    g.setColour(juce::Colour(g3x::ui::Colours::muted).withAlpha(0.2f));
    g.drawRoundedRectangle(lowerPanel, 13.0f, 1.0f);
}

void G3XRCompAudioProcessorEditor::resized()
{
    const auto scale = static_cast<float>(getWidth()) / 920.0f;
    auto content = getLocalBounds().reduced(28);
    content.removeFromTop(44);
    auto lower = content.removeFromBottom(94).reduced(12, 4);
    content.removeFromBottom(18);

    auto top = content.removeFromTop(static_cast<int>(content.getHeight() * 0.62f));
    const auto columnWidth = top.getWidth() / 3;
    for (int index = 0; index < 3; ++index)
    {
        auto column = top.removeFromLeft(index == 2 ? top.getWidth() : columnWidth);
        auto labelArea = column.removeFromTop(24);
        auto meterColumn = column.removeFromLeft(64);
        auto readoutArea = meterColumn.removeFromBottom(22);
        auto knobArea = column.reduced(8, 0);
        auto& label = index == 0 ? thresholdLabel : index == 1 ? ratioLabel : outputLabel;
        auto& knob = index == 0 ? threshold : index == 1 ? ratio : output;
        auto& meter = index == 0 ? inputMeter : index == 1 ? gainMeter : outputMeter;
        auto& readout = index == 0 ? inputReadout : index == 1 ? gainReadout : outputReadout;
        label.setBounds(labelArea);
        meter.setBounds(meterColumn.reduced(7, 4));
        readout.setBounds(readoutArea);
        knob.setBounds(knobArea);
    }

    auto controls = content.reduced(10, 2);
    const auto smallWidth = controls.getWidth() / 5;
    auto placeKnob = [&controls, smallWidth](juce::Slider& knob, juce::Label& label)
    {
        auto cell = controls.removeFromLeft(smallWidth).reduced(5, 0);
        label.setBounds(cell.removeFromTop(20));
        knob.setBounds(cell);
    };
    placeKnob(attack, attackLabel);
    placeKnob(release, releaseLabel);

    auto placeChoice = [&controls, smallWidth](juce::ComboBox& box, juce::Label& label)
    {
        auto cell = controls.removeFromLeft(smallWidth).reduced(8, 8);
        label.setBounds(cell.removeFromTop(20));
        box.setBounds(cell.removeFromTop(32));
    };
    placeChoice(releaseMode, releaseModeLabel);
    placeChoice(behavior, behaviorLabel);
    placeChoice(character, characterLabel);

    auto left = lower.removeFromLeft(lower.getWidth() / 2);
    auto layoutLower = [](juce::Rectangle<int> area, juce::Slider& knob, juce::Label& label)
    {
        label.setBounds(area.removeFromLeft(70));
        knob.setBounds(area);
    };
    layoutLower(left.reduced(4), mix, mixLabel);
    layoutLower(lower.reduced(4), trim, trimLabel);
    limiterReadout.setBounds(getWidth() / 2 - 70, getHeight() - 21, 140, 16);

    juce::ignoreUnused(scale);
}

void G3XRCompAudioProcessorEditor::timerCallback()
{
    const auto& meters = processor.getMeters();
    const auto inL = meters.inputDb[0].load(std::memory_order_relaxed);
    const auto inR = meters.inputDb[1].load(std::memory_order_relaxed);
    const auto outL = meters.outputDb[0].load(std::memory_order_relaxed);
    const auto outR = meters.outputDb[1].load(std::memory_order_relaxed);
    const auto gain = meters.gainChangeDb.load(std::memory_order_relaxed);
    const auto limiter = meters.limiterReductionDb.load(std::memory_order_relaxed);
    inputMeter.setValues(inL, inR);
    gainMeter.setValues(gain);
    outputMeter.setValues(outL, outR);
    inputReadout.setText(juce::String(juce::jmax(inL, inR), 1) + " dB", juce::dontSendNotification);
    gainReadout.setText(juce::String(gain, 1) + " dB", juce::dontSendNotification);
    outputReadout.setText(juce::String(juce::jmax(outL, outR), 1) + " dB", juce::dontSendNotification);
    limiterReadout.setText(limiter > 0.01f ? "LIMITER  −" + juce::String(limiter, 1) + " dB" : "LIMITER READY",
                           juce::dontSendNotification);
    limiterReadout.setColour(juce::Label::textColourId,
        limiter > 6.0f ? juce::Colour(g3x::ui::Colours::red)
                       : limiter > 0.01f ? juce::Colour(g3x::ui::Colours::amber)
                                         : juce::Colour(g3x::ui::Colours::muted));
}
