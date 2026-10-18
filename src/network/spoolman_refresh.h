#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include "../models/spool_store.h"

// Called only in the background worker. Never touches LVGL or the live spool store.
inline bool spoolman_fetch_inventory_snapshot(SpoolInfo *items, size_t &count, String &error) {
    count = 0;
    if (!items) { error = "No inventory buffer"; return false; }
    if (WiFi.status() != WL_CONNECTED) { error = "Wi-Fi disconnected"; return false; }
    HTTPClient http;
    http.setTimeout(6000);
    String url = String(SPOOLMAN_URL) + "/api/v1/spool";
    if (!http.begin(url)) { error = "Could not open connection"; return false; }
    const int status = http.GET();
    if (status != HTTP_CODE_OK) {
        error = String("Spoolman HTTP ") + String(status);
        http.end();
        return false;
    }
    // Parse the response before altering the live inventory.
    JsonDocument doc;
    const DeserializationError parseError = deserializeJson(doc, http.getStream());
    http.end();
    if (parseError) { error = String("JSON error: ") + parseError.c_str(); return false; }
    if (!doc.is<JsonArray>()) { error = "Unexpected inventory response"; return false; }
    JsonArray array = doc.as<JsonArray>();
    if (array.size() > MAX_SPOOLS) { error = "Inventory exceeds MAX_SPOOLS"; return false; }
    for (JsonObject spool : array) {
        SpoolInfo &info = items[count];
        info.id = spool["id"] | 0;
        if (info.id <= 0) { error = "Invalid spool ID"; return false; }
        info.vendor = spool["filament"]["vendor"]["name"] | "";
        info.name = spool["filament"]["name"] | "";
        info.material = spool["filament"]["material"] | "";
        info.color_hex = spool["filament"]["color_hex"] | "";
        info.multi_color_hexes = spool["filament"]["multi_color_hexes"] | "";
        info.multi_color_direction = spool["filament"]["multi_color_direction"] | "";
        info.remaining_weight = spool["remaining_weight"] | 0.0f;
        info.initial_weight = spool["initial_weight"] | 0.0f;
        info.spool_weight = spool["spool_weight"] | 0.0f;
        info.location = spool["location"] | "";
        info.archived = spool["archived"] | false;
        ++count;
    }
    return true;
}
