/**
 * @file main/ui/ui.c
 * @brief UI screens for the mnemonic tool.
 */

#include "ui.h"
#include "util/error.h"
#include <string.h>

#ifndef ESP_PLATFORM
#include <SDL2/SDL.h>
#endif

/* -- Helpers ---------------------------------------------------------- */
static lv_obj_t* main_scr = NULL;
static lv_obj_t* entered_ta = NULL;
static char     entered_buf[512];
static lv_obj_t* prev_screen = NULL;  // track for deferred cleanup

lv_obj_t* ui_make_screen(void) {
    lv_obj_t* s = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(s, lv_color_black(), 0);
    return s;
}

// Deferred screen deletion - safe to call from event handlers
static void delete_screen_cb(void* ptr) {
    lv_obj_t* scr = (lv_obj_t*)ptr;
    if (scr && scr != main_scr) lv_obj_delete(scr);
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
    union { ui_cb_t fn; void* vp; } u;
    u.vp = lv_event_get_user_data(e);
    if (u.fn) u.fn();
}

static lv_obj_t* add_btn(lv_obj_t* parent, const char* text, ui_cb_t cb, int y) {
    lv_obj_t* b = lv_button_create(parent);
    lv_obj_set_size(b, 200, 55);
    lv_obj_align(b, LV_ALIGN_CENTER, 0, y);
    if (cb) {
        union { ui_cb_t fn; void* vp; } u = { .fn = cb };
        lv_obj_add_event_cb(b, ui_btn_invoke, LV_EVENT_CLICKED, u.vp);
    }
    lv_obj_t* l = lv_label_create(b);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_24, 0);
    lv_obj_center(l);
    return b;
}

void ui_go_main(void) { if (main_scr) lv_scr_load(main_scr); }

/* -- Screens ---------------------------------------------------------- */
void ui_show_main(lv_event_cb_t on_new_wallet, lv_event_cb_t on_test_error) {
    ASSERT_OR_DIE(on_new_wallet, "null on_new_wallet");
    ASSERT_OR_DIE(on_test_error, "null on_test_error");

    if (!main_scr) {
        main_scr = lv_screen_active();

        // Build main screen with "New Wallet" button
        lv_obj_t* scr = main_scr;
        lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

        lv_obj_t* title = lv_label_create(scr);
        lv_label_set_text(title, "SeedMix");
        lv_obj_set_style_text_color(title, lv_color_white(), 0);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

        lv_obj_t* btn = lv_button_create(scr);
        lv_obj_set_size(btn, 240, 70);
        lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_event_cb(btn, on_new_wallet, LV_EVENT_CLICKED, NULL);
        lv_obj_t* bl = lv_label_create(btn);
        lv_label_set_text(bl, "New Wallet");
        lv_obj_set_style_text_font(bl, &lv_font_montserrat_28, 0);
        lv_obj_center(bl);

        // Test error button
        lv_obj_t* test_btn = lv_button_create(scr);
        lv_obj_set_size(test_btn, 80, 30);
        lv_obj_align(test_btn, LV_ALIGN_BOTTOM_RIGHT, -40, -10);
        lv_obj_set_style_bg_color(test_btn, lv_color_hex(0x440000), 0);
        lv_obj_add_event_cb(test_btn, on_test_error, LV_EVENT_CLICKED, NULL);
        lv_obj_t* tbl = lv_label_create(test_btn);
        lv_label_set_text(tbl, "test error!");
        lv_obj_set_style_text_font(tbl, &lv_font_montserrat_14, 0);
        lv_obj_center(tbl);

        lv_obj_t* footer = lv_label_create(scr);
        lv_label_set_text(footer, "BIP39 Mnemonic Tool");
        lv_obj_set_style_text_color(footer, lv_color_hex(0x808080), 0);
        lv_obj_set_style_text_font(footer, &lv_font_montserrat_14, 0);
        lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -10);
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
    add_btn(s, "12 words", on_12, -40);
    add_btn(s, "24 words", on_24,  30);
    ui_swap_screen(s);
}

