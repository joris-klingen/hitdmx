#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>

#include "Dmx/DmxBackend.h"

namespace hitdmx
{

class HitDmxAudioProcessor  : public juce::AudioProcessor,
                              private juce::AudioProcessorValueTreeState::Listener
{
public:
    HitDmxAudioProcessor();
    ~HitDmxAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                                   { return true; }

    const juce::String getName() const override                       { return JucePlugin_Name; }
    bool acceptsMidi()  const override                                { return false; }
    bool producesMidi() const override                                { return false; }
    bool isMidiEffect() const override                                { return false; }
    double getTailLengthSeconds() const override                      { return 0.0; }

    int getNumPrograms() override                                     { return 1; }
    int getCurrentProgram() override                                  { return 0; }
    void setCurrentProgram (int) override                             {}
    const juce::String getProgramName (int) override                  { return {}; }
    void changeProgramName (int, const juce::String&) override        {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getParameters() noexcept      { return parameters; }
    DmxBackend& getDmxBackend() noexcept                              { return *dmxBackend; }

    // UI state. Not stored in the parameter tree because they are not host-automatable.
    int  channelPage = 1;
    int  colourId    = 0;
    bool blackout    = false;

    static juce::String paramIdForChannel (int channel1to512);

private:
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState parameters;
    std::unique_ptr<DmxBackend> dmxBackend;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HitDmxAudioProcessor)
};

}
