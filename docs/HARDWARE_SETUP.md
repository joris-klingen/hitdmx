# Hardware setup: ENTTEC DMX USB Pro on macOS

hitccdmx talks to the ENTTEC DMX USB Pro over its **USB serial (VCP)
port**, using only macOS system frameworks — IOKit to discover the
device and POSIX `termios` for I/O. There is **no FTDI D2XX SDK**, no
`libftd2xx` to download or link, and nothing to install in
`/usr/local`. The finished `.vst3` is self-contained and native on
Apple Silicon.

macOS ships its own FTDI serial driver (`AppleUSBFTDI`), so when you
plug in the widget it appears as a callout device at
`/dev/cu.usbserial-EN…`. hitccdmx enumerates these via IOKit and opens
the matching one directly. No kernel extension, no `D2XXHelper.pkg`, no
reboot, no system-extension approval.

## At a glance

1. Plug in the ENTTEC DMX USB Pro.
2. Build hitccdmx (JUCE is fetched automatically; no other dependencies).
3. Let your DAW load the `.vst3` and connect.

## Build

```
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

(`-G Xcode` is optional; the default generator / `-G "Unix Makefiles"`
works just as well with the Command Line Tools.)

The plugin is at
`build/hitccdmx_artefacts/Release/VST3/hitccdmx.vst3`.

Verify the result is self-contained — there should be **no**
`libftd2xx` line, only system frameworks and the C++ runtime:

```
otool -L build/hitccdmx_artefacts/Release/VST3/hitccdmx.vst3/Contents/MacOS/hitccdmx
```

On **Apple Silicon**, confirm the binary is `arm64`:

```
lipo -info build/hitccdmx_artefacts/Release/VST3/hitccdmx.vst3/Contents/MacOS/hitccdmx
```

## Install the plugin

The build automatically copies `hitccdmx.vst3` to
`~/Library/Audio/Plug-Ins/VST3/` as a post-build step (JUCE's
`COPY_PLUGIN_AFTER_BUILD`), so a successful `cmake --build` is also a
successful install. Restart your DAW (or trigger a plugin rescan) to
pick up the new build.

To install system-wide instead (`/Library/Audio/Plug-Ins/VST3`, all
users), copy it manually with `sudo` — the automatic copy targets the
user folder by design so the build does not require elevated
permissions.

Because there is no third-party dynamic library, **no
library-validation workarounds are needed**: Logic / GarageBand load
hitccdmx the same as any other VST3, provided the bundle itself is signed
(the build applies an ad-hoc signature) or you have library validation
disabled host-wide.

## Validating end-to-end

With the device connected:

1. Open the plugin in your DAW.
2. The status panel should report `Found an ENTTEC DMX USB Pro.`
3. Click **Connect USB**. On success it changes to
   `Connected. Firmware <major>.<minor>` plus the device refresh rate.
4. Move Channel 1's slider. A fixture patched to address 1 should
   respond live.

If `Connect` succeeds but no light reacts, the issue is downstream —
XLR wiring, fixture address, or the **Blackout** button. Check the GUI
state first.

## Common symptoms

| Symptom in the status panel                                  | Likely cause                                                                                  |
|--------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| "No ENTTEC DMX USB Pro found." with the widget plugged in    | The callout port isn't named `usbserial-EN…`. Run `ls /dev/cu.*` to see what it enumerated as. |
| "Could not open /dev/cu.usbserial-… (Resource busy)"         | Another app (or a stale process) already has the port open. Close it and retry.               |
| "ENTTEC widget did not respond to GET_WIDGET_PARAMS."        | Opened a serial port that isn't an ENTTEC Pro, or the cable/firmware is unresponsive.         |
| No `/dev/cu.usbserial-*` appears at all                      | macOS hasn't bound its FTDI serial driver — try another cable/port; check System Information → USB. |
