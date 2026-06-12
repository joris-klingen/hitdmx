#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <memory>

#include "PluginProcessor.h"
#include "GuiParams.h"

namespace hitccdmx
{

class HitDmxAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                    private juce::Button::Listener,
                                    private juce::Timer
{
public:
    explicit HitDmxAudioProcessorEditor (HitDmxAudioProcessor&);
    ~HitDmxAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    void buttonClicked (juce::Button*) override;
    void timerCallback() override;

    void changePage (int direction);
    void refreshConsoleText();

    HitDmxAudioProcessor& processor;
    GuiParams gp;
    HitDmxLookAndFeel lookAndFeel;

    std::array<juce::Slider, kDmxUniverseSize> dmxLevelSlider;
    std::array<std::unique_ptr<SliderAttachment>, kDmxUniverseSize> dmxLevelSliderAttachment;

    juce::TextButton connectUsbButton  { "Connect USB" };
    juce::TextButton blackoutButton    { "Blackout" };
    juce::TextButton disconnectButton  { "Disconnect" };
    juce::ArrowButton pageRightButton  { "pageRightButton", 0.0f, juce::Colours::white };
    juce::ArrowButton pageLeftButton   { "pageLeftButton",  0.5f, juce::Colours::white };
    juce::OwnedArray<juce::ShapeButton> colorSelectButtons;

    juce::String consoleText;
    bool connectAttempt = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HitDmxAudioProcessorEditor)
};

}
