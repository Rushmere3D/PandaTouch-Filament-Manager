#pragma once

#include <lvgl.h>

#include "ui/ui_spools.h"
#include "ui/ui_scale.h"
#include "ui/ui_settings.h"


// ---------------------------------------------------------
// Application pages
// ---------------------------------------------------------

static lv_obj_t *ui_page_spools   = nullptr;
static lv_obj_t *ui_page_scale    = nullptr;
static lv_obj_t *ui_page_settings = nullptr;


// ---------------------------------------------------------
// Navigation buttons
// ---------------------------------------------------------

static lv_obj_t *ui_nav_spools   = nullptr;
static lv_obj_t *ui_nav_scale    = nullptr;
static lv_obj_t *ui_nav_settings = nullptr;


// ---------------------------------------------------------
// Page identifiers
// ---------------------------------------------------------

enum UiPage
{
    UI_PAGE_SPOOLS,
    UI_PAGE_SCALE,
    UI_PAGE_SETTINGS
};


// ---------------------------------------------------------
// Update navigation button appearance
// ---------------------------------------------------------

static void ui_update_nav_buttons(UiPage active_page)
{
    // Default button colour

    lv_obj_set_style_bg_color(
        ui_nav_spools,
        lv_color_hex(0x303840),
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        ui_nav_scale,
        lv_color_hex(0x303840),
        LV_PART_MAIN);

    lv_obj_set_style_bg_color(
        ui_nav_settings,
        lv_color_hex(0x303840),
        LV_PART_MAIN);


    // Highlight active button

    lv_obj_t *active_button = nullptr;

    switch (active_page)
    {
        case UI_PAGE_SPOOLS:
            active_button = ui_nav_spools;
            break;

        case UI_PAGE_SCALE:
            active_button = ui_nav_scale;
            break;

        case UI_PAGE_SETTINGS:
            active_button = ui_nav_settings;
            break;
    }

    if (active_button != nullptr)
    {
        lv_obj_set_style_bg_color(
            active_button,
            lv_color_hex(0x1976D2),
            LV_PART_MAIN);
    }
}


// ---------------------------------------------------------
// Switch pages
// ---------------------------------------------------------

static void ui_show_page(UiPage page)
{
    // Hide everything first

    lv_obj_add_flag(
        ui_page_spools,
        LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(
        ui_page_scale,
        LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(
        ui_page_settings,
        LV_OBJ_FLAG_HIDDEN);


    // Show requested page

    switch (page)
    {
        case UI_PAGE_SPOOLS:

            lv_obj_remove_flag(
                ui_page_spools,
                LV_OBJ_FLAG_HIDDEN);

            break;


        case UI_PAGE_SCALE:

            lv_obj_remove_flag(
                ui_page_scale,
                LV_OBJ_FLAG_HIDDEN);

            break;


        case UI_PAGE_SETTINGS:

            lv_obj_remove_flag(
                ui_page_settings,
                LV_OBJ_FLAG_HIDDEN);

            break;
    }

    ui_update_nav_buttons(page);
}


// ---------------------------------------------------------
// Navigation callback
// ---------------------------------------------------------

static void ui_nav_event_cb(lv_event_t *e)
{
    UiPage page =
        (UiPage)(intptr_t)lv_event_get_user_data(e);

    ui_show_page(page);
}


// ---------------------------------------------------------
// Create navigation button
// ---------------------------------------------------------

static lv_obj_t *ui_create_nav_button(
    lv_obj_t *parent,
    const char *text,
    UiPage page)
{
    lv_obj_t *button =
        lv_button_create(parent);

    lv_obj_set_size(
        button,
        200,
        70);

    lv_obj_add_event_cb(
        button,
        ui_nav_event_cb,
        LV_EVENT_CLICKED,
        (void *)(intptr_t)page);

    lv_obj_t *label =
        lv_label_create(button);

    lv_label_set_text(
        label,
        text);

    lv_obj_center(label);

    return button;
}


// ---------------------------------------------------------
// Create main application
// ---------------------------------------------------------

inline void ui_create_main()
{
    // -----------------------------------------------------
    // Main screen
    // -----------------------------------------------------

    lv_obj_t *screen =
        lv_obj_create(NULL);

    lv_obj_set_style_bg_color(
        screen,
        lv_color_hex(0x101418),
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        screen,
        LV_OPA_COVER,
        LV_PART_MAIN);

    lv_obj_clear_flag(
        screen,
        LV_OBJ_FLAG_SCROLLABLE);


    // -----------------------------------------------------
    // Application title
    // -----------------------------------------------------

    lv_obj_t *title =
        lv_label_create(screen);

    lv_label_set_text(
        title,
        "FILAMENT MANAGER");

    lv_obj_set_style_text_color(
        title,
        lv_color_white(),
        LV_PART_MAIN);

    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        25);


    // -----------------------------------------------------
    // Main content container
    // -----------------------------------------------------

    lv_obj_t *content =
        lv_obj_create(screen);

    lv_obj_set_size(
        content,
        780,
        310);

    lv_obj_align(
        content,
        LV_ALIGN_TOP_MID,
        0,
        65);

    lv_obj_set_style_border_width(
        content,
        0,
        LV_PART_MAIN);

    lv_obj_set_style_bg_opa(
        content,
        LV_OPA_TRANSP,
        LV_PART_MAIN);

    lv_obj_set_style_pad_all(
        content,
        0,
        LV_PART_MAIN);

    lv_obj_clear_flag(
        content,
        LV_OBJ_FLAG_SCROLLABLE);


    // -----------------------------------------------------
    // Create our three pages
    // -----------------------------------------------------

    ui_page_spools =
        ui_spools_create(content);

    ui_page_scale =
        ui_scale_create(content);

    ui_page_settings =
        ui_settings_create(content);


    // -----------------------------------------------------
    // Bottom navigation
    // -----------------------------------------------------

    lv_obj_t *nav =
        lv_obj_create(screen);

    lv_obj_set_size(
        nav,
        760,
        90);

    lv_obj_align(
        nav,
        LV_ALIGN_BOTTOM_MID,
        0,
        -5);

    lv_obj_set_flex_flow(
        nav,
        LV_FLEX_FLOW_ROW);

    lv_obj_set_flex_align(
        nav,
        LV_FLEX_ALIGN_SPACE_EVENLY,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    lv_obj_clear_flag(
        nav,
        LV_OBJ_FLAG_SCROLLABLE);


    // -----------------------------------------------------
    // Create navigation buttons
    // -----------------------------------------------------

    ui_nav_spools =
        ui_create_nav_button(
            nav,
            "SPOOLS",
            UI_PAGE_SPOOLS);

    ui_nav_scale =
        ui_create_nav_button(
            nav,
            "SCALE",
            UI_PAGE_SCALE);

    ui_nav_settings =
        ui_create_nav_button(
            nav,
            "SETTINGS",
            UI_PAGE_SETTINGS);


    // -----------------------------------------------------
    // Start on Spools page
    // -----------------------------------------------------

    ui_show_page(
        UI_PAGE_SPOOLS);


    // -----------------------------------------------------
    // Display our application
    // -----------------------------------------------------

    lv_screen_load(screen);
}