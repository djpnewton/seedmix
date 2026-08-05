/**
 * @file main/ui/ui.c
 * @brief UI implementation - creates the main screen.
 */

#include "ui.h"

/* -- Static object references ----------------------------------------- */
static lv_obj_t *main_screen  = NULL;
static lv_event_cb_t btn_callback = NULL;
static void (*done_cb)(void) = NULL;

/* -- Done button handler ---------------------------------------------- */
static void on_done_clicked(lv_event_t *e) {
    (void)e;
    if (done_cb) done_cb();
    done_cb = NULL;
    if (main_screen) lv_scr_load(main_screen);
}

/* -- Forward the button click to the registered callback -------------- */
static void generate_clicked(lv_event_t *e) {
    if (btn_callback) btn_callback(e);
}

/* -- Public API ------------------------------------------------------- */
void ui_create(void) {
    /* -- Main screen ------------------------------------------------- */
    lv_obj_t *scr = lv_screen_active();
    main_screen = scr;  /* remember for "back" navigation */
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    /* -- Title ------------------------------------------------------- */
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Entropy");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);


    /* -- Generate button (callback assigned later by ui_on_generate) -- */
    lv_obj_t *gen_btn = lv_button_create(scr);
    lv_obj_set_size(gen_btn, 200, 60);
    lv_obj_align(gen_btn, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_event_cb(gen_btn, generate_clicked, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_label = lv_label_create(gen_btn);
    lv_label_set_text(btn_label, "Generate");
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_28, 0);
    lv_obj_center(btn_label);

    /* -- Spinner (hidden by default) --------------------------------- */
    lv_obj_t *spinner = lv_spinner_create(scr);
    lv_obj_set_size(spinner, 40, 40);
    lv_obj_align(spinner, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);

    /* -- Footer ------------------------------------------------------ */
    lv_obj_t *footer = lv_label_create(scr);
    lv_label_set_text(footer, "Prototype - Linux build");
    lv_obj_set_style_text_color(footer, lv_color_hex(0x808080), 0);
    lv_obj_set_style_text_font(footer, &lv_font_montserrat_14, 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -10);
}

void ui_on_generate(lv_event_cb_t cb) {
    btn_callback = cb;
}

void ui_show_mnemonic(const char *words, void (*on_done)(void)) {
    done_cb = on_done;

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Your Mnemonic");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    lv_obj_t *warn = lv_label_create(scr);
    lv_label_set_text(warn, "Write these 12 words down.\nNever share them!");
    lv_obj_set_style_text_color(warn, lv_color_hex(0xFF4444), 0);
    lv_obj_set_style_text_font(warn, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(warn, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(warn, LV_ALIGN_TOP_MID, 0, 55);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, words);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label, 400);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *btn = lv_button_create(scr);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_add_event_cb(btn, on_done_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Done");
    lv_obj_center(btn_label);

    lv_scr_load(scr);
}

void ui_show_main(void) {
    if (main_screen) lv_scr_load(main_screen);
}
