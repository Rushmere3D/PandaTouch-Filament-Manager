#pragma once

#include <lvgl.h>

#include <stdlib.h>

#include <stdio.h>

#include "../models/spool_store.h"

constexpr size_t SPOOL_ROWS_VISIBLE = 4;

static size_t ui_spool_first_index = 0;

static size_t ui_spool_selected_index = 0;
static size_t ui_spool_filtered_indices[MAX_SPOOLS];
static size_t ui_spool_filtered_count = 0;
static size_t ui_spool_indexed_source_count = (size_t)-1;
static lv_obj_t *ui_spool_search_input = nullptr;
static lv_obj_t *ui_spool_search_keyboard = nullptr;
static lv_obj_t *ui_spool_search_clear = nullptr;


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

// Supports two or three comma-separated Spoolman colours.

inline size_t ui_spool_parse_colors(const String &multi, String colors[3])

{

    size_t count = 0;

    int start = 0;

    while (count < 3 && start < (int)multi.length())

    {

        int comma = multi.indexOf(',', start);

        String color = comma < 0 ? multi.substring(start) : multi.substring(start, comma);

        color.trim();

        if (color.startsWith("#")) color.remove(0, 1);

        if (color.length() < 6) break;

        colors[count++] = color;

        if (comma < 0) break;

        start = comma + 1;

    }

    return count;

}

inline void ui_spool_apply_swatch(lv_obj_t *swatch, const SpoolInfo &spool)

