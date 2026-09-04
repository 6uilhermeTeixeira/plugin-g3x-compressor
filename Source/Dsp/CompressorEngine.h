#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

namespace g3x
{
struct MeterSnapshot
{
    std::array<std::atomic<float>, 2> inputDb {{ -100.0f, -100.0f }};
    std::array<std::atomic<float>, 2> outputDb {{ -100.0f, -100.0f }};
    std::atomic<float> gainChangeDb { 0.0f };
    std::atomic<float> limiterReductionDb { 0.0f };
};

struct Parameters
{
    float thresholdDb = 0.0f;
    float ratio = 1.0f;
    float attackMs = 16.0f;
    float releaseMs = 160.0f;
    bool autoRelease = false;
    bool vintage = false;
    bool warm = false;
    float outputGainDb = 0.0f;
    float mix = 1.0f;
    float trimDb = 0.0f;
};

class CompressorEngine
{
public:
    void prepare(double newSampleRate, int maximumBlockSize, int channels);
    void reset() noexcept;
    void setParameters(const Parameters& newParameters) noexcept;
    void process(juce::AudioBuffer<float>& buffer) noexcept;

    [[nodiscard]] static float staticGainChangeDb(float inputDb, float thresholdDb,
                                                   float ratio, float kneeWidthDb = 12.0f) noexcept;
    [[nodiscard]] const MeterSnapshot& getMeters() const noexcept { return meters; }

private:
    [[nodiscard]] float detectorDb(const juce::AudioBuffer<float>& buffer, int sample) noexcept;
    [[nodiscard]] float processEnvelope(float targetGainDb) noexcept;
    [[nodiscard]] float saturateWarm(float sample, float amount) const noexcept;
    [[nodiscard]] float limit(float sample, float& reductionDb) noexcept;
    void updateCoefficients() noexcept;

    Parameters parameters;
    MeterSnapshot meters;
    double sampleRate = 44100.0;
    float envelopeDb = 0.0f;
    float rmsState = 0.0f;
    float slowReleaseState = 0.0f;
    float attackCoefficient = 0.0f;
    float releaseCoefficient = 0.0f;
    float fastReleaseCoefficient = 0.0f;
    float slowReleaseCoefficient = 0.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mix;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> trimGain;
};
}
