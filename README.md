# PandaTouch Filament Manager

Custom touchscreen firmware for the **BIGTREETECH Panda Touch V1.0**, designed to manage 3D-printing filament inventory through [Spoolman](https://github.com/Donkie/Spoolman), with a filament-weighing scale planned for a future release.

> **Project status: Work in progress.** The Spoolman inventory interface is running on Panda Touch hardware. The physical scale and automatic weight updates are **not implemented yet**.

## Features

### Working on hardware

- Connects to Wi-Fi and retrieves filament inventory from a Spoolman server.
- Displays spool vendor, material, name, remaining weight and colour.
- Supports single-colour, transparent and multi-colour spool swatches.
- Searches the locally loaded inventory by vendor, filament name, material or Spoolman spool ID.
- Paginates the inventory in groups of four, with first, previous, next and last controls.
- Opens a detail view with spool information, including remaining weight, initial weight, empty-spool weight, location and Spoolman ID.
- Provides **Spools**, **Scale** and **Settings** pages, touchscreen navigation, brightness control and Wi-Fi status.
- Includes a **USE ON SCALE** action as part of the developing scale workflow; this does **not** mean physical weighing is functional yet.
- Provides serial startup diagnostics for display, Wi-Fi, Spoolman and UI initialization.

### Planned / in development

- HX711 load-cell-based filament scale integration.
- Live weight display and tare controls.
- Selecting a spool for weighing and calculating remaining filament weight.
- Updating spool weights in Spoolman after weighing, with appropriate confirmation and safeguards.
- Further UI refinement and testing.

## Hardware

- **BIGTREETECH Panda Touch V1.0** with ESP32-S3.
- Built-in 800 × 480 RGB LCD and GT911 capacitive touchscreen.
- 2.4 GHz Wi-Fi connection to a reachable Spoolman instance.
- **Future scale hardware:** load cell, HX711 amplifier and associated controller/wiring (final architecture and connections are still being developed).

This is a custom firmware project, **not** the stock Panda Touch firmware. Flashing replaces the firmware currently installed on the device. Keep a backup or a known recovery method before experimenting.

## Software stack

- [PlatformIO](https://platformio.org/) in [Visual Studio Code](https://code.visualstudio.com/)
- ESP32 Arduino framework (current working environment: `pandatouch`)
- [LVGL 9.3.0](https://lvgl.io/)
- [TAMC_GT911](https://github.com/tamctec/gt911-arduino)
- [Arduino_GFX](https://github.com/moononournation/Arduino_GFX)
- [ArduinoJson](https://arduinojson.org/)
- [Spoolman](https://github.com/Donkie/Spoolman) REST API

The project currently uses the **`pandatouch`** PlatformIO environment for development and hardware testing. An alternative Arduino 3.x environment may exist in `platformio.ini`, but it should not be assumed to have been validated with this application.

## Getting started

### 1. Prerequisites

1. Install VS Code and the PlatformIO extension.
2. Have a Panda Touch V1.0 and a USB connection for flashing.
3. Run a Spoolman server reachable over your local network.
4. Make sure the Panda Touch can connect to the same network (or otherwise reach the server).

### 2. Clone the repository

```bash
git clone https://github.com/Rushmere3D/PandaTouch-Filament-Manager.git
cd PandaTouch-Filament-Manager
```

Open the cloned folder in VS Code.

### 3. Configure Wi-Fi and Spoolman

Before building, inspect the project's configuration and network files, particularly:

- `src/network/wifi_manager.h`
- `src/network/spoolman_client.h`
- Any local credentials or secrets header referenced by those files.

Set your Wi-Fi credentials and the URL of your own Spoolman server according to the current source configuration. A typical Spoolman URL has the form:

```text
http://YOUR-SPOOLMAN-HOST:7912
```

**Do not commit Wi-Fi passwords, tokens or private configuration files.** Check `.gitignore` and `git status` before pushing changes. Configuration is currently developer-oriented; a complete on-device setup wizard is not documented or guaranteed.

### 4. Build and flash

In the PlatformIO terminal:

```bash
pio run -e pandatouch
pio run -e pandatouch -t upload
```

If automatic port selection fails, specify your actual serial port, for example on Windows:

```bash
pio run -e pandatouch -t upload --upload-port COM7
```

`COM7` is only an example; use the port shown for your device.

### 5. View serial diagnostics

```bash
pio device monitor -b 115200
```

The firmware logs its startup sequence and reports Spoolman inventory loading. For the serial interface used during current development, diagnostics worked **without** the `-DARDUINO_USB_CDC_ON_BOOT=1` build flag. Do not add that flag unless your own USB/serial setup requires it.

## Using the interface

**Spools:** Browse the loaded Spoolman inventory, enter a search term, move through pages, and tap a row to see its extended details. Press **BACK** to return to the list. The search text and page position are retained when returning from details.

**Scale:** A placeholder/development page for the planned weighing workflow. The **USE ON SCALE** button is present, but a working physical scale is not yet part of the firmware.

**Settings:** Device settings including display brightness and Wi-Fi connection information.

Inventory is currently loaded into device memory from Spoolman. Do not assume changes made elsewhere in Spoolman will appear instantly without a refresh or restart.

## Current limitations

- **No functioning load-cell scale yet.** HX711 integration and calibration are future work.
- **No confirmed Spoolman weight-writing workflow yet.** The current integration reads spool information.
- Network configuration may require editing local source/configuration files before building.
- The project is under active development and is not a production-ready firmware release.

### Display tearing and rendering

This firmware builds on the original Arduino/PlatformIO Panda Touch display implementation. The 800 × 480 RGB LCD can exhibit tearing, flicker or timing-related artifacts depending on rendering mode and memory configuration. The ESP-IDF-based implementation offers more direct control of LCD timing; see [BIGTREETECH's PandaTouch_IDF project](https://github.com/bigtreetech/PandaTouch_IDF) for that alternative. The current application uses `pt_setup_display(PT_LVGL_RENDER_FULL_1)`.

## Project structure

Key files and folders currently used in development:

```text
src/
  main.cpp                     Firmware startup and main loop
  network/
    wifi_manager.h             Wi-Fi connection
    spoolman_client.h          Spoolman communication
  models/
    spool_store.h              In-memory spool data
  ui/
    ui_spools.h                Inventory, search and spool details
    ui_scale.h                 Scale page
    ui_settings.h              Settings page
  ui_main.h                    Main UI and navigation
  pt/                           Panda Touch display support
include/
  lv_conf.h                    LVGL configuration
platformio.ini                 Build environments and dependencies
docs/                          Hardware references and documentation
```

File layout and implementation may change as development continues.

## Credits and acknowledgements

- **[BIGTREETECH](https://github.com/bigtreetech)** for the Panda Touch hardware and the original [PandaTouch PlatformIO template](https://github.com/bigtreetech/PandaTouch_PlatformIO) on which this firmware is based.
- **[Spoolman](https://github.com/Donkie/Spoolman)** for the filament inventory management server and API.
- The maintainers and contributors of LVGL, Arduino_GFX, TAMC_GT911, ArduinoJson, PlatformIO and the ESP32 Arduino ecosystem.

This is an independent community project and is not an official BIGTREETECH or Spoolman firmware release. Refer to the repository's included licence and upstream dependency licences for applicable terms.

## Useful references

- [Panda Touch documentation](https://bttwiki.com/PandaTouch.html)
- [Original PandaTouch PlatformIO project](https://github.com/bigtreetech/PandaTouch_PlatformIO)
- [PandaTouch ESP-IDF project](https://github.com/bigtreetech/PandaTouch_IDF)
- [Spoolman](https://github.com/Donkie/Spoolman)
- [LVGL documentation](https://docs.lvgl.io/)
- [PlatformIO documentation](https://docs.platformio.org/)
- [Pinout reference](docs/PINOUT.md)
- [LCD datasheet](docs/QX05ZJGI70N-05545Y.pdf)
