#pragma once

#include <WiFi.h>
#include "secrets.h"

inline bool wifi_connect(uint32_t timeout_ms = 15000)
{
    Serial.println();
    Serial.println("Starting Wi-Fi...");
    Serial.print("Connecting to: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start_time = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - start_time >= timeout_ms)
        {
            Serial.println();
            Serial.println("Wi-Fi connection timed out.");
            return false;
        }

        delay(250);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi connected!");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    return true;
}