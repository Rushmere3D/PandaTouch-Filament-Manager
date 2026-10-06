#pragma once

#include <Arduino.h>

struct SpoolInfo
{
    int id = 0;

    String vendor;
    String name;
    String material;

    String color_hex;
    String multi_color_hexes;
    String multi_color_direction;

    float remaining_weight = 0.0f;
    float initial_weight = 0.0f;
    float spool_weight = 0.0f;

    String location;

    bool archived = false;
};