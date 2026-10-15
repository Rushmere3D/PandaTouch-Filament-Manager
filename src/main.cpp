
#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <math.h>
#include <cstring>

#include "pt/pt_display.h"
#include "ui_main.h"
#include "ui/ui_scale.h"
#include "network/wifi_manager.h"
#include "network/spoolman_client.h"

// -------------------- Scale connection --------------------

const char* SCALE_URL = "http://192.168.1.222/api/weight";
const char* SCALE_TARE_URL = "http://192.168.1.222/api/tare";

constexpr uint32_t SCALE_POLL_INTERVAL_MS = 500;
constexpr uint32_t SCALE_HTTP_TIMEOUT_MS = 1500;
constexpr uint8_t MAX_FAILED_POLLS = 3;
constexpr uint32_t TARE_MESSAGE_DURATION_MS = 2000;

// -------------------- Shared scale data --------------------

enum class TareState : uint8_t
{
    Idle,
    Pending,
    Running,
    Success,
    Failed
};

struct ScaleReading
{
    float grams;
    bool connected;
    uint32_t sequence;
    TareState tareState;
};

ScaleReading scaleReading = {
    0.0f, false, 0, TareState::Idle
};

SemaphoreHandle_t scaleMutex = nullptr;
TaskHandle_t scaleTaskHandle = nullptr;

// -------------------- TARE request from UI --------------------

void requestScaleTare()
{
    if (scaleMutex == nullptr)
        return;

    if (xSemaphoreTake(scaleMutex, 0) != pdTRUE)
        return;

    if (scaleReading.connected &&
        scaleReading.tareState == TareState::Idle)
    {
        scaleReading.tareState = TareState::Pending;
        scaleReading.sequence++;

        Serial.println("[SCALE] TARE queued");
    }

    xSemaphoreGive(scaleMutex);
}

// -------------------- Background HTTP task --------------------

