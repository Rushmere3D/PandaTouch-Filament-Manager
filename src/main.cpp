
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <math.h>

#include "pt/pt_display.h"
#include "ui_main.h"
#include "ui/ui_scale.h"
#include "network/wifi_manager.h"
#include "network/spoolman_client.h"

// -------------------- Scale connection --------------------

// Replace with your scale ESP32's actual IP address.
const char* SCALE_URL = "http://192.168.1.222/api/weight";

constexpr uint32_t SCALE_POLL_INTERVAL_MS = 3000;
constexpr uint32_t SCALE_HTTP_TIMEOUT_MS = 1500;

// -------------------- Shared scale data --------------------

struct ScaleReading
{
    float grams;
    bool connected;
    uint32_t sequence;
};

ScaleReading scaleReading = {0.0f, false, 0};

SemaphoreHandle_t scaleMutex = nullptr;
TaskHandle_t scaleTaskHandle = nullptr;

// -------------------- Background HTTP task --------------------

void scalePollingTask(void* parameter)
{
    uint32_t sequence = 0;

    for (;;)
    {
        float grams = 0.0f;
        bool connected = false;

        if (WiFi.status() == WL_CONNECTED)
        {
            HTTPClient http;
            http.setTimeout(SCALE_HTTP_TIMEOUT_MS);

            if (http.begin(SCALE_URL))
            {
                const int statusCode = http.GET();

                if (statusCode == HTTP_CODE_OK)
                {
                    String response = http.getString();

                    JsonDocument doc;
                    DeserializationError error =
                        deserializeJson(doc, response);

                    if (!error && doc["weight"].is<float>())
                    {
                        grams = doc["weight"].as<float>();
                        connected = isfinite(grams);
                    }
                }

                http.end();
            }
        }

        // Publish the result without calling LVGL.
        if (xSemaphoreTake(scaleMutex, portMAX_DELAY) == pdTRUE)
        {
            scaleReading.grams = grams;
            scaleReading.connected = connected;
            scaleReading.sequence = ++sequence;

            xSemaphoreGive(scaleMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(SCALE_POLL_INTERVAL_MS));
    }
}

// -------------------- Main-loop UI update --------------------

void updateScaleDisplay()
{
    static uint32_t lastSequence = 0;

    ScaleReading reading;

    if (xSemaphoreTake(scaleMutex, 0) != pdTRUE)
        return;

    reading = scaleReading;
    xSemaphoreGive(scaleMutex);

    if (reading.sequence == lastSequence)
        return;

    lastSequence = reading.sequence;

    if (reading.connected)
    {
        Serial.printf("[SCALE] Weight: %.1f g\n", reading.grams);
        ui_scale_update_weight(reading.grams);
    }
    else
    {
        Serial.println("[SCALE] Scale disconnected");
        ui_scale_set_disconnected();
    }
}

// -------------------- Setup --------------------

void setup()
{
    Serial.begin(115200);

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

    // Create the mutex before starting the background task.
    scaleMutex = xSemaphoreCreateMutex();

    if (scaleMutex == nullptr)
    {
        Serial.println("[SCALE] Failed to create mutex");
        ui_scale_set_disconnected();
    }
    else
    {
        BaseType_t result = xTaskCreatePinnedToCore(
            scalePollingTask,
            "ScalePolling",
            8192,
            nullptr,
            1,
            &scaleTaskHandle,
            0
        );

        if (result == pdPASS)
        {
            Serial.println("[SCALE] Background polling started");
        }
        else
        {
            Serial.println("[SCALE] Failed to start background task");
            ui_scale_set_disconnected();
        }
    }

    Serial.println("[STARTUP] Setup complete");
}

// -------------------- Main loop --------------------

void loop()
{
    pt_loop_display();

    if (scaleMutex != nullptr && scaleTaskHandle != nullptr)
    {
        updateScaleDisplay();
    }
}
