#include "CompressorEngine.h"
#include <cmath>

namespace g3x
{
namespace
{
constexpr float floorDb = -100.0f;

float coefficientForMs(float milliseconds, double sampleRate) noexcept
{
    return std::exp(-1.0f / (0.001f * juce::jmax(0.01f, milliseconds) * static_cast<float>(sampleRate)));
}

float finiteOrZero(float value) noexcept
{
    return std::isfinite(value) ? value : 0.0f;
}
}

void CompressorEngine::prepare(double newSampleRate, int, int)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    outputGain.reset(sampleRate, 0.02);
    mix.reset(sampleRate, 0.02);
    trimGain.reset(sampleRate, 0.02);
    reset();
    setParameters(parameters);
}

void CompressorEngine::reset() noexcept
{
    envelopeDb = 0.0f;
    slowReleaseState = 0.0f;
    rmsState = 0.0f;
    outputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(parameters.outputGainDb));
    mix.setCurrentAndTargetValue(parameters.mix);
    trimGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(parameters.trimDb));
}

void CompressorEngine::setParameters(const Parameters& newParameters) noexcept
{
    parameters = newParameters;
    parameters.thresholdDb = juce::jlimit(-60.0f, 0.0f, parameters.thresholdDb);
    parameters.ratio = juce::jlimit(0.5f, 50.0f, parameters.ratio);
    parameters.attackMs = juce::jlimit(0.5f, 500.0f, parameters.attackMs);
    parameters.releaseMs = juce::jlimit(5.0f, 5000.0f, parameters.releaseMs);
    parameters.mix = juce::jlimit(0.0f, 1.0f, parameters.mix);
    updateCoefficients();
    outputGain.setTargetValue(juce::Decibels::decibelsToGain(parameters.outputGainDb));
    mix.setTargetValue(parameters.mix);
    trimGain.setTargetValue(juce::Decibels::decibelsToGain(parameters.trimDb));
}

void CompressorEngine::updateCoefficients() noexcept
{
    attackCoefficient = coefficientForMs(parameters.attackMs, sampleRate);
    releaseCoefficient = coefficientForMs(parameters.releaseMs, sampleRate);
    fastReleaseCoefficient = coefficientForMs(juce::jmax(5.0f, parameters.releaseMs * 0.28f), sampleRate);
    slowReleaseCoefficient = coefficientForMs(juce::jmin(5000.0f, parameters.releaseMs * 2.4f), sampleRate);
}

float CompressorEngine::staticGainChangeDb(float inputDb, float thresholdDb,
                                            float ratio, float kneeWidthDb) noexcept
{
    if (!std::isfinite(inputDb) || !std::isfinite(thresholdDb) || !std::isfinite(ratio))
        return 0.0f;

    ratio = juce::jlimit(0.5f, 50.0f, ratio);
    const auto slope = 1.0f / ratio - 1.0f;
    const auto distance = inputDb - thresholdDb;
    const auto halfKnee = juce::jmax(0.0f, kneeWidthDb) * 0.5f;

    if (halfKnee <= 0.0f || distance >= halfKnee)
        return distance > -halfKnee ? slope * distance : 0.0f;
    if (distance <= -halfKnee)
        return 0.0f;

    const auto x = distance + halfKnee;
    return slope * x * x / (4.0f * halfKnee);
}

float CompressorEngine::detectorDb(const juce::AudioBuffer<float>& buffer, int sample) noexcept
{
    float peak = 0.0f;
    float energy = 0.0f;
    const auto channels = buffer.getNumChannels();
    for (int channel = 0; channel < channels; ++channel)
    {
        const auto value = std::abs(finiteOrZero(buffer.getSample(channel, sample)));
        peak = juce::jmax(peak, value);
        energy += value * value;
    }

    energy /= static_cast<float>(juce::jmax(1, channels));
    const auto rmsCoefficient = coefficientForMs(25.0f, sampleRate);
    rmsState = rmsCoefficient * rmsState + (1.0f - rmsCoefficient) * energy;
    const auto rms = std::sqrt(juce::jmax(0.0f, rmsState));
    const auto hybrid = 0.65f * peak + 0.35f * rms;
    return juce::Decibels::gainToDecibels(hybrid, floorDb);
}

