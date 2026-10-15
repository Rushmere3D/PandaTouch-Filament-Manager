
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "pt/pt_display.h"
#include "ui_main.h"
#include "ui/ui_scale.h"
#include "network/wifi_manager.h"
#include "network/spoolman_client.h"

// -------------------- Scale connection --------------------

// Replace this IP with your scale ESP32's actual IP address.
const char* SCALE_URL = "http://192.168.1.222/api/weight";

constexpr uint32_t SCALE_POLL_INTERVAL_MS = 3000;
uint32_t lastScalePollMs = 0;

// -------------------- Scale polling --------------------

void pollScale()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[SCALE] Wi-Fi disconnected");
        ui_scale_set_disconnected();
        return;
    }

    HTTPClient http;
    http.setTimeout(1500);

    if (!http.begin(SCALE_URL))
    {
        Serial.println("[SCALE] Failed to initialise HTTP client");
        ui_scale_set_disconnected();
        return;
    }

    int statusCode = http.GET();

    if (statusCode == HTTP_CODE_OK)
    {
        String response = http.getString();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error && doc["weight"].is<float>())
        {
            float grams = doc["weight"].as<float>();

            Serial.printf("[SCALE] Weight: %.1f g\n", grams);
            ui_scale_update_weight(grams);
        }
        else
        {
            Serial.println("[SCALE] Invalid JSON response");
            ui_scale_set_disconnected();
        }
    }
    else
    {
        Serial.printf("[SCALE] HTTP error: %d\n", statusCode);
        ui_scale_set_disconnected();
    }

    http.end();
}

// -------------------- Setup --------------------

void setup()
{
    Serial.begin(115200);

    // Allow ESP32-S3 USB serial to initialise.
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
    Serial.println(
        WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"
    );

    Serial.println("[SPOOLMAN] Loading inventory...");
    spoolman_test_spool_list();
    Serial.println("[SPOOLMAN] Inventory request finished");

    Serial.println("[UI] Creating interface...");
    ui_create_main();

    Serial.println("[STARTUP] Setup complete");
}

// -------------------- Main loop --------------------

void loop()
{
    pt_loop_display();

    if (millis() - lastScalePollMs >= SCALE_POLL_INTERVAL_MS)
    {
        lastScalePollMs = millis();
        pollScale();
    }
}
