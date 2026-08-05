/**
 * @file main/main.c
 * @brief Shared application entry - sets up UI screens.
 */

#include "lvgl.h"
#include "app.h"
#include "ui/ui.h"
#include "crypto/mnemonic.h"

/* -- Callbacks -------------------------------------------------------- */
static mnemonic_t *current_mnemonic = NULL;

static void on_mnemonic_done(void) {
    if (current_mnemonic) {
        mnemonic_discard(current_mnemonic);
        current_mnemonic = NULL;
    }
}

static void on_generate_click(lv_event_t *e) {
    (void)e;
    current_mnemonic = mnemonic_generate();
    ui_show_mnemonic(mnemonic_words(current_mnemonic), on_mnemonic_done);
}

/* -- Initialization --------------------------------------------------- */
void app_init(void) {
    lv_display_t *disp = lv_display_get_default();
    lv_theme_t *th = lv_theme_default_init(
        disp,
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_CYAN),
        true,
        &lv_font_montserrat_20
    );
    lv_disp_set_theme(disp, th);

    mnemonic_init();
    ui_create();
    ui_on_generate(on_generate_click);
}
