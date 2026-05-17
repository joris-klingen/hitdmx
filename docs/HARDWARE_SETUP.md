# Hardware setup: ENTTEC DMX USB Pro on macOS

HitDmx talks to the ENTTEC DMX USB Pro through FTDI's **D2XX** API. The
FTDI library is **statically linked** into the plugin, so the finished
`.vst3` is self-contained — there is no `libftd2xx.dylib` to install,
sign, or distribute, and HitDmx does not depend on anything in
`/usr/local/lib` at runtime.

You only need FTDI's SDK at **build time** (for the `.a` and header) and
you need to make sure macOS releases the device so the plugin can open
it.

## At a glance

1. Download FTDI's macOS D2XX driver, extract the tarball.
2. Drop `libftd2xx.a` and `ftd2xx.h` somewhere the build can find them.
3. Build HitDmx.
4. Install the `.vst3` and let your DAW load it.

Modern (universal-binary, 2020+) FTDI builds detach the
`AppleUSBFTDI` system driver via IOKit from inside `libftd2xx`. If the
tarball you downloaded includes `D2XXHelper.pkg`, install it; if it
doesn't, skip — newer builds handle the hand-off internally.

## Install the SDK

Download the **macOS** D2XX driver from
<https://ftdichip.com/drivers/d2xx-drivers/>. The archive contains
`ftd2xx.h`, `WinTypes.h`, `libftd2xx.<version>.dylib`, and
`libftd2xx.a`.

If present, run `D2XXHelper.pkg` and approve the FTDI system extension
in **System Settings → Privacy & Security**. Reboot. If it isn't in the
tarball, skip this step.

Place the build dependencies somewhere the default CMake config picks
them up (`/usr/local`):

```
sudo mkdir -p /usr/local/include /usr/local/lib
sudo cp ftd2xx.h WinTypes.h /usr/local/include/
sudo cp libftd2xx.a /usr/local/lib/
```

On **Apple Silicon**, verify `libftd2xx.a` covers `arm64`:

```
lipo -info /usr/local/lib/libftd2xx.a
```

It should list `arm64` (or be `Mach-O universal`). An x86_64-only
archive will fail to link against an arm64 host build.

## Build

```
cmake -S . -B build -G Xcode -DHITDMX_FTDI_D2XX_DIR=/usr/local
cmake --build build --config Release
```

The plugin is at
`build/HitDmx_artefacts/Release/VST3/HitDmx.vst3`.

Verify the result is self-contained — there should be **no**
`libftd2xx` line in `otool -L`:

```
otool -L build/HitDmx_artefacts/Release/VST3/HitDmx.vst3/Contents/MacOS/HitDmx
```

Only system frameworks (`CoreFoundation`, `IOKit`, `AppKit`, etc.) and
the C++ runtime should appear.

## Install the plugin

```
mkdir -p ~/Library/Audio/Plug-Ins/VST3
cp -R build/HitDmx_artefacts/Release/VST3/HitDmx.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/
```

Because the FTDI code is statically linked, **no library-validation
workarounds are needed**: Logic / GarageBand will load HitDmx the same
as any other VST3, provided the bundle itself is signed (or you have
library validation disabled host-wide).

## Validating end-to-end

With the device connected:

1. Open the plugin in your DAW.
2. The status panel should report `Found a compatible device.`
3. Click **Connect USB**. On success it changes to
   `Connected. Firmware <major>.<minor>` plus the device refresh rate
   and latency.
4. Move Channel 1's slider. A fixture patched to address 1 should
   respond live.

If `Connect` succeeds but no light reacts, the issue is downstream —
XLR wiring, fixture address, or the **Blackout** button. Check the GUI
state first.

## Common symptoms

| Symptom in the status panel                                          | Likely cause                                                            |
|----------------------------------------------------------------------|-------------------------------------------------------------------------|
| "No FTDI-compatible devices found." with the widget plugged in       | Older `libftd2xx.a` that still needs `D2XXHelper`. Install it and reboot. |
| "Could not open FTDI device."                                        | `AppleUSBFTDI` is holding the device. Reboot after the driver install.  |
| Build fails: "could not find ftd2xx.h" or "could not find libftd2xx.a"| `HITDMX_FTDI_D2XX_DIR` is wrong; point it at the directory containing them. |
| Plugin loads but `Connect` errors out on Apple Silicon                | `libftd2xx.a` is x86_64 only; download the universal/arm64 build.       |
