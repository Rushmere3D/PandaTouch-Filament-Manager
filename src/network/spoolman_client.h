#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

inline bool spoolman_test_connection()
{
    // Make sure Wi-Fi is available first
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println();
        Serial.println("Spoolman test failed: Wi-Fi is not connected.");
        return false;
    }

    String url = String(SPOOLMAN_URL) + "/api/v1/info";

    Serial.println();
    Serial.println("Testing Spoolman connection...");
    Serial.print("URL: ");
    Serial.println(url);

    HTTPClient http;

    http.begin(url);

    int http_code = http.GET();

    if (http_code > 0)
    {
        Serial.print("HTTP response code: ");
        Serial.println(http_code);

        String response = http.getString();

        Serial.println("Spoolman response:");
        Serial.println(response);

        http.end();

        return http_code == 200;
    }
    else
    {
        Serial.print("HTTP request failed: ");
        Serial.println(http.errorToString(http_code));

        http.end();

        return false;
    }
}