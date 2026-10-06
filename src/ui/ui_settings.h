#pragma once

#include <lvgl.h>
#include "../pt/pt_display.h"


// ---------------------------------------------------------
// Brightness slider callback
// ---------------------------------------------------------

static void ui_settings_brightness_cb(lv_event_t *e)
{
    lv_obj_t *slider =
        (lv_obj_t *)lv_event_get_target(e);

    lv_obj_t *label =
        (lv_obj_t *)lv_event_get_user_data(e);

    int32_t brightness =
        lv_slider_get_value(slider);

    // Change the Panda Touch backlight
    pt_set_backlight(
        (uint8_t)brightness,
        true);

    // Update displayed percentage
    lv_label_set_text_fmt(
        label,
        "Brightness: %ld%%",
        (long)brightness);
}


// ---------------------------------------------------------
// Settings page
// ---------------------------------------------------------

inline lv_obj_t *ui_settings_create(lv_obj_t *parent)
{
    lv_obj_t *page = lv_obj_create(parent);

    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(page, lv_color_hex(0x101418), LV_PART_MAIN);
    lv_obj_set_style_border_width(page, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(page, 20, LV_PART_MAIN);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);


    // -----------------------------------------------------
    // Brightness title/value
    // -----------------------------------------------------

    lv_obj_t *brightness_label =
        lv_label_create(page);

    lv_label_set_text_fmt(
        brightness_label,
        "Brightness: %d%%",
        pt_backlight_percent);

    lv_obj_set_style_text_color(
        brightness_label,
        lv_color_white(),
        LV_PART_MAIN);

    lv_obj_align(
        brightness_label,
        LV_ALIGN_TOP_MID,
        0,
        50);


    // -----------------------------------------------------
    // Brightness slider
    // -----------------------------------------------------

    lv_obj_t *brightness_slider =
        lv_slider_create(page);

    lv_obj_set_width(
        brightness_slider,
        450);

    lv_slider_set_range(
        brightness_slider,
        20,
        100);

    lv_slider_set_value(
        brightness_slider,
        pt_backlight_percent,
        LV_ANIM_OFF);

    lv_obj_align(
        brightness_slider,
        LV_ALIGN_TOP_MID,
        0,
        100);

    lv_obj_add_event_cb(
        brightness_slider,
        ui_settings_brightness_cb,
        LV_EVENT_VALUE_CHANGED,
        brightness_label);


    return page;
}