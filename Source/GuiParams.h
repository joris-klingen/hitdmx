#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace hitdmx
{

inline constexpr int kSlidersPerPage  = 32;
inline constexpr int kColourOptions   = 9;

struct GuiParams
{
    GuiParams();

    // Designer-set parameters.
    int padding             = 10;
    int titleHeight         = 45;
    int consoleHeight       = 70;
    int sliderWidth         = 32;
    int sliderHeight        = 150;
    int sliderLabelHeight   = 25;
    int numSliderRows       = 2;
    int footerHeight        = 30;
    int connectButtonWidth  = 110;
    int arrowButtonWidth    = 20;

    // Calculated parameters.
    int numSliderColumns = 0;
    int width = 0;
    int height = 0;
    int consoleY = 0;
    int channelSelectorY = 0;
    int sliderLabel1Y = 0;
    int sliderRow1Y = 0;
    int sliderLabel2Y = 0;
    int sliderRow2Y = 0;
    int footerY = 0;
    int consoleWidth = 0;

    juce::Font fontH1      { juce::FontOptions ("Caviar Dreams", 40.0f, juce::Font::plain) };
    juce::Font fontH2      { juce::FontOptions ("Caviar Dreams", 25.0f, juce::Font::plain) };
    juce::Font fontH3      { juce::FontOptions ("Caviar Dreams", 20.0f, juce::Font::plain) };
    juce::Font fontConsole { juce::FontOptions ("Terminal",     16.0f, juce::Font::plain) };

    juce::Colour backgroundColour          { 50, 50, 50 };
    juce::Colour textColour                { juce::Colours::white };
    juce::Colour consoleBackgroundColour   { 32, 32, 32 };
    juce::Colour pluginFaceColours[kColourOptions] {
        juce::Colours::darkcyan,
        juce::Colour (180, 10, 91),
        juce::Colour (103, 65, 114),
        juce::Colour (1,   50, 67),
        juce::Colour (68, 108, 179),
        juce::Colour (30, 130, 76),
        juce::Colour (150, 75, 45),
        juce::Colour (165, 127, 38),
        juce::Colour (98, 110, 110)
    };

    juce::String title  { "  Flamingo Hitmix' dmx" };
    juce::String footer { "Based on Garage Lights (GPLv3)" };

    void drawBackground (juce::Graphics&, int colourId) const;
    void drawStaticText (juce::Graphics&) const;
    void drawConsoleText (juce::Graphics&, const juce::String&) const;
    void drawChannelLabels (juce::Graphics&, int channelPage) const;
};

class HitDmxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour&, bool, bool) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool, bool) override;
};

}
