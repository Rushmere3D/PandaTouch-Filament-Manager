#pragma once

#include <lvgl.h>
#include <WiFi.h>
#include "../pt/pt_display.h"

// The Panda Touch display swaps red and blue in the current display pipeline.
// Use a BGR-swapped value for the intended on-screen blue accent.
static inline lv_color_t ui_settings_accent_blue()
{
    return lv_color_hex(0xD27619); // displays as #1976D2
}

static inline void ui_settings_style_slider(lv_obj_t *slider)
{
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x394650), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, ui_settings_accent_blue(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, ui_settings_accent_blue(), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_KNOB);
}

static inline void ui_settings_style_switch(lv_obj_t *sw)
{
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x394650), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, ui_settings_accent_blue(), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x252D34), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(sw, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, LV_PART_KNOB);
}

static void ui_settings_brightness_cb(lv_event_t *e)
{
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    int32_t brightness = lv_slider_get_value(slider);
    pt_set_backlight((uint8_t)brightness, true);
    pt_screen_note_activity();
    lv_label_set_text_fmt(label, "Brightness: %ld%%", (long)brightness);
}

static void ui_settings_dimming_cb(lv_event_t *e)
{
    lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);
    pt_screen_set_dimming_enabled(lv_obj_has_state(sw, LV_STATE_CHECKED));
}

static void ui_settings_dim_timeout_cb(lv_event_t *e)
{
    lv_obj_t *dropdown = (lv_obj_t *)lv_event_get_target(e);
    uint32_t selected = lv_dropdown_get_selected(dropdown);
    const uint16_t minutes[] = {1, 2, 5, 10, 30};
    if (selected < 5)
        pt_screen_set_dim_timeout_minutes(minutes[selected]);
}

static void ui_settings_dim_brightness_cb(lv_event_t *e)
{
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    int32_t brightness = lv_slider_get_value(slider);
    pt_screen_set_dim_percent((uint8_t)brightness);
    lv_label_set_text_fmt(label, "Dim brightness: %ld%%", (long)brightness);
}

