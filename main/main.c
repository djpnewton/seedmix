/**
 * @file main/main.c
 * @brief BIP39 mnemonic generator workflow - generates, enters, combines, and finalizes mnemonics.
 */

#include "app.h"
#include "crypto/mnemonic.h"
#include "hal.h"
#include "lvgl.h"
#include "ui/log.h"
#include "ui/ui.h"
#include "ui/word_entry.h"
#include "util/error.h"
#include "util/log.h"
#include "util/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -- Workflow state --------------------------------------------------- */
static mnemonic_t* current    = NULL;
static unsigned    word_count = 12;

/* -- Forward declarations --------------------------------------------- */
static void on_other_source(void);
static void on_camera_image(void);
static void on_camera_use(void);
static void on_camera_retake(void);
static void on_camera_back(void);
static void on_scan_qr(void);
static void on_dice_rolls(void);
static void on_coin_flips(void);
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
    ui_show_source(on_generate, on_enter_manual, on_other_source, on_show_state, on_finish,
                   current != NULL);
}

static void on_enter_manual(void) {
    we_handle = ui_word_entry_begin(word_count, on_we_complete, on_we_cancel);
}

static void on_other_source(void) {
    ui_show_other_source(on_camera_image, on_scan_qr, on_dice_rolls, on_coin_flips, go_source);
}

/* -- Camera image source --------------------------------------------- */
static hal_camera_frame_t camera_frame;  /* current captured frame (owned) */
static uint8_t*           camera_rgb565; /* RGB565 preview buffer (owned) */
static uint32_t           camera_w = 0, camera_h = 0;

static inline uint8_t clip8(int v) { return (uint8_t)(v < 0 ? 0 : (v > 255 ? 255 : v)); }

static uint16_t yuv_to_rgb565(int y, int u, int v) {
    int c = y - 16;
    int d = u - 128;
    int e = v - 128;
    int r = (298 * c + 409 * e + 128) >> 8;
    int g = (298 * c - 100 * d - 208 * e + 128) >> 8;
    int b = (298 * c + 516 * d + 128) >> 8;
    return (uint16_t)(((clip8(r) >> 3) << 11) | ((clip8(g) >> 2) << 5) | (clip8(b) >> 3));
}

static void camera_release(void) {
    if (camera_rgb565) {
        free(camera_rgb565);
        camera_rgb565 = NULL;
    }
    hal_camera_frame_free(&camera_frame);
    camera_w = camera_h = 0;
}

static uint8_t* camera_frame_to_rgb565(const hal_camera_frame_t* f, uint32_t* out_w,
                                       uint32_t* out_h) {
    ASSERT_OR_DIE(f && f->data && f->width > 0 && f->height > 0, "invalid camera frame");
    *out_w = f->width;
    *out_h = f->height;

    uint32_t bpp;
    switch (f->pixfmt) {
    case HAL_CAMERA_FMT_GRAY8:
        bpp = 1;
        break;
    case HAL_CAMERA_FMT_YUYV:
        bpp = 2;
        break;
    case HAL_CAMERA_FMT_RGB565:
        bpp = 2;
        break;
    default:
        bpp = 0;
        break;
    }

    size_t   n   = (size_t)f->width * f->height;
    uint8_t* rgb = (uint8_t*)calloc(n, 2);
    ASSERT_OR_DIE(rgb, "out of memory converting camera frame");
    uint16_t* dst    = (uint16_t*)rgb;
    uint32_t  stride = f->bytes_per_line ? f->bytes_per_line : f->width * bpp;

    switch (f->pixfmt) {
    case HAL_CAMERA_FMT_RGB565:
        for (uint32_t y = 0; y < f->height; y++) {
            memcpy(dst + (size_t)y * f->width, f->data + (size_t)y * stride, f->width * 2);
        }
        break;
    case HAL_CAMERA_FMT_GRAY8:
        for (uint32_t y = 0; y < f->height; y++) {
            const uint8_t* row = f->data + (size_t)y * stride;
            for (uint32_t x = 0; x < f->width; x++) {
                dst[(size_t)y * f->width + x] = yuv_to_rgb565(row[x], 128, 128);
            }
        }
        break;
    case HAL_CAMERA_FMT_YUYV:
        for (uint32_t y = 0; y < f->height; y++) {
            const uint8_t* row = f->data + (size_t)y * stride;
            for (uint32_t x = 0; x + 1 < f->width; x += 2) {
                uint8_t y0                        = row[x * 2 + 0];
                uint8_t u                         = row[x * 2 + 1];
                uint8_t y1                        = row[x * 2 + 2];
                uint8_t v                         = row[x * 2 + 3];
                dst[(size_t)y * f->width + x]     = yuv_to_rgb565(y0, u, v);
                dst[(size_t)y * f->width + x + 1] = yuv_to_rgb565(y1, u, v);
            }
        }
        break;
    case HAL_CAMERA_FMT_JPEG:
    case HAL_CAMERA_FMT_UNKNOWN:
    default:
        free(rgb);
        FATAL("Unsupported camera pixel format.");
        return NULL;
    }
    return rgb;
}

