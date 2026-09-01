/**
 * @file main/ui/ui.c
 * @brief UI screens for the mnemonic tool.
 */

#include "ui.h"
#include "assets/logo_img.h"
#include "assets/splash_img.h"
#include "mnemonic_view.h"
#include "src/widgets/label/lv_label_private.h"
#include "ui_internal.h"
#include "util/error.h"
#include "util/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ESP_PLATFORM
#include <SDL2/SDL.h>
#endif

/* -- Helpers ---------------------------------------------------------- */
static lv_obj_t* main_scr   = NULL;
static lv_obj_t* entered_ta = NULL;
static char      entered_buf[512];
static lv_obj_t* prev_screen = NULL; // track for deferred cleanup

static lv_obj_t*      camera_img = NULL;       // live camera feed image widget
static lv_image_dsc_t camera_dsc;              // descriptor backing the live feed
static lv_image_dsc_t seedqr_dsc;              // descriptor backing the SeedQR image
static uint8_t*       seedqr_buf       = NULL; // RGB565 buffer for the SeedQR image
static size_t         seedqr_buf_bytes = 0;

// Recursively zero the text of every label under `obj`.  lv_label_set_text()
// copies strings into LVGL-allocated memory; freeing the object does NOT wipe
// those copies, so mnemonic words and entropy hex would linger in the heap.
static void wipe_label_texts(lv_obj_t* obj) {
    uint32_t n = lv_obj_get_child_count(obj);
    for (uint32_t i = 0; i < n; i++) {
        wipe_label_texts(lv_obj_get_child(obj, i));
    }
    if (lv_obj_has_class(obj, &lv_label_class)) {
        lv_label_t* label = (lv_label_t*)obj;
        if (label->text) secure_memzero(label->text, strlen(label->text));
        if (label->dot_tmp_alloc && label->dot.tmp_ptr) {
            secure_memzero(label->dot.tmp_ptr, strlen(label->dot.tmp_ptr));
        } else {
            secure_memzero(label->dot.tmp, sizeof(label->dot.tmp));
        }
    }
}

void ui_scrub_screen(lv_obj_t* scr) {
    if (scr && scr != main_scr) wipe_label_texts(scr);
}

lv_obj_t* ui_make_screen(void) {
    lv_obj_t* s = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s, lv_color_black(), 0);
    return s;
}

// Deferred screen deletion - safe to call from event handlers
static void delete_screen_cb(void* ptr) {
    lv_obj_t* scr = (lv_obj_t*)ptr;
    if (scr && scr != main_scr) {
        wipe_label_texts(scr); // wipe secrets before freeing
        lv_obj_delete(scr);
    }
}

void ui_swap_screen(lv_obj_t* new_scr) {
    if (prev_screen && prev_screen != main_scr) {
        lv_async_call(delete_screen_cb, prev_screen);
    }
    prev_screen = new_scr;
    lv_scr_load(new_scr);
}

lv_obj_t* ui_add_title(lv_obj_t* parent, const char* text) {
    lv_obj_t* t = lv_label_create(parent);
    lv_label_set_text(t, text);
    lv_obj_set_style_text_color(t, lv_color_white(), 0);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_28, 0);
    lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 15);
    return t;
}

void ui_btn_invoke(lv_event_t* e) {
    union {
        ui_cb_t fn;
        void*   vp;
    } u;
    u.vp = lv_event_get_user_data(e);
    if (u.fn) u.fn();
}

/* -- Button creation -------------------------------------------------- */
static const struct {
    lv_coord_t       w;
    lv_coord_t       h;
    const lv_font_t* font;
} btn_sizes[] = {
    [UI_BTN_SIZE_SMALL] = {80, 30, &lv_font_montserrat_14},
    [UI_BTN_SIZE_MED]   = {160, 44, &lv_font_montserrat_24},
    [UI_BTN_SIZE_LARGE] = {200, 44, &lv_font_montserrat_24},
    [UI_BTN_SIZE_WIDE]  = {180, 44, &lv_font_montserrat_24},
    [UI_BTN_SIZE_HERO]  = {240, 56, &lv_font_montserrat_28},
};

