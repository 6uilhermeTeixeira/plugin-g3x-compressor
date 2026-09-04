#include "Dsp/CompressorEngine.h"
#include <iostream>
#include <cmath>

namespace
{
int failures = 0;

void expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void expectNear(float actual, float expected, float tolerance, const char* message)
{
    expect(std::abs(actual - expected) <= tolerance, message);
}
}

int main()
{
    using g3x::CompressorEngine;

    expectNear(CompressorEngine::staticGainChangeDb(-40.0f, -20.0f, 4.0f), 0.0f, 0.0001f,
               "signal below knee must be unchanged");
    expectNear(CompressorEngine::staticGainChangeDb(0.0f, -20.0f, 4.0f), -15.0f, 0.0001f,
               "4:1 curve above knee");
    expectNear(CompressorEngine::staticGainChangeDb(0.0f, -20.0f, 0.5f), 20.0f, 0.0001f,
               "0.5:1 upward expansion curve");
    expectNear(CompressorEngine::staticGainChangeDb(-10.0f, -20.0f, 1.0f), 0.0f, 0.0001f,
               "unity ratio");

    const auto below = CompressorEngine::staticGainChangeDb(-26.001f, -20.0f, 4.0f);
    const auto boundary = CompressorEngine::staticGainChangeDb(-26.0f, -20.0f, 4.0f);
    expectNear(below, boundary, 0.001f, "lower knee continuity");
    const auto above = CompressorEngine::staticGainChangeDb(-13.999f, -20.0f, 4.0f);
    const auto upper = CompressorEngine::staticGainChangeDb(-14.0f, -20.0f, 4.0f);
    expectNear(above, upper, 0.001f, "upper knee continuity");

    CompressorEngine engine;
    engine.prepare(48000.0, 512, 2);
    g3x::Parameters neutral;
    engine.setParameters(neutral);
    juce::AudioBuffer<float> audio(2, 512);
    for (int channel = 0; channel < 2; ++channel)
        for (int sample = 0; sample < audio.getNumSamples(); ++sample)
            audio.setSample(channel, sample, 0.25f * std::sin(0.05f * static_cast<float>(sample)));
    auto original = audio;
    engine.process(audio);
    for (int channel = 0; channel < 2; ++channel)
        for (int sample = 0; sample < audio.getNumSamples(); ++sample)
            expectNear(audio.getSample(channel, sample), original.getSample(channel, sample), 1.0e-6f,
                       "neutral path identity");

    audio.clear();
    audio.setSample(0, 0, std::numeric_limits<float>::quiet_NaN());
    audio.setSample(1, 0, std::numeric_limits<float>::infinity());
    engine.process(audio);
    for (int channel = 0; channel < 2; ++channel)
        for (int sample = 0; sample < audio.getNumSamples(); ++sample)
            expect(std::isfinite(audio.getSample(channel, sample)), "output must remain finite");

    if (failures == 0)
        std::cout << "All G3X Compressor DSP tests passed\n";
    return failures == 0 ? 0 : 1;
}
