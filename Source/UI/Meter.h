#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace g3x::ui
{
class Meter final : public juce::Component
{
public:
    enum class Kind { level, gainChange };
    explicit Meter(Kind meterKind);
    void setValues(float first, float second = -100.0f);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    Kind kind;
    float valueA = -100.0f;
    float valueB = -100.0f;
    float holdA = -100.0f;
    float holdB = -100.0f;
};
}
