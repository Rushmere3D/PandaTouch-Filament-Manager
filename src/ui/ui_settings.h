#pragma once

#include <lvgl.h>
#include <WiFi.h>
#include "../pt/pt_display.h"

// The Panda Touch swaps red and blue in the current display pipeline.
static inline lv_color_t ui_settings_accent_blue()
{
    return lv_color_hex(0x3A3126); // displays as #26313A
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

static inline void ui_settings_style_dropdown(lv_obj_t *dropdown)
{
    lv_obj_set_style_bg_color(dropdown, lv_color_hex(0x394650), LV_PART_MAIN);
    lv_obj_set_style_bg_color(dropdown, lv_color_hex(0x4B5B68), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(dropdown, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(dropdown, 0, LV_PART_MAIN);
}

static inline uint16_t ui_settings_timeout_selection(uint16_t minutes)
{
    if (minutes == 1) return 0;
    if (minutes == 2) return 1;
    if (minutes == 5) return 2;
    if (minutes == 10) return 3;
    return 4;
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
    const uint16_t minutes[] = {1, 2, 5, 10, 30};
    uint32_t selected = lv_dropdown_get_selected(dropdown);
    if (selected < 5) pt_screen_set_dim_timeout_minutes(minutes[selected]);
}

static void ui_settings_dim_brightness_cb(lv_event_t *e)
{
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    int32_t brightness = lv_slider_get_value(slider);
    pt_screen_set_dim_percent((uint8_t)brightness);
    lv_label_set_text_fmt(label, "Dim brightness: %ld%%", (long)brightness);
}

static void ui_settings_screen_off_cb(lv_event_t *e)
{
    lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);
    pt_screen_set_off_enabled(lv_obj_has_state(sw, LV_STATE_CHECKED));
}

static void ui_settings_off_timeout_cb(lv_event_t *e)
{
    lv_obj_t *dropdown = (lv_obj_t *)lv_event_get_target(e);
    const uint16_t minutes[] = {1, 2, 5, 10, 30};
    uint32_t selected = lv_dropdown_get_selected(dropdown);
    if (selected < 5) pt_screen_set_off_timeout_minutes(minutes[selected]);
}

// Stage 6A: read-only scale diagnostics, updated from the UI thread.
static lv_obj_t *ui_settings_scale_status = nullptr;
static lv_obj_t *ui_settings_scale_address = nullptr;
static lv_obj_t *ui_settings_scale_factor = nullptr;

inline void ui_settings_set_scale_info(bool connected, const char *weightUrl,
                                       bool factorValid, float factor)
{
    if (!ui_settings_scale_status) return;
    lv_label_set_text(ui_settings_scale_status,
                      connected ? "Status: Connected" : "Status: Disconnected");
    if (weightUrl && ui_settings_scale_address)
        lv_label_set_text_fmt(ui_settings_scale_address, "Weight API: %s", weightUrl);
    if (ui_settings_scale_factor)
    {
        if (connected && factorValid)
            {
                char calibrationText[80];
                snprintf(calibrationText, sizeof(calibrationText),
                         "Calibration: %.4f counts/g", (double)factor);
                lv_label_set_text(ui_settings_scale_factor, calibrationText);
            }
        else
            lv_label_set_text(ui_settings_scale_factor, "Calibration: Unavailable");
    }
}


// Stage 6C: Calibration wizard. No HTTP operations are performed from LVGL callbacks.
static lv_obj_t *ui_cal_overlay = nullptr;
// Stage 6C.1: live scale data on the wizard, updated from the LVGL thread.
static lv_obj_t *ui_cal_live_weight = nullptr;
static lv_obj_t *ui_cal_live_stability = nullptr;
static bool ui_cal_busy = false;
static bool ui_cal_connected = false;
static bool ui_cal_stable = false;
static bool ui_cal_tared = false;
static lv_obj_t *ui_cal_message = nullptr;
static lv_obj_t *ui_cal_tare_button = nullptr;
static lv_obj_t *ui_cal_start_button = nullptr;
static lv_obj_t *ui_cal_weight_dropdown = nullptr;
static void (*ui_cal_tare_callback)() = nullptr;
static void (*ui_cal_start_callback)(float) = nullptr;
static const float ui_cal_weights[] = {50, 100, 200, 500, 1000};

inline void ui_settings_set_calibration_callbacks(void (*tareCb)(), void (*startCb)(float))
{
    ui_cal_tare_callback = tareCb;
    ui_cal_start_callback = startCb;
}

static inline void ui_settings_calibration_refresh_buttons()
{
    if (ui_cal_tare_button) {
        if (ui_cal_busy || !ui_cal_connected)
            lv_obj_add_state(ui_cal_tare_button, LV_STATE_DISABLED);
        else lv_obj_remove_state(ui_cal_tare_button, LV_STATE_DISABLED);
    }
    if (ui_cal_start_button) {
        if (ui_cal_busy || !ui_cal_connected || !ui_cal_stable || !ui_cal_tared)
            lv_obj_add_state(ui_cal_start_button, LV_STATE_DISABLED);
        else lv_obj_remove_state(ui_cal_start_button, LV_STATE_DISABLED);
    }
}

inline void ui_settings_calibration_message(const char *message, bool busy)
{
    ui_cal_busy = busy;
    if (ui_cal_message) lv_label_set_text(ui_cal_message, message);
    ui_settings_calibration_refresh_buttons();
}

inline void ui_settings_calibration_tare_complete(bool success)
{
    ui_cal_tared = success;
    ui_settings_calibration_message(success ?
        "Tare complete. Place known weight; wait for STABLE." :
        "Tare failed. Check connection and retry.", false);
}

inline void ui_settings_calibration_live_reading(bool connected, float grams,
                                                  bool stable, bool tareBusy)
{
    ui_cal_connected = connected;
    ui_cal_stable = connected && stable && !tareBusy;
    if (ui_cal_live_weight) {
        char weight[48];
        if (connected) snprintf(weight, sizeof(weight), "Live weight: %.1f g", (double)grams);
        else snprintf(weight, sizeof(weight), "Live weight: -- g");
        lv_label_set_text(ui_cal_live_weight, weight);
    }
    if (ui_cal_live_stability) {
        lv_label_set_text(ui_cal_live_stability, !connected ? "DISCONNECTED" :
                          tareBusy ? "TARING..." : stable ? "STABLE" : "STABILISING...");
        lv_obj_set_style_text_color(ui_cal_live_stability,
            lv_color_hex(ui_cal_stable ? 0x70E0A0 : 0xFFD080), LV_PART_MAIN);
    }
    ui_settings_calibration_refresh_buttons();
}

static void ui_cal_close_cb(lv_event_t *)
{
    if (ui_cal_overlay) lv_obj_add_flag(ui_cal_overlay, LV_OBJ_FLAG_HIDDEN);
}

static void ui_cal_tare_cb(lv_event_t *)
{
    if (ui_cal_tare_callback) {
        ui_cal_tared = false;
        ui_cal_stable = false;
        ui_settings_calibration_message("Taring empty scale...", true);
        ui_cal_tare_callback();
    }
}

static void ui_cal_start_cb(lv_event_t *)
{
    if (!ui_cal_start_callback || !ui_cal_weight_dropdown) return;
    uint32_t selected = lv_dropdown_get_selected(ui_cal_weight_dropdown);
    if (selected >= sizeof(ui_cal_weights)/sizeof(ui_cal_weights[0])) return;
    ui_cal_start_callback(ui_cal_weights[selected]);
}

static void ui_cal_open_cb(lv_event_t *)
{
    if (!ui_cal_overlay) {
        ui_cal_overlay = lv_obj_create(lv_layer_top());
        lv_obj_set_size(ui_cal_overlay, 690, 385);
        lv_obj_center(ui_cal_overlay);
        lv_obj_set_style_bg_color(ui_cal_overlay, lv_color_hex(0x202830), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(ui_cal_overlay, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(ui_cal_overlay, 2, LV_PART_MAIN);
        lv_obj_set_style_border_color(ui_cal_overlay, lv_color_hex(0x708090), LV_PART_MAIN);
        lv_obj_clear_flag(ui_cal_overlay, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t *heading = lv_label_create(ui_cal_overlay);
        lv_label_set_text(heading, "Scale calibration");
        lv_obj_set_style_text_color(heading, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_pos(heading, 20, 12);

        lv_obj_t *instructions = lv_label_create(ui_cal_overlay);
        lv_label_set_text(instructions,
            "1. Remove all weight, then press TARE.\n"
            "2. Place your known weight on the scale.\n"
            "3. Wait for a stable reading, then press CALIBRATE.");
        lv_obj_set_style_text_color(instructions, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_pos(instructions, 20, 54);

        ui_cal_live_weight = lv_label_create(ui_cal_overlay);
        lv_label_set_text(ui_cal_live_weight, "Live weight: -- g");
        lv_obj_set_style_text_color(ui_cal_live_weight, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_pos(ui_cal_live_weight, 20, 124);

        ui_cal_live_stability = lv_label_create(ui_cal_overlay);
        lv_label_set_text(ui_cal_live_stability, "DISCONNECTED");
        lv_obj_set_style_text_color(ui_cal_live_stability, lv_color_hex(0xFFD080), LV_PART_MAIN);
        lv_obj_set_pos(ui_cal_live_stability, 310, 124);

        ui_cal_tare_button = lv_button_create(ui_cal_overlay);
        lv_obj_set_size(ui_cal_tare_button, 175, 50);
        lv_obj_set_pos(ui_cal_tare_button, 20, 160);
        lv_obj_add_event_cb(ui_cal_tare_button, ui_cal_tare_cb, LV_EVENT_CLICKED, nullptr);
        lv_obj_t *tareText = lv_label_create(ui_cal_tare_button);
        lv_label_set_text(tareText, "TARE EMPTY");
        lv_obj_center(tareText);

        ui_cal_weight_dropdown = lv_dropdown_create(ui_cal_overlay);
        lv_dropdown_set_options(ui_cal_weight_dropdown, "50 g\n100 g\n200 g\n500 g\n1000 g");
        lv_dropdown_set_selected(ui_cal_weight_dropdown, 2);
        lv_obj_set_width(ui_cal_weight_dropdown, 145);
        lv_obj_set_pos(ui_cal_weight_dropdown, 215, 160);

        ui_cal_start_button = lv_button_create(ui_cal_overlay);
        lv_obj_set_size(ui_cal_start_button, 175, 50);
        lv_obj_set_pos(ui_cal_start_button, 385, 160);
        lv_obj_add_event_cb(ui_cal_start_button, ui_cal_start_cb, LV_EVENT_CLICKED, nullptr);
        lv_obj_t *startText = lv_label_create(ui_cal_start_button);
        lv_label_set_text(startText, "CALIBRATE");
        lv_obj_center(startText);

        ui_cal_message = lv_label_create(ui_cal_overlay);
        lv_obj_set_width(ui_cal_message, 630);
        lv_label_set_long_mode(ui_cal_message, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_color(ui_cal_message, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_pos(ui_cal_message, 20, 245);

        lv_obj_t *closeButton = lv_button_create(ui_cal_overlay);
        lv_obj_set_size(closeButton, 140, 45);
        lv_obj_set_pos(closeButton, 500, 305);
        lv_obj_add_event_cb(closeButton, ui_cal_close_cb, LV_EVENT_CLICKED, nullptr);
        lv_obj_t *closeText = lv_label_create(closeButton);
        lv_label_set_text(closeText, "CLOSE");
        lv_obj_center(closeText);
    }
    lv_obj_remove_flag(ui_cal_overlay, LV_OBJ_FLAG_HIDDEN);
    ui_cal_tared = false;
    ui_settings_calibration_message("Remove weight and tare before calibration.", false);
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

    // Screen-off controls use the unused space below the Wi-Fi information.
    lv_obj_t *off_label = lv_label_create(page);
    lv_label_set_text(off_label, "Screen off");
    lv_obj_set_style_text_color(off_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(off_label, LV_ALIGN_TOP_LEFT, 20, 157);

    lv_obj_t *off_switch = lv_switch_create(page);
    lv_obj_align(off_switch, LV_ALIGN_TOP_LEFT, 240, 149);
    ui_settings_style_switch(off_switch);
    if (pt_screen_off_enabled) lv_obj_add_state(off_switch, LV_STATE_CHECKED);
    lv_obj_add_event_cb(off_switch, ui_settings_screen_off_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    lv_obj_t *off_timeout_label = lv_label_create(page);
    lv_label_set_text(off_timeout_label, "Turn off after");
    lv_obj_set_style_text_color(off_timeout_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(off_timeout_label, LV_ALIGN_TOP_LEFT, 20, 211);

    lv_obj_t *off_timeout_dropdown = lv_dropdown_create(page);
    lv_dropdown_set_options(off_timeout_dropdown,
        "1 minute\n2 minutes\n5 minutes\n10 minutes\n30 minutes");
    lv_obj_set_width(off_timeout_dropdown, 150);
    lv_obj_align(off_timeout_dropdown, LV_ALIGN_TOP_LEFT, 165, 199);
    ui_settings_style_dropdown(off_timeout_dropdown);
    lv_dropdown_set_selected(off_timeout_dropdown,
                             ui_settings_timeout_selection(pt_screen_get_off_timeout_minutes()));
    lv_obj_add_event_cb(off_timeout_dropdown, ui_settings_off_timeout_cb,
                        LV_EVENT_VALUE_CHANGED, nullptr);

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
    if (pt_screen_dimming_enabled) lv_obj_add_state(dimming_switch, LV_STATE_CHECKED);
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
    ui_settings_style_dropdown(timeout_dropdown);
    lv_dropdown_set_selected(timeout_dropdown,
                             ui_settings_timeout_selection(pt_screen_get_dim_timeout_minutes()));
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

    // Stage 6A: scale information below existing display settings.
    // Allow vertical scrolling so existing controls keep their tested positions.
    lv_obj_add_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);
    lv_obj_t *scale_heading = lv_label_create(page);
    lv_label_set_text(scale_heading, "Scale connection (Stage 6A)");
    lv_obj_set_style_text_color(scale_heading, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_pos(scale_heading, 20, 282);

    ui_settings_scale_status = lv_label_create(page);
    lv_label_set_text(ui_settings_scale_status, "Status: Waiting for scale");
    lv_obj_set_style_text_color(ui_settings_scale_status, lv_color_hex(0xB0B8C0), LV_PART_MAIN);
    lv_obj_set_pos(ui_settings_scale_status, 20, 313);

    ui_settings_scale_address = lv_label_create(page);
    lv_label_set_text(ui_settings_scale_address, "Weight API: --");
    lv_obj_set_width(ui_settings_scale_address, 720);
    lv_label_set_long_mode(ui_settings_scale_address, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(ui_settings_scale_address, lv_color_hex(0xB0B8C0), LV_PART_MAIN);
    lv_obj_set_pos(ui_settings_scale_address, 20, 340);

    ui_settings_scale_factor = lv_label_create(page);
    lv_label_set_text(ui_settings_scale_factor, "Calibration: Unavailable");
    lv_obj_set_style_text_color(ui_settings_scale_factor, lv_color_hex(0xB0B8C0), LV_PART_MAIN);
    lv_obj_set_pos(ui_settings_scale_factor, 20, 368);

    lv_obj_t *firmware = lv_label_create(page);
    lv_label_set_text(firmware, "Scale firmware: Not reported by current API");
    lv_obj_set_style_text_color(firmware, lv_color_hex(0xB0B8C0), LV_PART_MAIN);
    lv_obj_set_pos(firmware, 20, 395);

    // Stage 6C: entry point for the calibration wizard.
    lv_obj_t *calButton = lv_button_create(page);
    lv_obj_set_size(calButton, 245, 48);
    lv_obj_set_pos(calButton, 20, 435);
    lv_obj_add_event_cb(calButton, ui_cal_open_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *calLabel = lv_label_create(calButton);
    lv_label_set_text(calLabel, "CALIBRATE SCALE");
    lv_obj_center(calLabel);

    // Extends scrollable content beyond the visible 310px settings viewport.
    lv_obj_t *bottom = lv_obj_create(page);
    lv_obj_set_size(bottom, 1, 1);
    lv_obj_set_pos(bottom, 20, 510);
    lv_obj_set_style_bg_opa(bottom, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(bottom, 0, LV_PART_MAIN);
    return page;
}
