# Hardware setup: ENTTEC DMX USB Pro on macOS

HitDmx talks to the ENTTEC DMX USB Pro through FTDI's **D2XX** API — a
packet-level interface to the FT232 chip inside the widget. macOS's
built-in `AppleUSBFTDI` driver claims the device by default and must
release it before D2XX can open it.

Setup has two steps:

1. Install FTDI's D2XX driver + the `D2XXHelper` system extension that
   hands off the device from `AppleUSBFTDI` to D2XX.
2. Drop the `libftd2xx.dylib` and `ftd2xx.h` somewhere the build can
   find them.

## Install

1. Download the **macOS** D2XX driver from
   <https://ftdichip.com/drivers/d2xx-drivers/>. The tarball contains
   a `D2XXHelper.pkg` installer plus `ftd2xx.h`, `WinTypes.h`, and a
   `libftd2xx.<version>.dylib`.

2. Run `D2XXHelper.pkg`.

3. Approve the system extension in **System Settings → Privacy &
   Security**. You may see *"System software from FTDI was blocked"*
   → click **Allow**.

4. Reboot. Without this, `AppleUSBFTDI` will keep the device and
   `FT_Open` will fail.

5. Install the library and header to `/usr/local` so the default
   `HITDMX_FTDI_D2XX_DIR=/usr/local` finds them:

   ```
   sudo mkdir -p /usr/local/include /usr/local/lib
   sudo cp ftd2xx.h WinTypes.h /usr/local/include/
   sudo cp libftd2xx.*.dylib /usr/local/lib/
   sudo ln -sf /usr/local/lib/libftd2xx.*.dylib /usr/local/lib/libftd2xx.dylib
   sudo install_name_tool -id /usr/local/lib/libftd2xx.dylib \
       /usr/local/lib/libftd2xx.dylib
   ```

6. On **Apple Silicon**, make sure the dylib you installed is a
   universal or `arm64` build (recent FTDI releases ship a universal
   binary):

   ```
   file /usr/local/lib/libftd2xx.dylib
   ```

   should say `Mach-O universal binary` or `arm64`. An x86_64-only
   dylib will not link against an arm64 host build of the plugin.

## Build

```
cmake -S . -B build -G Xcode -DHITDMX_FTDI_D2XX_DIR=/usr/local
cmake --build build --config Release
```

Verify the plugin links against the real library:

```
otool -L build/HitDmx_artefacts/Release/VST3/HitDmx.vst3/Contents/MacOS/HitDmx
```

The `libftd2xx.dylib` line should resolve to `/usr/local/lib/...`.

## Code signing / hosts

Hosts that sandbox plugins heavily (Logic, GarageBand) refuse to load
unsigned/un-notarized plugins, and reject any plugin that loads an
unsigned third-party dylib. For local development, either:

- Run the host with `--disable-library-validation`, or
- Sign the plugin and the `libftd2xx.dylib` yourself with your
  Developer ID.

## Validating end-to-end

With the device connected:

1. Open the plugin in your DAW.
2. The status panel should report `Found a compatible device.`
3. Click **Connect USB**. On success the panel changes to
   `Connected. Firmware <major>.<minor>` with the device refresh rate
   and latency.
4. Move Channel 1's slider. If you have a fixture patched to address 1,
   it should respond live.

If `Connect` succeeds but no light reacts, the issue is downstream of
the plugin — XLR wiring, fixture address, or the **Blackout** button
still being on. Check the GUI state first.

## Common symptoms

| Symptom in the status panel                                          | Likely cause                                                       |
|----------------------------------------------------------------------|--------------------------------------------------------------------|
| "No FTDI-compatible devices found." with the widget plugged in       | `D2XXHelper` not installed, or not approved in System Settings.    |
| "Could not open FTDI device."                                        | `AppleUSBFTDI` still has the device. Reboot after installing.      |
| "Found a compatible device" but `Connect` fails immediately          | Library architecture mismatch (x86_64 dylib loaded by arm64 host). |
| Host refuses to load the plugin at all                                | Library validation; sign the dylib or disable validation in the host. |
