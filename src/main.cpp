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
#include "network/spoolman_update.h"

const char* SCALE_URL = "http://192.168.1.222/api/weight";
const char* SCALE_TARE_URL = "http://192.168.1.222/api/tare";
constexpr uint32_t SCALE_POLL_INTERVAL_MS = 500;
constexpr uint32_t SCALE_HTTP_TIMEOUT_MS = 1500;
constexpr uint8_t MAX_FAILED_POLLS = 3;
constexpr uint32_t TARE_MESSAGE_DURATION_MS = 2000;
// Stage 4: a full two-second window of valid raw readings within +/-1 g.
constexpr uint32_t SCALE_STABLE_WINDOW_MS = 2000;
constexpr float SCALE_STABLE_TOLERANCE_G = 1.0f;
constexpr uint8_t SCALE_STABLE_SAMPLES = 5;

// Stage 3B: the network worker never calls LVGL or edits spool_store.
struct SpoolUpdateState {
    bool busy = false;
    bool finished = false;
    bool success = false;
    int spoolId = 0;
    float grams = 0;
    char message[160] = {};
};
static SpoolUpdateState spoolUpdate;
static SemaphoreHandle_t spoolUpdateMutex = nullptr;

void spoolUpdateTask(void *parameter)
{
    (void)parameter;
    int id;
    float grams;
    xSemaphoreTake(spoolUpdateMutex, portMAX_DELAY);
    id = spoolUpdate.spoolId;
    grams = spoolUpdate.grams;
    xSemaphoreGive(spoolUpdateMutex);

    String detail;
    bool success = spoolman_set_remaining_weight_verified(id, grams, detail);
    Serial.printf("[SPOOLMAN] Update spool #%d: %s - %s\n", id,
                  success ? "SUCCESS" : "FAILED", detail.c_str());

    xSemaphoreTake(spoolUpdateMutex, portMAX_DELAY);
    spoolUpdate.success = success;
    snprintf(spoolUpdate.message, sizeof(spoolUpdate.message), "%s", detail.c_str());
    spoolUpdate.finished = true;
    xSemaphoreGive(spoolUpdateMutex);
    vTaskDelete(nullptr);
}

void requestSpoolUpdate(int id, float grams)
{
    if (spoolUpdateMutex == nullptr || id <= 0 || !isfinite(grams) || grams < 0) return;
    xSemaphoreTake(spoolUpdateMutex, portMAX_DELAY);
    if (spoolUpdate.busy) { xSemaphoreGive(spoolUpdateMutex); return; }
    spoolUpdate.busy = true;
    spoolUpdate.finished = false;
    spoolUpdate.spoolId = id;
    spoolUpdate.grams = grams;
    xSemaphoreGive(spoolUpdateMutex);
    if (xTaskCreatePinnedToCore(spoolUpdateTask, "SpoolmanUpdate", 8192,
                                nullptr, 1, nullptr, 0) != pdPASS) {
        xSemaphoreTake(spoolUpdateMutex, portMAX_DELAY);
        spoolUpdate.finished = true;
        spoolUpdate.success = false;
        snprintf(spoolUpdate.message, sizeof(spoolUpdate.message), "Could not start Spoolman update task");
        xSemaphoreGive(spoolUpdateMutex);
    }
}

void processSpoolUpdateResult()
{
    if (spoolUpdateMutex == nullptr) return;
    SpoolUpdateState result;
    bool ready = false;
    if (xSemaphoreTake(spoolUpdateMutex, 0) == pdTRUE) {
        if (spoolUpdate.finished) {
            result = spoolUpdate;
            spoolUpdate.finished = false;
            spoolUpdate.busy = false;
            ready = true;
        }
        xSemaphoreGive(spoolUpdateMutex);
    }
    if (ready) {
        ui_scale_finish_update(result.spoolId, result.grams,
                               result.success, result.message);
        if (result.success) ui_spools_refresh_rows();
    }
}

enum class TareState : uint8_t { Idle, Pending, Running, Success, Failed };

struct ScaleReading {
    float grams;
    bool connected;
    bool stable;
    uint32_t sequence;
    TareState tareState;
};

ScaleReading scaleReading = {0.0f, false, false, 0, TareState::Idle};
SemaphoreHandle_t scaleMutex = nullptr;
TaskHandle_t scaleTaskHandle = nullptr;

void requestScaleTare()
{
    if (scaleMutex == nullptr) return;
    if (xSemaphoreTake(scaleMutex, 0) != pdTRUE) return;
    if (scaleReading.connected && scaleReading.tareState == TareState::Idle) {
        scaleReading.tareState = TareState::Pending;
        scaleReading.stable = false;
        scaleReading.sequence++;
        Serial.println("[SCALE] TARE queued");
    }
    xSemaphoreGive(scaleMutex);
}

