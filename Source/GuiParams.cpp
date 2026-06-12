#include "GuiParams.h"

namespace hitccdmx
{

GuiParams::GuiParams()
{
    numSliderColumns = kSlidersPerPage / numSliderRows;
    width            = sliderWidth * numSliderColumns + padding * (numSliderColumns + 1);
    consoleY         = titleHeight + padding;
    channelSelectorY = consoleY + consoleHeight + padding;
    sliderLabel1Y    = channelSelectorY + sliderLabelHeight;
    sliderRow1Y      = sliderLabel1Y + sliderLabelHeight;
    sliderLabel2Y    = sliderRow1Y + sliderHeight + padding;
    sliderRow2Y      = sliderLabel2Y + sliderLabelHeight;
    footerY          = sliderRow2Y + sliderHeight + padding;
    height           = footerY + footerHeight;
    consoleWidth     = width - connectButtonWidth * 2 - padding * 4;
}

void GuiParams::drawBackground (juce::Graphics& g, int colourId) const
{
    g.fillAll (backgroundColour);

    g.setColour (pluginFaceColours[colourId]);
    g.fillRoundedRectangle (0.0f, (float) titleHeight,
                            (float) width, (float) (height - titleHeight - footerHeight),
                            (float) padding);

    g.setColour (consoleBackgroundColour);
    g.fillRoundedRectangle ((float) (padding * 2 + connectButtonWidth), (float) consoleY,
                            (float) consoleWidth, (float) consoleHeight, (float) padding);

    g.setColour (textColour);
    g.drawLine ((float) 30, (float) (channelSelectorY + sliderLabelHeight / 2),
                (float) (width - 30), (float) (channelSelectorY + sliderLabelHeight / 2));
    g.setColour (pluginFaceColours[colourId]);
    g.fillRect (width / 3, channelSelectorY, width / 3, sliderLabelHeight);
    g.setColour (textColour);
}

void GuiParams::drawStaticText (juce::Graphics& g) const
{
    g.setColour (textColour);
    g.setFont (fontH1);
    g.drawFittedText (title, padding, 0, width - padding * 2, titleHeight,
                      juce::Justification::centredLeft, 1);

    g.setFont (fontH2);
    g.drawFittedText (footer, padding, footerY, width - padding * 2, footerHeight,
                      juce::Justification::centredRight, 1);

    g.drawFittedText ("Channels", 0, channelSelectorY, width, sliderLabelHeight,
                      juce::Justification::centred, 1);
}

void GuiParams::drawConsoleText (juce::Graphics& g, const juce::String& consoleText) const
{
    g.setFont (fontConsole);
    g.drawFittedText (consoleText,
                      padding * 2 + connectButtonWidth, consoleY,
                      consoleWidth, consoleHeight,
                      juce::Justification::centred, 4);
}

void GuiParams::drawChannelLabels (juce::Graphics& g, int channelPage) const
{
    int startingChannel = (channelPage - 1) * kSlidersPerPage + 1;
    int channelLabel    = startingChannel;
    g.setFont (fontH3);

    for (int row = 0; row < numSliderRows; ++row)
    {
        const int yValue = (row == 0) ? sliderLabel1Y : sliderLabel2Y;

        for (int column = 0; column < numSliderColumns; ++column)
        {
            const int xValue = padding * (column + 1) + sliderWidth * column;

            g.setColour (textColour);
            g.drawFittedText (juce::String (channelLabel), xValue, yValue,
                              sliderWidth, sliderLabelHeight,
                              juce::Justification::centred, 1);

            g.setColour (juce::Colours::white.withAlpha (0.2f));
            g.fillRect (xValue, yValue + sliderLabelHeight, sliderWidth, sliderHeight);

            ++channelLabel;
        }
    }
}

void HitDmxLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool isMouseOverButton, bool isButtonDown)
{
    auto colour = backgroundColour;
    if (isMouseOverButton) colour = juce::Colour (150, 150, 150);
    if (isButtonDown)      colour = juce::Colour (0,   0,   0);
    g.setColour (colour);
    g.fillRect (button.getLocalBounds());
}

void HitDmxLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool)
{
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions ("Terminal", 16.0f, juce::Font::plain));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds(),
                      juce::Justification::centred, 2);
}

}
