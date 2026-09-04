#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs
{
constexpr auto threshold = "thresholdDb";
constexpr auto ratio = "ratio";
constexpr auto attack = "attackMs";
constexpr auto release = "releaseMs";
constexpr auto releaseMode = "releaseMode";
constexpr auto behavior = "behavior";
constexpr auto character = "character";
constexpr auto output = "outputGainDb";
constexpr auto mix = "mixPercent";
constexpr auto trim = "trimDb";
}

G3XCompressorAudioProcessor::G3XCompressorAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "G3XRCompState", createParameterLayout())
{
    threshold = state.getRawParameterValue(IDs::threshold);
    ratio = state.getRawParameterValue(IDs::ratio);
    attack = state.getRawParameterValue(IDs::attack);
    release = state.getRawParameterValue(IDs::release);
    releaseMode = state.getRawParameterValue(IDs::releaseMode);
    behavior = state.getRawParameterValue(IDs::behavior);
    character = state.getRawParameterValue(IDs::character);
    outputGain = state.getRawParameterValue(IDs::output);
    mix = state.getRawParameterValue(IDs::mix);
    trim = state.getRawParameterValue(IDs::trim);
}

juce::AudioProcessorValueTreeState::ParameterLayout G3XCompressorAudioProcessor::createParameterLayout()
{
    using Float = juce::AudioParameterFloat;
    using Choice = juce::AudioParameterChoice;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<Float>(juce::ParameterID { IDs::threshold, 1 }, "Threshold",
        juce::NormalisableRange<float> { -60.0f, 0.0f, 0.01f }, 0.0f, "dB"));

    auto ratioRange = juce::NormalisableRange<float> { 0.5f, 50.0f, 0.001f };
    ratioRange.setSkewForCentre(1.0f);
    layout.add(std::make_unique<Float>(juce::ParameterID { IDs::ratio, 1 }, "Ratio", ratioRange, 1.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction([] (float value, int)
        { return juce::String(value, value < 10.0f ? 2 : 1) + ":1"; })));

    auto attackRange = juce::NormalisableRange<float> { 0.5f, 500.0f, 0.01f };
    attackRange.setSkewForCentre(16.0f);
    layout.add(std::make_unique<Float>(juce::ParameterID { IDs::attack, 1 }, "Attack", attackRange, 16.0f, "ms"));

    auto releaseRange = juce::NormalisableRange<float> { 5.0f, 5000.0f, 0.01f };
    releaseRange.setSkewForCentre(160.0f);
    layout.add(std::make_unique<Float>(juce::ParameterID { IDs::release, 1 }, "Release", releaseRange, 160.0f, "ms"));
    layout.add(std::make_unique<Choice>(juce::ParameterID { IDs::releaseMode, 1 }, "Release Mode",
                                        juce::StringArray { "Manual", "Auto" }, 0));
    layout.add(std::make_unique<Choice>(juce::ParameterID { IDs::behavior, 1 }, "Behavior",
                                        juce::StringArray { "Modern", "Vintage" }, 0));
    layout.add(std::make_unique<Choice>(juce::ParameterID { IDs::character, 1 }, "Character",
                                        juce::StringArray { "Clean", "Warm" }, 0));
    layout.add(std::make_unique<Float>(juce::ParameterID { IDs::output, 1 }, "Output",
        juce::NormalisableRange<float> { -30.0f, 30.0f, 0.01f }, 0.0f, "dB"));
    layout.add(std::make_unique<Float>(juce::ParameterID { IDs::mix, 1 }, "Mix",
        juce::NormalisableRange<float> { 0.0f, 100.0f, 0.01f }, 100.0f, "%"));
    layout.add(std::make_unique<Float>(juce::ParameterID { IDs::trim, 1 }, "Trim",
        juce::NormalisableRange<float> { -18.0f, 18.0f, 0.01f }, 0.0f, "dB"));
    return layout;
}

