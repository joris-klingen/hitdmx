#include "PluginEditor.h"

namespace hitdmx
{

HitDmxAudioProcessorEditor::HitDmxAudioProcessorEditor (HitDmxAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
    setSize (gp.width, gp.height);

    auto setupTextButton = [this] (juce::TextButton& b, juce::Rectangle<int> bounds,
                                   const juce::String& text)
    {
        addAndMakeVisible (b);
        b.setBounds (bounds);
        b.setLookAndFeel (&lookAndFeel);
        b.setButtonText (text);
        b.addListener (this);
    };

    setupTextButton (connectUsbButton,
                     { gp.consoleWidth + gp.padding * 3 + gp.connectButtonWidth,
                       gp.consoleY,
                       gp.connectButtonWidth, (gp.consoleHeight - gp.padding) / 2 },
                     "Connect USB");

    setupTextButton (disconnectButton,
                     { gp.consoleWidth + gp.padding * 3 + gp.connectButtonWidth,
                       gp.consoleY + (gp.consoleHeight + gp.padding) / 2,
                       gp.connectButtonWidth, (gp.consoleHeight - gp.padding) / 2 },
                     "Disconnect");

    setupTextButton (blackoutButton,
                     { gp.padding, gp.consoleY, gp.connectButtonWidth, gp.consoleHeight },
                     "Blackout");

    addAndMakeVisible (pageLeftButton);
    pageLeftButton.setBounds (gp.width * 23 / 60 - gp.arrowButtonWidth / 2,
                              gp.channelSelectorY + gp.sliderLabelHeight / 2 - gp.arrowButtonWidth / 2,
                              gp.arrowButtonWidth, gp.arrowButtonWidth);
    pageLeftButton.addListener (this);

    addAndMakeVisible (pageRightButton);
    pageRightButton.setBounds (gp.width * 37 / 60 - gp.arrowButtonWidth / 2,
                               gp.channelSelectorY + gp.sliderLabelHeight / 2 - gp.arrowButtonWidth / 2,
                               gp.arrowButtonWidth, gp.arrowButtonWidth);
    pageRightButton.addListener (this);

    juce::Path square;
    square.addRectangle (0, 0, gp.arrowButtonWidth, gp.arrowButtonWidth);
    for (int i = 0; i < kColourOptions; ++i)
    {
        auto* button = colorSelectButtons.add (
            new juce::ShapeButton ("colour" + juce::String (i),
                                   gp.pluginFaceColours[i],
                                   gp.pluginFaceColours[i],
                                   gp.pluginFaceColours[i]));
        addAndMakeVisible (button);
        button->setShape (square, true, true, true);
        button->addListener (this);
        const int xValue = gp.width
                         - gp.padding * (kColourOptions + 1)
                         - gp.arrowButtonWidth * kColourOptions
                         + gp.arrowButtonWidth * i + gp.padding * i;
        const int yValue = gp.titleHeight / 2 - gp.arrowButtonWidth / 2;
        button->setBounds (xValue, yValue, gp.arrowButtonWidth, gp.arrowButtonWidth);
    }

    auto& parameters = processor.getParameters();
    for (int slider = 0; slider < kDmxUniverseSize; ++slider)
    {
        auto& s = dmxLevelSlider[(size_t) slider];
        addChildComponent (s);
        s.setSliderStyle (juce::Slider::SliderStyle::LinearBarVertical);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);

        dmxLevelSliderAttachment[(size_t) slider] = std::make_unique<SliderAttachment> (
            parameters,
            HitDmxAudioProcessor::paramIdForChannel (slider + 1),
            s);

        const int column = slider % gp.numSliderColumns;
        const int xValue = gp.padding * (column + 1) + gp.sliderWidth * column;
        const int yValue = ((slider % kSlidersPerPage) < gp.numSliderColumns)
                                ? gp.sliderRow1Y
                                : gp.sliderRow2Y;
        s.setBounds (xValue, yValue, gp.sliderWidth, gp.sliderHeight);
    }

    // Initial page visibility (start at processor.channelPage, do not change it).
    changePage (0);

    startTimerHz (10);
}

HitDmxAudioProcessorEditor::~HitDmxAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
    for (auto& b : { &connectUsbButton, &disconnectButton, &blackoutButton })
        b->setLookAndFeel (nullptr);
}

void HitDmxAudioProcessorEditor::paint (juce::Graphics& g)
{
    gp.drawBackground (g, processor.colourId);
    gp.drawStaticText (g);
    refreshConsoleText();
    gp.drawConsoleText (g, consoleText);
    gp.drawChannelLabels (g, processor.channelPage);
}

void HitDmxAudioProcessorEditor::resized() {}

void HitDmxAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    auto& dmx = processor.getDmxBackend();

    if (button == &connectUsbButton)
    {
        if (! dmx.isConnected())
        {
            dmx.connect();
            connectAttempt = true;
        }
    }
    else if (button == &pageLeftButton)   { changePage (-1); }
    else if (button == &pageRightButton)  { changePage (+1); }
    else if (button == &blackoutButton)
    {
        processor.blackout = ! processor.blackout;
        blackoutButton.setToggleState (processor.blackout,
                                       juce::NotificationType::dontSendNotification);
        dmx.setBlackout (processor.blackout);
    }
    else if (button == &disconnectButton)
    {
        dmx.disconnect();
    }
    else
    {
        for (int i = 0; i < colorSelectButtons.size(); ++i)
            if (button == colorSelectButtons[i])
                processor.colourId = i;
    }
    repaint();
}

void HitDmxAudioProcessorEditor::changePage (int direction)
{
    auto setPageVisible = [this] (int page, bool visible)
    {
        const int startingChannel = (page - 1) * kSlidersPerPage;
        for (int i = 0; i < kSlidersPerPage; ++i)
        {
            const int index = startingChannel + i;
            if (index >= 0 && index < kDmxUniverseSize)
                dmxLevelSlider[(size_t) index].setVisible (visible);
        }
    };

    setPageVisible (processor.channelPage, false);

    processor.channelPage += direction;
    if (processor.channelPage < 1)  processor.channelPage = 16;
    if (processor.channelPage > 16) processor.channelPage = 1;

    setPageVisible (processor.channelPage, true);
    repaint();
}

void HitDmxAudioProcessorEditor::refreshConsoleText()
{
    auto& dmx = processor.getDmxBackend();

    if (connectAttempt)
    {
        connectUsbButton.setButtonText (dmx.isConnected() ? "Connected" : "Connect USB");
        connectAttempt = false;
    }

    consoleText = dmx.getStatusText();
}

void HitDmxAudioProcessorEditor::timerCallback()
{
    // Rescan + refresh the status panel periodically so the user sees device hot-plug.
    processor.getDmxBackend().scanDevices();
    repaint();
}

}
