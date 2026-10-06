#include "pt/pt_display.h"
#include "ui_main.h"
#include "network/wifi_manager.h"

void setup()
{
    Serial.begin(115200);

    // Initialise Panda Touch LCD, touchscreen and LVGL
    pt_setup_display(PT_LVGL_RENDER_FULL_1);

    // Connect Panda Touch to Wi-Fi
    wifi_connect();

    // Create our Filament Manager interface
    // after Wi-Fi connection information is available
    ui_create_main();
}

void loop()
{
    // Keep LVGL/display/touch processing running
    pt_loop_display();
}