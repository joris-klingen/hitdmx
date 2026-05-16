#pragma once

#include "DmxBackend.h"

#include <array>
#include <atomic>
#include <juce_events/juce_events.h>

namespace hitdmx
{

// ENTTEC DMX USB Pro backend that talks to the FTDI D2XX driver.
// Send loop runs on a juce::Timer at the device refresh rate.
class EnttecProBackend : public DmxBackend, private juce::Timer
{
public:
    EnttecProBackend();
    ~EnttecProBackend() override;

    bool connect() override;
    void disconnect() override;
    bool isConnected() const override                       { return connected.load(); }

    int  scanDevices() override;
    int  numDevices() const override                        { return numDevicesDetected; }

    juce::String getStatusText() const override;

    void setChannel(int channel1to512, juce::uint8 value) override;
    void setBlackout(bool enabled) override                 { blackout.store(enabled); }

private:
    void timerCallback() override;

    bool sendDmxFrame();
    int  sendPacket(int label, const unsigned char* data, int length);
    int  receivePacket(int label, unsigned char* data, unsigned int expectedLength);
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
