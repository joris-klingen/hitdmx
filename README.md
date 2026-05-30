# hitdmx

A macOS VST3 plugin that exposes a full DMX-512 universe as
host-automatable parameters, so you can drive lighting from a DAW
through an **ENTTEC DMX USB Pro**.

This is a refactor and modernisation of
[spensbot/Garage-Lights](https://github.com/spensbot/Garage-Lights) (GPLv3).
The original was a Projucer/VST2 project built against JUCE 5 with
hardcoded developer paths.

## What changed in the refactor

- **Build system**: Projucer project replaced by a CMake build using
  [JUCE 8](https://github.com/juce-framework/JUCE) fetched via
  `FetchContent`. The plugin targets **VST3** on **macOS**.
- **API modernisation**: brought up to JUCE 8.
  - `AudioProcessorValueTreeState` is now constructed with a real
    `ParameterLayout` of `AudioParameterFloat` objects instead of the
    deprecated `createAndAddParameter`.
  - `ScopedPointer` → `std::unique_ptr`, `boolean` → `bool`,
    `AudioSampleBuffer` → `juce::AudioBuffer<float>`.
  - Fonts created via `juce::FontOptions`.
  - All types fully namespace-qualified; plugin code lives in
    `namespace hitdmx`.
- **ENTTEC USB Pro driver**: protocol code moved into
  `Source/EnttecProDmx.{h,cpp}`, cleaned up, and freed of its hardcoded
  `C:/Users/Spenser/...` library path. Device I/O no longer uses the
  proprietary FTDI **D2XX** SDK: the widget is reached over its USB
  serial (VCP) port using only macOS system frameworks — IOKit for
  discovery and POSIX `termios` for I/O. There is nothing to download
  or install, and the build links no third-party libraries.
- **Thread safety**: the DMX send buffer is updated under a
  `CriticalSection`; the timer callback takes a snapshot before
  writing.
- **GUI**: same layout and look, but split into `GuiParams.{h,cpp}`,
  page change made safe against out-of-range indices, and a polling
  timer drives device hot-plug detection.

## Building

Requires CMake 3.22+ and Xcode (or the Command Line Tools). JUCE is
fetched automatically; there are no other dependencies (see
[`docs/HARDWARE_SETUP.md`](docs/HARDWARE_SETUP.md)).

```
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

`-G Xcode` is optional — the default generator (or `-G "Unix
Makefiles"`) works just as well.

The plugin talks to the widget over its USB serial port using only
macOS system frameworks, so the resulting `.vst3` is self-contained
and links no third-party libraries. You can copy it between machines
without installing any runtime on the destination.

The VST3 is at `build/hitdmx_artefacts/Release/VST3/hitdmx.vst3`, and
the build also copies it to `~/Library/Audio/Plug-Ins/VST3/`
automatically so your DAW picks it up on the next rescan.

## License

GPLv3, inherited from Garage-Lights. See `LICENSE.txt`.
