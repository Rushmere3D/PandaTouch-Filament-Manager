#pragma once

#include <lvgl.h>
#include <stdlib.h>
#include <stdio.h>
#include "../models/spool_store.h"


// ============================================================
// Spool browser state
// ============================================================

constexpr size_t SPOOL_ROWS_VISIBLE = 4;

static size_t ui_spool_first_index = 0;

static lv_obj_t *ui_spool_rows[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_swatches[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_vendors[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_materials[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_names[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_weights[SPOOL_ROWS_VISIBLE];

static lv_obj_t *ui_spool_page_label = nullptr;


// ============================================================
// Convert Spoolman colour into Panda Touch display colour
//
// Spoolman:
//   RRGGBB
//   RRGGBBAA
//
// Panda Touch display path swaps red and blue, so compensate
// here.
// ============================================================

inline lv_color_t ui_spool_hex_to_color(const String &hex)
{
    String clean_hex = hex;

    if (clean_hex.startsWith("#"))
    {
        clean_hex.remove(0, 1);
    }

    if (clean_hex.length() < 6)
    {
        return lv_color_hex(0x808080);
    }

    String rgb_hex =
        clean_hex.substring(0, 6);

    uint32_t value =
        strtoul(rgb_hex.c_str(), nullptr, 16);

    uint8_t red =
        (value >> 16) & 0xFF;

    uint8_t green =
        (value >> 8) & 0xFF;

    uint8_t blue =
        value & 0xFF;

    uint32_t panda_color =
        ((uint32_t)blue << 16) |
        ((uint32_t)green << 8) |
        red;

    return lv_color_hex(panda_color);
}


// ============================================================
// Extract alpha from RRGGBBAA
// ============================================================

inline uint8_t ui_spool_hex_alpha(const String &hex)
{
    String clean_hex = hex;

    if (clean_hex.startsWith("#"))
    {
        clean_hex.remove(0, 1);
    }

    if (clean_hex.length() < 8)
    {
        return 255;
    }

    String alpha_hex =
        clean_hex.substring(6, 8);

    return (uint8_t)strtoul(
        alpha_hex.c_str(),
        nullptr,
        16);
}


// ============================================================
// Extract first two colours from multi_color_hexes
//
// Example:
//   f431e4,9831e7
// ============================================================

inline bool ui_spool_get_multi_colors(
    const String &multi,
    String &colour1,
    String &colour2)
{
    int comma =
        multi.indexOf(',');

    if (comma < 0)
    {
        return false;
    }

    colour1 =
        multi.substring(0, comma);

    int second_comma =
        multi.indexOf(',', comma + 1);

    if (second_comma < 0)
    {
        colour2 =
            multi.substring(comma + 1);
    }
    else
    {
        colour2 =
            multi.substring(
                comma + 1,
                second_comma);
    }

    colour1.trim();
    colour2.trim();

    return (
        colour1.length() >= 6 &&
        colour2.length() >= 6);
}


// ============================================================
// Show normal single-colour swatch
// ============================================================

inline void ui_spool_show_single_color(
    size_t row,
    const String &hex)
{
    // IMPORTANT:
    // Remove any gradient left behind when this reusable row
    // previously displayed a multicolour spool.
    lv_obj_set_style_bg_grad_dir(
        ui_spool_swatches[row],
        LV_GRAD_DIR_NONE,
        LV_PART_MAIN);


    if (hex.length() >= 6)
    {
        lv_obj_set_style_bg_color(
            ui_spool_swatches[row],
            ui_spool_hex_to_color(hex),
            LV_PART_MAIN);

        uint8_t alpha =
            ui_spool_hex_alpha(hex);


        if (alpha < 255)
        {
            // Transparent / translucent filament
            lv_obj_set_style_border_width(
                ui_spool_swatches[row],
                3,
                LV_PART_MAIN);

            lv_obj_set_style_border_color(
                ui_spool_swatches[row],
                lv_color_white(),
                LV_PART_MAIN);
        }
        else
        {
            // Normal opaque filament
            lv_obj_set_style_border_width(
                ui_spool_swatches[row],
                1,
                LV_PART_MAIN);

            lv_obj_set_style_border_color(
                ui_spool_swatches[row],
                lv_color_hex(0x606870),
                LV_PART_MAIN);
        }
    }
    else
    {
        // Unknown colour
        lv_obj_set_style_bg_color(
            ui_spool_swatches[row],
            lv_color_hex(0x808080),
            LV_PART_MAIN);

        lv_obj_set_style_border_width(
            ui_spool_swatches[row],
            1,
            LV_PART_MAIN);

        lv_obj_set_style_border_color(
            ui_spool_swatches[row],
            lv_color_hex(0x606870),
            LV_PART_MAIN);
    }
}


// ============================================================
// Show two-colour swatch
//
// Uses the swatch's own background gradient rather than child
// objects. Because the swatch itself is circular, the colours
// stay inside the circle.
// ============================================================

inline void ui_spool_show_multi_color(
    size_t row,
    const String &colour1,
    const String &colour2)
{
    lv_color_t first =
        ui_spool_hex_to_color(colour1);

    lv_color_t second =
        ui_spool_hex_to_color(colour2);


    // First half colour
    lv_obj_set_style_bg_color(
        ui_spool_swatches[row],
        first,
        LV_PART_MAIN);


    // Second half colour
    lv_obj_set_style_bg_grad_color(
        ui_spool_swatches[row],
        second,
        LV_PART_MAIN);


    // Horizontal: left -> right
    lv_obj_set_style_bg_grad_dir(
        ui_spool_swatches[row],
        LV_GRAD_DIR_HOR,
        LV_PART_MAIN);


    // Keep colour 1 until approximately halfway.
    lv_obj_set_style_bg_main_stop(
        ui_spool_swatches[row],
        127,
        LV_PART_MAIN);


    // Start colour 2 immediately after halfway.
    //
    // Keeping these stops adjacent gives us a nearly hard
    // division rather than a broad blended gradient.
    lv_obj_set_style_bg_grad_stop(
        ui_spool_swatches[row],
        128,
        LV_PART_MAIN);


    lv_obj_set_style_border_width(
        ui_spool_swatches[row],
        1,
        LV_PART_MAIN);

    lv_obj_set_style_border_color(
        ui_spool_swatches[row],
        lv_color_hex(0x808890),
        LV_PART_MAIN);
}


// ============================================================
// Refresh four reusable rows
// ============================================================

inline void ui_spools_refresh_rows()
{
    for (size_t row = 0;
         row < SPOOL_ROWS_VISIBLE;
         row++)
    {
        size_t spool_index =
            ui_spool_first_index + row;


        if (spool_index >= spool_count)
        {
            lv_obj_add_flag(
                ui_spool_rows[row],
                LV_OBJ_FLAG_HIDDEN);

            continue;
        }


        const SpoolInfo &spool =
            spool_store[spool_index];


        lv_obj_remove_flag(
            ui_spool_rows[row],
            LV_OBJ_FLAG_HIDDEN);


        // ----------------------------------------------------
        // Colour swatch
        // ----------------------------------------------------

        String colour1;
        String colour2;

        if (
            spool.multi_color_hexes.length() > 0 &&
            ui_spool_get_multi_colors(
                spool.multi_color_hexes,
                colour1,
                colour2))
        {
            ui_spool_show_multi_color(
                row,
                colour1,
                colour2);
        }
        else
        {
            ui_spool_show_single_color(
                row,
                spool.color_hex);
        }


        // ----------------------------------------------------
        // Text
        // ----------------------------------------------------

        lv_label_set_text(
            ui_spool_vendors[row],
            spool.vendor.c_str());

        lv_label_set_text(
            ui_spool_materials[row],
            spool.material.c_str());

        lv_label_set_text(
            ui_spool_names[row],
            spool.name.c_str());


        // ----------------------------------------------------
        // Remaining weight
        // ----------------------------------------------------

        char weight_text[24];

        snprintf(
            weight_text,
            sizeof(weight_text),
            "%d g",
            (int)(spool.remaining_weight + 0.5f));

        lv_label_set_text(
            ui_spool_weights[row],
            weight_text);
    }


    // ========================================================
    // Page indicator
    // ========================================================

    if (ui_spool_page_label != nullptr)
    {
        char page_text[40];

        if (spool_count == 0)
        {
            snprintf(
                page_text,
                sizeof(page_text),
                "0 spools");
        }
        else
        {
            size_t first =
                ui_spool_first_index + 1;

            size_t last =
                ui_spool_first_index +
                SPOOL_ROWS_VISIBLE;

            if (last > spool_count)
            {
                last = spool_count;
            }

            snprintf(
                page_text,
                sizeof(page_text),
                "%u-%u of %u",
                (unsigned int)first,
                (unsigned int)last,
                (unsigned int)spool_count);
        }

        lv_label_set_text(
            ui_spool_page_label,
            page_text);
    }
}


// ============================================================
// Previous page
// ============================================================

inline void ui_spools_previous_cb(lv_event_t *e)
{
    if (ui_spool_first_index >= SPOOL_ROWS_VISIBLE)
    {
        ui_spool_first_index -=
            SPOOL_ROWS_VISIBLE;
    }
    else
    {
        ui_spool_first_index = 0;
    }

    ui_spools_refresh_rows();
}


// ============================================================
// Next page
// ============================================================

inline void ui_spools_next_cb(lv_event_t *e)
{
    if (
        ui_spool_first_index +
        SPOOL_ROWS_VISIBLE <
        spool_count)
    {
        ui_spool_first_index +=
            SPOOL_ROWS_VISIBLE;

        ui_spools_refresh_rows();
    }
}


// ============================================================
// Create Spools page
// ============================================================

inline lv_obj_t *ui_spools_create(lv_obj_t *parent)
{
    // ========================================================
    // Main page
    // ========================================================

    lv_obj_t *page =
        lv_obj_create(parent);

    lv_obj_set_size(
        page,
        LV_PCT(100),
        LV_PCT(100));

    lv_obj_set_style_bg_color(
        page,
        lv_color_hex(0x101418),
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        page,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_pad_all(
        page,
        10,
        LV_PART_MAIN);

    lv_obj_clear_flag(
        page,
        LV_OBJ_FLAG_SCROLLABLE);


    // ========================================================
    // Page position
    // ========================================================

    ui_spool_page_label =
        lv_label_create(page);

    lv_obj_set_style_text_color(
        ui_spool_page_label,
        lv_color_hex(0xA0A8B0),
        LV_PART_MAIN);

    lv_obj_align(
        ui_spool_page_label,
        LV_ALIGN_TOP_RIGHT,
        -10,
        3);


    // ========================================================
    // List area
    // ========================================================

    lv_obj_t *list =
        lv_obj_create(page);

    lv_obj_set_size(
        list,
        LV_PCT(100),
        220);

    lv_obj_align(
        list,
        LV_ALIGN_TOP_MID,
        0,
        25);

    lv_obj_set_style_bg_color(
        list,
        lv_color_hex(0x101418),
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        list,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_pad_all(
        list,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_pad_row(
        list,
        4,
        LV_PART_MAIN);

    lv_obj_set_flex_flow(
        list,
        LV_FLEX_FLOW_COLUMN);

    lv_obj_clear_flag(
        list,
        LV_OBJ_FLAG_SCROLLABLE);


    // ========================================================
    // Create four reusable rows
    // ========================================================

    for (size_t row = 0;
         row < SPOOL_ROWS_VISIBLE;
         row++)
    {
        // ----------------------------------------------------
        // Row
        // ----------------------------------------------------

        ui_spool_rows[row] =
            lv_obj_create(list);

        lv_obj_set_width(
            ui_spool_rows[row],
            LV_PCT(100));

        lv_obj_set_height(
            ui_spool_rows[row],
            48);

        lv_obj_set_style_bg_color(
            ui_spool_rows[row],
            lv_color_hex(0x1A2026),
            LV_PART_MAIN);

        lv_obj_set_style_border_width(
            ui_spool_rows[row],
            0,
            LV_PART_MAIN);

        lv_obj_set_style_radius(
            ui_spool_rows[row],
            6,
            LV_PART_MAIN);

        lv_obj_set_style_pad_all(
            ui_spool_rows[row],
            7,
            LV_PART_MAIN);

        lv_obj_clear_flag(
            ui_spool_rows[row],
            LV_OBJ_FLAG_SCROLLABLE);


        // ----------------------------------------------------
        // Colour swatch
        // ----------------------------------------------------

        ui_spool_swatches[row] =
            lv_obj_create(
                ui_spool_rows[row]);

        lv_obj_set_size(
            ui_spool_swatches[row],
            30,
            30);

        lv_obj_align(
            ui_spool_swatches[row],
            LV_ALIGN_LEFT_MID,
            0,
            0);

        lv_obj_set_style_pad_all(
            ui_spool_swatches[row],
            0,
            LV_PART_MAIN);

        lv_obj_set_style_border_width(
            ui_spool_swatches[row],
            1,
            LV_PART_MAIN);

        lv_obj_set_style_border_color(
            ui_spool_swatches[row],
            lv_color_hex(0x606870),
            LV_PART_MAIN);

        lv_obj_set_style_radius(
            ui_spool_swatches[row],
            LV_RADIUS_CIRCLE,
            LV_PART_MAIN);

        lv_obj_clear_flag(
            ui_spool_swatches[row],
            LV_OBJ_FLAG_SCROLLABLE);


        // ----------------------------------------------------
        // Vendor
        // ----------------------------------------------------

        ui_spool_vendors[row] =
            lv_label_create(
                ui_spool_rows[row]);

        lv_obj_set_width(
            ui_spool_vendors[row],
            140);

        lv_label_set_long_mode(
            ui_spool_vendors[row],
            LV_LABEL_LONG_DOT);

        lv_obj_set_style_text_color(
            ui_spool_vendors[row],
            lv_color_white(),
            LV_PART_MAIN);

        lv_obj_align(
            ui_spool_vendors[row],
            LV_ALIGN_LEFT_MID,
            50,
            0);


        // ----------------------------------------------------
        // Material
        // ----------------------------------------------------

        ui_spool_materials[row] =
            lv_label_create(
                ui_spool_rows[row]);

        lv_obj_set_width(
            ui_spool_materials[row],
            80);

        lv_label_set_long_mode(
            ui_spool_materials[row],
            LV_LABEL_LONG_DOT);

        lv_obj_set_style_text_color(
            ui_spool_materials[row],
            lv_color_hex(0xAAB2BA),
            LV_PART_MAIN);

        lv_obj_align(
            ui_spool_materials[row],
            LV_ALIGN_LEFT_MID,
            200,
            0);


        // ----------------------------------------------------
        // Filament name
        // ----------------------------------------------------

        ui_spool_names[row] =
            lv_label_create(
                ui_spool_rows[row]);

        lv_obj_set_width(
            ui_spool_names[row],
            235);

        lv_label_set_long_mode(
            ui_spool_names[row],
            LV_LABEL_LONG_DOT);

        lv_obj_set_style_text_color(
            ui_spool_names[row],
            lv_color_white(),
            LV_PART_MAIN);

        lv_obj_align(
            ui_spool_names[row],
            LV_ALIGN_LEFT_MID,
            290,
            0);


        // ----------------------------------------------------
        // Remaining weight
        // ----------------------------------------------------

        ui_spool_weights[row] =
            lv_label_create(
                ui_spool_rows[row]);

        lv_obj_set_width(
            ui_spool_weights[row],
            95);

        lv_obj_set_style_text_align(
            ui_spool_weights[row],
            LV_TEXT_ALIGN_RIGHT,
            LV_PART_MAIN);

        lv_obj_set_style_text_color(
            ui_spool_weights[row],
            lv_color_white(),
            LV_PART_MAIN);

        lv_obj_align(
            ui_spool_weights[row],
            LV_ALIGN_RIGHT_MID,
            -5,
            0);
    }


    // ========================================================
    // Previous
    // ========================================================

    lv_obj_t *previous_button =
        lv_button_create(page);

    lv_obj_set_size(
        previous_button,
        130,
        42);

    lv_obj_align(
        previous_button,
        LV_ALIGN_BOTTOM_LEFT,
        10,
        0);

    lv_obj_add_event_cb(
        previous_button,
        ui_spools_previous_cb,
        LV_EVENT_CLICKED,
        nullptr);

    lv_obj_t *previous_label =
        lv_label_create(previous_button);

    lv_label_set_text(
        previous_label,
        "< PREV");

    lv_obj_center(previous_label);


    // ========================================================
    // Next
    // ========================================================

    lv_obj_t *next_button =
        lv_button_create(page);

    lv_obj_set_size(
        next_button,
        130,
        42);

    lv_obj_align(
        next_button,
        LV_ALIGN_BOTTOM_RIGHT,
        -10,
        0);

    lv_obj_add_event_cb(
        next_button,
        ui_spools_next_cb,
        LV_EVENT_CLICKED,
        nullptr);

    lv_obj_t *next_label =
        lv_label_create(next_button);

    lv_label_set_text(
        next_label,
        "NEXT >");

    lv_obj_center(next_label);


    // ========================================================
    // Initial display
    // ========================================================

    ui_spool_first_index = 0;

    ui_spools_refresh_rows();

    return page;
}