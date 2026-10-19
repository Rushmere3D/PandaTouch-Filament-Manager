PandaTouch Filament Manager

Custom touchscreen firmware for the BIGTREETECH Panda Touch V1.0, designed to manage 3D-printing filament inventory through Spoolman and weigh filament spools using a separate Wi-Fi-connected ESP32/HX711 scale.

> **Project status: Work in progress — tested on Panda Touch hardware through Stage 5B.** Inventory browsing, scale readings, remote tare, weight calculations, confirmed Spoolman weight updates, and manual/automatic inventory refresh are working in the development setup. Calibration from the Panda Touch and an on-device network setup wizard are **not implemented yet**.

**Features**

Working on hardware

**Spoolman inventory**

• Connects to Wi-Fi and retrieves filament inventory from a Spoolman server.

• Displays spool vendor, material, name, remaining weight and colour.

• Supports single-colour, transparent and multi-colour spool swatches.

• Searches inventory by vendor, filament name, material or Spoolman spool ID.

• Paginates inventory four spools at a time, with first, previous, next and last controls.

• Opens a detail view showing remaining weight, initial weight, empty-spool weight, location and Spoolman ID.

• Provides a compact refresh icon for manual inventory synchronisation.

• Automatically refreshes inventory approximately every 60 seconds, without blocking touchscreen interaction.

• Retains the existing inventory if refresh fails, and preserves the search, page and selected spool across refreshes.

**Wi-Fi filament scale**

• Receives live gross-weight readings from a separate ESP32 controller connected to an HX711 and load cell.
• Offers TARE from the Panda Touch using the scale’s HTTP API.
• Selects an inventory item using USE ON SCALE and calculates filament remaining from gross weight minus the spool’s recorded empty weight.
• Compares the measured filament weight against the value recorded in Spoolman.
• Detects stable weight readings over a two-second window, using raw measurements and a ±1 g tolerance (up to 2 g total range).
• Shows STABLE / STABILISING feedback and prevents Spoolman updates until the measurement is stable.
• Requires confirmation before updating Spoolman, then verifies the saved remaining weight with a follow-up API request.
• Refreshes local spool information after a verified successful update.
• Handles disconnected scale readings and blocks updates when the required empty-spool weight is unavailable or the calculated result is invalid.

Interface and diagnostics

• Spools, Scale and Settings pages with touchscreen navigation.
• Display brightness control and Wi-Fi status.
• Serial diagnostics for display, network, inventory and scale operations.

Planned / in development

• Remote scale calibration controls and scale connection information on the Settings page (Stage 6).
• Improved device/network configuration without editing firmware source.
• Further UI refinement, resilience testing and packaging for wider use.

Hardware

Touchscreen controller

• BIGTREETECH Panda Touch V1.0 with ESP32-S3.
• Built-in 800 × 480 RGB LCD and GT911 capacitive touchscreen.
• 2.4 GHz Wi-Fi access to a reachable Spoolman server and the scale controller.

ESP32 Filament Scale firmware

• ESP32-WROOM-32 development board.
• HX711 load-cell amplifier and a suitable load cell.
• HX711 DOUT → GPIO16, SCK → GPIO17 on the scale ESP32 in the tested setup.
• The scale runs its own firmware and exposes weight and tare endpoints over Wi-Fi.
• Calibration is stored on the scale controller, not on the Panda Touch.

The filament scale is a separate ESP32-based device that communicates with Panda Touch Filament Manager over Wi-Fi.