void G3XCompressorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

bool G3XCompressorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    return input == output && (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo());
}

void G3XCompressorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    g3x::Parameters values;
    values.thresholdDb = threshold->load();
    values.ratio = ratio->load();
    values.attackMs = attack->load();
    values.releaseMs = release->load();
    values.autoRelease = releaseMode->load() > 0.5f;
    values.vintage = behavior->load() > 0.5f;
    values.warm = character->load() > 0.5f;
    values.outputGainDb = outputGain->load();
    values.mix = mix->load() * 0.01f;
    values.trimDb = trim->load();
    engine.setParameters(values);
    engine.process(buffer);
}

void G3XCompressorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto tree = state.copyState();
    tree.setProperty("stateVersion", 1, nullptr);
    if (auto xml = tree.createXml())
        copyXmlToBinary(*xml, destData);
}

void G3XCompressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(state.state.getType()))
            state.replaceState(juce::ValueTree::fromXml(*xml));
}

int G3XCompressorAudioProcessor::getNumPrograms()
{
    return 8;
}

const juce::String G3XCompressorAudioProcessor::getProgramName(int index)
{
    static constexpr std::array names { "Vocal Leveler", "Bass Vintage", "Drum Punch", "Parallel Smash",
                                        "Mix Glue", "Clean Peaks", "Upward Detail", "Pump Effect" };
    return juce::isPositiveAndBelow(index, static_cast<int>(names.size())) ? names[static_cast<size_t>(index)] : "";
}

void G3XCompressorAudioProcessor::setCurrentProgram(int index)
{
    struct Preset { float threshold, ratio, attack, release, output, mix, trim; int releaseMode, behavior, character; };
    static constexpr std::array presets {
        Preset { -18.0f, 3.0f, 18.0f, 180.0f, 3.0f, 100.0f, 0.0f, 1, 0, 0 },
        Preset { -20.0f, 4.0f, 28.0f, 260.0f, 3.0f, 100.0f, 0.0f, 1, 1, 1 },
        Preset { -14.0f, 5.0f, 32.0f, 95.0f, 2.0f, 100.0f, 0.0f, 0, 0, 1 },
        Preset { -32.0f, 10.0f, 4.0f, 120.0f, 8.0f, 38.0f, 0.0f, 1, 0, 1 },
        Preset { -12.0f, 2.0f, 30.0f, 320.0f, 1.5f, 100.0f, 0.0f, 1, 0, 0 },
        Preset { -8.0f, 8.0f, 1.5f, 70.0f, 0.0f, 100.0f, 0.0f, 0, 0, 0 },
        Preset { -30.0f, 0.72f, 45.0f, 420.0f, -1.0f, 100.0f, 0.0f, 1, 1, 0 },
        Preset { -26.0f, 8.0f, 0.8f, 460.0f, 5.0f, 100.0f, 0.0f, 0, 1, 1 }
    };
    if (!juce::isPositiveAndBelow(index, static_cast<int>(presets.size())))
        return;
    currentProgram = index;
    const auto& preset = presets[static_cast<size_t>(index)];
    auto set = [this](const char* id, float plainValue)
    {
        if (auto* parameter = state.getParameter(id))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(plainValue));
    };
    set(IDs::threshold, preset.threshold); set(IDs::ratio, preset.ratio);
    set(IDs::attack, preset.attack); set(IDs::release, preset.release);
    set(IDs::output, preset.output); set(IDs::mix, preset.mix); set(IDs::trim, preset.trim);
    set(IDs::releaseMode, static_cast<float>(preset.releaseMode));
    set(IDs::behavior, static_cast<float>(preset.behavior));
    set(IDs::character, static_cast<float>(preset.character));
}

juce::AudioProcessorEditor* G3XCompressorAudioProcessor::createEditor()
{
    return new G3XCompressorAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new G3XCompressorAudioProcessor();
}
