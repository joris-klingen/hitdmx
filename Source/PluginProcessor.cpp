#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace hitdmx
{

juce::String HitDmxAudioProcessor::paramIdForChannel (int channel1to512)
{
    return "ch" + juce::String (channel1to512);
}

juce::AudioProcessorValueTreeState::ParameterLayout
HitDmxAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int i = 1; i <= kDmxUniverseSize; ++i)
    {
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID (paramIdForChannel (i), 1),
            "Channel " + juce::String (i),
            juce::NormalisableRange<float> (0.0f, 255.0f, 1.0f),
            0.0f));
    }
    return layout;
}

HitDmxAudioProcessor::HitDmxAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "DmxUniverse", createParameterLayout())
{
    for (int i = 1; i <= kDmxUniverseSize; ++i)
        parameters.addParameterListener (paramIdForChannel (i), this);
}

HitDmxAudioProcessor::~HitDmxAudioProcessor()
{
    for (int i = 1; i <= kDmxUniverseSize; ++i)
        parameters.removeParameterListener (paramIdForChannel (i), this);

    dmx.disconnect();
}

void HitDmxAudioProcessor::prepareToPlay (double, int) {}
void HitDmxAudioProcessor::releaseResources() {}

bool HitDmxAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;
    return mainOut == layouts.getMainInputChannelSet();
}

void HitDmxAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
}

juce::AudioProcessorEditor* HitDmxAudioProcessor::createEditor()
{
    return new HitDmxAudioProcessorEditor (*this);
}

void HitDmxAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void HitDmxAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

void HitDmxAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (! parameterID.startsWith ("ch"))
        return;
    const int channel = parameterID.substring (2).getIntValue();
    dmx.setChannel (channel, (juce::uint8) juce::jlimit (0, 255, (int) newValue));
}

}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new hitdmx::HitDmxAudioProcessor();
}