void scalePollingTask(void* parameter)
{
    float stableWeights[SCALE_STABLE_SAMPLES] = {};
    uint32_t stableTimes[SCALE_STABLE_SAMPLES] = {};
    uint8_t stableCount = 0;
    uint8_t failedPolls = 0;
    bool lastKnownConnected = false;
    float lastKnownGrams = 0.0f;

    for (;;) {
        bool doTare = false;
        if (xSemaphoreTake(scaleMutex, portMAX_DELAY) == pdTRUE) {
            if (scaleReading.tareState == TareState::Pending) {
                scaleReading.tareState = TareState::Running;
                doTare = true;
            }
            xSemaphoreGive(scaleMutex);
        }

        if (doTare) {
            stableCount = 0;
            bool success = false;
            if (WiFi.status() == WL_CONNECTED) {
                HTTPClient http;
                http.setTimeout(5000);
                if (http.begin(SCALE_TARE_URL)) {
                    const int statusCode = http.POST("");
                    Serial.printf("[SCALE] TARE HTTP status: %d\n", statusCode);
                    if (statusCode < 0)
                        Serial.printf("[SCALE] TARE HTTP error: %s\n", http.errorToString(statusCode).c_str());
                    if (statusCode == HTTP_CODE_OK) {
                        String response = http.getString();
                        JsonDocument doc;
                        if (!deserializeJson(doc, response)) {
                            success = doc["success"].as<bool>();
                            Serial.printf("[SCALE] TARE response success: %s\n", success ? "true" : "false");
                        }
                    }
                    http.end();
                }
            }
            if (xSemaphoreTake(scaleMutex, portMAX_DELAY) == pdTRUE) {
                scaleReading.tareState = success ? TareState::Success : TareState::Failed;
                scaleReading.sequence++;
                xSemaphoreGive(scaleMutex);
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        float grams = 0.0f;
        bool connected = false;
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            http.setTimeout(SCALE_HTTP_TIMEOUT_MS);
            if (http.begin(SCALE_URL)) {
                const int statusCode = http.GET();
                if (statusCode == HTTP_CODE_OK) {
                    String response = http.getString();
                    JsonDocument doc;
                    DeserializationError error = deserializeJson(doc, response);
                    if (!error && doc["weight"].is<float>()) {
                        grams = doc["weight"].as<float>();
                        connected = isfinite(grams);
                    }
                }
                http.end();
            }
        }

        // Never reuse a previous measurement for stability after a failed poll.
        if (!connected || doTare) stableCount = 0;
        bool stable = false;
        if (connected && !doTare) {
            if (stableCount == SCALE_STABLE_SAMPLES) {
                for (uint8_t i = 1; i < SCALE_STABLE_SAMPLES; ++i) {
                    stableWeights[i - 1] = stableWeights[i];
                    stableTimes[i - 1] = stableTimes[i];
                }
                --stableCount;
            }
            stableWeights[stableCount] = grams;
            stableTimes[stableCount] = millis();
            ++stableCount;
            if (stableCount == SCALE_STABLE_SAMPLES &&
                (uint32_t)(stableTimes[stableCount - 1] - stableTimes[0]) >= SCALE_STABLE_WINDOW_MS) {
                float lo = stableWeights[0], hi = stableWeights[0];
                for (uint8_t i = 1; i < stableCount; ++i) {
                    if (stableWeights[i] < lo) lo = stableWeights[i];
                    if (stableWeights[i] > hi) hi = stableWeights[i];
                }
                stable = (hi - lo) <= (2.0f * SCALE_STABLE_TOLERANCE_G);
            }
        }
        if (connected) {
            failedPolls = 0;
            lastKnownConnected = true;
            lastKnownGrams = grams;
        } else {
            if (failedPolls < MAX_FAILED_POLLS) failedPolls++;
            if (failedPolls >= MAX_FAILED_POLLS) lastKnownConnected = false;
        }

        if (xSemaphoreTake(scaleMutex, portMAX_DELAY) == pdTRUE) {
            scaleReading.grams = lastKnownGrams;
            scaleReading.connected = lastKnownConnected;
            scaleReading.stable = lastKnownConnected && connected && stable &&
                                  scaleReading.tareState == TareState::Idle;
            ++scaleReading.sequence;
            xSemaphoreGive(scaleMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(SCALE_POLL_INTERVAL_MS));
    }
}

void updateScaleDisplay()
{
    static uint32_t lastSequence = 0;
    static TareState lastTareState = TareState::Idle;
    static uint32_t tareMessageUntil = 0;
    static const char* tareMessage = nullptr;
    static char lastWeightText[32] = "";
    static bool lastButtonEnabled = false;
    static bool lastConnected = false;
    static bool lastStable = false;

    ScaleReading reading;
    if (xSemaphoreTake(scaleMutex, 0) != pdTRUE) return;
    reading = scaleReading;
    xSemaphoreGive(scaleMutex);

    const uint32_t now = millis();
    if (reading.sequence != lastSequence) {
        lastSequence = reading.sequence;
        char weightText[32];
        if (reading.connected)
            snprintf(weightText, sizeof(weightText), "%.0f g", reading.grams);
        else
            snprintf(weightText, sizeof(weightText), "-- g");

        const bool weightChanged = strcmp(lastWeightText, weightText) != 0;
        const bool connectionChanged = reading.connected != lastConnected;
        if (weightChanged) {
            if (ui_scale_weight != nullptr)
                lv_label_set_text(ui_scale_weight, weightText);
            strncpy(lastWeightText, weightText, sizeof(lastWeightText) - 1);
            lastWeightText[sizeof(lastWeightText) - 1] = '\0';
        }
        const bool stabilityChanged = reading.stable != lastStable;
        if (weightChanged || connectionChanged || stabilityChanged) {
            // Stage 2: calculate net filament on the UI thread only.
            ui_scale_sync_calculation_from_display(reading.connected, reading.grams);
            ui_scale_set_stability(reading.stable);
            lastConnected = reading.connected;
            lastStable = reading.stable;
        }

        const bool tareBusy = (reading.tareState == TareState::Pending ||
                               reading.tareState == TareState::Running);
        const bool buttonEnabled = reading.connected && !tareBusy;
        if (buttonEnabled != lastButtonEnabled) {
            ui_scale_set_tare_enabled(buttonEnabled);
            lastButtonEnabled = buttonEnabled;
        }

        if (reading.tareState != lastTareState) {
            lastTareState = reading.tareState;
            switch (reading.tareState) {
                case TareState::Pending:
                case TareState::Running:
                    tareMessage = "Taring...";
                    tareMessageUntil = 0;
                    break;
                case TareState::Success:
                    tareMessage = "Tare complete";
                    tareMessageUntil = now + TARE_MESSAGE_DURATION_MS;
                    Serial.println("[SCALE] Remote TARE successful");
                    break;
                case TareState::Failed:
                    tareMessage = "Tare failed";
                    tareMessageUntil = now + TARE_MESSAGE_DURATION_MS;
                    Serial.println("[SCALE] Remote TARE failed");
                    break;
                case TareState::Idle:
                    break;
            }
        }

        if (reading.tareState == TareState::Success ||
            reading.tareState == TareState::Failed) {
            if (xSemaphoreTake(scaleMutex, 0) == pdTRUE) {
                if (scaleReading.tareState == reading.tareState)
                    scaleReading.tareState = TareState::Idle;
                xSemaphoreGive(scaleMutex);
            }
        }
    }

    const bool tareBusy = (reading.tareState == TareState::Pending ||
                           reading.tareState == TareState::Running);
    const char* desiredStatus = nullptr;
    if (tareBusy)
        desiredStatus = "Taring...";
    else if (tareMessage != nullptr &&
             (tareMessageUntil == 0 || (int32_t)(tareMessageUntil - now) > 0))
        desiredStatus = tareMessage;
    else {
        tareMessage = nullptr;
        tareMessageUntil = 0;
        desiredStatus = reading.connected ? "Scale connected" : "Scale disconnected";
    }
    if (ui_scale_status != nullptr &&
        strcmp(lv_label_get_text(ui_scale_status), desiredStatus) != 0)
        ui_scale_set_tare_status(desiredStatus);
}

void setup()
{
    Serial.begin(115200);
    unsigned long start = millis();
    while (!Serial && millis() - start < 3000) delay(10);
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
    ui_scale_set_tare_callback(requestScaleTare);
    spoolUpdateMutex = xSemaphoreCreateMutex();
    if (spoolUpdateMutex) ui_scale_set_update_callback(requestSpoolUpdate);
    scaleMutex = xSemaphoreCreateMutex();
    if (scaleMutex == nullptr) {
        Serial.println("[SCALE] Failed to create mutex");
        ui_scale_set_disconnected();
    } else {
        BaseType_t result = xTaskCreatePinnedToCore(
            scalePollingTask, "ScalePolling", 8192, nullptr, 1, &scaleTaskHandle, 0);
        if (result == pdPASS)
            Serial.println("[SCALE] Background polling started");
        else {
            Serial.println("[SCALE] Failed to start background task");
            ui_scale_set_disconnected();
        }
    }
    Serial.println("[STARTUP] Setup complete");
}

void loop()
{
    pt_loop_display();
    processSpoolUpdateResult();
    if (scaleMutex != nullptr && scaleTaskHandle != nullptr)
        updateScaleDisplay();
}
