#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>
#include "secrets.h"

// PATCH sets remaining_weight directly; GET confirms the server persisted it.
inline bool spoolman_set_remaining_weight_verified(int spool_id, float grams, String &detail)
{
    if (spool_id <= 0 || !isfinite(grams) || grams < 0.0f) {
        detail = "Invalid spool ID or measured weight"; return false;
    }
    if (WiFi.status() != WL_CONNECTED) {
        detail = "Wi-Fi is disconnected"; return false;
    }
    const String url = String(SPOOLMAN_URL) + "/api/v1/spool/" + String(spool_id);
    HTTPClient http;
    http.setTimeout(5000);
    if (!http.begin(url)) { detail = "Could not open Spoolman connection"; return false; }
    http.addHeader("Content-Type", "application/json");
    JsonDocument body;
    body["remaining_weight"] = floorf(grams + 0.5f);
    String payload;
    serializeJson(body, payload);
    int status = http.sendRequest("PATCH", payload);
    String response = http.getString();
    http.end();
    if (status < 200 || status >= 300) {
        detail = "Spoolman rejected update (HTTP " + String(status) + "): " + response.substring(0, 85);
        return false;
    }
    if (!http.begin(url)) { detail = "Update sent, but verification connection failed"; return false; }
    http.setTimeout(5000);
    status = http.GET();
    response = http.getString();
    http.end();
    if (status != 200) {
        detail = "Update sent; could not verify saved value (HTTP " + String(status) + ")";
        return false;
    }
    JsonDocument check;
    if (deserializeJson(check, response)) {
        detail = "Update sent; could not parse verification response";
        return false;
    }
    float saved = check["remaining_weight"] | -1.0f;
    if (!isfinite(saved) || fabsf(saved - grams) > 0.11f) {
        detail = "Update sent; saved value does not match measurement";
        return false;
    }
    detail = "Spool #" + String(spool_id) + " updated and verified: " + String(saved, 0) + " g";
    return true;
}
