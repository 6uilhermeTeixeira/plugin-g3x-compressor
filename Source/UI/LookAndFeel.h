#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace g3x::ui
{
struct Colours
{
    static constexpr juce::uint32 background = 0xff10161b;
    static constexpr juce::uint32 panel = 0xff182128;
    static constexpr juce::uint32 panelLight = 0xff202c34;
    static constexpr juce::uint32 ink = 0xffe8ede9;
    static constexpr juce::uint32 muted = 0xff8d9ca3;
    static constexpr juce::uint32 teal = 0xff4dd5bd;
    static constexpr juce::uint32 amber = 0xffffb84a;
    static constexpr juce::uint32 red = 0xffff5e57;
};

class LookAndFeel final : public juce::LookAndFeel_V4
{
public:
    LookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    juce::Font getLabelFont(juce::Label&) override;
};
}