static lv_obj_t* add_btn_impl(lv_obj_t* parent, const char* text, ui_btn_size_t size,
                              lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs) {
    lv_obj_t* b = lv_button_create(parent);
    lv_obj_set_size(b, btn_sizes[size].w, btn_sizes[size].h);
    lv_obj_align(b, align, x_ofs, y_ofs);
    lv_obj_t* l = lv_label_create(b);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_font(l, btn_sizes[size].font, 0);
    lv_obj_center(l);
    return b;
}

lv_obj_t* ui_add_btn(lv_obj_t* parent, const char* text, ui_cb_t cb, ui_btn_size_t size,
                     lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs) {
    lv_obj_t* b = add_btn_impl(parent, text, size, align, x_ofs, y_ofs);
    if (cb) {
        union {
            ui_cb_t fn;
            void*   vp;
        } u = {.fn = cb};
        lv_obj_add_event_cb(b, ui_btn_invoke, LV_EVENT_CLICKED, u.vp);
    }
    return b;
}

lv_obj_t* ui_add_btn_evt(lv_obj_t* parent, const char* text, lv_event_cb_t evt_cb, void* user_data,
                         ui_btn_size_t size, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs) {
    lv_obj_t* b = add_btn_impl(parent, text, size, align, x_ofs, y_ofs);
    if (evt_cb) lv_obj_add_event_cb(b, evt_cb, LV_EVENT_CLICKED, user_data);
    return b;
}

void ui_go_main(void) {
    if (main_scr) lv_scr_load(main_scr);
}

/* -- Splash screen ---------------------------------------------------- */
typedef struct {
    ui_cb_t   on_done;
    lv_obj_t* screen;
} splash_ctx_t;

static void splash_timer_cb(lv_timer_t* t) {
    splash_ctx_t* ctx     = (splash_ctx_t*)lv_timer_get_user_data(t);
    ui_cb_t       on_done = ctx->on_done;
    lv_obj_t*     screen  = ctx->screen;

    lv_timer_delete(t);
    lv_free(ctx);

    if (on_done) on_done(); // swaps to the next screen
    lv_obj_delete(screen);  // splash is no longer active - safe to free
}

void ui_show_splash(ui_cb_t on_done) {
    lv_obj_t* s = ui_make_screen();

    lv_obj_t* img = lv_image_create(s);
    lv_image_set_src(img, &splash_img_dsc);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    lv_scr_load(s);

    splash_ctx_t* ctx = lv_malloc(sizeof(*ctx));
    ASSERT_OR_DIE(ctx, "splash ctx alloc");
    ctx->on_done  = on_done;
    ctx->screen   = s;
    lv_timer_t* t = lv_timer_create(splash_timer_cb, 2000, ctx);
    lv_timer_set_repeat_count(t, 1);
}

/* -- Screens ---------------------------------------------------------- */
void ui_show_main(lv_event_cb_t on_new_wallet, lv_event_cb_t on_test_error) {
    ASSERT_OR_DIE(on_new_wallet, "null on_new_wallet");
    ASSERT_OR_DIE(on_test_error, "null on_test_error");

    if (!main_scr) {
        main_scr = ui_make_screen();

        // Build main screen with "New Wallet" button
        lv_obj_t* scr = main_scr;
        lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

        lv_obj_t* logo = lv_image_create(scr);
        lv_image_set_src(logo, &logo_img_dsc);
        lv_obj_align(logo, LV_ALIGN_TOP_LEFT, 10, 10);

        ui_add_btn_evt(scr, "New Wallet", on_new_wallet, NULL, UI_BTN_SIZE_HERO, LV_ALIGN_CENTER, 0,
                       0);

        // Test error button
        lv_obj_t* test_btn = ui_add_btn_evt(scr, "test error!", on_test_error, NULL,
                                            UI_BTN_SIZE_SMALL, LV_ALIGN_BOTTOM_RIGHT, -40, -10);
        lv_obj_set_style_bg_color(test_btn, lv_color_hex(0x440000), 0);
    }
    lv_scr_load(main_scr);
    // clean up any leaked screen from before main was first shown
    if (prev_screen && prev_screen != main_scr) {
        lv_async_call(delete_screen_cb, prev_screen);
        prev_screen = NULL;
    }
}

