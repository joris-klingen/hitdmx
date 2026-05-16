# Hardware setup: ENTTEC DMX USB Pro

HitDmx talks to the ENTTEC DMX USB Pro through FTDI's **D2XX** API — a
packet-level interface to the FT232 chip inside the widget. The OS-default
**VCP** driver (which exposes the device as a virtual serial port) cannot
be used, and the two drivers cannot both claim the device at the same time.

So the hardware setup is the same on every platform, in two steps:

1. Install the FTDI **D2XX** driver/SDK.
2. Make sure the OS-default VCP/serial driver releases the device so D2XX
   can open it.

Then point CMake at the SDK and rebuild:

```
cmake -S . -B build \
  -DHITDMX_USE_FTDI_D2XX=ON \
  -DHITDMX_FTDI_D2XX_DIR=/path/to/D2XX_SDK
cmake --build build --config Release
```

D2XX downloads live at
<https://ftdichip.com/drivers/d2xx-drivers/>. Each download is a tarball
or installer containing `ftd2xx.h`, `WinTypes.h`, and the platform
library. `HITDMX_FTDI_D2XX_DIR` must point at the directory that
contains the header and the import library (`ftd2xx.lib` on Windows) or
the shared library (`libftd2xx.so` / `libftd2xx.dylib`).

---

## macOS

Apple's IOKit ships its own FTDI driver (`AppleUSBFTDI`) and claims the
USB Pro automatically when you plug it in. D2XX cannot open the device
while Apple's driver holds it.

The reliable fix is FTDI's `D2XXHelper`, a small system extension that
detaches Apple's driver whenever D2XX opens a device:

1. Download the D2XX driver for macOS from the FTDI page above.
2. Run the included `D2XXHelper.pkg` installer.
3. Approve the system extension in **System Settings → Privacy &
   Security → "System software from FTDI was blocked"** → *Allow*.
4. Reboot.
5. Copy `libftd2xx.dylib` somewhere on the linker path, e.g.
   `/usr/local/lib`, and run
   `sudo install_name_tool -id /usr/local/lib/libftd2xx.dylib /usr/local/lib/libftd2xx.dylib`
   if FTDI hasn't done it already.

Apple Silicon: use the universal-binary variant of `libftd2xx.dylib`
(the recent FTDI builds are universal). Building HitDmx as an arm64
binary against an x86_64-only `libftd2xx.dylib` will not link.

Verify the chain with:

```
otool -L build/HitDmx_artefacts/VST3/HitDmx.vst3/Contents/MacOS/HitDmx
```

The line referring to `libftd2xx.dylib` should resolve to a real file.

Hosts that sandbox plugins heavily (Logic, GarageBand) need to be given
permission to load unsigned/notarized libraries; for development, run
your DAW with `--disable-library-validation` or sign the plugin yourself.

### Common macOS symptoms

| Symptom in the status panel                                          | Likely cause                                                       |
|----------------------------------------------------------------------|--------------------------------------------------------------------|
| "No FTDI-compatible devices found." with the widget plugged in       | `D2XXHelper` not installed or not approved in System Settings.     |
| "Could not open FTDI device."                                        | Apple's `AppleUSBFTDI` still has the device. Reboot after install. |
| "Found a compatible device" but `Connect` flips back immediately     | Library architecture mismatch (x86_64 dylib vs arm64 host).        |

---

## Linux

The kernel ships `ftdi_sio` and `usbserial`, which together claim the
USB Pro as `/dev/ttyUSB0`. D2XX needs the device unbound.

1. Install dependencies (Debian/Ubuntu):

   ```
   sudo apt-get install build-essential libusb-1.0-0-dev
   ```

2. Download the D2XX `.tgz` for Linux from the FTDI page, extract it,
   and install the shared library:

   ```
   tar xzf libftd2xx-x86_64-*.tgz
   cd release
   sudo cp -P build/libftd2xx.* /usr/local/lib/
   sudo ldconfig
   sudo cp ftd2xx.h WinTypes.h /usr/local/include/
   ```

