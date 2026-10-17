#pragma once

#include <lvgl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../models/spool_store.h"

static bool ui_scale_has_selected_spool = false;
static size_t ui_scale_selected_spool_index = 0;

static lv_obj_t *ui_scale_selected_vendor = nullptr;
static lv_obj_t *ui_scale_selected_name = nullptr;
static lv_obj_t *ui_scale_selected_material = nullptr;
static lv_obj_t *ui_scale_selected_id = nullptr;
static lv_obj_t *ui_scale_remaining = nullptr;
static lv_obj_t *ui_scale_empty_spool = nullptr;
static lv_obj_t *ui_scale_weight = nullptr;
static lv_obj_t *ui_scale_status = nullptr;
static lv_obj_t *ui_scale_tare_button = nullptr;
static lv_obj_t *ui_scale_measured_filament = nullptr;
static lv_obj_t *ui_scale_weight_difference = nullptr;
static lv_obj_t *ui_scale_update_button = nullptr;
static lv_obj_t *ui_scale_confirm_overlay = nullptr;
static int ui_scale_pending_spool_id = 0;
static float ui_scale_pending_remaining = 0.0f;
static bool ui_scale_update_busy = false;
typedef void (*UiScaleUpdateCallback)(int, float);
static UiScaleUpdateCallback ui_scale_update_callback = nullptr;
inline void ui_scale_set_update_callback(UiScaleUpdateCallback cb) { ui_scale_update_callback = cb; }
inline void ui_scale_refresh_selected_spool();

// Scale readings are updated by main.cpp on the LVGL/UI thread.
static bool ui_scale_live_weight_valid = false;
static float ui_scale_live_weight_grams = 0.0f;

inline lv_obj_t *ui_scale_create_caption(lv_obj_t *parent, const char *text, int x, int y)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, lv_color_hex(0x8F99A3), LV_PART_MAIN);
    return label;
}

inline lv_obj_t *ui_scale_create_value(lv_obj_t *parent, int x, int y, int width)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, width);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    return label;
}

// The first implementation is deliberately display-only: no Spoolman API writes.
inline bool ui_scale_get_update_candidate(int &spool_id, float &old_weight, float &new_weight)
{
    if (!ui_scale_has_selected_spool || ui_scale_selected_spool_index >= spool_count ||
        !ui_scale_live_weight_valid)
        return false;

    const SpoolInfo &spool = spool_store[ui_scale_selected_spool_index];
    if (spool.id <= 0 || !isfinite(spool.spool_weight) || spool.spool_weight <= 0.0f ||
        !isfinite(spool.remaining_weight) || spool.remaining_weight < 0.0f)
        return false;

    float measured = ui_scale_live_weight_grams - spool.spool_weight;
    if (!isfinite(measured) || measured < 0.0f ||
        (isfinite(spool.initial_weight) && spool.initial_weight > 0.0f &&
         measured > spool.initial_weight))
        return false;

    spool_id = spool.id;
    old_weight = spool.remaining_weight;
    new_weight = floorf(measured + 0.5f);
    return true;
}

inline void ui_scale_refresh_update_button()
{
    if (ui_scale_update_button == nullptr) return;
    int id = 0;
    float old_weight = 0.0f, new_weight = 0.0f;
    bool enabled = !ui_scale_update_busy && ui_scale_update_callback != nullptr &&
                   ui_scale_confirm_overlay == nullptr &&
                   ui_scale_get_update_candidate(id, old_weight, new_weight);
    if (enabled)
        lv_obj_remove_state(ui_scale_update_button, LV_STATE_DISABLED);
    else
        lv_obj_add_state(ui_scale_update_button, LV_STATE_DISABLED);
}

inline void ui_scale_close_confirmation(lv_event_t *event)
{
    (void)event;
    if (ui_scale_confirm_overlay != nullptr)
    {
        lv_obj_delete(ui_scale_confirm_overlay);
        ui_scale_confirm_overlay = nullptr;
    }
    ui_scale_refresh_update_button();
}