float CompressorEngine::processEnvelope(float targetGainDb) noexcept
{
    const bool movingAwayFromUnity = std::abs(targetGainDb) > std::abs(envelopeDb);
    if (movingAwayFromUnity)
    {
        envelopeDb = attackCoefficient * envelopeDb + (1.0f - attackCoefficient) * targetGainDb;
        slowReleaseState = envelopeDb;
        return envelopeDb;
    }

    float coefficient = releaseCoefficient;
    const auto depth = juce::jlimit(0.0f, 1.0f, std::abs(envelopeDb) / 12.0f);
    if (parameters.autoRelease)
    {
        slowReleaseState = slowReleaseCoefficient * slowReleaseState
                         + (1.0f - slowReleaseCoefficient) * targetGainDb;
        const auto fast = fastReleaseCoefficient * envelopeDb
                        + (1.0f - fastReleaseCoefficient) * targetGainDb;
        const auto slowWeight = parameters.vintage ? (1.0f - depth) : depth;
        envelopeDb = juce::jmap(slowWeight, fast, slowReleaseState);
        return envelopeDb;
    }

    if (std::abs(envelopeDb) < 3.0f)
        coefficient = parameters.vintage ? std::sqrt(coefficient) : coefficient * coefficient;
    else
        coefficient = parameters.vintage ? coefficient * coefficient : std::sqrt(coefficient);

    envelopeDb = coefficient * envelopeDb + (1.0f - coefficient) * targetGainDb;
    return envelopeDb;
}

float CompressorEngine::saturateWarm(float sample, float amount) const noexcept
{
    const auto drive = 1.0f + 1.8f * juce::jlimit(0.0f, 1.0f, amount);
    const auto biased = std::tanh(sample * drive + 0.08f * amount) - std::tanh(0.08f * amount);
    return biased / std::tanh(drive);
}

float CompressorEngine::limit(float sample, float& reductionDb) noexcept
{
    const auto magnitude = std::abs(sample);
    if (magnitude <= 1.0f)
        return sample;

    reductionDb = juce::jmax(reductionDb, juce::Decibels::gainToDecibels(magnitude));
    return juce::jlimit(-1.0f, 1.0f, sample);
}

void CompressorEngine::process(juce::AudioBuffer<float>& buffer) noexcept
{
    juce::ScopedNoDenormals noDenormals;
    const auto channels = juce::jmin(2, buffer.getNumChannels());
    std::array<float, 2> inputPeak {};
    std::array<float, 2> outputPeak {};
    float maxLimiterReduction = 0.0f;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto detectedDb = detectorDb(buffer, sample);
        const auto targetDb = staticGainChangeDb(detectedDb, parameters.thresholdDb, parameters.ratio);
        const auto gainChangeDb = processEnvelope(targetDb);
        const auto dynamicsGain = juce::Decibels::decibelsToGain(gainChangeDb);
        const auto wetMix = mix.getNextValue();
        const auto makeup = outputGain.getNextValue();
        const auto postTrim = trimGain.getNextValue();
        const auto warmth = parameters.warm ? juce::jlimit(0.0f, 1.0f, std::abs(gainChangeDb) / 12.0f) : 0.0f;

        for (int channel = 0; channel < channels; ++channel)
        {
            auto dry = finiteOrZero(buffer.getSample(channel, sample));
            inputPeak[static_cast<size_t>(channel)] = juce::jmax(inputPeak[static_cast<size_t>(channel)], std::abs(dry));
            auto wet = dry * dynamicsGain;
            if (parameters.warm)
                wet = saturateWarm(wet, warmth);
            auto value = ((1.0f - wetMix) * dry + wetMix * wet) * makeup;
            value = limit(value, maxLimiterReduction) * postTrim;
            value = finiteOrZero(value);
            buffer.setSample(channel, sample, value);
            outputPeak[static_cast<size_t>(channel)] = juce::jmax(outputPeak[static_cast<size_t>(channel)], std::abs(value));
        }
    }

    for (int channel = 0; channel < 2; ++channel)
    {
        meters.inputDb[static_cast<size_t>(channel)].store(
            juce::Decibels::gainToDecibels(inputPeak[static_cast<size_t>(channel)], floorDb), std::memory_order_relaxed);
        meters.outputDb[static_cast<size_t>(channel)].store(
            juce::Decibels::gainToDecibels(outputPeak[static_cast<size_t>(channel)], floorDb), std::memory_order_relaxed);
    }
    meters.gainChangeDb.store(envelopeDb, std::memory_order_relaxed);
    meters.limiterReductionDb.store(maxLimiterReduction, std::memory_order_relaxed);
}
}
