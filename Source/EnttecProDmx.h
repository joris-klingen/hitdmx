#pragma once

#include <array>
#include <atomic>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

namespace hitdmx
{

inline constexpr int kDmxUniverseSize = 512;

// Talks to an ENTTEC DMX USB Pro via the FTDI D2XX driver.
// Send loop runs on a juce::Timer at the device refresh rate.
class EnttecProDmx : private juce::Timer
{
public:
    EnttecProDmx();
    ~EnttecProDmx() override;

    bool connect();
    void disconnect();
    bool isConnected() const                                { return connected.load(); }

    int  scanDevices();
    int  numDevices() const                                 { return numDevicesDetected; }

    juce::String getStatusText() const;

    void setChannel (int channel1to512, juce::uint8 value);
    void setBlackout (bool enabled)                         { blackout.store (enabled); }

private:
    void timerCallback() override;

    bool sendDmxFrame();
    int  sendPacket (int label, const unsigned char* data, int length);
    int  receivePacket (int label, unsigned char* data, unsigned int expectedLength);
    void closePort();

    std::atomic<bool> connected { false };
    std::atomic<bool> blackout  { false };
    int numDevicesDetected { 0 };

    // Index 0 is the DMX start code (kept 0). Channels 1..512 follow.
    static constexpr int kDataLen = 513;
    std::array<unsigned char, kDataLen> dmxData {};
    std::array<unsigned char, kDataLen> blackoutData {};

    void* deviceHandle { nullptr };
    int firmwareMajor { 0 }, firmwareMinor { 0 };
    int refreshRate { 0 };
    int latencyTimer { 0 };
    juce::String lastError;
    juce::CriticalSection dataLock;
};

}