inline void ui_scale_confirm_live(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED || ui_scale_update_busy || !ui_scale_update_callback) return;
    // The confirmation dialog has frozen the spool ID and measured weight.
    const int id = ui_scale_pending_spool_id;
    const float grams = ui_scale_pending_remaining;
    ui_scale_close_confirmation(nullptr);
    ui_scale_update_busy = true;
    ui_scale_refresh_update_button();
    ui_scale_update_callback(id, grams);
}

inline void ui_scale_finish_update(int id, float grams, bool success, const char *message)
{
    if (success)
    {
        for (size_t i = 0; i < spool_count; ++i)
            if (spool_store[i].id == id) { spool_store[i].remaining_weight = grams; break; }
        ui_scale_refresh_selected_spool();
    }
    ui_scale_update_busy = false;
    ui_scale_refresh_update_button();
    // Show the result without changing the normal live scale status.
    lv_obj_t *popup = lv_obj_create(lv_screen_active());
    lv_obj_set_size(popup, 440, 160);
    lv_obj_center(popup);
    lv_obj_set_style_bg_color(popup, lv_color_hex(0x1A2026), LV_PART_MAIN);
    lv_obj_set_style_border_width(popup, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(popup, success ? lv_color_hex(0x3A9D73) : lv_color_hex(0xB75C5C), LV_PART_MAIN);
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *label = lv_label_create(popup);
    lv_obj_set_width(label, 390);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(label, message ? message : (success ? "Update successful" : "Update failed"));
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 12);
    lv_obj_t *ok = lv_button_create(popup);
    lv_obj_set_size(ok, 130, 40);
    lv_obj_align(ok, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_event_cb(ok, [](lv_event_t *e) { lv_obj_delete(lv_obj_get_parent(lv_event_get_target_obj(e))); }, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *ok_label = lv_label_create(ok);
    lv_label_set_text(ok_label, "OK");
    lv_obj_set_style_text_color(ok_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_center(ok_label);
}

inline void ui_scale_open_confirmation(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED || ui_scale_confirm_overlay != nullptr)
        return;

    int id = 0;
    float old_weight = 0.0f, new_weight = 0.0f;
    if (!ui_scale_get_update_candidate(id, old_weight, new_weight)) return;

    if (ui_scale_update_busy || ui_scale_update_callback == nullptr) return;
    ui_scale_pending_spool_id = id;
    ui_scale_pending_remaining = new_weight;

    // A full-screen overlay keeps other UI controls from being pressed.
    ui_scale_confirm_overlay = lv_obj_create(lv_screen_active());
    lv_obj_set_size(ui_scale_confirm_overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(ui_scale_confirm_overlay, 0, 0);
    lv_obj_set_style_bg_color(ui_scale_confirm_overlay, lv_color_hex(0x080B0E), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_scale_confirm_overlay, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_scale_confirm_overlay, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(ui_scale_confirm_overlay, 0, LV_PART_MAIN);
    lv_obj_clear_flag(ui_scale_confirm_overlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *panel = lv_obj_create(ui_scale_confirm_overlay);
    lv_obj_set_size(panel, 550, 315);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1A2026), LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, 12, LV_PART_MAIN);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "CONFIRM SPOOLMAN UPDATE");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);

    char details[192];
    snprintf(details, sizeof(details),
             "Spoolman ID: #%d\nCurrent remaining: %.0f g\nMeasured remaining: %.0f g\nDifference: %+.0f g",
             id, old_weight, new_weight, new_weight - old_weight);
    lv_obj_t *detail = lv_label_create(panel);
    lv_label_set_text(detail, details);
    lv_obj_set_pos(detail, 25, 62);
    lv_obj_set_style_text_color(detail, lv_color_white(), LV_PART_MAIN);

    lv_obj_t *notice = lv_label_create(panel);
    lv_label_set_text(notice, "This will change the selected Spoolman record");
    lv_obj_align(notice, LV_ALIGN_BOTTOM_MID, 0, -80);
    lv_obj_set_style_text_color(notice, lv_color_hex(0xA0A8B0), LV_PART_MAIN);

    lv_obj_t *cancel = lv_button_create(panel);
    lv_obj_set_size(cancel, 195, 45);
    lv_obj_align(cancel, LV_ALIGN_BOTTOM_LEFT, 25, -12);
    lv_obj_set_style_bg_color(cancel, lv_color_hex(0x394650), LV_PART_MAIN);
    lv_obj_add_event_cb(cancel, ui_scale_close_confirmation, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *cancel_text = lv_label_create(cancel);
    lv_label_set_text(cancel_text, "CANCEL");
    lv_obj_center(cancel_text);

    lv_obj_t *confirm = lv_button_create(panel);
    lv_obj_set_size(confirm, 195, 45);
    lv_obj_align(confirm, LV_ALIGN_BOTTOM_RIGHT, -25, -12);
    lv_obj_set_style_bg_color(confirm, lv_color_hex(0x315D74), LV_PART_MAIN);
    lv_obj_add_event_cb(confirm, ui_scale_confirm_live, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *confirm_text = lv_label_create(confirm);
    lv_label_set_text(confirm_text, "CONFIRM UPDATE");
    lv_obj_center(confirm_text);
    ui_scale_refresh_update_button();
}

inline void ui_scale_refresh_calculated_weight()
{
    if (ui_scale_measured_filament == nullptr || ui_scale_weight_difference == nullptr)
        return;

    const char *measured = "-- g";
    const char *difference = "-- g";
    char measured_text[40];
    char difference_text[40];

    if (!ui_scale_has_selected_spool || ui_scale_selected_spool_index >= spool_count)
    {
        measured = "Select a spool";
        difference = "-- g";
    }
    else
    {
        const SpoolInfo &spool = spool_store[ui_scale_selected_spool_index];
        if (!isfinite(spool.spool_weight) || spool.spool_weight <= 0.0f)
        {
            measured = "Empty weight required";
        }
        else if (ui_scale_live_weight_valid)
        {
            const float net_grams = ui_scale_live_weight_grams - spool.spool_weight;
            if (!isfinite(net_grams) || net_grams < 0.0f)
            {
                measured = "Check scale / spool";
            }
            else
            {
                snprintf(measured_text, sizeof(measured_text), "%.0f g", net_grams);
                measured = measured_text;
                if (isfinite(spool.remaining_weight) && spool.remaining_weight >= 0.0f)
                {
                    snprintf(difference_text, sizeof(difference_text), "%+.0f g", net_grams - spool.remaining_weight);
                    difference = difference_text;
                }
            }
        }
    }

    if (strcmp(lv_label_get_text(ui_scale_measured_filament), measured) != 0)
        lv_label_set_text(ui_scale_measured_filament, measured);
    if (strcmp(lv_label_get_text(ui_scale_weight_difference), difference) != 0)
        lv_label_set_text(ui_scale_weight_difference, difference);
    ui_scale_refresh_update_button();
}

inline void ui_scale_refresh_selected_spool()
{
    if (ui_scale_selected_vendor == nullptr) return;

    if (!ui_scale_has_selected_spool || ui_scale_selected_spool_index >= spool_count)
    {
        lv_label_set_text(ui_scale_selected_vendor, "No spool selected");
        lv_label_set_text(ui_scale_selected_name, "Choose a spool from the Spools page");
        lv_label_set_text(ui_scale_selected_material, "-");
        lv_label_set_text(ui_scale_selected_id, "-");
        lv_label_set_text(ui_scale_remaining, "-");
        lv_label_set_text(ui_scale_empty_spool, "-");
        ui_scale_refresh_calculated_weight();
        return;
    }

    const SpoolInfo &spool = spool_store[ui_scale_selected_spool_index];
    lv_label_set_text(ui_scale_selected_vendor, spool.vendor.c_str());
    lv_label_set_text(ui_scale_selected_name, spool.name.c_str());
    lv_label_set_text(ui_scale_selected_material, spool.material.c_str());

    char text[40];
    snprintf(text, sizeof(text), "#%d", spool.id);
    lv_label_set_text(ui_scale_selected_id, text);

    if (isfinite(spool.remaining_weight) && spool.remaining_weight >= 0.0f)
        snprintf(text, sizeof(text), "%.0f g", spool.remaining_weight);
    else
        snprintf(text, sizeof(text), "-");
    lv_label_set_text(ui_scale_remaining, text);

    if (isfinite(spool.spool_weight) && spool.spool_weight > 0.0f)
        snprintf(text, sizeof(text), "%.0f g", spool.spool_weight);
    else
        snprintf(text, sizeof(text), "Unknown");
    lv_label_set_text(ui_scale_empty_spool, text);

    ui_scale_refresh_calculated_weight();
}

inline void ui_scale_select_spool(size_t spool_index)
{
    if (spool_index >= spool_count) return;
    ui_scale_selected_spool_index = spool_index;
    ui_scale_has_selected_spool = true;
    ui_scale_refresh_selected_spool();
}

// Used by the current main.cpp when it receives a fresh weight.
inline void ui_scale_update_weight(float grams)
{
    if (ui_scale_weight == nullptr || ui_scale_status == nullptr) return;
    ui_scale_live_weight_valid = isfinite(grams);
    ui_scale_live_weight_grams = grams;
    char text[32];
    snprintf(text, sizeof(text), "%.0f g", grams);
    if (strcmp(lv_label_get_text(ui_scale_weight), text) != 0)
        lv_label_set_text(ui_scale_weight, text);
    lv_label_set_text(ui_scale_status, "Scale connected");
    ui_scale_refresh_calculated_weight();
}

inline void ui_scale_set_disconnected()
{
    if (ui_scale_weight == nullptr || ui_scale_status == nullptr) return;
    ui_scale_live_weight_valid = false;
    lv_label_set_text(ui_scale_weight, "-- g");
    lv_label_set_text(ui_scale_status, "Scale disconnected");
    ui_scale_refresh_calculated_weight();
}

// Called only when main.cpp has changed the live-weight text.
// The existing status / TARE handling in main.cpp remains authoritative.
inline void ui_scale_sync_calculation_from_display(bool connected, float grams)
{
    ui_scale_live_weight_valid = connected && isfinite(grams);
    if (ui_scale_live_weight_valid)
        ui_scale_live_weight_grams = grams;
    ui_scale_refresh_calculated_weight();
}

typedef void (*UiScaleTareCallback)();
static UiScaleTareCallback ui_scale_tare_callback = nullptr;

inline void ui_scale_set_tare_callback(UiScaleTareCallback callback)
{
    ui_scale_tare_callback = callback;
}

inline void ui_scale_tare_clicked(lv_event_t *event)
{
    if (lv_event_get_code(event) == LV_EVENT_CLICKED &&
        ui_scale_tare_callback != nullptr)
        ui_scale_tare_callback();
}

inline void ui_scale_set_tare_enabled(bool enabled)
{
    if (ui_scale_tare_button == nullptr) return;
    if (enabled)
        lv_obj_remove_state(ui_scale_tare_button, LV_STATE_DISABLED);
    else
        lv_obj_add_state(ui_scale_tare_button, LV_STATE_DISABLED);
}

inline void ui_scale_set_tare_status(const char *message)
{
    if (ui_scale_status != nullptr)
        lv_label_set_text(ui_scale_status, message);
}

inline lv_obj_t *ui_scale_create(lv_obj_t *parent)
{
    lv_obj_t *page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(page, lv_color_hex(0x101418), LV_PART_MAIN);
    lv_obj_set_style_border_width(page, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(page, 10, LV_PART_MAIN);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *selected_heading = lv_label_create(page);
    lv_label_set_text(selected_heading, "SELECTED SPOOL");
    lv_obj_set_pos(selected_heading, 20, 8);
    lv_obj_set_style_text_color(selected_heading, lv_color_hex(0x8F99A3), LV_PART_MAIN);

    ui_scale_selected_vendor = ui_scale_create_value(page, 20, 38, 310);
    ui_scale_selected_name = ui_scale_create_value(page, 20, 66, 310);
    ui_scale_selected_material = ui_scale_create_value(page, 20, 94, 150);

    ui_scale_create_caption(page, "Spoolman ID", 20, 132);
    ui_scale_selected_id = ui_scale_create_value(page, 125, 132, 120);
    ui_scale_create_caption(page, "Remaining", 20, 166);
    ui_scale_remaining = ui_scale_create_value(page, 125, 166, 120);
    ui_scale_create_caption(page, "Empty spool", 20, 200);
    ui_scale_empty_spool = ui_scale_create_value(page, 125, 200, 160);

    // Comparison rows use the unused space beneath the selected spool details.
    ui_scale_create_caption(page, "Measured filament", 20, 242);
    ui_scale_measured_filament = ui_scale_create_value(page, 180, 242, 195);
    ui_scale_create_caption(page, "Difference", 20, 269);
    ui_scale_weight_difference = ui_scale_create_value(page, 180, 269, 195);

    lv_obj_t *scale_panel = lv_obj_create(page);
    lv_obj_set_size(scale_panel, 350, 220);
    lv_obj_set_pos(scale_panel, 390, 8);
    lv_obj_set_style_bg_color(scale_panel, lv_color_hex(0x1A2026), LV_PART_MAIN);
    lv_obj_set_style_border_width(scale_panel, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(scale_panel, 8, LV_PART_MAIN);
    lv_obj_clear_flag(scale_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *scale_heading = lv_label_create(scale_panel);
    lv_label_set_text(scale_heading, "SCALE");
    lv_obj_align(scale_heading, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_text_color(scale_heading, lv_color_hex(0x8F99A3), LV_PART_MAIN);

    ui_scale_weight = lv_label_create(scale_panel);
    lv_label_set_text(ui_scale_weight, "-- g");
    lv_obj_set_style_text_color(ui_scale_weight, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_scale_weight, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_align(ui_scale_weight, LV_ALIGN_CENTER, 0, -20);

    ui_scale_status = lv_label_create(scale_panel);
    lv_label_set_text(ui_scale_status, "Waiting for scale");
    lv_obj_set_style_text_color(ui_scale_status, lv_color_hex(0xA0A8B0), LV_PART_MAIN);
    lv_obj_align(ui_scale_status, LV_ALIGN_CENTER, 0, 25);

    ui_scale_tare_button = lv_button_create(page);
    lv_obj_add_event_cb(ui_scale_tare_button, ui_scale_tare_clicked, LV_EVENT_CLICKED, nullptr);
    lv_obj_set_size(ui_scale_tare_button, 180, 42);
    lv_obj_align(ui_scale_tare_button, LV_ALIGN_BOTTOM_RIGHT, -20, 0);
    lv_obj_set_style_bg_color(ui_scale_tare_button, lv_color_hex(0x394650), LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_scale_tare_button, lv_color_hex(0x4B5B68), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(ui_scale_tare_button, lv_color_hex(0x252D34), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(ui_scale_tare_button, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_scale_tare_button, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_text_color(ui_scale_tare_button, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_color(ui_scale_tare_button, lv_color_hex(0x7D8993), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_shadow_width(ui_scale_tare_button, 0, LV_PART_MAIN);

    lv_obj_t *tare_label = lv_label_create(ui_scale_tare_button);
    lv_label_set_text(tare_label, "TARE");
    lv_obj_center(tare_label);
    lv_obj_set_style_text_color(tare_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_color(tare_label, lv_color_hex(0x7D8993), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_state(ui_scale_tare_button, LV_STATE_DISABLED);
    lv_obj_add_state(tare_label, LV_STATE_DISABLED);

    ui_scale_update_button = lv_button_create(page);
    lv_obj_set_size(ui_scale_update_button, 185, 42);
    lv_obj_align(ui_scale_update_button, LV_ALIGN_BOTTOM_RIGHT, -210, 0);
    lv_obj_set_style_bg_color(ui_scale_update_button, lv_color_hex(0x315D74), LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_scale_update_button, lv_color_hex(0x252D34), LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_bg_opa(ui_scale_update_button, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(ui_scale_update_button, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(ui_scale_update_button, ui_scale_open_confirmation, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *update_text = lv_label_create(ui_scale_update_button);
    lv_label_set_text(update_text, "UPDATE SPOOLMAN");
    lv_obj_center(update_text);
    lv_obj_add_state(ui_scale_update_button, LV_STATE_DISABLED);

    ui_scale_refresh_selected_spool();
    return page;
}
