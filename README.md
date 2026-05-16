# HitDmx

A VST3/Standalone audio plugin that exposes a full DMX-512 universe as
host-automatable parameters, so you can drive lighting from a DAW.

This is a refactor and modernisation of
[spensbot/Garage-Lights](https://github.com/spensbot/Garage-Lights) (GPLv3).
The original was a Projucer/VST2 project built against JUCE 5 with a
Windows-only ENTTEC DMX USB Pro backend and hardcoded developer paths.

## What changed in the refactor

- **Build system**: Projucer project replaced by a CMake build using
  [JUCE 8](https://github.com/juce-framework/JUCE) fetched via
  `FetchContent`. The plugin now targets **VST3** and **Standalone**
  (no VST2).
- **API modernisation**: brought up to JUCE 8.
  - `AudioProcessorValueTreeState` is now constructed with a real
    `ParameterLayout` of `AudioParameterFloat` objects instead of the
    deprecated `createAndAddParameter`.
  - `ScopedPointer` → `std::unique_ptr`, `boolean` → `bool`,
    `AudioSampleBuffer` → `juce::AudioBuffer<float>`.
  - Fonts created via `juce::FontOptions`.
  - All types fully namespace-qualified; plugin code lives in
    `namespace hitdmx`.
- **DMX backend made pluggable**: `Source/Dmx/DmxBackend.h` defines an
  abstract interface, with two implementations:
  - `EnttecProBackend` — the original ENTTEC USB Pro protocol code,
    cleaned up into a real `.cpp` and freed of its hardcoded
    `C:/Users/Spenser/...` library path.
  - `NullDmxBackend` — used when the FTDI D2XX SDK is not available,
    so the plugin builds and loads on any platform.
- **Thread safety**: the DMX send buffer is updated under a
  `CriticalSection`; the FTDI timer callback takes a snapshot before
  writing.
- **GUI**: same layout and look, but split into `GuiParams.{h,cpp}`,
  page change made safe against out-of-range indices, and a polling
  timer drives device hot-plug detection.

## Building

Requires CMake 3.22+ and a C++17 compiler. JUCE is fetched automatically.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The VST3 will be at
`build/HitDmx_artefacts/Release/VST3/HitDmx.vst3`.

### Enabling real DMX output

Hardware output requires the FTDI D2XX SDK
(<https://ftdichip.com/drivers/d2xx-drivers/>). Configure with:

```
cmake -S . -B build \
  -DHITDMX_USE_FTDI_D2XX=ON \
  -DHITDMX_FTDI_D2XX_DIR=/path/to/ftdi-d2xx-sdk
```

Without that flag, the plugin builds with `NullDmxBackend` and the
status panel will say so.

## License

GPLv3, inherited from Garage-Lights. See `LICENSE.txt`.