void ui_show_word_count(ui_cb_t on_12, ui_cb_t on_24) {
    ASSERT_OR_DIE(on_12, "null on_12");
    ASSERT_OR_DIE(on_24, "null on_24");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "How many words?");
    ui_add_btn(s, "12 words", on_12, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, -40);
    ui_add_btn(s, "24 words", on_24, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, 30);
    ui_swap_screen(s);
}

void ui_show_source(ui_cb_t on_generate, ui_cb_t on_enter, ui_cb_t on_other_source,
                    ui_cb_t on_state, ui_cb_t on_finish, bool is_additional) {
    ASSERT_OR_DIE(on_generate, "null on_generate");
    ASSERT_OR_DIE(on_enter, "null on_enter");
    ASSERT_OR_DIE(on_other_source, "null on_other_source");
    ASSERT_OR_DIE(on_state, "null on_state");
    ASSERT_OR_DIE(on_finish, "null on_finish");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, is_additional ? "Choose Additional Source" : "Choose Initial Source");
    ui_add_btn(s, "Generate Here", on_generate, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, -40);
    ui_add_btn(s, "Enter Manually", on_enter, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, 30);
    ui_add_btn(s, "Other Source", on_other_source, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, 100);

    // state button (bottom-left)
    ui_add_btn(s, "State", on_state, UI_BTN_SIZE_SMALL, LV_ALIGN_BOTTOM_LEFT, 10, -10);

    // finish button (bottom-right)
    ui_add_btn(s, "Finish", on_finish, UI_BTN_SIZE_SMALL, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    ui_swap_screen(s);
}

void ui_show_other_source(ui_cb_t on_camera, ui_cb_t on_scan_qr, ui_cb_t on_dice, ui_cb_t on_coins,
                          ui_cb_t on_touch, ui_cb_t on_back) {
    ASSERT_OR_DIE(on_camera, "null on_camera");
    ASSERT_OR_DIE(on_scan_qr, "null on_scan_qr");
    ASSERT_OR_DIE(on_dice, "null on_dice");
    ASSERT_OR_DIE(on_coins, "null on_coins");
    ASSERT_OR_DIE(on_touch, "null on_touch");
    ASSERT_OR_DIE(on_back, "null on_back");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "Other Sources");
    ui_add_btn(s, "Camera Image", on_camera, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, -86);
    ui_add_btn(s, "Scan QR", on_scan_qr, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, -34);
    ui_add_btn(s, "Dice Rolls", on_dice, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, 18);
    ui_add_btn(s, "Coin Flips", on_coins, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, 70);
    ui_add_btn(s, "Touch Screen", on_touch, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, 122);

    /* back button (bottom-right) */
    ui_add_btn(s, "Back", on_back, UI_BTN_SIZE_SMALL, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    ui_swap_screen(s);
}

/* -- Touch screen entropy --------------------------------------------- */
static lv_obj_t* touch_status = NULL;

static void touch_status_delete_cb(lv_event_t* e) {
    (void)e;
    touch_status = NULL; // invalidate the pointer when the label is deleted
}

static void touch_area_tap_cb(lv_event_t* e) {
    union {
        ui_tap_cb_t fn;
        void*       vp;
    } u;
    u.vp = lv_event_get_user_data(e);
    if (!u.fn) return;

    lv_indev_t* indev = lv_event_get_indev(e);
    if (!indev) return;
    lv_point_t p;
    lv_indev_get_point(indev, &p);
    u.fn(p.x, p.y);
}