inline lv_obj_t *ui_settings_create(lv_obj_t *parent)
{
    lv_obj_t *page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(page, lv_color_hex(0x101418), LV_PART_MAIN);
    lv_obj_set_style_border_width(page, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(page, 20, LV_PART_MAIN);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *wifi_title = lv_label_create(page);
    lv_label_set_text(wifi_title, "Wi-Fi");
    lv_obj_set_style_text_color(wifi_title, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(wifi_title, LV_ALIGN_TOP_LEFT, 20, 10);

    lv_obj_t *wifi_info = lv_label_create(page);
    lv_obj_set_style_text_color(wifi_info, lv_color_hex(0xB0B8C0), LV_PART_MAIN);
    if (WiFi.status() == WL_CONNECTED)
    {
        String ip = WiFi.localIP().toString();
        lv_label_set_text_fmt(wifi_info,
            "Status: Connected\nNetwork: %s\nIP Address: %s\nSignal: %d dBm",
            WiFi.SSID().c_str(), ip.c_str(), WiFi.RSSI());
    }
    else
    {
        lv_label_set_text(wifi_info, "Status: Disconnected");
    }
    lv_obj_align(wifi_info, LV_ALIGN_TOP_LEFT, 20, 45);

    lv_obj_t *display_title = lv_label_create(page);
    lv_label_set_text(display_title, "Display");
    lv_obj_set_style_text_color(display_title, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(display_title, LV_ALIGN_TOP_LEFT, 400, 10);

    lv_obj_t *brightness_label = lv_label_create(page);
    lv_label_set_text_fmt(brightness_label, "Brightness: %d%%", pt_backlight_percent);
    lv_obj_set_style_text_color(brightness_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(brightness_label, LV_ALIGN_TOP_LEFT, 400, 42);

    lv_obj_t *brightness_slider = lv_slider_create(page);
    lv_obj_set_width(brightness_slider, 300);
    lv_slider_set_range(brightness_slider, 20, 100);
    lv_slider_set_value(brightness_slider, pt_backlight_percent, LV_ANIM_OFF);
    lv_obj_align(brightness_slider, LV_ALIGN_TOP_LEFT, 400, 68);
    ui_settings_style_slider(brightness_slider);
    lv_obj_add_event_cb(brightness_slider, ui_settings_brightness_cb,
                        LV_EVENT_VALUE_CHANGED, brightness_label);

    lv_obj_t *dimming_label = lv_label_create(page);
    lv_label_set_text(dimming_label, "Screen dimming");
    lv_obj_set_style_text_color(dimming_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(dimming_label, LV_ALIGN_TOP_LEFT, 400, 105);

    lv_obj_t *dimming_switch = lv_switch_create(page);
    lv_obj_align(dimming_switch, LV_ALIGN_TOP_LEFT, 640, 98);
    ui_settings_style_switch(dimming_switch);
    if (pt_screen_dimming_enabled)
        lv_obj_add_state(dimming_switch, LV_STATE_CHECKED);
    lv_obj_add_event_cb(dimming_switch, ui_settings_dimming_cb,
                        LV_EVENT_VALUE_CHANGED, nullptr);

    lv_obj_t *timeout_label = lv_label_create(page);
    lv_label_set_text(timeout_label, "Dim after");
    lv_obj_set_style_text_color(timeout_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(timeout_label, LV_ALIGN_TOP_LEFT, 400, 148);

    lv_obj_t *timeout_dropdown = lv_dropdown_create(page);
    lv_dropdown_set_options(timeout_dropdown,
        "1 minute\n2 minutes\n5 minutes\n10 minutes\n30 minutes");
    lv_obj_set_width(timeout_dropdown, 150);
    lv_obj_align(timeout_dropdown, LV_ALIGN_TOP_LEFT, 550, 137);
    lv_obj_set_style_bg_color(timeout_dropdown, lv_color_hex(0x394650), LV_PART_MAIN);
    lv_obj_set_style_bg_color(timeout_dropdown, lv_color_hex(0x4B5B68), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(timeout_dropdown, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(timeout_dropdown, 0, LV_PART_MAIN);

    uint16_t timeout_minutes = pt_screen_get_dim_timeout_minutes();
    uint16_t timeout_selection = 2;
    if (timeout_minutes == 1) timeout_selection = 0;
    else if (timeout_minutes == 2) timeout_selection = 1;
    else if (timeout_minutes == 5) timeout_selection = 2;
    else if (timeout_minutes == 10) timeout_selection = 3;
    else if (timeout_minutes == 30) timeout_selection = 4;
    lv_dropdown_set_selected(timeout_dropdown, timeout_selection);
    lv_obj_add_event_cb(timeout_dropdown, ui_settings_dim_timeout_cb,
                        LV_EVENT_VALUE_CHANGED, nullptr);

    lv_obj_t *dim_brightness_label = lv_label_create(page);
    lv_label_set_text_fmt(dim_brightness_label, "Dim brightness: %d%%", pt_screen_dim_percent);
    lv_obj_set_style_text_color(dim_brightness_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(dim_brightness_label, LV_ALIGN_TOP_LEFT, 400, 198);

    lv_obj_t *dim_brightness_slider = lv_slider_create(page);
    lv_obj_set_width(dim_brightness_slider, 300);
    lv_slider_set_range(dim_brightness_slider, 5, 30);
    lv_slider_set_value(dim_brightness_slider, pt_screen_dim_percent, LV_ANIM_OFF);
    lv_obj_align(dim_brightness_slider, LV_ALIGN_TOP_LEFT, 400, 226);
    ui_settings_style_slider(dim_brightness_slider);
    lv_obj_add_event_cb(dim_brightness_slider, ui_settings_dim_brightness_cb,
                        LV_EVENT_VALUE_CHANGED, dim_brightness_label);
    return page;
}
