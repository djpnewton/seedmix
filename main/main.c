/**
 * @file main/main.c
 * @brief BIP39 mnemonic generator workflow - generates, enters, combines, and finalizes mnemonics.
 */

#include "app.h"
#include "crypto/mnemonic.h"
#include "lvgl.h"
#include "ui/log.h"
#include "ui/ui.h"
#include "ui/word_entry.h"
#include "util/error.h"
#include "util/utils.h"

/* -- Workflow state --------------------------------------------------- */
static mnemonic_t* current    = NULL;
static unsigned    word_count = 12;

/* -- Forward declarations --------------------------------------------- */
static void on_scan_qr(void);
static void on_show_state(void);
static void go_back_source(void);
static void go_source(void);
static void on_finish(void);
static void show_merge_screen(mnemonic_t* new_m, mnemonic_type_t result_type);
static void on_new_wallet(lv_event_t* e);
static void on_test_error(lv_event_t* e);

static void on_generating_msg(const char* msg) {
    ui_show_msg(msg);
    ui_delay_ms(1500);
}

static void on_generate(void) {
    ui_show_msg("Generating words...");
    ui_delay_ms(500);
    mnemonic_t* m = mnemonic_generate(word_count, on_generating_msg);
    if (current) {
        ui_log_add("Combined with generated %u-word", word_count);
        show_merge_screen(m, MNEMONIC_TYPE_MERGED);
    } else {
        current = m;
        ui_log_add("Generated %u-word mnemonic", word_count);
        ui_show_mnemonic(mnemonic_words(current), MNEMONIC_TYPE_GENERATED, go_source);
    }
}

static word_entry_handle_t we_handle         = NULL;
static mnemonic_t*         pending_new       = NULL;
static mnemonic_type_t     merge_result_type = MNEMONIC_TYPE_GENERATED;

static void on_enter_manual(void);
static void on_we_complete(void);

static void on_merge_done(void) {
    ASSERT_OR_DIE(pending_new, "no pending mnemonic");
    current     = mnemonic_combine(current, pending_new);
    pending_new = NULL;
    ui_show_mnemonic(mnemonic_words(current), merge_result_type, go_source);
}

static void show_merge_screen(mnemonic_t* new_m, mnemonic_type_t result_type) {
    uint8_t ca[32], na[32], ma[32];
    size_t  elen = mnemonic_entropy_size(current);
    mnemonic_to_entropy(current, ca);
    mnemonic_to_entropy(new_m, na);
    for (size_t i = 0; i < elen; i++) ma[i] = ca[i] ^ na[i];

    mnemonic_t* preview = mnemonic_from_entropy(ma, elen);
    merge_result_type   = result_type;
    pending_new         = new_m;

    char ca_hex[65], na_hex[65], ma_hex[65];
    bytes_to_hex(ca, elen, ca_hex, sizeof(ca_hex));
    bytes_to_hex(na, elen, na_hex, sizeof(na_hex));
    bytes_to_hex(ma, elen, ma_hex, sizeof(ma_hex));

    ui_show_merge_process(mnemonic_words(current), ca_hex, na_hex, ma_hex, mnemonic_words(preview),
                          on_merge_done);

    mnemonic_discard(preview);
}

static void on_we_cancel(void) {
    ui_word_entry_discard(we_handle);
    we_handle = NULL;
    ui_show_source(on_generate, on_enter_manual, on_scan_qr, on_show_state, on_finish,
                   current != NULL);
}

static void on_enter_manual(void) {
    we_handle = ui_word_entry_begin(word_count, on_we_complete, on_we_cancel);
}

static void on_scan_qr(void) { FATAL("QR scanning not implemented yet."); }

static void on_we_complete(void) {
    const char* txt = ui_word_entry_result(we_handle);
    char        buf[512];
    strncpy(buf, txt, sizeof(buf) - 1);
    ui_word_entry_discard(we_handle);
    we_handle = NULL;

    mnemonic_t* m = mnemonic_from_string(buf);
    if (!m) {
        ui_show_main(on_new_wallet, on_test_error);
        return;
    }
    if (current) {
        ui_log_add("Combined with entered %u-word", word_count);
        show_merge_screen(m, MNEMONIC_TYPE_MERGED);
    } else {
        current = m;
        ui_log_add("Entered %u-word mnemonic", word_count);
        ui_show_mnemonic(mnemonic_words(current), MNEMONIC_TYPE_ENTERED, go_source);
    }
}

// -- Re-enter source with correct title based on state -----------------
static void go_source(void) {
    ui_show_source(on_generate, on_enter_manual, on_scan_qr, on_show_state, on_finish,
                   current != NULL);
}

static void go_back_source(void) {
    ui_show_source(on_generate, on_enter_manual, on_scan_qr, on_show_state, on_finish,
                   current != NULL);
}

/* -- Step: word count chosen ------------------------------------------ */
static void on_12(void) {
    word_count = 12;
    ui_show_source(on_generate, on_enter_manual, on_scan_qr, on_show_state, on_finish,
                   current != NULL);
}
static void on_24(void) {
    word_count = 24;
    ui_show_source(on_generate, on_enter_manual, on_scan_qr, on_show_state, on_finish,
                   current != NULL);
}

/* -- State screen ----------------------------------------------------- */
static void on_show_state(void) {
    ui_show_state(go_back_source, current ? mnemonic_words(current) : NULL);
}

static void on_finish(void) {
    if (!current) {
        ui_go_main();
        return;
    }
    ui_show_mnemonic(mnemonic_words(current), MNEMONIC_TYPE_FINAL, ui_go_main);
    ui_log_add("Finished");
    mnemonic_discard(current);
    current = NULL;
    if (we_handle) {
        ui_word_entry_discard(we_handle);
        we_handle = NULL;
    }
}

/* -- Entry: "New Wallet" button --------------------------------------- */
static void on_new_wallet(lv_event_t* e) {
    (void)e;
    ASSERT_OR_DIE(!current, "current mnemonic should be NULL");
    ASSERT_OR_DIE(!we_handle, "word entry handle should be NULL");
    ui_show_word_count(on_12, on_24);
}

/* -- Test error screen ------------------------------------------------ */
static void on_test_error(lv_event_t* e) {
    (void)e;
    FATAL("This is a test of the fatal error screen.");
}

/* -- Initialization --------------------------------------------------- */
void app_init(void) {
    lv_display_t* disp = lv_display_get_default();
    lv_theme_t*   th =
        lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE),
                              lv_palette_main(LV_PALETTE_CYAN), true, &lv_font_montserrat_20);
    lv_disp_set_theme(disp, th);

    mnemonic_init();

    ui_show_main(on_new_wallet, on_test_error);
}