void ui_show_touch_screen(ui_tap_cb_t on_tap, ui_cb_t on_cancel) {
    ASSERT_OR_DIE(on_tap, "null on_tap");
    ASSERT_OR_DIE(on_cancel, "null on_cancel");

    lv_obj_t* s = ui_make_screen();

    // Full-screen touch target (behind the cancel button).
    lv_obj_t* area = lv_obj_create(s);
    lv_obj_set_size(area, LV_PCT(100), LV_PCT(100));
    lv_obj_align(area, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(area, lv_color_hex(0x0a0a0a), 0);
    lv_obj_set_style_border_width(area, 0, 0);
    lv_obj_add_flag(area, LV_OBJ_FLAG_CLICKABLE);
    union {
        ui_tap_cb_t fn;
        void*       vp;
    } u = {.fn = on_tap};
    lv_obj_add_event_cb(area, touch_area_tap_cb, LV_EVENT_CLICKED, u.vp);

    ui_add_title(area, "Touch Screen");

    touch_status = lv_label_create(area);
    lv_obj_add_event_cb(touch_status, touch_status_delete_cb, LV_EVENT_DELETE, NULL);
    lv_label_set_text(touch_status, "Tap anywhere to collect entropy");
    lv_obj_set_style_text_color(touch_status, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(touch_status, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_align(touch_status, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(touch_status, LV_ALIGN_CENTER, 0, 0);

    // Cancel button - centered below the status text.
    // A short tap passes through and still collects entropy, hold (long
    // press) to activate cancel.
    lv_obj_t* cancel_btn = lv_button_create(s);
    lv_obj_set_size(cancel_btn, 240, 44);
    lv_obj_align(cancel_btn, LV_ALIGN_CENTER, 0, 70);
    lv_obj_add_event_cb(cancel_btn, touch_area_tap_cb, LV_EVENT_CLICKED, u.vp);
    union {
        ui_cb_t fn;
        void*   vp;
    } u_cancel = {.fn = on_cancel};
    lv_obj_add_event_cb(cancel_btn, ui_btn_invoke, LV_EVENT_LONG_PRESSED, u_cancel.vp);
    lv_obj_t* cancel_lbl = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_lbl, "Cancel (hold to activate)");
    lv_obj_set_style_text_font(cancel_lbl, &lv_font_montserrat_14, 0);
    lv_obj_center(cancel_lbl);

    ui_swap_screen(s);
}

void ui_touch_screen_set_status(const char* text) {
    if (touch_status) lv_label_set_text(touch_status, text);
}

/* -- Dice sides picker ----------------------------------------------- */
static ui_uint_cb_t dice_on_sides = NULL;

static void dice_sides_btn_cb(lv_event_t* e) {
    if (!dice_on_sides) return;
    uintptr_t v = (uintptr_t)lv_event_get_user_data(e);
    dice_on_sides((uint8_t)v);
}

void ui_show_dice_sides(ui_uint_cb_t on_sides, ui_cb_t on_back) {
    ASSERT_OR_DIE(on_sides, "null on_sides");
    ASSERT_OR_DIE(on_back, "null on_back");

    dice_on_sides = on_sides;

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "Dice Sides");

    lv_obj_t* lbl = lv_label_create(s);
    lv_label_set_text(lbl, "How many sides?");
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 60);

    static const unsigned sides[] = {2, 4, 6, 8, 10, 12, 20};
    for (size_t i = 0; i < sizeof(sides) / sizeof(sides[0]); i++) {
        int  row = (int)(i / 3);
        int  col = (int)(i % 3);
        char face[8];
        snprintf(face, sizeof(face), "%u", sides[i]);

        lv_obj_t* b = lv_button_create(s);
        lv_obj_set_size(b, 80, 52);
        lv_obj_align(b, LV_ALIGN_CENTER, (col - 1) * 92, -10 + row * 68);
        lv_obj_add_event_cb(b, dice_sides_btn_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)sides[i]);

        lv_obj_t* l = lv_label_create(b);
        lv_label_set_text(l, face);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_24, 0);
        lv_obj_center(l);
    }

    ui_add_btn(s, "Back", on_back, UI_BTN_SIZE_SMALL, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    ui_swap_screen(s);
}

/* -- Dice roll entropy ----------------------------------------------- */
static lv_obj_t*    dice_status  = NULL;
static ui_uint_cb_t dice_on_roll = NULL;

static void dice_status_delete_cb(lv_event_t* e) {
    (void)e;
    dice_status = NULL; // invalidate the pointer when the label is deleted
}

static void dice_btn_cb(lv_event_t* e) {
    if (!dice_on_roll) return;
    uintptr_t v = (uintptr_t)lv_event_get_user_data(e);
    dice_on_roll((uint8_t)v);
}