void ui_show_source(ui_cb_t on_generate, ui_cb_t on_enter, ui_cb_t on_scan_qr, ui_cb_t on_state, ui_cb_t on_finish, bool is_additional) {
    ASSERT_OR_DIE(on_generate, "null on_generate");
    ASSERT_OR_DIE(on_enter, "null on_enter");
    ASSERT_OR_DIE(on_scan_qr, "null on_scan_qr");
    ASSERT_OR_DIE(on_state, "null on_state");
    ASSERT_OR_DIE(on_finish, "null on_finish");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, is_additional ? "Choose Additional Source" : "Choose Initial Source");
    add_btn(s, "Generate Here", on_generate, -40);
    add_btn(s, "Enter Manually", on_enter,    30);
    add_btn(s, "Scan QR Code", on_scan_qr, 100);

    // state button (top-left)
    lv_obj_t* state_btn = lv_button_create(s);
    lv_obj_set_size(state_btn, 70, 30);
    lv_obj_align(state_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    union { ui_cb_t fn; void* vp; } u = { .fn = on_state };
    lv_obj_add_event_cb(state_btn, ui_btn_invoke, LV_EVENT_CLICKED, u.vp);
    lv_obj_t* sl = lv_label_create(state_btn);
    lv_label_set_text(sl, "State");
    lv_obj_set_style_text_font(sl, &lv_font_montserrat_14, 0);
    lv_obj_center(sl);

    // finish button (top-right)
    lv_obj_t* fin_btn = lv_button_create(s);
    lv_obj_set_size(fin_btn, 70, 30);
    lv_obj_align(fin_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    union { ui_cb_t fn; void* vp; } u2 = { .fn = on_finish };
    lv_obj_add_event_cb(fin_btn, ui_btn_invoke, LV_EVENT_CLICKED, u2.vp);
    lv_obj_t* fl = lv_label_create(fin_btn);
    lv_label_set_text(fl, "Finish");
    lv_obj_set_style_text_font(fl, &lv_font_montserrat_14, 0);
    lv_obj_center(fl);

    ui_swap_screen(s);
}

void ui_show_mnemonic(const char* words, mnemonic_type_t type, ui_cb_t on_ok) {
    ASSERT_OR_DIE(words, "null words");
    ASSERT_OR_DIE(on_ok, "null on_ok");

    lv_obj_t* s = ui_make_screen();

    const char* title;
    bool show_warning = false;
    switch (type) {
        case MNEMONIC_TYPE_GENERATED: title = "Generated Mnemonic";   break;
        case MNEMONIC_TYPE_ENTERED:   title = "Entered Mnemonic";     break;
        case MNEMONIC_TYPE_MERGED:    title = "Merged Mnemonic";      break;
        case MNEMONIC_TYPE_FINAL:
        default:                      title = "Final Mnemonic";
                                      show_warning = true;             break;
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

    lv_obj_t* l = lv_label_create(s);
    lv_label_set_text(l, words);
    lv_obj_set_style_text_color(l, lv_color_white(), 0);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(l, 440);
    lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
    lv_obj_align(l, LV_ALIGN_CENTER, 0, 0);

    add_btn(s, "Ok", on_ok, type == MNEMONIC_TYPE_FINAL ? 130 : 80);
    ui_swap_screen(s);
}

void ui_show_merge_process(
    const char* current_words, const char* current_entropy_hex,
    const char* new_entropy_hex,
    const char* merged_entropy_hex, const char* merged_words,
    ui_cb_t on_ok)
{
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

    struct { const char* label; const char* value; } lines[] = {
        {"Current mnemonic:", current_words},
        {"Current entropy:",  current_entropy_hex},
        {"", ""},
        {"New entropy:",      new_entropy_hex},
        {"", ""},
        {"--- XOR merge ---", ""},
        {"Merged entropy:",   merged_entropy_hex},
        {"Merged mnemonic:",  merged_words},
        {NULL, NULL}
    };

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

    lv_obj_t* ok = lv_button_create(s);
    lv_obj_set_size(ok, 160, 50);
    lv_obj_align(ok, LV_ALIGN_BOTTOM_MID, 0, -10);
    union { ui_cb_t fn; void* vp; } u = { .fn = on_ok };
    lv_obj_add_event_cb(ok, ui_btn_invoke, LV_EVENT_CLICKED, u.vp);
    lv_obj_t* ol = lv_label_create(ok);
    lv_label_set_text(ol, "Ok");
    lv_obj_set_style_text_font(ol, &lv_font_montserrat_24, 0);
    lv_obj_center(ol);

    ui_swap_screen(s);
}

void ui_show_enter_words(ui_cb_t on_ok) {
    ASSERT_OR_DIE(on_ok, "null on_ok");

    lv_obj_t* s = ui_make_screen();
    ui_add_title(s, "Enter Mnemonic");

    lv_obj_t* ta = lv_textarea_create(s);
    entered_ta = ta;  // store for later retrieval
    lv_obj_set_size(ta, 420, 180);
    lv_obj_align(ta, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_placeholder_text(ta, "Type your mnemonic words here...");
    lv_obj_set_style_text_font(ta, &lv_font_montserrat_14, 0);
    lv_obj_add_event_cb(ta, NULL, LV_EVENT_ALL, NULL); // just to hold reference

    add_btn(s, "OK", on_ok, 100);
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

