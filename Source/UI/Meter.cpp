#include "Meter.h"
#include "LookAndFeel.h"

namespace g3x::ui
{
Meter::Meter(Kind meterKind) : kind(meterKind)
{
    setTitle(kind == Kind::level ? "Level meter" : "Gain change meter");
    setDescription("Click to reset peak hold");
}

void Meter::setValues(float first, float second)
{
    valueA = first;
    valueB = second;
    holdA = juce::jmax(holdA, first);
    holdB = juce::jmax(holdB, second);
    repaint();
}

void Meter::mouseDown(const juce::MouseEvent&)
{
    holdA = valueA;
    holdB = valueB;
    repaint();
}

void Meter::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced(2.0f);
    g.setColour(juce::Colour(Colours::background));
    g.fillRoundedRectangle(area, 6.0f);

    if (kind == Kind::level)
    {
        auto channel = area.reduced(7.0f, 8.0f);
        const auto gap = 4.0f;
        const auto width = (channel.getWidth() - gap) * 0.5f;
        for (int index = 0; index < 2; ++index)
        {
            auto lane = channel.withX(channel.getX() + index * (width + gap)).withWidth(width);
            const auto db = index == 0 ? valueA : valueB;
            const auto norm = juce::jlimit(0.0f, 1.0f, juce::jmap(db, -60.0f, 0.0f, 0.0f, 1.0f));
            auto lit = lane.withTop(lane.getBottom() - lane.getHeight() * norm);
            juce::ColourGradient gradient(juce::Colour(Colours::red), lane.getX(), lane.getY(),
                                          juce::Colour(Colours::teal), lane.getX(), lane.getBottom(), false);
            gradient.addColour(0.16, juce::Colour(Colours::amber));
            g.setGradientFill(gradient);
            g.fillRoundedRectangle(lit, 2.0f);
        }
    }
    else
    {
        const auto centreY = area.getCentreY();
        g.setColour(juce::Colour(Colours::muted).withAlpha(0.35f));
        g.drawHorizontalLine(static_cast<int>(centreY), area.getX() + 7.0f, area.getRight() - 7.0f);
        const auto db = juce::jlimit(-24.0f, 12.0f, valueA);
        const auto endY = juce::jmap(db, -24.0f, 12.0f, area.getBottom() - 7.0f, area.getY() + 7.0f);
        g.setColour(db <= 0.0f ? juce::Colour(Colours::amber) : juce::Colour(Colours::teal));
        g.fillRoundedRectangle(area.getX() + 9.0f, juce::jmin(centreY, endY), area.getWidth() - 18.0f,
                               juce::jmax(2.0f, std::abs(endY - centreY)), 2.0f);
    }
}
}
