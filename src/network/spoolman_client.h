#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include "../models/spool_store.h"


inline bool spoolman_test_connection()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println();
        Serial.println("Spoolman test failed: Wi-Fi is not connected.");
        return false;
    }

    String url =
        String(SPOOLMAN_URL) + "/api/v1/info";

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


inline bool spoolman_test_spool_list()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println();
        Serial.println("Spool list failed: Wi-Fi is not connected.");
        return false;
    }

    String url =
        String(SPOOLMAN_URL) + "/api/v1/spool";

    Serial.println();
    Serial.println("Requesting Spoolman spool list...");
    Serial.print("URL: ");
    Serial.println(url);

    HTTPClient http;

    http.begin(url);

    int http_code = http.GET();

    Serial.print("HTTP response code: ");
    Serial.println(http_code);

    if (http_code != 200)
    {
        Serial.print("HTTP request failed: ");
        Serial.println(http.errorToString(http_code));

        http.end();
        return false;
    }

    String response = http.getString();

    Serial.print("Response length: ");
    Serial.print(response.length());
    Serial.println(" bytes");

    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, response);

    if (error)
    {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());

        http.end();
        return false;
    }

    spool_store_clear();
    
    JsonArray spools =
        doc.as<JsonArray>();

    Serial.println();
    Serial.print("Spools received: ");
    Serial.println(spools.size());

    Serial.println();
    Serial.println("==============================");
    Serial.println("SPOOLMAN INVENTORY");
    Serial.println("==============================");

    for (JsonObject spool : spools)
    {
        SpoolInfo info;

        info.id =
            spool["id"] | 0;

        info.vendor =
            spool["filament"]["vendor"]["name"] | "";

        info.name =
            spool["filament"]["name"] | "";

        info.material =
            spool["filament"]["material"] | "";

        info.color_hex =
            spool["filament"]["color_hex"] | "";

        info.multi_color_hexes =
            spool["filament"]["multi_color_hexes"] | "";

        info.multi_color_direction =
        spool["filament"]["multi_color_direction"] | "";

        info.remaining_weight =
            spool["remaining_weight"] | 0.0f;

        info.initial_weight =
            spool["initial_weight"] | 0.0f;

        info.spool_weight =
            spool["spool_weight"] | 0.0f;

        info.location =
            spool["location"] | "";

        info.archived =
            spool["archived"] | false;

        // Save this spool for the application
        if (!spool_store_add(info))
        {
            Serial.println("WARNING: Spool store is full!");
            break;
        }
        
        // Print it for debugging
        Serial.println();

        Serial.print("Spool ID: ");
        Serial.println(info.id);

        Serial.print("Vendor: ");
        Serial.println(info.vendor);

        Serial.print("Name: ");
        Serial.println(info.name);

        Serial.print("Material: ");
        Serial.println(info.material);

        Serial.print("Remaining: ");
        Serial.print(info.remaining_weight, 1);
        Serial.println(" g");

        Serial.print("Initial: ");
        Serial.print(info.initial_weight, 1);
        Serial.println(" g");

        Serial.print("Empty spool: ");
        Serial.print(info.spool_weight, 1);
        Serial.println(" g");

        Serial.print("Colour: ");

        if (info.color_hex.length() > 0)
        {
            Serial.print("#");
            Serial.println(info.color_hex);
        }
        else if (info.multi_color_hexes.length() > 0)
        {
            Serial.println(info.multi_color_hexes);
        }
        else
        {
            Serial.println("Not specified");
        }

        Serial.print("Location: ");
        Serial.println(info.location);

        Serial.print("Archived: ");
        Serial.println(
            info.archived ? "Yes" : "No");

        Serial.println("------------------------------");
    }

    Serial.println();
    Serial.print("Spools stored in memory: ");
    Serial.println(spool_count);

    http.end();

    return true;
}