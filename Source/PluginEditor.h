#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "UI/LookAndFeel.h"
#include "UI/Meter.h"

class G3XCompressorAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit G3XCompressorAudioProcessorEditor(G3XCompressorAudioProcessor&);
    ~G3XCompressorAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void timerCallback() override;
    void configureKnob(juce::Slider&, juce::Label&, const juce::String& title, const juce::String& suffix);
    void configureChoice(juce::ComboBox&, juce::Label&, const juce::String& title,
                         std::initializer_list<const char*> items);

    G3XCompressorAudioProcessor& processor;
    g3x::ui::LookAndFeel lookAndFeel;

    juce::Slider threshold, ratio, output, attack, release, mix, trim;
    juce::Label thresholdLabel, ratioLabel, outputLabel, attackLabel, releaseLabel, mixLabel, trimLabel;
    juce::ComboBox releaseMode, behavior, character;
    juce::Label releaseModeLabel, behaviorLabel, characterLabel;
    g3x::ui::Meter inputMeter { g3x::ui::Meter::Kind::level };
    g3x::ui::Meter gainMeter { g3x::ui::Meter::Kind::gainChange };
    g3x::ui::Meter outputMeter { g3x::ui::Meter::Kind::level };
    juce::Label inputReadout, gainReadout, outputReadout, limiterReadout;

    std::unique_ptr<SliderAttachment> thresholdAttachment, ratioAttachment, outputAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment, releaseAttachment, mixAttachment, trimAttachment;
    std::unique_ptr<ComboAttachment> releaseModeAttachment, behaviorAttachment, characterAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(G3XCompressorAudioProcessorEditor)
};
