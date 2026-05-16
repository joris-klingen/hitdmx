#pragma once

#include "DmxBackend.h"

namespace hitdmx
{

class NullDmxBackend : public DmxBackend
{
public:
    bool connect() override                              { return false; }
    void disconnect() override                           {}
    bool isConnected() const override                    { return false; }

    int  scanDevices() override                          { return 0; }
    int  numDevices() const override                     { return 0; }

    juce::String getStatusText() const override
    {
        return "No DMX backend compiled in.\n"
               "Rebuild with -DHITDMX_USE_FTDI_D2XX=ON and the FTDI D2XX SDK to enable hardware output.";
    }

    void setChannel(int, juce::uint8) override           {}
    void setBlackout(bool) override                      {}
};

}