void scalePollingTask(void* parameter)
{
    uint32_t sequence = 0;
    uint8_t failedPolls = 0;

    bool lastKnownConnected = false;
    float lastKnownGrams = 0.0f;

    for (;;)
    {
        bool doTare = false;

        // Check for a pending TARE request.
        if (xSemaphoreTake(scaleMutex, portMAX_DELAY) == pdTRUE)
        {
            if (scaleReading.tareState == TareState::Pending)
            {
                scaleReading.tareState = TareState::Running;
                doTare = true;
            }

            xSemaphoreGive(scaleMutex);
        }

        // -------------------- Remote TARE --------------------

        if (doTare)
        {
            bool success = false;

            if (WiFi.status() == WL_CONNECTED)
            {
                HTTPClient http;
                http.setTimeout(5000);

                if (http.begin(SCALE_TARE_URL))
                {
                    const int statusCode = http.POST("");

                    Serial.printf(
                        "[SCALE] TARE HTTP status: %d\n",
                        statusCode
                    );

                    if (statusCode < 0)
                    {
                        Serial.printf(
                            "[SCALE] TARE HTTP error: %s\n",
                            http.errorToString(statusCode).c_str()
                        );
                    }

                    if (statusCode == HTTP_CODE_OK)
                    {
                        String response = http.getString();

                        JsonDocument doc;

                        if (!deserializeJson(doc, response))
                        {
                            success = doc["success"].as<bool>();

                            Serial.printf(
                                "[SCALE] TARE response success: %s\n",
                                success ? "true" : "false"
                            );
                        }
                    }

                    http.end();
                }
            }

            if (xSemaphoreTake(scaleMutex, portMAX_DELAY) == pdTRUE)
            {
                scaleReading.tareState =
                    success ? TareState::Success : TareState::Failed;

                scaleReading.sequence++;
                xSemaphoreGive(scaleMutex);
            }

            // Give the scale time to produce a fresh reading.
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        // -------------------- Weight polling --------------------

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

        // -------------------- Connection filtering --------------------

        if (connected)
        {
            failedPolls = 0;
            lastKnownConnected = true;
            lastKnownGrams = grams;
        }
        else
        {
            if (failedPolls < MAX_FAILED_POLLS)
                failedPolls++;

            if (failedPolls >= MAX_FAILED_POLLS)
                lastKnownConnected = false;
        }

        // Publish data without calling LVGL.
        if (xSemaphoreTake(scaleMutex, portMAX_DELAY) == pdTRUE)
        {
            scaleReading.grams = lastKnownGrams;
            scaleReading.connected = lastKnownConnected;
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
    static TareState lastTareState = TareState::Idle;

    static uint32_t tareMessageUntil = 0;
    static const char* tareMessage = nullptr;

    static char lastWeightText[32] = "";
    static bool lastButtonEnabled = false;

    ScaleReading reading;

    if (xSemaphoreTake(scaleMutex, 0) != pdTRUE)
        return;

    reading = scaleReading;
    xSemaphoreGive(scaleMutex);

    const uint32_t now = millis();

    // -------------------- New scale data --------------------

    if (reading.sequence != lastSequence)
    {
        lastSequence = reading.sequence;

        char weightText[32];

        if (reading.connected)
        {
            snprintf(
                weightText,
                sizeof(weightText),
                "%.1f g",
                reading.grams
            );
        }
        else
        {
            snprintf(
                weightText,
                sizeof(weightText),
                "-- g"
            );
        }

        // Update weight only when the displayed value changes.
        if (strcmp(lastWeightText, weightText) != 0)
        {
            if (ui_scale_weight != nullptr)
                lv_label_set_text(ui_scale_weight, weightText);

            strncpy(
                lastWeightText,
                weightText,
                sizeof(lastWeightText) - 1
            );

            lastWeightText[sizeof(lastWeightText) - 1] = '\0';
        }

        // -------------------- TARE button --------------------

        const bool tareBusy =
            reading.tareState == TareState::Pending ||
            reading.tareState == TareState::Running;

        const bool buttonEnabled =
            reading.connected && !tareBusy;

        if (buttonEnabled != lastButtonEnabled)
        {
            ui_scale_set_tare_enabled(buttonEnabled);
            lastButtonEnabled = buttonEnabled;
        }

        // -------------------- TARE state changes --------------------

        if (reading.tareState != lastTareState)
        {
            lastTareState = reading.tareState;

            switch (reading.tareState)
            {
                case TareState::Pending:
                case TareState::Running:
                    tareMessage = "Taring...";
                    tareMessageUntil = 0;
                    break;

                case TareState::Success:
                    tareMessage = "Tare complete";
                    tareMessageUntil =
                        now + TARE_MESSAGE_DURATION_MS;

                    Serial.println(
                        "[SCALE] Remote TARE successful"
                    );
                    break;

                case TareState::Failed:
                    tareMessage = "Tare failed";
                    tareMessageUntil =
                        now + TARE_MESSAGE_DURATION_MS;

                    Serial.println(
                        "[SCALE] Remote TARE failed"
                    );
                    break;

                case TareState::Idle:
                    break;
            }
        }

        // Acknowledge a completed TARE.
        if (reading.tareState == TareState::Success ||
            reading.tareState == TareState::Failed)
        {
            if (xSemaphoreTake(scaleMutex, 0) == pdTRUE)
            {
                if (scaleReading.tareState == reading.tareState)
                    scaleReading.tareState = TareState::Idle;

                xSemaphoreGive(scaleMutex);
            }
        }
    }

    // -------------------- Status priority --------------------

    const bool tareBusy =
        reading.tareState == TareState::Pending ||
        reading.tareState == TareState::Running;

    const char* desiredStatus = nullptr;

    if (tareBusy)
    {
        desiredStatus = "Taring...";
    }
    else if (tareMessage != nullptr &&
             (tareMessageUntil == 0 ||
              (int32_t)(tareMessageUntil - now) > 0))
    {
        desiredStatus = tareMessage;
    }
    else
    {
        tareMessage = nullptr;
        tareMessageUntil = 0;

        desiredStatus = reading.connected
            ? "Scale connected"
            : "Scale disconnected";
    }

    // Avoid repeatedly invalidating the LVGL status label.
    if (ui_scale_status != nullptr &&
        strcmp(
            lv_label_get_text(ui_scale_status),
            desiredStatus
        ) != 0)
    {
        ui_scale_set_tare_status(desiredStatus);
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

    ui_scale_set_tare_callback(requestScaleTare);

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
