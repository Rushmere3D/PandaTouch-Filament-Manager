
#include <Arduino.h>

#include "pt/pt_display.h"
#include "ui_main.h"
#include "network/wifi_manager.h"
#include "network/spoolman_client.h"

void setup()
{
    Serial.begin(115200);

    // Allow the ESP32-S3 USB serial connection to initialise.
    // Maximum wait: 3 seconds, even without a PC connected.
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000)
    {
        delay(10);
    }

    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println("Panda Touch Filament Manager");
    Serial.println("================================");

    Serial.println("[STARTUP] Initialising display...");
    pt_setup_display(PT_LVGL_RENDER_FULL_1);
    Serial.println("[STARTUP] Display initialised");

    Serial.println("[WIFI] Starting connection...");
    wifi_connect();
    Serial.print("[WIFI] Status: ");
    Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");

    Serial.println("[SPOOLMAN] Loading inventory...");
    spoolman_test_spool_list();
    Serial.println("[SPOOLMAN] Inventory request finished");

    Serial.println("[UI] Creating interface...");
    ui_create_main();

    Serial.println("[STARTUP] Setup complete");
}

void loop()
{
    pt_loop_display();
}
