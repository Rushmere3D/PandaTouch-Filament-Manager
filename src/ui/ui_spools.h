#pragma once

#include <lvgl.h>

// ---------------------------------------------------------
// Temporary spool data
// ---------------------------------------------------------
// For now this data is hard-coded.
//
// Later these records will be replaced by information
// downloaded from the Spoolman API.

struct UiSpool
{
    const char *vendor;
    const char *material;
    const char *color;
    int remaining_g;
};

static UiSpool demo_spools[] =
{
    {"eSUN",      "PLA+", "Black",  742},
    {"Polymaker", "PETG", "Orange", 326},
    {"Bambu",     "PLA",  "White",  918},
    {"Overture",  "ASA",  "Black",  481}
};


// ---------------------------------------------------------
// Create one row in the spool list
// ---------------------------------------------------------

static void ui_spools_create_row(
    lv_obj_t *parent,
    const UiSpool &spool)
{
    // Row container
    lv_obj_t *row = lv_obj_create(parent);

    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, 62);

    lv_obj_set_style_radius(row, 8, LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        row,
        lv_color_hex(0x20262C),
        LV_PART_MAIN);

    lv_obj_set_style_border_width(
        row,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_pad_left(row, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_right(row, 18, LV_PART_MAIN);

    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);


    // Vendor
    lv_obj_t *vendor = lv_label_create(row);

    lv_label_set_text(
        vendor,
        spool.vendor);

    lv_obj_set_style_text_color(
        vendor,
        lv_color_white(),
        LV_PART_MAIN);

    lv_obj_align(
        vendor,
        LV_ALIGN_LEFT_MID,
        0,
        0);


    // Material
    lv_obj_t *material = lv_label_create(row);

    lv_label_set_text(
        material,
        spool.material);

    lv_obj_set_style_text_color(
        material,
        lv_color_hex(0xB0B8C0),
        LV_PART_MAIN);

    lv_obj_align(
        material,
        LV_ALIGN_LEFT_MID,
        180,
        0);


    // Colour
    lv_obj_t *color = lv_label_create(row);

    lv_label_set_text(
        color,
        spool.color);

    lv_obj_set_style_text_color(
        color,
        lv_color_hex(0xB0B8C0),
        LV_PART_MAIN);

    lv_obj_align(
        color,
        LV_ALIGN_LEFT_MID,
        330,
        0);


    // Remaining weight
    lv_obj_t *weight = lv_label_create(row);

    lv_label_set_text_fmt(
        weight,
        "%d g",
        spool.remaining_g);

    lv_obj_set_style_text_color(
        weight,
        lv_color_white(),
        LV_PART_MAIN);

    lv_obj_align(
        weight,
        LV_ALIGN_RIGHT_MID,
        0,
        0);
}


// ---------------------------------------------------------
// Create complete Spools page
// ---------------------------------------------------------

inline lv_obj_t *ui_spools_create(lv_obj_t *parent)
{
    lv_obj_t *page = lv_obj_create(parent);

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
        15,
        LV_PART_MAIN);

    lv_obj_set_flex_flow(
        page,
        LV_FLEX_FLOW_COLUMN);

    lv_obj_set_style_pad_row(
        page,
        8,
        LV_PART_MAIN);


    // Add each temporary spool to the list
    for (const UiSpool &spool : demo_spools)
    {
        ui_spools_create_row(
            page,
            spool);
    }

    return page;
}