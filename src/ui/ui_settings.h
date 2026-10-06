#pragma once

#include <lvgl.h>
#include <WiFi.h>
#include "../pt/pt_display.h"

static void ui_settings_brightness_cb(lv_event_t *e)
{
    lv_obj_t *slider =
        (lv_obj_t *)lv_event_get_target(e);

    lv_obj_t *label =
        (lv_obj_t *)lv_event_get_user_data(e);

    int32_t brightness =
        lv_slider_get_value(slider);

    pt_set_backlight(
        (uint8_t)brightness,
        true);

    lv_label_set_text_fmt(
        label,
        "Brightness: %ld%%",
        (long)brightness);
}

inline lv_obj_t *ui_settings_create(lv_obj_t *parent)
{
    lv_obj_t *page = lv_obj_create(parent);

    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));

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
        20,
        LV_PART_MAIN);

    lv_obj_clear_flag(
        page,
        LV_OBJ_FLAG_SCROLLABLE);

    // -------------------------------------------------
    // Wi-Fi information
    // -------------------------------------------------

    lv_obj_t *wifi_title =
        lv_label_create(page);

    lv_label_set_text(
        wifi_title,
        "Wi-Fi");

    lv_obj_set_style_text_color(
        wifi_title,
        lv_color_white(),
        LV_PART_MAIN);

    lv_obj_align(
        wifi_title,
        LV_ALIGN_TOP_LEFT,
        20,
        10);

    lv_obj_t *wifi_info =
        lv_label_create(page);

    lv_obj_set_style_text_color(
        wifi_info,
        lv_color_hex(0xB0B8C0),
        LV_PART_MAIN);

    if (WiFi.status() == WL_CONNECTED)
    {
        String ip =
            WiFi.localIP().toString();

        lv_label_set_text_fmt(
            wifi_info,
            "Status: Connected\n"
            "Network: %s\n"
            "IP Address: %s\n"
            "Signal: %d dBm",
            WiFi.SSID().c_str(),
            ip.c_str(),
            WiFi.RSSI());
    }
    else
    {
        lv_label_set_text(
            wifi_info,
            "Status: Disconnected");
    }

    lv_obj_align(
        wifi_info,
        LV_ALIGN_TOP_LEFT,
        20,
        45);

    // -------------------------------------------------
    // Brightness
    // -------------------------------------------------

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
        LV_ALIGN_TOP_LEFT,
        400,
        45);

    lv_obj_t *brightness_slider =
        lv_slider_create(page);

    lv_obj_set_width(
        brightness_slider,
        300);

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
        LV_ALIGN_TOP_LEFT,
        400,
        85);

    lv_obj_add_event_cb(
        brightness_slider,
        ui_settings_brightness_cb,
        LV_EVENT_VALUE_CHANGED,
        brightness_label);

    return page;
}