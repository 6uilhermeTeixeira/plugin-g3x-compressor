#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Dsp/CompressorEngine.h"

class G3XRCompAudioProcessor final : public juce::AudioProcessor
{
public:
    using AudioProcessor::processBlock;
    G3XRCompAudioProcessor();
    ~G3XRCompAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    const g3x::MeterSnapshot& getMeters() const noexcept { return engine.getMeters(); }

    juce::AudioProcessorValueTreeState state;

private:
    g3x::CompressorEngine engine;
    std::atomic<float>* threshold = nullptr;
    std::atomic<float>* ratio = nullptr;
    std::atomic<float>* attack = nullptr;
    std::atomic<float>* release = nullptr;
    std::atomic<float>* releaseMode = nullptr;
    std::atomic<float>* behavior = nullptr;
    std::atomic<float>* character = nullptr;
    std::atomic<float>* outputGain = nullptr;
    std::atomic<float>* mix = nullptr;
    std::atomic<float>* trim = nullptr;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(G3XRCompAudioProcessor)
};