void ui_show_dice(unsigned sides, ui_uint_cb_t on_roll, ui_cb_t on_cancel) {
    ASSERT_OR_DIE(sides >= 2, "sides must be >= 2");
    ASSERT_OR_DIE(on_roll, "null on_roll");
    ASSERT_OR_DIE(on_cancel, "null on_cancel");

    dice_on_roll = on_roll;

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "Dice Rolls");

    dice_status = lv_label_create(s);
    lv_obj_add_event_cb(dice_status, dice_status_delete_cb, LV_EVENT_DELETE, NULL);
    lv_label_set_text(dice_status, "Roll the die, tap the result");
    lv_obj_set_style_text_color(dice_status, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(dice_status, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_align(dice_status, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(dice_status, LV_ALIGN_TOP_MID, 0, 60);

    // 1..sides face buttons, wrapped into a scrollable grid.
    lv_obj_t* grid = lv_obj_create(s);
    lv_obj_set_size(grid, 440, 180);
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(grid, lv_color_black(), 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(grid, 8, 0);
    lv_obj_set_style_pad_column(grid, 8, 0);
    lv_obj_set_scroll_dir(grid, LV_DIR_VER);

    for (unsigned v = 1; v <= sides; v++) {
        char face[8];
        snprintf(face, sizeof(face), "%u", v);

        lv_obj_t* b = lv_button_create(grid);
        lv_obj_set_size(b, 64, 48);
        lv_obj_add_event_cb(b, dice_btn_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)v);

        lv_obj_t* l = lv_label_create(b);
        lv_label_set_text(l, face);
        lv_obj_set_style_text_font(l, &lv_font_montserrat_24, 0);
        lv_obj_center(l);
    }

    ui_add_btn(s, "Back", on_cancel, UI_BTN_SIZE_SMALL, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    ui_swap_screen(s);
}

void ui_dice_set_status(const char* text) {
    if (dice_status) lv_label_set_text(dice_status, text);
}

/* -- Coin flip entropy ----------------------------------------------- */
static lv_obj_t*    coin_status  = NULL;
static ui_uint_cb_t coin_on_flip = NULL;

static void coin_status_delete_cb(lv_event_t* e) {
    (void)e;
    coin_status = NULL; // invalidate the pointer when the label is deleted
}

static void coin_btn_cb(lv_event_t* e) {
    if (!coin_on_flip) return;
    lv_obj_t* lbl = lv_obj_get_child(lv_event_get_target(e), 0);
    if (!lbl) return;
    const char* txt = lv_label_get_text(lbl);
    if (!txt || !txt[0]) return;
    coin_on_flip((txt[0] == 'T') ? 1 : 0); // "Tails" -> 1, "Heads" -> 0
}

void ui_show_coin(ui_uint_cb_t on_flip, ui_cb_t on_cancel) {
    ASSERT_OR_DIE(on_flip, "null on_flip");
    ASSERT_OR_DIE(on_cancel, "null on_cancel");

    coin_on_flip = on_flip;

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "Coin Flips");

    coin_status = lv_label_create(s);
    lv_obj_add_event_cb(coin_status, coin_status_delete_cb, LV_EVENT_DELETE, NULL);
    lv_label_set_text(coin_status, "Flip a coin, tap the result");
    lv_obj_set_style_text_color(coin_status, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(coin_status, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_align(coin_status, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(coin_status, LV_ALIGN_TOP_MID, 0, 60);

    ui_add_btn_evt(s, "Heads", coin_btn_cb, NULL, UI_BTN_SIZE_WIDE, LV_ALIGN_CENTER, -95, 20);
    ui_add_btn_evt(s, "Tails", coin_btn_cb, NULL, UI_BTN_SIZE_WIDE, LV_ALIGN_CENTER, 95, 20);

    ui_add_btn(s, "Back", on_cancel, UI_BTN_SIZE_SMALL, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    ui_swap_screen(s);
}

void ui_coin_set_status(const char* text) {
    if (coin_status) lv_label_set_text(coin_status, text);
}

void ui_show_camera_feed(ui_cb_t on_use, ui_cb_t on_cancel) {
    ASSERT_OR_DIE(on_use, "null on_use");
    ASSERT_OR_DIE(on_cancel, "null on_cancel");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "Camera");

    memset(&camera_dsc, 0, sizeof(camera_dsc));
    camera_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    camera_dsc.header.cf    = LV_COLOR_FORMAT_RGB565;

    camera_img = lv_image_create(s);
    lv_obj_set_size(camera_img, 300, 200);
    lv_obj_align(camera_img, LV_ALIGN_TOP_MID, 0, 45);

    /* "Use Image" (left) and "Cancel" (right). */
    ui_add_btn(s, "Use Image", on_use, UI_BTN_SIZE_WIDE, LV_ALIGN_BOTTOM_LEFT, 20, -10);
    ui_add_btn(s, "Cancel", on_cancel, UI_BTN_SIZE_WIDE, LV_ALIGN_BOTTOM_RIGHT, -20, -10);

    ui_swap_screen(s);
}

void ui_camera_feed_update(const uint8_t* rgb565, uint32_t w, uint32_t h) {
    if (!camera_img) return;
    ASSERT_OR_DIE(rgb565, "null rgb565");
    ASSERT_OR_DIE(w > 0 && h > 0, "invalid camera frame size");

    // Fit the frame into the 300x200 preview area, preserving aspect ratio
    uint32_t disp_w = w, disp_h = h;
    if (w > 300 || h > 200) {
        uint32_t zx = (256 * 300) / w;
        uint32_t zy = (256 * 200) / h;
        uint32_t z  = (zx < zy) ? zx : zy;
        disp_w      = (w * z) / 256;
        disp_h      = (h * z) / 256;
    }
    lv_obj_set_size(camera_img, disp_w, disp_h);

    camera_dsc.header.w      = (uint16_t)w;
    camera_dsc.header.h      = (uint16_t)h;
    camera_dsc.header.stride = (uint16_t)(w * 2);
    camera_dsc.data_size     = w * h * 2;
    camera_dsc.data          = rgb565;

    // Set the source first so the image has valid dimensions, then apply the
    // stretch alignment
    lv_image_set_src(camera_img, &camera_dsc);
    lv_image_set_inner_align(camera_img, LV_IMAGE_ALIGN_STRETCH);
}

void ui_show_seedqr(const uint8_t* cells, uint32_t size, ui_cb_t on_done) {
    ASSERT_OR_DIE(cells, "null cells");
    ASSERT_OR_DIE(on_done, "null on_done");

    lv_obj_t* s = ui_make_screen();

    // Title on the right so the QR can use the full screen height.
    // Match the Done button's right edge (-30) so the column lines up.
    lv_obj_t* t = lv_label_create(s);
    lv_label_set_text(t, "SeedQR");
    lv_obj_set_style_text_color(t, lv_color_white(), 0);
    lv_obj_set_style_text_font(t, &lv_font_montserrat_28, 0);
    lv_obj_align(t, LV_ALIGN_TOP_RIGHT, -35, 15);

    // White quiet-zone border (4 modules), then the data modules
    uint32_t border = 4;
    uint32_t total  = size + 2 * border;
    uint32_t scale  = 300 / total; // full screen height
    if (scale < 4) scale = 4;
    uint32_t px = scale * total;

    // Build one RGB565 buffer and show it as a single image
    seedqr_buf_bytes = (size_t)px * px * 2;
    seedqr_buf       = calloc(seedqr_buf_bytes, 1);
    ASSERT_OR_DIE(seedqr_buf, "out of memory for SeedQR buffer");
    memset(seedqr_buf, 0xFF, seedqr_buf_bytes); // white (0xFFFF)

    uint16_t* dst = (uint16_t*)seedqr_buf;
    uint32_t  off = border * scale;
    for (uint32_t y = 0; y < size; y++) {
        for (uint32_t x = 0; x < size; x++) {
            if (!cells[(size_t)y * size + x]) continue; // already white
            uint32_t by = off + y * scale;
            uint32_t bx = off + x * scale;
            for (uint32_t dy = 0; dy < scale; dy++) {
                for (uint32_t dx = 0; dx < scale; dx++) {
                    dst[(size_t)(by + dy) * px + (bx + dx)] = 0x0000;
                }
            }
        }
    }

    memset(&seedqr_dsc, 0, sizeof(seedqr_dsc));
    seedqr_dsc.header.magic  = LV_IMAGE_HEADER_MAGIC;
    seedqr_dsc.header.cf     = LV_COLOR_FORMAT_RGB565;
    seedqr_dsc.header.w      = (uint16_t)px;
    seedqr_dsc.header.h      = (uint16_t)px;
    seedqr_dsc.header.stride = (uint16_t)(px * 2);
    seedqr_dsc.data_size     = (uint32_t)seedqr_buf_bytes;
    seedqr_dsc.data          = seedqr_buf;

    lv_obj_t* img = lv_image_create(s);
    lv_image_set_src(img, &seedqr_dsc);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 10, 0);

    ui_add_btn(s, "Done", on_done, UI_BTN_SIZE_MED, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    ui_swap_screen(s);
}

void ui_seedqr_cleanup(void) {
    if (seedqr_buf) {
        secure_memzero(seedqr_buf, seedqr_buf_bytes);
        free(seedqr_buf);
        seedqr_buf = NULL;
    }
}

void ui_show_qr_scan(ui_cb_t on_scan, ui_cb_t on_cancel) {
    ASSERT_OR_DIE(on_scan, "null on_scan");
    ASSERT_OR_DIE(on_cancel, "null on_cancel");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "Scan QR");

    memset(&camera_dsc, 0, sizeof(camera_dsc));
    camera_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    camera_dsc.header.cf    = LV_COLOR_FORMAT_RGB565;

    camera_img = lv_image_create(s);
    lv_obj_set_size(camera_img, 300, 200);
    lv_obj_align(camera_img, LV_ALIGN_TOP_MID, 0, 45);

    ui_add_btn(s, "Scan", on_scan, UI_BTN_SIZE_WIDE, LV_ALIGN_BOTTOM_LEFT, 20, -10);
    ui_add_btn(s, "Cancel", on_cancel, UI_BTN_SIZE_WIDE, LV_ALIGN_BOTTOM_RIGHT, -20, -10);

    ui_swap_screen(s);
}

void ui_show_mnemonic(const char* words, mnemonic_type_t type, ui_cb_t on_ok, ui_cb_t on_export) {
    ASSERT_OR_DIE(words, "null words");
    ASSERT_OR_DIE(on_ok, "null on_ok");

    lv_obj_t* s = ui_make_screen();

    const char* title;
    bool        show_warning = false;
    switch (type) {
    case MNEMONIC_TYPE_GENERATED:
        title = "Generated Mnemonic";
        break;
    case MNEMONIC_TYPE_ENTERED:
        title = "Entered Mnemonic";
        break;
    case MNEMONIC_TYPE_MERGED:
        title = "Merged Mnemonic";
        break;
    case MNEMONIC_TYPE_FINAL:
    default:
        title        = "Final Mnemonic";
        show_warning = true;
        break;
    }
    ui_add_title(s, title);

    if (show_warning) {
        lv_obj_t* w = lv_label_create(s);
        lv_label_set_text(w, "Write these words down.\nNever share them!");
        lv_obj_set_style_text_color(w, lv_color_hex(0xFF4444), 0);
        lv_obj_set_style_text_font(w, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_align(w, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(w, LV_ALIGN_TOP_MID, 0, 55);
    }

    lv_obj_t* grid = ui_mnemonic_view_create(s);
    lv_obj_align(grid, LV_ALIGN_TOP_MID, 0, show_warning ? 85 : 55);
    ui_mnemonic_view_set_words(grid, words);
    lv_obj_update_layout(grid);

    if (on_export) {
        ui_add_btn(s, "Show SeedQR", on_export, UI_BTN_SIZE_WIDE, LV_ALIGN_BOTTOM_LEFT, 20, -10);
        ui_add_btn(s, "Ok", on_ok, UI_BTN_SIZE_MED, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
    } else {
        lv_obj_t* ok = ui_add_btn(s, "Ok", on_ok, UI_BTN_SIZE_MED, LV_ALIGN_CENTER, 0, 0);
        lv_obj_align_to(ok, grid, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);
    }

    ui_swap_screen(s);
}

void ui_show_merge_process(const char* current_words, const char* current_entropy_hex,
                           const char* new_entropy_hex, const char* merged_entropy_hex,
                           const char* merged_words, ui_cb_t on_ok) {
    ASSERT_OR_DIE(on_ok, "null on_ok");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "Merging Entropy");

    lv_obj_t* cont = lv_obj_create(s);
    lv_obj_set_size(cont, 440, 200);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x111111), 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);

    struct {
        const char* label;
        const char* value;
    } lines[] = {{"Current mnemonic:", current_words},
                 {"Current entropy:", current_entropy_hex},
                 {"", ""},
                 {"New entropy:", new_entropy_hex},
                 {"", ""},
                 {"--- XOR merge ---", ""},
                 {"Merged entropy:", merged_entropy_hex},
                 {"Merged mnemonic:", merged_words},
                 {NULL, NULL}};

    for (int i = 0; lines[i].label; i++) {
        if (lines[i].label[0]) {
            lv_obj_t* lbl = lv_label_create(cont);
            lv_label_set_text(lbl, lines[i].label);
            lv_obj_set_style_text_color(lbl, lv_color_hex(0x888888), 0);
            lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
        }
        if (lines[i].value[0]) {
            lv_obj_t* val = lv_label_create(cont);
            lv_label_set_text(val, lines[i].value);
            lv_obj_set_style_text_color(val, lv_color_white(), 0);
            lv_obj_set_style_text_font(val, &lv_font_montserrat_14, 0);
            lv_obj_set_width(val, 420);
            lv_label_set_long_mode(val, LV_LABEL_LONG_WRAP);
        }
    }

    ui_add_btn(s, "Ok", on_ok, UI_BTN_SIZE_MED, LV_ALIGN_BOTTOM_MID, 0, -10);

    ui_swap_screen(s);
}

void ui_show_enter_words(ui_cb_t on_ok) {
    ASSERT_OR_DIE(on_ok, "null on_ok");

    secure_memzero(entered_buf, sizeof(entered_buf));

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "Enter Mnemonic");

    lv_obj_t* ta = lv_textarea_create(s);
    entered_ta   = ta; // store for later retrieval
    lv_obj_set_size(ta, 420, 180);
    lv_obj_align(ta, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_placeholder_text(ta, "Type your mnemonic words here...");
    lv_obj_set_style_text_font(ta, &lv_font_montserrat_14, 0);
    lv_obj_add_event_cb(ta, NULL, LV_EVENT_ALL, NULL); // just to hold reference

    ui_add_btn(s, "OK", on_ok, UI_BTN_SIZE_LARGE, LV_ALIGN_CENTER, 0, 100);
    lv_group_focus_obj(ta);
    ui_swap_screen(s);
}

const char* ui_get_entered_words(void) {
    if (!entered_ta) return "";
    const char* txt = lv_textarea_get_text(entered_ta);
    strncpy(entered_buf, txt, sizeof(entered_buf) - 1);
    entered_buf[sizeof(entered_buf) - 1] = '\0';
    return entered_buf;
}

void ui_show_msg(const char* msg) {
    ASSERT_OR_DIE(msg, "null msg");

    lv_obj_t* s = ui_make_screen();
    lv_obj_t* l = lv_label_create(s);
    lv_label_set_text(l, msg);
    lv_obj_set_style_text_color(l, lv_color_white(), 0);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(l, 440);
    lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
    lv_obj_align(l, LV_ALIGN_CENTER, 0, 0);
    ui_swap_screen(s);
    lv_refr_now(NULL);
}

void ui_delay_ms(uint32_t ms) {
    uint32_t start = lv_tick_get();
    while (lv_tick_get() - start < ms) {
        // if on main thread keep the ui responsive by pumping the timer handler
        // should only be called from the main thread (which could be a UI callback i tihnk)
        lv_timer_handler();
#ifdef ESP_PLATFORM
        vTaskDelay(pdMS_TO_TICKS(5));
#else
        SDL_Delay(5);
#endif
    }
}
