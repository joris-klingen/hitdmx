#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <juce_core/juce_core.h>

namespace hitdmx
{

inline constexpr int kDmxUniverseSize = 512;

class DmxBackend
{
public:
    virtual ~DmxBackend() = default;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual int  scanDevices() = 0;
    virtual int  numDevices() const = 0;

    virtual juce::String getStatusText() const = 0;

    virtual void setChannel(int channel1to512, juce::uint8 value) = 0;
    virtual void setBlackout(bool enabled) = 0;

    static std::unique_ptr<DmxBackend> createDefault();
};

}
