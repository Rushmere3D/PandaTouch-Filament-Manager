#pragma once

#include <lvgl.h>
#include <stdlib.h>
#include <stdio.h>
#include "../models/spool_store.h"

constexpr size_t SPOOL_ROWS_VISIBLE = 4;

static size_t ui_spool_first_index = 0;
static size_t ui_spool_selected_index = 0;

typedef void (*UiSpoolUseOnScaleCallback)(size_t spool_index);
static UiSpoolUseOnScaleCallback ui_spool_use_on_scale_callback = nullptr;

inline void ui_spools_set_use_on_scale_callback(UiSpoolUseOnScaleCallback callback)
{
    ui_spool_use_on_scale_callback = callback;
}

static lv_obj_t *ui_spool_list_view = nullptr;
static lv_obj_t *ui_spool_detail_view = nullptr;
static lv_obj_t *ui_spool_page_label = nullptr;
static lv_obj_t *ui_spool_first_button = nullptr;
static lv_obj_t *ui_spool_previous_button = nullptr;
static lv_obj_t *ui_spool_next_button = nullptr;
static lv_obj_t *ui_spool_last_button = nullptr;

static lv_obj_t *ui_spool_rows[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_swatches[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_vendors[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_materials[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_names[SPOOL_ROWS_VISIBLE];
static lv_obj_t *ui_spool_weights[SPOOL_ROWS_VISIBLE];

static lv_obj_t *ui_detail_swatch = nullptr;
static lv_obj_t *ui_detail_title = nullptr;
static lv_obj_t *ui_detail_vendor = nullptr;
static lv_obj_t *ui_detail_material = nullptr;
static lv_obj_t *ui_detail_remaining = nullptr;
static lv_obj_t *ui_detail_initial = nullptr;
static lv_obj_t *ui_detail_spool_weight = nullptr;
static lv_obj_t *ui_detail_location = nullptr;
static lv_obj_t *ui_detail_id = nullptr;

inline lv_color_t ui_spool_hex_to_color(const String &hex)
{
    String clean_hex = hex;
    if (clean_hex.startsWith("#")) clean_hex.remove(0, 1);
    if (clean_hex.length() < 6) return lv_color_hex(0x808080);

    uint32_t value = strtoul(clean_hex.substring(0, 6).c_str(), nullptr, 16);
    uint8_t red = (value >> 16) & 0xFF;
    uint8_t green = (value >> 8) & 0xFF;
    uint8_t blue = value & 0xFF;

    uint32_t panda_color =
        ((uint32_t)blue << 16) |
        ((uint32_t)green << 8) |
        red;

    return lv_color_hex(panda_color);
}

inline uint8_t ui_spool_hex_alpha(const String &hex)
{
    String clean_hex = hex;
    if (clean_hex.startsWith("#")) clean_hex.remove(0, 1);
    if (clean_hex.length() < 8) return 255;
    return (uint8_t)strtoul(clean_hex.substring(6, 8).c_str(), nullptr, 16);
}

inline bool ui_spool_get_multi_colors(const String &multi, String &colour1, String &colour2)
{
    int comma = multi.indexOf(',');
    if (comma < 0) return false;

    colour1 = multi.substring(0, comma);
    int second_comma = multi.indexOf(',', comma + 1);
    colour2 = second_comma < 0
        ? multi.substring(comma + 1)
        : multi.substring(comma + 1, second_comma);

    colour1.trim();
    colour2.trim();
    return colour1.length() >= 6 && colour2.length() >= 6;
}

inline void ui_spool_apply_single_color(lv_obj_t *swatch, const String &hex)
{
    lv_obj_set_style_bg_grad_dir(swatch, LV_GRAD_DIR_NONE, LV_PART_MAIN);

    if (hex.length() >= 6)
    {
        lv_obj_set_style_bg_color(swatch, ui_spool_hex_to_color(hex), LV_PART_MAIN);

        if (ui_spool_hex_alpha(hex) < 255)
        {
            lv_obj_set_style_border_width(swatch, 3, LV_PART_MAIN);
            lv_obj_set_style_border_color(swatch, lv_color_white(), LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_border_width(swatch, 1, LV_PART_MAIN);
            lv_obj_set_style_border_color(swatch, lv_color_hex(0x606870), LV_PART_MAIN);
        }
    }
    else
    {
        lv_obj_set_style_bg_color(swatch, lv_color_hex(0x808080), LV_PART_MAIN);
        lv_obj_set_style_border_width(swatch, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(swatch, lv_color_hex(0x606870), LV_PART_MAIN);
    }
}

inline void ui_spool_apply_multi_color(lv_obj_t *swatch, const String &colour1, const String &colour2)
{
    lv_obj_set_style_bg_color(swatch, ui_spool_hex_to_color(colour1), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(swatch, ui_spool_hex_to_color(colour2), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(swatch, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_bg_main_stop(swatch, 127, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_stop(swatch, 128, LV_PART_MAIN);
    lv_obj_set_style_border_width(swatch, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(swatch, lv_color_hex(0x808890), LV_PART_MAIN);
}

inline void ui_spool_apply_swatch(lv_obj_t *swatch, const SpoolInfo &spool)
{
    String colour1;
    String colour2;

    if (spool.multi_color_hexes.length() > 0 &&
        ui_spool_get_multi_colors(spool.multi_color_hexes, colour1, colour2))
    {
        ui_spool_apply_multi_color(swatch, colour1, colour2);
    }
    else
    {
        ui_spool_apply_single_color(swatch, spool.color_hex);
    }
}

inline lv_obj_t *ui_spool_create_value_label(lv_obj_t *parent, int x, int y, int width)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, width);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    return label;
}

inline lv_obj_t *ui_spool_create_caption(lv_obj_t *parent, const char *text, int x, int y)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, lv_color_hex(0x8F99A3), LV_PART_MAIN);
    return label;
}

inline void ui_spools_update_nav_buttons()
{
    bool at_first = (ui_spool_first_index == 0);
    bool at_last =
        (spool_count == 0) ||
        (ui_spool_first_index + SPOOL_ROWS_VISIBLE >= spool_count);

    if (ui_spool_first_button != nullptr)
    {
        if (at_first) lv_obj_add_state(ui_spool_first_button, LV_STATE_DISABLED);
        else lv_obj_remove_state(ui_spool_first_button, LV_STATE_DISABLED);
    }

    if (ui_spool_previous_button != nullptr)
    {
        if (at_first) lv_obj_add_state(ui_spool_previous_button, LV_STATE_DISABLED);
        else lv_obj_remove_state(ui_spool_previous_button, LV_STATE_DISABLED);
    }

    if (ui_spool_next_button != nullptr)
    {
        if (at_last) lv_obj_add_state(ui_spool_next_button, LV_STATE_DISABLED);
        else lv_obj_remove_state(ui_spool_next_button, LV_STATE_DISABLED);
    }

    if (ui_spool_last_button != nullptr)
    {
        if (at_last) lv_obj_add_state(ui_spool_last_button, LV_STATE_DISABLED);
        else lv_obj_remove_state(ui_spool_last_button, LV_STATE_DISABLED);
    }
}

inline void ui_spools_refresh_rows()
{
    for (size_t row = 0; row < SPOOL_ROWS_VISIBLE; row++)
    {
        size_t spool_index = ui_spool_first_index + row;

        if (spool_index >= spool_count)
        {
            lv_obj_add_flag(ui_spool_rows[row], LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        const SpoolInfo &spool = spool_store[spool_index];
        lv_obj_remove_flag(ui_spool_rows[row], LV_OBJ_FLAG_HIDDEN);

        ui_spool_apply_swatch(ui_spool_swatches[row], spool);
        lv_label_set_text(ui_spool_vendors[row], spool.vendor.c_str());
        lv_label_set_text(ui_spool_materials[row], spool.material.c_str());
        lv_label_set_text(ui_spool_names[row], spool.name.c_str());

        char weight_text[24];
        snprintf(weight_text, sizeof(weight_text), "%d g",
                 (int)(spool.remaining_weight + 0.5f));
        lv_label_set_text(ui_spool_weights[row], weight_text);
    }

    if (ui_spool_page_label != nullptr)
    {
        char page_text[40];

        if (spool_count == 0)
        {
            snprintf(page_text, sizeof(page_text), "0 spools");
        }
        else
        {
            size_t first = ui_spool_first_index + 1;
            size_t last = ui_spool_first_index + SPOOL_ROWS_VISIBLE;
            if (last > spool_count) last = spool_count;

            snprintf(page_text, sizeof(page_text), "%u-%u of %u",
                     (unsigned int)first,
                     (unsigned int)last,
                     (unsigned int)spool_count);
        }

        lv_label_set_text(ui_spool_page_label, page_text);
    }

    ui_spools_update_nav_buttons();
}

inline void ui_spool_refresh_detail()
{
    if (ui_spool_selected_index >= spool_count) return;

    const SpoolInfo &spool = spool_store[ui_spool_selected_index];
    ui_spool_apply_swatch(ui_detail_swatch, spool);

    lv_label_set_text(ui_detail_title, spool.name.c_str());
    lv_label_set_text(ui_detail_vendor, spool.vendor.c_str());
    lv_label_set_text(ui_detail_material, spool.material.c_str());

    char text[48];

    snprintf(text, sizeof(text), "%d g", (int)(spool.remaining_weight + 0.5f));
    lv_label_set_text(ui_detail_remaining, text);

    snprintf(text, sizeof(text), "%d g", (int)(spool.initial_weight + 0.5f));
    lv_label_set_text(ui_detail_initial, text);

    snprintf(text, sizeof(text), "%d g", (int)(spool.spool_weight + 0.5f));
    lv_label_set_text(ui_detail_spool_weight, text);

    lv_label_set_text(ui_detail_location,
                      spool.location.length() > 0 ? spool.location.c_str() : "-");

    snprintf(text, sizeof(text), "#%d", spool.id);
    lv_label_set_text(ui_detail_id, text);
}

inline void ui_spool_show_list_view()
{
    lv_obj_add_flag(ui_spool_detail_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_spool_list_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_spool_page_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_spool_first_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_spool_previous_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_spool_next_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_spool_last_button, LV_OBJ_FLAG_HIDDEN);
}

inline void ui_spool_show_detail_view(size_t spool_index)
{
    if (spool_index >= spool_count) return;

    ui_spool_selected_index = spool_index;
    ui_spool_refresh_detail();

    lv_obj_add_flag(ui_spool_list_view, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_spool_page_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_spool_first_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_spool_previous_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_spool_next_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_spool_last_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_spool_detail_view, LV_OBJ_FLAG_HIDDEN);
}

inline void ui_spool_row_clicked_cb(lv_event_t *e)
{
    size_t row = (size_t)(uintptr_t)lv_event_get_user_data(e);
    size_t spool_index = ui_spool_first_index + row;
    ui_spool_show_detail_view(spool_index);
}

inline void ui_spool_back_cb(lv_event_t *e)
{
    (void)e;
    ui_spool_show_list_view();
}

inline void ui_spools_first_cb(lv_event_t *e)
{
    (void)e;
    ui_spool_first_index = 0;
    ui_spools_refresh_rows();
}

inline void ui_spools_previous_cb(lv_event_t *e)
{
    (void)e;
    if (ui_spool_first_index >= SPOOL_ROWS_VISIBLE)
        ui_spool_first_index -= SPOOL_ROWS_VISIBLE;
    else
        ui_spool_first_index = 0;

    ui_spools_refresh_rows();
}

inline void ui_spools_next_cb(lv_event_t *e)
{
    (void)e;
    if (ui_spool_first_index + SPOOL_ROWS_VISIBLE < spool_count)
    {
        ui_spool_first_index += SPOOL_ROWS_VISIBLE;
        ui_spools_refresh_rows();
    }
}

inline void ui_spools_last_cb(lv_event_t *e)
{
    (void)e;

    if (spool_count == 0)
        ui_spool_first_index = 0;
    else
        ui_spool_first_index =
            ((spool_count - 1) / SPOOL_ROWS_VISIBLE) * SPOOL_ROWS_VISIBLE;

    ui_spools_refresh_rows();
}

inline void ui_spool_use_on_scale_cb(lv_event_t *e)
{
    (void)e;

    if (ui_spool_selected_index >= spool_count) return;

    ui_spool_show_list_view();

    if (ui_spool_use_on_scale_callback != nullptr)
        ui_spool_use_on_scale_callback(ui_spool_selected_index);
}

inline lv_obj_t *ui_spools_create(lv_obj_t *parent)
{
    lv_obj_t *page = lv_obj_create(parent);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(page, lv_color_hex(0x101418), LV_PART_MAIN);
    lv_obj_set_style_border_width(page, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(page, 10, LV_PART_MAIN);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    ui_spool_page_label = lv_label_create(page);
    lv_obj_set_style_text_color(ui_spool_page_label, lv_color_hex(0xA0A8B0), LV_PART_MAIN);
    lv_obj_align(ui_spool_page_label, LV_ALIGN_TOP_RIGHT, -10, 3);

    // --------------------------------------------------------
    // LIST VIEW
    // --------------------------------------------------------
    ui_spool_list_view = lv_obj_create(page);
    lv_obj_set_size(ui_spool_list_view, LV_PCT(100), 220);
    lv_obj_align(ui_spool_list_view, LV_ALIGN_TOP_MID, 0, 25);
    lv_obj_set_style_bg_color(ui_spool_list_view, lv_color_hex(0x101418), LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_spool_list_view, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_spool_list_view, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(ui_spool_list_view, 4, LV_PART_MAIN);
    lv_obj_set_flex_flow(ui_spool_list_view, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(ui_spool_list_view, LV_OBJ_FLAG_SCROLLABLE);

    for (size_t row = 0; row < SPOOL_ROWS_VISIBLE; row++)
    {
        ui_spool_rows[row] = lv_obj_create(ui_spool_list_view);
        lv_obj_set_width(ui_spool_rows[row], LV_PCT(100));
        lv_obj_set_height(ui_spool_rows[row], 48);
        lv_obj_set_style_bg_color(ui_spool_rows[row], lv_color_hex(0x1A2026), LV_PART_MAIN);
        lv_obj_set_style_bg_color(ui_spool_rows[row], lv_color_hex(0x26313A), LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_set_style_border_width(ui_spool_rows[row], 0, LV_PART_MAIN);
        lv_obj_set_style_radius(ui_spool_rows[row], 6, LV_PART_MAIN);
        lv_obj_set_style_pad_all(ui_spool_rows[row], 7, LV_PART_MAIN);
        lv_obj_clear_flag(ui_spool_rows[row], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(ui_spool_rows[row], LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(ui_spool_rows[row], ui_spool_row_clicked_cb,
                            LV_EVENT_CLICKED, (void *)(uintptr_t)row);

        ui_spool_swatches[row] = lv_obj_create(ui_spool_rows[row]);
        lv_obj_set_size(ui_spool_swatches[row], 30, 30);
        lv_obj_align(ui_spool_swatches[row], LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_pad_all(ui_spool_swatches[row], 0, LV_PART_MAIN);
        lv_obj_set_style_border_width(ui_spool_swatches[row], 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(ui_spool_swatches[row], lv_color_hex(0x606870), LV_PART_MAIN);
        lv_obj_set_style_radius(ui_spool_swatches[row], LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_clear_flag(ui_spool_swatches[row], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(ui_spool_swatches[row], LV_OBJ_FLAG_CLICKABLE);

        ui_spool_vendors[row] = lv_label_create(ui_spool_rows[row]);
        lv_obj_set_width(ui_spool_vendors[row], 140);
        lv_label_set_long_mode(ui_spool_vendors[row], LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_color(ui_spool_vendors[row], lv_color_white(), LV_PART_MAIN);
        lv_obj_align(ui_spool_vendors[row], LV_ALIGN_LEFT_MID, 50, 0);

        ui_spool_materials[row] = lv_label_create(ui_spool_rows[row]);
        lv_obj_set_width(ui_spool_materials[row], 80);
        lv_label_set_long_mode(ui_spool_materials[row], LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_color(ui_spool_materials[row], lv_color_hex(0xAAB2BA), LV_PART_MAIN);
        lv_obj_align(ui_spool_materials[row], LV_ALIGN_LEFT_MID, 200, 0);

        ui_spool_names[row] = lv_label_create(ui_spool_rows[row]);
        lv_obj_set_width(ui_spool_names[row], 235);
        lv_label_set_long_mode(ui_spool_names[row], LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_color(ui_spool_names[row], lv_color_white(), LV_PART_MAIN);
        lv_obj_align(ui_spool_names[row], LV_ALIGN_LEFT_MID, 290, 0);

        ui_spool_weights[row] = lv_label_create(ui_spool_rows[row]);
        lv_obj_set_width(ui_spool_weights[row], 95);
        lv_obj_set_style_text_align(ui_spool_weights[row], LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
        lv_obj_set_style_text_color(ui_spool_weights[row], lv_color_white(), LV_PART_MAIN);
        lv_obj_align(ui_spool_weights[row], LV_ALIGN_RIGHT_MID, -5, 0);
    }

    // --------------------------------------------------------
    // FOUR-BUTTON PAGE NAVIGATION
    // --------------------------------------------------------

    ui_spool_first_button = lv_button_create(page);
    lv_obj_set_size(ui_spool_first_button, 150, 42);
    lv_obj_align(ui_spool_first_button, LV_ALIGN_BOTTOM_LEFT, 10, 0);
    lv_obj_add_event_cb(ui_spool_first_button, ui_spools_first_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *first_label = lv_label_create(ui_spool_first_button);
    lv_label_set_text(first_label, "|< FIRST");
    lv_obj_center(first_label);

    ui_spool_previous_button = lv_button_create(page);
    lv_obj_set_size(ui_spool_previous_button, 150, 42);
    lv_obj_align(ui_spool_previous_button, LV_ALIGN_BOTTOM_LEFT, 200, 0);
    lv_obj_add_event_cb(ui_spool_previous_button, ui_spools_previous_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *previous_label = lv_label_create(ui_spool_previous_button);
    lv_label_set_text(previous_label, "< PREV");
    lv_obj_center(previous_label);

    ui_spool_next_button = lv_button_create(page);
    lv_obj_set_size(ui_spool_next_button, 150, 42);
    lv_obj_align(ui_spool_next_button, LV_ALIGN_BOTTOM_RIGHT, -200, 0);
    lv_obj_add_event_cb(ui_spool_next_button, ui_spools_next_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *next_label = lv_label_create(ui_spool_next_button);
    lv_label_set_text(next_label, "NEXT >");
    lv_obj_center(next_label);

    ui_spool_last_button = lv_button_create(page);
    lv_obj_set_size(ui_spool_last_button, 150, 42);
    lv_obj_align(ui_spool_last_button, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
    lv_obj_add_event_cb(ui_spool_last_button, ui_spools_last_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *last_label = lv_label_create(ui_spool_last_button);
    lv_label_set_text(last_label, "LAST >|");
    lv_obj_center(last_label);


    // --------------------------------------------------------
    // DETAIL VIEW
    // --------------------------------------------------------
    ui_spool_detail_view = lv_obj_create(page);
    lv_obj_set_size(ui_spool_detail_view, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(ui_spool_detail_view, lv_color_hex(0x101418), LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_spool_detail_view, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_spool_detail_view, 10, LV_PART_MAIN);
    lv_obj_clear_flag(ui_spool_detail_view, LV_OBJ_FLAG_SCROLLABLE);

    ui_detail_swatch = lv_obj_create(ui_spool_detail_view);
    lv_obj_set_size(ui_detail_swatch, 64, 64);
    lv_obj_set_pos(ui_detail_swatch, 10, 8);
    lv_obj_set_style_pad_all(ui_detail_swatch, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(ui_detail_swatch, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_clear_flag(ui_detail_swatch, LV_OBJ_FLAG_SCROLLABLE);

    ui_detail_title = lv_label_create(ui_spool_detail_view);
    lv_obj_set_pos(ui_detail_title, 92, 8);
    lv_obj_set_width(ui_detail_title, 500);
    lv_label_set_long_mode(ui_detail_title, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_color(ui_detail_title, lv_color_white(), LV_PART_MAIN);

    ui_spool_create_caption(ui_spool_detail_view, "Vendor", 92, 38);
    ui_detail_vendor = ui_spool_create_value_label(ui_spool_detail_view, 160, 38, 260);

    ui_spool_create_caption(ui_spool_detail_view, "Material", 92, 62);
    ui_detail_material = ui_spool_create_value_label(ui_spool_detail_view, 160, 62, 180);

    // Two-column information area
    ui_spool_create_caption(ui_spool_detail_view, "Remaining", 20, 105);
    ui_detail_remaining = ui_spool_create_value_label(ui_spool_detail_view, 135, 105, 150);

    ui_spool_create_caption(ui_spool_detail_view, "Initial", 20, 140);
    ui_detail_initial = ui_spool_create_value_label(ui_spool_detail_view, 135, 140, 150);

    ui_spool_create_caption(ui_spool_detail_view, "Empty spool", 20, 175);
    ui_detail_spool_weight = ui_spool_create_value_label(ui_spool_detail_view, 135, 175, 150);

    ui_spool_create_caption(ui_spool_detail_view, "Location", 380, 105);
    ui_detail_location = ui_spool_create_value_label(ui_spool_detail_view, 475, 105, 230);

    ui_spool_create_caption(ui_spool_detail_view, "Spoolman ID", 380, 140);
    ui_detail_id = ui_spool_create_value_label(ui_spool_detail_view, 475, 140, 150);

    lv_obj_t *back_button = lv_button_create(ui_spool_detail_view);
    lv_obj_set_size(back_button, 130, 42);
    lv_obj_align(back_button, LV_ALIGN_BOTTOM_LEFT, 10, 0);
    lv_obj_add_event_cb(back_button, ui_spool_back_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *back_label = lv_label_create(back_button);
    lv_label_set_text(back_label, "< BACK");
    lv_obj_center(back_label);

    lv_obj_t *use_scale_button = lv_button_create(ui_spool_detail_view);
    lv_obj_set_size(use_scale_button, 180, 42);
    lv_obj_align(use_scale_button, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
    lv_obj_add_event_cb(use_scale_button, ui_spool_use_on_scale_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *use_scale_label = lv_label_create(use_scale_button);
    lv_label_set_text(use_scale_label, "USE ON SCALE");
    lv_obj_center(use_scale_label);

    lv_obj_add_flag(ui_spool_detail_view, LV_OBJ_FLAG_HIDDEN);

    ui_spool_first_index = 0;
    ui_spools_refresh_rows();
    ui_spool_show_list_view();

    return page;
}
