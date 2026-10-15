#pragma once

#include <lvgl.h>
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
        return;
    }

    const SpoolInfo &spool = spool_store[ui_scale_selected_spool_index];

    lv_label_set_text(ui_scale_selected_vendor, spool.vendor.c_str());
    lv_label_set_text(ui_scale_selected_name, spool.name.c_str());
    lv_label_set_text(ui_scale_selected_material, spool.material.c_str());

    char text[40];

    snprintf(text, sizeof(text), "#%d", spool.id);
    lv_label_set_text(ui_scale_selected_id, text);

    snprintf(text, sizeof(text), "%d g", (int)(spool.remaining_weight + 0.5f));
    lv_label_set_text(ui_scale_remaining, text);

    snprintf(text, sizeof(text), "%d g", (int)(spool.spool_weight + 0.5f));
    lv_label_set_text(ui_scale_empty_spool, text);
}

inline void ui_scale_select_spool(size_t spool_index)
{
    if (spool_index >= spool_count) return;

    ui_scale_selected_spool_index = spool_index;
    ui_scale_has_selected_spool = true;
    ui_scale_refresh_selected_spool();
}

inline void ui_scale_update_weight(float grams)
{
    if (ui_scale_weight == nullptr || ui_scale_status == nullptr)
        return;

    char text[32];
    snprintf(text, sizeof(text), "%.1f g", grams);

    lv_label_set_text(ui_scale_weight, text);
    lv_label_set_text(ui_scale_status, "Scale connected");
}

inline void ui_scale_set_disconnected()
{
    if (ui_scale_weight == nullptr || ui_scale_status == nullptr)
        return;

    lv_label_set_text(ui_scale_weight, "-- g");
    lv_label_set_text(ui_scale_status, "Scale disconnected");
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
    ui_scale_empty_spool = ui_scale_create_value(page, 125, 200, 120);

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
    lv_obj_set_size(ui_scale_tare_button, 180, 42);
    lv_obj_align(ui_scale_tare_button, LV_ALIGN_BOTTOM_RIGHT, -20, 0);

    // Explicit state styles override LVGL's default orange button theme.
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

    // Keep TARE disabled until the scale controller is connected.
    lv_obj_add_state(ui_scale_tare_button, LV_STATE_DISABLED);
    lv_obj_add_state(tare_label, LV_STATE_DISABLED);

    ui_scale_refresh_selected_spool();

    return page;
}