static const char* camera_pixfmt_name(hal_camera_pixfmt_t f) {
    switch (f) {
    case HAL_CAMERA_FMT_GRAY8:
        return "GRAY8";
    case HAL_CAMERA_FMT_YUYV:
        return "YUYV";
    case HAL_CAMERA_FMT_RGB565:
        return "RGB565";
    case HAL_CAMERA_FMT_JPEG:
        return "JPEG";
    default:
        return "UNKNOWN";
    }
}

static void camera_capture_and_preview(void) {
    ui_show_msg("Getting camera image...");
    ui_delay_ms(300);

    if (!hal_camera_capture(&camera_frame)) {
        FATAL("Failed to capture camera image.");
    }

    LOG_INFO("camera frame: %ux%u pixfmt=%s size=%zu bytes_per_line=%u", camera_frame.width,
             camera_frame.height, camera_pixfmt_name(camera_frame.pixfmt), camera_frame.size,
             camera_frame.bytes_per_line);

    camera_rgb565 = camera_frame_to_rgb565(&camera_frame, &camera_w, &camera_h);
    ui_show_camera_image(camera_rgb565, camera_w, camera_h, on_camera_use, on_camera_retake,
                         on_camera_back);
}

static void on_camera_image(void) {
    if (!hal_camera_available()) {
        FATAL("Camera not available.");
    }
    camera_capture_and_preview();
}

static void on_camera_retake(void) {
    camera_release();
    camera_capture_and_preview();
}

static void on_camera_back(void) {
    camera_release();
    ui_show_other_source(on_camera_image, on_scan_qr, on_dice_rolls, on_coin_flips, go_source);
}

static void on_camera_use(void) {
    /* Derive entropy (16 or 32 bytes) from the raw camera bytes. */
    size_t  elen = (word_count == 24) ? 32 : 16;
    uint8_t entropy[32];
    sha256_expand(camera_frame.data, camera_frame.size, entropy, elen);

    mnemonic_t* m = mnemonic_from_entropy(entropy, elen);
    memset(entropy, 0, sizeof(entropy));

    camera_release();

    if (current) {
        ui_log_add("Combined with %u-word camera image", word_count);
        show_merge_screen(m, MNEMONIC_TYPE_MERGED);
    } else {
        current = m;
        ui_log_add("Generated %u-word mnemonic from camera image", word_count);
        ui_show_mnemonic(mnemonic_words(current), MNEMONIC_TYPE_GENERATED, go_source);
    }
}

static void on_scan_qr(void) { FATAL("Scan QR not yet implemented."); }

static void on_dice_rolls(void) { FATAL("Dice rolls not yet implemented."); }

static void on_coin_flips(void) { FATAL("Coin flips not yet implemented."); }

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
    ui_show_source(on_generate, on_enter_manual, on_other_source, on_show_state, on_finish,
                   current != NULL);
}

static void go_back_source(void) {
    ui_show_source(on_generate, on_enter_manual, on_other_source, on_show_state, on_finish,
                   current != NULL);
}

/* -- Step: word count chosen ------------------------------------------ */
static void on_12(void) {
    word_count = 12;
    ui_show_source(on_generate, on_enter_manual, on_other_source, on_show_state, on_finish,
                   current != NULL);
}
static void on_24(void) {
    word_count = 24;
    ui_show_source(on_generate, on_enter_manual, on_other_source, on_show_state, on_finish,
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
