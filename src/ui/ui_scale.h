#pragma once

#include <lvgl.h>

// ---------------------------------------------------------
// Scale page
// ---------------------------------------------------------

inline lv_obj_t *ui_scale_create(lv_obj_t *parent)
{
    lv_obj_t *page = lv_obj_create(parent);

    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(page, lv_color_hex(0x101418), LV_PART_MAIN);
    lv_obj_set_style_border_width(page, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(page, 15, LV_PART_MAIN);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);


    // -----------------------------------------------------
    // Large weight display
    // -----------------------------------------------------

    lv_obj_t *weight = lv_label_create(page);

    // Dummy value for now
    lv_label_set_text(weight, "742 g");

    lv_obj_set_style_text_color(
        weight,
        lv_color_white(),
        LV_PART_MAIN);

    lv_obj_set_style_text_font(
        weight,
        &lv_font_montserrat_18,
        LV_PART_MAIN);

    lv_obj_align(
        weight,
        LV_ALIGN_TOP_MID,
        0,
        20);


    // -----------------------------------------------------
    // Stability indicator
    // -----------------------------------------------------

    lv_obj_t *stable = lv_label_create(page);

    lv_label_set_text(stable, "STABLE");

    lv_obj_set_style_text_color(
        stable,
        lv_color_hex(0x4CAF50),
        LV_PART_MAIN);

    lv_obj_align(
        stable,
        LV_ALIGN_TOP_MID,
        0,
        85);


    // -----------------------------------------------------
    // Selected spool
    // -----------------------------------------------------

    lv_obj_t *spool = lv_label_create(page);

    lv_label_set_text(
        spool,
        "Selected: eSUN PLA+ - Black");

    lv_obj_set_style_text_color(
        spool,
        lv_color_hex(0xB0B8C0),
        LV_PART_MAIN);

    lv_obj_align(
        spool,
        LV_ALIGN_TOP_MID,
        0,
        125);


    // -----------------------------------------------------
    // Tare button
    // -----------------------------------------------------

    lv_obj_t *tare_button = lv_button_create(page);

    lv_obj_set_size(tare_button, 180, 60);

    lv_obj_align(
        tare_button,
        LV_ALIGN_BOTTOM_MID,
        -110,
        -15);

    lv_obj_t *tare_label = lv_label_create(tare_button);

    lv_label_set_text(
        tare_label,
        "TARE");

    lv_obj_center(tare_label);


    // -----------------------------------------------------
    // Select spool button
    // -----------------------------------------------------

    lv_obj_t *select_button = lv_button_create(page);

    lv_obj_set_size(select_button, 180, 60);

    lv_obj_align(
        select_button,
        LV_ALIGN_BOTTOM_MID,
        110,
        -15);

    lv_obj_t *select_label = lv_label_create(select_button);

    lv_label_set_text(
        select_label,
        "SELECT SPOOL");

    lv_obj_center(select_label);


    return page;
}