**Scale firmware repository:** [Rushmere3D/ESP32-Filament-Scale](https://github.com/Rushmere3D/ESP32-Filament-Scale)

The scale repository includes:

- ESP32 firmware and PlatformIO configuration
- ESP32-to-HX711 wiring instructions
- Initial tare and calibration procedures
- Wi-Fi configuration instructions
- HTTP API documentation for weight readings and remote tare

### Installation

1. Follow the instructions in the [ESP32 Filament Scale README](https://github.com/Rushmere3D/ESP32-Filament-Scale).
2. Configure the scale's Wi-Fi credentials locally.
3. Build and upload the scale firmware using PlatformIO.
4. Calibrate the scale using its serial commands.
5. Note the IP address reported by the scale.
6. Configure Panda Touch Filament Manager to use that IP address for its scale API.

The Panda Touch and ESP32 scale must be able to communicate over the local network. Each device has its own firmware and must be flashed separately.

**Note:** Remote calibration from the Panda Touch is planned for Stage 6. Until then, use the [scale calibration guide](https://github.com/Rushmere3D/ESP32-Filament-Scale/blob/main/docs/calibration.md).

The Panda Touch and scale are two separate devices communicating over the local network. The scale is not connected directly to the Panda Touch’s GPIO or I²C connector.

This is a custom firmware project, not the stock Panda Touch firmware. Flashing replaces the firmware currently installed on the device. Keep a backup or a known recovery method before experimenting.

Software stack

• PlatformIO in Visual Studio Code
• ESP32 Arduino framework (tested Panda Touch environment: pandatouch)
• LVGL 9.3.0
• TAMC_GT911
• Arduino_GFX
• ArduinoJson
• HTTP-based communication with Spoolman and the scale ESP32

The project uses the pandatouch PlatformIO environment for development and hardware testing. Other environments in platformio.ini, if present, should not be assumed to have been validated with this application.

Getting started

1. Prerequisites

1. Install VS Code and the PlatformIO extension.
2. Have a Panda Touch V1.0 and a USB connection for flashing.
3. Run a Spoolman server reachable from the Panda Touch.
4. For weighing, build and flash the separate ESP32/HX711 scale controller and connect it to the same reachable network.

The Panda Touch firmware alone can browse Spoolman inventory, but live weighing requires the separate scale hardware and firmware.

2. Clone the repository

git clone https://github.com/Rushmere3D/PandaTouch-Filament-Manager.git
cd PandaTouch-Filament-Manager

Open the cloned folder in VS Code.

3. Configure Wi-Fi, Spoolman and scale

1. Review the configuration files supplied with the repository, including any secrets.example.h template.
2. Configure the Wi-Fi credentials and Spoolman URL used by src/network/wifi_manager.h and src/network/spoolman_client.h.
3. In src/main.cpp, set the scale’s HTTP address to match the IP address or hostname of your own scale ESP32. The development firmware uses SCALE_URL and SCALE_TARE_URL for these endpoints.

A typical Spoolman URL is:

http://YOUR-SPOOLMAN-HOST:7912

Example scale endpoints (replace the hostname/IP with your own):

http://YOUR-SCALE-HOST/api/weight
http://YOUR-SCALE-HOST/api/tare

The scale API used by the current firmware responds to GET /api/weight with a JSON object containing a numeric weight in grams, and accepts POST /api/tare with a JSON response containing success.

Do not commit Wi-Fi passwords, tokens or private configuration files. Check .gitignore and git status before pushing changes. Configuration remains developer-oriented; an on-device setup wizard is not yet available.

4. Build and flash

In the PlatformIO terminal:

pio run -e pandatouch
pio run -e pandatouch -t upload

If automatic port selection fails, specify the correct serial port, for example on Windows:

pio run -e pandatouch -t upload --upload-port COM7

COM7 is an example, not a fixed requirement.

The separate scale ESP32 requires its own firmware build and upload; flashing the Panda Touch does not program the scale controller.

5. View serial diagnostics

pio device monitor -b 115200

The firmware logs startup, inventory refresh and Spoolman update activity. In the development setup, serial diagnostics worked without the -DARDUINO_USB_CDC_ON_BOOT=1 flag; only change USB/serial build settings if required by your hardware configuration.

Using the interface

Spools: Browse inventory, search, paginate and tap a row for details. The detail view includes USE ON SCALE. Tap the refresh icon to fetch current inventory immediately. The firmware also synchronises inventory automatically approximately every 60 seconds. Search, pagination and spool selection are retained when refreshing; a failed request does not discard the existing list.

Scale: Select a spool from its detail view. The Scale page shows live gross weight, calculated filament weight and the difference from Spoolman. Use TARE to zero the connected scale. Wait for the STABLE indicator before using UPDATE SPOOLMAN. Confirm the proposed update; the firmware writes the remaining weight and checks that Spoolman saved it successfully. An empty-spool weight must be recorded for the selected spool to calculate remaining filament correctly.

Settings: Includes device settings such as display brightness and Wi-Fi information. Remote scale calibration is not available on this page yet; that is planned for Stage 6.

Weight update safeguards

• A successful scale connection alone is not sufficient: the weight must also be stable.
• Stability is evaluated from raw readings over a two-second window, not from the rounded on-screen weight.
• The displayed weight and Spoolman update are rounded to whole grams.
• Invalid calculations and missing empty-spool weights prevent an update.
• The firmware requires user confirmation and verifies the Spoolman API result.
• Inventory refresh and weight-writing operations are coordinated to avoid running simultaneously.

Current limitations

• Calibration from Panda Touch is not yet implemented. The scale must already be calibrated using its own firmware/workflow.
• Two-device setup: live weighing requires a separate powered, network-connected ESP32/HX711 scale.
• Wi-Fi, Spoolman and scale addresses may require editing source/configuration files before building.
• Spoolman inventory is held in device memory; automatic synchronisation is periodic, not instant push notification.
• The project is under active development and is not yet a production-ready firmware release.

Display tearing and rendering

This firmware builds on the original Arduino/PlatformIO Panda Touch display implementation. The 800 × 480 RGB LCD can exhibit tearing, flicker or timing-related artifacts depending on rendering mode and memory configuration. The ESP-IDF-based implementation offers more direct control of LCD timing; see BIGTREETECH’s PandaTouch_IDF project. The current application uses pt_setup_display(PT_LVGL_RENDER_FULL_1).

Project structure

Key files and folders in the current development layout:

src/
  main.cpp                     Startup, scale polling/stability and task coordination
  network/
    wifi_manager.h             Wi-Fi connection
    spoolman_client.h          Spoolman inventory communication
    spoolman_update.h          Verified Spoolman weight updates
    spoolman_refresh.h         Background inventory refresh
  models/
    spool_store.h              In-memory spool data
  ui/
    ui_spools.h                Inventory, search, refresh and spool details
    ui_scale.h                 Live scale, tare, stability and weight updates
    ui_settings.h              Settings page
  ui_main.h                    Main UI and navigation
  pt/                          Panda Touch display support
include/
  lv_conf.h                    LVGL configuration
platformio.ini                 Build environments and dependencies
docs/                          Hardware references and documentation

Development milestones

|Stage|Status |Functionality                                     |
|-----|-------|--------------------------------------------------|
|1    |Working|Select Spoolman spool for scale                   |
|2    |Working|Live weighing, filament calculation and comparison|
|3B   |Working|Confirmed and verified Spoolman weight updates    |
|4    |Working|Weight stability detection and update safeguards  |
|5A   |Working|Manual inventory refresh                          |
|5B   |Working|Automatic inventory synchronisation (~60 seconds) |
|6    |Planned|Remote scale settings and calibration             |

Credits and acknowledgements

• BIGTREETECH for the Panda Touch hardware and original PandaTouch PlatformIO template on which this firmware is based.
• Spoolman for the filament inventory management server and API.
• The maintainers and contributors of LVGL, Arduino_GFX, TAMC_GT911, ArduinoJson, PlatformIO and the ESP32 Arduino ecosystem.

This is an independent community project and is not an official BIGTREETECH or Spoolman firmware release. Refer to the repository’s included licence and upstream dependency licences for applicable terms.

Useful references

• Panda Touch documentation
• Original PandaTouch PlatformIO project
• PandaTouch ESP-IDF project
• Spoolman
• LVGL documentation
• PlatformIO documentation
• Pinout reference
• LCD datasheet
