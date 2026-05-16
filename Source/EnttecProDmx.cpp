#include "EnttecProDmx.h"

#include <ftd2xx.h>

#include <cstring>

namespace hitdmx
{

// ENTTEC USB Pro packet labels (see ENTTEC's "DMX USB Pro Widget API Specification").
namespace
{
    constexpr int GET_WIDGET_PARAMS       = 3;
    constexpr int GET_WIDGET_PARAMS_REPLY = 3;
    constexpr int SET_DMX_TX_MODE         = 6;
    constexpr unsigned char DMX_START_CODE = 0x7E;
    constexpr unsigned char DMX_END_CODE   = 0xE7;
    constexpr int HEADER_LENGTH = 4;
    constexpr int MAX_PACKET_SIZE = 512;

    #pragma pack(push, 1)
    struct DmxUsbProParams
    {
        unsigned char FirmwareLSB;
        unsigned char FirmwareMSB;
        unsigned char BreakTime;
        unsigned char MaBTime;
        unsigned char RefreshRate;
    };
    #pragma pack(pop)
}

EnttecProDmx::EnttecProDmx()
{
    dmxData.fill (0);
    blackoutData.fill (0);
    scanDevices();
}

EnttecProDmx::~EnttecProDmx()
{
    stopTimer();
    disconnect();
}

int EnttecProDmx::scanDevices()
{
    DWORD count = 0;
    auto status = FT_ListDevices ((PVOID)(uintptr_t) &count, nullptr, FT_LIST_NUMBER_ONLY);
    numDevicesDetected = (status == FT_OK) ? (int) count : 0;
    return numDevicesDetected;
}

bool EnttecProDmx::connect()
{
    if (connected.load())
        return true;

    FT_HANDLE handle = nullptr;
    if (FT_Open (0, &handle) != FT_OK)
    {
        lastError = "Could not open FTDI device.";
        return false;
    }
    deviceHandle = handle;

    UCHAR lat = 0;
    FT_GetLatencyTimer (handle, &lat);
    latencyTimer = lat;

    FT_SetTimeouts (handle, 120, 100);
    FT_Purge (handle, FT_PURGE_RX);

    int size = 0;
    if (sendPacket (GET_WIDGET_PARAMS, reinterpret_cast<unsigned char*> (&size), 2) <= 0)
    {
        FT_Purge (handle, FT_PURGE_TX);
        if (sendPacket (GET_WIDGET_PARAMS, reinterpret_cast<unsigned char*> (&size), 2) <= 0)
        {
            closePort();
            lastError = "ENTTEC widget did not respond to GET_WIDGET_PARAMS.";
            return false;
        }
    }

    DmxUsbProParams params {};
    if (receivePacket (GET_WIDGET_PARAMS_REPLY,
                       reinterpret_cast<unsigned char*> (&params),
                       sizeof (params)) <= 0)
    {
        if (receivePacket (GET_WIDGET_PARAMS_REPLY,
                           reinterpret_cast<unsigned char*> (&params),
                           sizeof (params)) <= 0)
        {
            closePort();
            lastError = "ENTTEC widget did not return parameter reply.";
            return false;
        }
    }

    firmwareMajor = params.FirmwareMSB;
    firmwareMinor = params.FirmwareLSB;
    refreshRate   = params.RefreshRate;

    connected.store (true);
    lastError = {};
    startTimerHz (40);
    return true;
}

void EnttecProDmx::disconnect()
{
    stopTimer();
    closePort();
    connected.store (false);
    scanDevices();
}

void EnttecProDmx::closePort()
{
    if (deviceHandle != nullptr)
    {
        FT_Close (static_cast<FT_HANDLE> (deviceHandle));
        deviceHandle = nullptr;
    }
}

juce::String EnttecProDmx::getStatusText() const
{
    if (connected.load())
    {
        return "Connected. Firmware "
             + juce::String (firmwareMajor) + "." + juce::String (firmwareMinor)
             + "\nRefresh rate: " + juce::String (refreshRate)
             + "\nLatency: " + juce::String (latencyTimer);
    }

    if (! lastError.isEmpty())
        return lastError;

    if (numDevicesDetected == 0)
        return "No FTDI-compatible devices found. Plug in an ENTTEC DMX USB Pro and retry.";
    if (numDevicesDetected == 1)
        return "Found a compatible device. Click \"Connect USB\" to open it.";
    return "Found " + juce::String (numDevicesDetected)
         + " compatible devices. Please leave only one connected.";
}

void EnttecProDmx::setChannel (int channel, juce::uint8 value)
{
    if (channel < 1 || channel > kDmxUniverseSize)
        return;
    const juce::ScopedLock lock (dataLock);
    dmxData[(size_t) channel] = value;
}

void EnttecProDmx::timerCallback()
{
    if (! connected.load())
        return;

    if (! sendDmxFrame())
    {
        closePort();
        connected.store (false);
    }
}

bool EnttecProDmx::sendDmxFrame()
{
    const bool useBlackout = blackout.load();
    std::array<unsigned char, kDataLen> snapshot;
    {
        const juce::ScopedLock lock (dataLock);
        snapshot = useBlackout ? blackoutData : dmxData;
    }
    return sendPacket (SET_DMX_TX_MODE, snapshot.data(), kDataLen) > 0;
}

int EnttecProDmx::sendPacket (int label, const unsigned char* data, int length)
{
    if (deviceHandle == nullptr)
        return 0;

    unsigned char header[HEADER_LENGTH];
    header[0] = DMX_START_CODE;
    header[1] = (unsigned char) label;
    header[2] = (unsigned char) (length & 0xFF);
    header[3] = (unsigned char) (length >> 8);

    DWORD written = 0;
    auto handle = static_cast<FT_HANDLE> (deviceHandle);

    if (FT_Write (handle, header, HEADER_LENGTH, &written) != FT_OK || (int) written != HEADER_LENGTH)
        return 0;
    if (FT_Write (handle, const_cast<unsigned char*> (data), (DWORD) length, &written) != FT_OK
        || (int) written != length)
        return 0;
    unsigned char endCode = DMX_END_CODE;
    if (FT_Write (handle, &endCode, 1, &written) != FT_OK || written != 1)
        return 0;
    return 1;
}

int EnttecProDmx::receivePacket (int label, unsigned char* data, unsigned int expectedLength)
{
    if (deviceHandle == nullptr)
        return 0;

    auto handle = static_cast<FT_HANDLE> (deviceHandle);
    unsigned char byte = 0;
    DWORD bytesRead = 0;
    unsigned char buffer[600];

    while (byte != (unsigned char) label)
    {
        while (byte != DMX_START_CODE)
        {
            if (FT_Read (handle, &byte, 1, &bytesRead) != FT_OK || bytesRead == 0)
                return 0;
        }
        if (FT_Read (handle, &byte, 1, &bytesRead) != FT_OK || bytesRead == 0)
            return 0;
    }

    unsigned int length = 0;
    if (FT_Read (handle, &byte, 1, &bytesRead) != FT_OK || bytesRead == 0)
        return 0;
    length = byte;
    if (FT_Read (handle, &byte, 1, &bytesRead) != FT_OK)
        return 0;
    length += ((unsigned int) byte) << 8;

    if (length > (unsigned int) MAX_PACKET_SIZE)
        return 0;
    if (FT_Read (handle, buffer, length, &bytesRead) != FT_OK || bytesRead != length)
        return 0;
    if (FT_Read (handle, &byte, 1, &bytesRead) != FT_OK || bytesRead == 0)
        return 0;
    if (byte != DMX_END_CODE)
        return 0;

    std::memcpy (data, buffer, expectedLength);
    return 1;
}

}
