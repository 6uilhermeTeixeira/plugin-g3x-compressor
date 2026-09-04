#include "LookAndFeel.h"

namespace g3x::ui
{
LookAndFeel::LookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(Colours::ink));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(Colours::background).withAlpha(0.7f));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::textColourId, juce::Colour(Colours::ink));
}

juce::Font LookAndFeel::getLabelFont(juce::Label& label)
{
    return juce::Font(juce::FontOptions { label.getHeight() < 22 ? 12.0f : 14.0f }.withStyle("Bold"));
}

void LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                   float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                   juce::Slider&)
{
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                         static_cast<float>(width), static_cast<float>(height)).reduced(9.0f);
    const auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
    bounds = bounds.withSizeKeepingCentre(diameter, diameter);
    const auto centre = bounds.getCentre();
    const auto radius = diameter * 0.5f;
    const auto angle = juce::jmap(sliderPos, rotaryStartAngle, rotaryEndAngle);

    g.setColour(juce::Colour(Colours::background));
    g.fillEllipse(bounds);
    g.setColour(juce::Colour(Colours::panelLight));
    g.drawEllipse(bounds, 2.0f);

    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(Colours::muted).withAlpha(0.25f));
    g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc(centre.x, centre.y, radius - 4.0f, radius - 4.0f, 0.0f,
                        rotaryStartAngle, angle, true);
    g.setColour(juce::Colour(Colours::teal));
    g.strokePath(value, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    juce::Path pointer;
    pointer.addRoundedRectangle(-2.0f, -radius + 13.0f, 4.0f, radius * 0.34f, 2.0f);
    g.setColour(juce::Colour(Colours::amber));
    g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

void LookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                   bool highlighted, bool down)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const auto active = button.getToggleState();
    g.setColour(active ? juce::Colour(Colours::teal).withAlpha(down ? 0.72f : 1.0f)
                       : juce::Colour(Colours::panelLight).brighter(highlighted ? 0.1f : 0.0f));
    g.fillRoundedRectangle(bounds, 7.0f);
    g.setColour(active ? juce::Colour(Colours::background) : juce::Colour(Colours::ink));
    g.setFont(juce::FontOptions { 12.0f }.withStyle("Bold"));
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}
}