{

    String colors[3];

    size_t count = ui_spool_parse_colors(spool.multi_color_hexes, colors);

    // Three reusable child strips allow 2- and 3-colour swatches.

    // LVGL's simple background gradient only supports two colours.

    lv_obj_t *strips[3];

    for (int i = 0; i < 3; i++) strips[i] = lv_obj_get_child(swatch, i);

    lv_obj_set_style_bg_grad_dir(swatch, LV_GRAD_DIR_NONE, LV_PART_MAIN);

    lv_obj_set_style_border_width(swatch, 1, LV_PART_MAIN);

    lv_obj_set_style_border_color(swatch, lv_color_hex(0x808890), LV_PART_MAIN);

    if (count >= 2)

    {

        lv_obj_set_style_bg_color(swatch, ui_spool_hex_to_color(colors[0]), LV_PART_MAIN);

        int width = lv_obj_get_width(swatch) - 2;

        int height = lv_obj_get_height(swatch) - 2;

        if (width < 3) width = 18;

        if (height < 3) height = 42;

        for (int i = 0; i < 3; i++)

        {

            if (i >= (int)count)

            {

                lv_obj_add_flag(strips[i], LV_OBJ_FLAG_HIDDEN);

                continue;

            }

            int left = (width * i) / (int)count;

            int right = (width * (i + 1)) / (int)count;

            lv_obj_set_pos(strips[i], 1 + left, 1);

            lv_obj_set_size(strips[i], right - left, height);

            lv_obj_set_style_bg_color(strips[i], ui_spool_hex_to_color(colors[i]), LV_PART_MAIN);

            lv_obj_remove_flag(strips[i], LV_OBJ_FLAG_HIDDEN);

        }

    }

    else

    {

        for (int i = 0; i < 3; i++) lv_obj_add_flag(strips[i], LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_style_bg_color(swatch, ui_spool_hex_to_color(spool.color_hex), LV_PART_MAIN);

        if (ui_spool_hex_alpha(spool.color_hex) < 255)

        {

            lv_obj_set_style_border_width(swatch, 3, LV_PART_MAIN);

            lv_obj_set_style_border_color(swatch, lv_color_white(), LV_PART_MAIN);

        }

        else

        {

            lv_obj_set_style_border_color(swatch, lv_color_hex(0x606870), LV_PART_MAIN);

        }

    }

}

// Create the strips once, not every time a spool page is refreshed.

inline void ui_spool_prepare_swatch(lv_obj_t *swatch)

{

    for (int i = 0; i < 3; i++)

    {

        lv_obj_t *strip = lv_obj_create(swatch);

        lv_obj_set_style_pad_all(strip, 0, LV_PART_MAIN);

        lv_obj_set_style_border_width(strip, 0, LV_PART_MAIN);

        lv_obj_set_style_radius(strip, 0, LV_PART_MAIN);

        lv_obj_clear_flag(strip, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_clear_flag(strip, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_add_flag(strip, LV_OBJ_FLAG_HIDDEN);

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

// Shared neutral button palette for the Spools page.

inline void ui_spool_style_button(lv_obj_t *button)

{

    lv_obj_set_style_bg_color(button, lv_color_hex(0x394650), LV_PART_MAIN);

    lv_obj_set_style_bg_color(button, lv_color_hex(0x4B5B68), LV_PART_MAIN | LV_STATE_PRESSED);

    lv_obj_set_style_bg_color(button, lv_color_hex(0x252D34), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_set_style_text_color(button, lv_color_white(), LV_PART_MAIN);

    lv_obj_set_style_text_color(button, lv_color_hex(0x7D8993), LV_PART_MAIN | LV_STATE_DISABLED);

    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);

    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);

    lv_obj_set_style_radius(button, 7, LV_PART_MAIN);

    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DISABLED);

}

inline void ui_spools_refresh_rows();

inline void ui_spools_rebuild_filter()
{
    ui_spool_indexed_source_count = spool_count;
    ui_spool_filtered_count = 0;
    String query = ui_spool_search_input ? String(lv_textarea_get_text(ui_spool_search_input)) : String("");
    query.trim();
    query.toLowerCase();

    for (size_t i = 0; i < spool_count && i < MAX_SPOOLS; i++)
    {
        const SpoolInfo &spool = spool_store[i];
        String id = String(spool.id);
        String vendor = spool.vendor; vendor.toLowerCase();
        String name = spool.name; name.toLowerCase();
        String material = spool.material; material.toLowerCase();
        if (query.isEmpty() || vendor.indexOf(query) >= 0 ||
            name.indexOf(query) >= 0 || material.indexOf(query) >= 0 ||
            id.indexOf(query) >= 0)
            ui_spool_filtered_indices[ui_spool_filtered_count++] = i;
    }
    ui_spool_first_index = 0;
    ui_spools_refresh_rows();
}

inline void ui_spools_search_changed_cb(lv_event_t *e)
{
    (void)e;
    ui_spools_rebuild_filter();
}

inline void ui_spools_search_close_keyboard()
{
    if (ui_spool_search_keyboard != nullptr)
    {
        lv_obj_delete(ui_spool_search_keyboard);
        ui_spool_search_keyboard = nullptr;
    }
    if (ui_spool_search_input != nullptr)
        lv_obj_remove_state(ui_spool_search_input, LV_STATE_FOCUSED);
}

inline void ui_spools_keyboard_done_cb(lv_event_t *e)
{
    (void)e;
    ui_spools_search_close_keyboard();
}

inline void ui_spools_search_focus_cb(lv_event_t *e)
{
    (void)e;
    if (ui_spool_search_keyboard != nullptr) return;
    ui_spool_search_keyboard = lv_keyboard_create(lv_screen_active());
    lv_obj_set_size(ui_spool_search_keyboard, LV_PCT(100), 215);
    lv_obj_align(ui_spool_search_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(ui_spool_search_keyboard, ui_spool_search_input);
    lv_obj_add_event_cb(ui_spool_search_keyboard, ui_spools_keyboard_done_cb, LV_EVENT_READY, nullptr);
    lv_obj_add_event_cb(ui_spool_search_keyboard, ui_spools_keyboard_done_cb, LV_EVENT_CANCEL, nullptr);
    lv_obj_move_foreground(ui_spool_search_keyboard);
}

inline void ui_spools_search_clear_cb(lv_event_t *e)
{
    (void)e;
    lv_textarea_set_text(ui_spool_search_input, "");
    ui_spools_search_close_keyboard();
    ui_spools_rebuild_filter();
}

inline void ui_spools_update_nav_buttons()

{

    bool at_first = (ui_spool_first_index == 0);

    bool at_last =

        (ui_spool_filtered_count == 0) ||

        (ui_spool_first_index + SPOOL_ROWS_VISIBLE >= ui_spool_filtered_count);

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
    // Inventory is fetched asynchronously. Reindex when the source count changes.
    if (ui_spool_indexed_source_count != spool_count)
    {
        ui_spools_rebuild_filter();
        return;
    }

    for (size_t row = 0; row < SPOOL_ROWS_VISIBLE; row++)

    {

        size_t result_index = ui_spool_first_index + row;
        size_t spool_index = result_index < ui_spool_filtered_count ? ui_spool_filtered_indices[result_index] : spool_count;

        if (spool_index >= spool_count)

        {

            lv_obj_add_flag(ui_spool_rows[row], LV_OBJ_FLAG_HIDDEN);

            continue;

        }

        const SpoolInfo &spool = spool_store[spool_index];

        lv_obj_remove_flag(ui_spool_rows[row], LV_OBJ_FLAG_HIDDEN);

        ui_spool_apply_swatch(ui_spool_swatches[row], spool);

        lv_label_set_text(ui_spool_vendors[row], spool.vendor.c_str());

        char secondary_text[72];

        snprintf(secondary_text, sizeof(secondary_text), "Spool #%d  -  %s", spool.id, spool.material.c_str());

        lv_label_set_text(ui_spool_materials[row], secondary_text);

        lv_label_set_text(ui_spool_names[row], spool.name.c_str());

        char weight_text[32];

        snprintf(weight_text, sizeof(weight_text), "%dg remaining",

                 (int)(spool.remaining_weight + 0.5f));

        lv_label_set_text(ui_spool_weights[row], weight_text);

    }

    if (ui_spool_page_label != nullptr)

    {

        char page_text[40];

        if (ui_spool_filtered_count == 0)

        {

            snprintf(page_text, sizeof(page_text), "No matches");

        }

        else

        {

            size_t first = ui_spool_first_index + 1;

            size_t last = ui_spool_first_index + SPOOL_ROWS_VISIBLE;

            if (last > ui_spool_filtered_count) last = ui_spool_filtered_count;

            snprintf(page_text, sizeof(page_text), "%u-%u of %u",

                     (unsigned int)first,

                     (unsigned int)last,

                     (unsigned int)ui_spool_filtered_count);

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

    size_t result_index = ui_spool_first_index + row;
        size_t spool_index = result_index < ui_spool_filtered_count ? ui_spool_filtered_indices[result_index] : spool_count;

    if (spool_index < spool_count) ui_spool_show_detail_view(spool_index);

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

    if (ui_spool_first_index + SPOOL_ROWS_VISIBLE < ui_spool_filtered_count)

    {

        ui_spool_first_index += SPOOL_ROWS_VISIBLE;

        ui_spools_refresh_rows();

    }

}

inline void ui_spools_last_cb(lv_event_t *e)

{

    (void)e;

    if (ui_spool_filtered_count == 0)

        ui_spool_first_index = 0;

    else

        ui_spool_first_index =

            ((ui_spool_filtered_count - 1) / SPOOL_ROWS_VISIBLE) * SPOOL_ROWS_VISIBLE;

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

    ui_spool_search_input = lv_textarea_create(page);
    lv_obj_set_size(ui_spool_search_input, 535, 30);
    lv_obj_align(ui_spool_search_input, LV_ALIGN_TOP_LEFT, 8, -5);
    lv_textarea_set_one_line(ui_spool_search_input, true);
    lv_textarea_set_placeholder_text(ui_spool_search_input, "Search spools...");
    lv_obj_set_style_bg_color(ui_spool_search_input, lv_color_hex(0x26313A), LV_PART_MAIN);
    lv_obj_set_style_text_color(ui_spool_search_input, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_spool_search_input, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(ui_spool_search_input, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_top(ui_spool_search_input, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(ui_spool_search_input, 3, LV_PART_MAIN);
    lv_obj_add_event_cb(ui_spool_search_input, ui_spools_search_focus_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(ui_spool_search_input, ui_spools_search_changed_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    ui_spool_search_clear = lv_button_create(page);
    ui_spool_style_button(ui_spool_search_clear);
    lv_obj_set_size(ui_spool_search_clear, 45, 30);
    lv_obj_align(ui_spool_search_clear, LV_ALIGN_TOP_LEFT, 550, -5);
    lv_obj_add_event_cb(ui_spool_search_clear, ui_spools_search_clear_cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *clear_label = lv_label_create(ui_spool_search_clear);
    lv_label_set_text(clear_label, "X");
    lv_obj_center(clear_label);

    ui_spool_list_view = lv_obj_create(page);

    lv_obj_set_size(ui_spool_list_view, LV_PCT(100), 240);

    lv_obj_align(ui_spool_list_view, LV_ALIGN_TOP_MID, 0, 27);

    lv_obj_set_style_bg_color(ui_spool_list_view, lv_color_hex(0x101418), LV_PART_MAIN);

    lv_obj_set_style_border_width(ui_spool_list_view, 0, LV_PART_MAIN);

    lv_obj_set_style_pad_all(ui_spool_list_view, 0, LV_PART_MAIN);

    lv_obj_set_style_pad_row(ui_spool_list_view, 0, LV_PART_MAIN);

    lv_obj_set_flex_flow(ui_spool_list_view, LV_FLEX_FLOW_COLUMN);

    lv_obj_clear_flag(ui_spool_list_view, LV_OBJ_FLAG_SCROLLABLE);

    for (size_t row = 0; row < SPOOL_ROWS_VISIBLE; row++)

    {

        ui_spool_rows[row] = lv_obj_create(ui_spool_list_view);

        lv_obj_set_width(ui_spool_rows[row], LV_PCT(100));

        lv_obj_set_height(ui_spool_rows[row], 54);

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

        lv_obj_set_size(ui_spool_swatches[row], 20, 44);

        lv_obj_align(ui_spool_swatches[row], LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_set_style_pad_all(ui_spool_swatches[row], 0, LV_PART_MAIN);

        lv_obj_set_style_border_width(ui_spool_swatches[row], 1, LV_PART_MAIN);

        lv_obj_set_style_border_color(ui_spool_swatches[row], lv_color_hex(0x606870), LV_PART_MAIN);

        lv_obj_set_style_radius(ui_spool_swatches[row], 3, LV_PART_MAIN);

        lv_obj_clear_flag(ui_spool_swatches[row], LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_clear_flag(ui_spool_swatches[row], LV_OBJ_FLAG_CLICKABLE);

        ui_spool_prepare_swatch(ui_spool_swatches[row]);

        ui_spool_vendors[row] = lv_label_create(ui_spool_rows[row]);

        lv_obj_set_width(ui_spool_vendors[row], 210);

        lv_label_set_long_mode(ui_spool_vendors[row], LV_LABEL_LONG_DOT);

        lv_obj_set_style_text_color(ui_spool_vendors[row], lv_color_white(), LV_PART_MAIN);

        lv_obj_align(ui_spool_vendors[row], LV_ALIGN_LEFT_MID, 36, -11);

        ui_spool_materials[row] = lv_label_create(ui_spool_rows[row]);

        lv_obj_set_width(ui_spool_materials[row], 280);

        lv_label_set_long_mode(ui_spool_materials[row], LV_LABEL_LONG_DOT);

        lv_obj_set_style_text_color(ui_spool_materials[row], lv_color_hex(0xAAB2BA), LV_PART_MAIN);

        lv_obj_align(ui_spool_materials[row], LV_ALIGN_LEFT_MID, 36, 7);

        ui_spool_names[row] = lv_label_create(ui_spool_rows[row]);

        lv_obj_set_width(ui_spool_names[row], 310);

        lv_label_set_long_mode(ui_spool_names[row], LV_LABEL_LONG_DOT);

        lv_obj_set_style_text_color(ui_spool_names[row], lv_color_white(), LV_PART_MAIN);

        lv_obj_align(ui_spool_names[row], LV_ALIGN_LEFT_MID, 238, 0);

        ui_spool_weights[row] = lv_label_create(ui_spool_rows[row]);

        lv_obj_set_width(ui_spool_weights[row], 150);

        lv_obj_set_style_text_align(ui_spool_weights[row], LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);

        lv_obj_set_style_text_color(ui_spool_weights[row], lv_color_hex(0xAAB2BA), LV_PART_MAIN);

        lv_obj_align(ui_spool_weights[row], LV_ALIGN_RIGHT_MID, -5, 0);

    }

    // --------------------------------------------------------

    // FOUR-BUTTON PAGE NAVIGATION

    // --------------------------------------------------------

    ui_spool_first_button = lv_button_create(page);

    ui_spool_style_button(ui_spool_first_button);

    lv_obj_set_size(ui_spool_first_button, 150, 42);

    lv_obj_align(ui_spool_first_button, LV_ALIGN_BOTTOM_LEFT, 10, 0);

    lv_obj_add_event_cb(ui_spool_first_button, ui_spools_first_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *first_label = lv_label_create(ui_spool_first_button);

    lv_label_set_text(first_label, "|< FIRST");

    lv_obj_center(first_label);

    ui_spool_previous_button = lv_button_create(page);

    ui_spool_style_button(ui_spool_previous_button);

    lv_obj_set_size(ui_spool_previous_button, 150, 42);

    lv_obj_align(ui_spool_previous_button, LV_ALIGN_BOTTOM_LEFT, 200, 0);

    lv_obj_add_event_cb(ui_spool_previous_button, ui_spools_previous_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *previous_label = lv_label_create(ui_spool_previous_button);

    lv_label_set_text(previous_label, "< PREV");

    lv_obj_center(previous_label);

    ui_spool_next_button = lv_button_create(page);

    ui_spool_style_button(ui_spool_next_button);

    lv_obj_set_size(ui_spool_next_button, 150, 42);

    lv_obj_align(ui_spool_next_button, LV_ALIGN_BOTTOM_RIGHT, -200, 0);

    lv_obj_add_event_cb(ui_spool_next_button, ui_spools_next_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *next_label = lv_label_create(ui_spool_next_button);

    lv_label_set_text(next_label, "NEXT >");

    lv_obj_center(next_label);

    ui_spool_last_button = lv_button_create(page);

    ui_spool_style_button(ui_spool_last_button);

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

    lv_obj_set_size(ui_detail_swatch, 16, 64);

    lv_obj_set_pos(ui_detail_swatch, 10, 8);

    lv_obj_set_style_pad_all(ui_detail_swatch, 0, LV_PART_MAIN);

    lv_obj_set_style_radius(ui_detail_swatch, 4, LV_PART_MAIN);

    lv_obj_clear_flag(ui_detail_swatch, LV_OBJ_FLAG_SCROLLABLE);

    ui_spool_prepare_swatch(ui_detail_swatch);

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

    ui_spool_style_button(back_button);

    lv_obj_set_size(back_button, 130, 42);

    lv_obj_align(back_button, LV_ALIGN_BOTTOM_LEFT, 10, 0);

    lv_obj_add_event_cb(back_button, ui_spool_back_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *back_label = lv_label_create(back_button);

    lv_label_set_text(back_label, "< BACK");

    lv_obj_center(back_label);

    lv_obj_t *use_scale_button = lv_button_create(ui_spool_detail_view);

    ui_spool_style_button(use_scale_button);

    lv_obj_set_size(use_scale_button, 180, 42);

    lv_obj_align(use_scale_button, LV_ALIGN_BOTTOM_RIGHT, -10, 0);

    lv_obj_add_event_cb(use_scale_button, ui_spool_use_on_scale_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *use_scale_label = lv_label_create(use_scale_button);

    lv_label_set_text(use_scale_label, "USE ON SCALE");

    lv_obj_center(use_scale_label);

    lv_obj_add_flag(ui_spool_detail_view, LV_OBJ_FLAG_HIDDEN);

    ui_spool_first_index = 0;

    ui_spools_rebuild_filter();

    ui_spool_show_list_view();

    return page;

}