3. Install the udev rule (the FTDI tarball ships
   `release/build/ftd2xx.rules`, but if not, use this):

   ```
   sudo tee /etc/udev/rules.d/99-ftdi-d2xx.rules <<'EOF'
   ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", \
       MODE="0666", RUN+="/bin/sh -c 'echo $kernel > /sys/bus/usb/drivers/ftdi_sio/unbind'"
   EOF
   sudo udevadm control --reload
   sudo udevadm trigger
   ```

   The `unbind` action detaches `ftdi_sio` from the device on plug-in.
   `MODE="0666"` lets non-root users open the device; tighten to a
   group such as `plugdev` if you prefer.

4. Replug the widget so the new rules apply.

5. Point CMake at the install:

   ```
   cmake -S . -B build \
     -DHITDMX_USE_FTDI_D2XX=ON \
     -DHITDMX_FTDI_D2XX_DIR=/usr/local/include
   ```

   (The CMake passes `-lftd2xx` for non-Windows, so the headers can sit
   anywhere on the include path and the library anywhere on the linker
   path — `/usr/local/{include,lib}` is just the common choice.)

### Quick sanity check

```
lsusb | grep -i future        # should list "Future Technology Devices ... FT232"
ls -l /dev/bus/usb/$(lsusb | grep -i future | awk '{print $2}')/...
```

If `dmesg | tail` shows `ftdi_sio` still attaching at plug-in time, the
udev rule isn't firing — verify the path in the rule and re-run
`udevadm trigger`.

---

## Windows

The FTDI **CDM** driver package installs both VCP and D2XX, but only one
of them is active for a given device. By default the USB Pro is
configured to expose VCP, so you need to switch it.

1. Install the **CDM** driver package from FTDI (one installer covers
   both x86 and x64).
2. Plug in the USB Pro.
3. Open **Device Manager**, expand **Universal Serial Bus controllers**,
   find **USB Serial Converter** (the FT232 underneath the COM port).
   Right-click → **Properties** → **Advanced** tab → **uncheck "Load
   VCP"** → OK.
4. Unplug and replug the widget. The COM port entry should disappear;
   the converter remains. D2XX can now open it.
5. Make sure `ftd2xx.dll` is loadable. The CDM installer drops it into
   `C:\Windows\System32`, which is on the default search path. If you
   distribute the plugin to a machine without the driver, ship the DLL
   alongside `HitDmx.vst3`.

6. Configure the build (use forward slashes or escape backslashes):

   ```
   cmake -S . -B build ^
     -DHITDMX_USE_FTDI_D2XX=ON ^
     -DHITDMX_FTDI_D2XX_DIR="C:/path/to/CDM/i386"     :: or amd64 for 64-bit
   cmake --build build --config Release
   ```

   `HITDMX_FTDI_D2XX_DIR` should point at the directory containing
   `ftd2xx.h` and `ftd2xx.lib` matching the architecture of your build.

### Common Windows symptoms

| Symptom                                                              | Likely cause                                                       |
|----------------------------------------------------------------------|--------------------------------------------------------------------|
| Plugin fails to load with "ftd2xx.dll not found"                     | DLL is not on the search path; copy it next to the VST3.           |
| `Connect USB` flips back to `Connect USB` after one click            | VCP is still active for the device; uncheck "Load VCP" and replug. |
| "Found 0 compatible devices" but the widget is plugged in            | Driver not installed, or wrong architecture (32-bit DLL, 64-bit host). |

---

## Validating end-to-end

With the device connected and the build configured for D2XX:

1. Launch HitDmx (Standalone is easiest for first-time testing).
2. The status panel should report `Found a compatible device.`
3. Click **Connect USB**. On success the panel changes to
   `Connected. Firmware <major>.<minor>` with the device refresh rate
   and latency.
4. Move Channel 1's slider. If you have a fixture patched to address 1,
   it should respond live; if not, point a DMX-reading tool (e.g.
   ENTTEC's PRO Manager, or another USB Pro plus their software) at the
   bus to confirm the frame.

If `Connect` succeeds but no light reacts, the issue is downstream of
the plugin — XLR wiring, fixture address, or a stuck blackout button.
HitDmx's blackout button toggles `processor.blackout`; check the GUI
state before chasing hardware.
