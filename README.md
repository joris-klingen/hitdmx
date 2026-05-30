# HitDmx

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
  `C:/Users/Spenser/...` library path.
- **Thread safety**: the DMX send buffer is updated under a
  `CriticalSection`; the FTDI timer callback takes a snapshot before
  writing.
- **GUI**: same layout and look, but split into `GuiParams.{h,cpp}`,
  page change made safe against out-of-range indices, and a polling
  timer drives device hot-plug detection.

## Building

Requires CMake 3.22+, Xcode, and the FTDI D2XX SDK installed (see
[`docs/HARDWARE_SETUP.md`](docs/HARDWARE_SETUP.md)). JUCE is fetched
automatically.

```
cmake -S . -B build -G Xcode \
  -DHITDMX_FTDI_D2XX_DIR=/usr/local
cmake --build build --config Release
```

`HITDMX_FTDI_D2XX_DIR` should be the directory containing `ftd2xx.h`
and `libftd2xx.a` (either directly or in `include/` and `lib/`
subdirectories). `/usr/local` is the default and matches the install
location in the hardware setup guide.

FTDI's library is **statically linked** into the plugin: the resulting
`.vst3` is self-contained and does not depend on `libftd2xx.dylib` at
runtime. You can copy it between machines without installing any FTDI
runtime on the destination.

The VST3 is at `build/HitDmx_artefacts/Release/VST3/HitDmx.vst3`, and
the build also copies it to `~/Library/Audio/Plug-Ins/VST3/`
automatically so your DAW picks it up on the next rescan.

## License

GPLv3, inherited from Garage-Lights. See `LICENSE.txt`.
