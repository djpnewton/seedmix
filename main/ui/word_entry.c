/**
 * @file main/ui/word_entry.c
 * @brief Guided BIP39 word entry with autocomplete keyboard.
 */

#include "word_entry.h"
#include "crypto/bip39_wordlist.h"
#include "ui_internal.h"
#include "util/error.h"
#include "util/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -- State ------------------------------------------------------------ */
typedef struct {
    unsigned  total;
    unsigned  current;
    char      result[512];
    char      prefix[32];
    lv_obj_t* match_container;
    lv_obj_t* status;
    lv_obj_t* keyboard;
    lv_obj_t* entry_screen; // word entry screen (for returning from confirm)
    ui_cb_t   on_done;
    ui_cb_t   on_cancel;
    char      selected[32];
} we_ctx_t;

static void we_update_status(we_ctx_t* c) {
    char buf[32];
    int  res = snprintf(buf, sizeof(buf), "Word %u of %u", c->current + 1, c->total);
    ASSERT_OR_DIE(res > 0 && (size_t)res < sizeof(buf), "status string too long");
    if (c->status) lv_label_set_text(c->status, buf);
}

/* -- Forward declarations --------------------------------------------- */
static void we_go_back(lv_event_t* e);
static void we_select_word(lv_event_t* e);
static void we_keyboard_cb(lv_event_t* e);
static void we_confirm(lv_event_t* e);
static void we_cancel_confirm(lv_event_t* e);

/* -- Refresh match list ----------------------------------------------- */
static void we_refresh_matches(we_ctx_t* c) {
    if (!c->match_container) return;
    lv_obj_clean(c->match_container);
    c->selected[0] = '\0';

    const char* match_buf[2048];
    size_t      n = bip39_wordlist_lookup(c->prefix, match_buf);
    for (size_t i = 0; i < n; i++) {
        lv_obj_t* lbl = lv_label_create(c->match_container);
        lv_label_set_text(lbl, match_buf[i]);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(lbl, ui_font(14), 0);
        lv_obj_set_style_bg_color(lbl, lv_color_hex(0x333333), 0);
        lv_obj_set_style_pad_hor(lbl, 8, 0);
        lv_obj_set_style_pad_ver(lbl, 4, 0);
        lv_obj_add_flag(lbl, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(lbl, we_select_word, LV_EVENT_CLICKED, c);
    }

    if (!c->keyboard) return;
    lv_obj_t* kb = c->keyboard;
    char      test[sizeof(c->prefix) + 2]; // prefix + 1 char + null
    for (int i = 0; i < 26; i++) {
        int res = snprintf(test, sizeof(test), "%s%c", c->prefix, "qwertyuiopasdfghjklzxcvbnm"[i]);
        ASSERT_OR_DIE(res > 0 && (size_t)res < sizeof(test), "test string too long");
        if (bip39_wordlist_lookup(test, match_buf) > 0) {
            lv_buttonmatrix_clear_button_ctrl(kb, (uint32_t)i, LV_BUTTONMATRIX_CTRL_DISABLED);
        } else {
            lv_buttonmatrix_set_button_ctrl(kb, (uint32_t)i, LV_BUTTONMATRIX_CTRL_DISABLED);
        }
    }
}

/* -- Event handlers --------------------------------------------------- */
static void we_keyboard_cb(lv_event_t* e) {
    we_ctx_t* c = lv_event_get_user_data(e);
    ASSERT_OR_DIE(c, "null context");
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t*   kb  = lv_event_get_target(e);
        const char* txt = lv_textarea_get_text(lv_keyboard_get_textarea(kb));
        strncpy(c->prefix, txt, sizeof(c->prefix) - 1);
        c->selected[0] = '\0';
        we_refresh_matches(c);
    } else if (lv_event_get_code(e) == LV_EVENT_READY) {
        // keyboard "Done" - confirm selected word
        if (c->selected[0]) {
            strncpy(c->prefix, c->selected, sizeof(c->prefix) - 1);
            c->selected[0] = '\0';
            ui_word_entry_next(c);
        }
    }
}

static void we_select_word(lv_event_t* e) {
    we_ctx_t* c = lv_event_get_user_data(e);
    ASSERT_OR_DIE(c, "null context");
    const char* word = lv_label_get_text(lv_event_get_target(e));
    if (word && *word) {
        strncpy(c->selected, word, sizeof(c->selected) - 1);
        // show confirm screen
        lv_obj_t* s     = ui_make_screen();
        lv_obj_t* title = lv_label_create(s);
        lv_label_set_text(title, "Confirm Word");
        lv_obj_set_style_text_color(title, lv_color_white(), 0);
        lv_obj_set_style_text_font(title, ui_font(28), 0);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, ui_scale(15));

        lv_obj_t* w = lv_label_create(s);
        lv_label_set_text(w, word);
        lv_obj_set_style_text_color(w, lv_color_white(), 0);
        lv_obj_set_style_text_font(w, ui_font(48), 0);
        lv_obj_align(w, LV_ALIGN_CENTER, 0, ui_scale(-10));

        ui_add_btn_evt(s, "Yes", we_confirm, c, UI_BTN_SIZE_MED, LV_ALIGN_CENTER, -90, 60);

        ui_add_btn_evt(s, "No", we_cancel_confirm, c, UI_BTN_SIZE_MED, LV_ALIGN_CENTER, 90, 60);

        ui_nav_build(s);
        lv_scr_load(s);
    }
}

static void we_confirm(lv_event_t* e) {
    we_ctx_t* c = lv_event_get_user_data(e);
    ASSERT_OR_DIE(c, "null context");
    lv_obj_t* confirm_screen = lv_obj_get_parent(lv_event_get_target(e));
    if (!c->selected[0]) return;
    strncpy(c->prefix, c->selected, sizeof(c->prefix) - 1);
    c->selected[0] = '\0';
    ui_word_entry_next(c); // loads entry_screen first
    lv_obj_delete(confirm_screen);
}

static void we_cancel_confirm(lv_event_t* e) {
    we_ctx_t* c = lv_event_get_user_data(e);
    ASSERT_OR_DIE(c, "null context");
    lv_obj_t* confirm_screen = lv_obj_get_parent(lv_event_get_target(e));
    c->selected[0]           = '\0';
    if (c->entry_screen) {
        ui_nav_build(c->entry_screen);
        lv_scr_load(c->entry_screen); // load first
    }
    lv_obj_delete(confirm_screen); // then delete old
}

/* -- Public API ------------------------------------------------------- */
word_entry_handle_t ui_word_entry_begin(unsigned total_words, ui_cb_t on_done, ui_cb_t on_cancel) {
    ASSERT_OR_DIE(total_words > 0 && total_words <= 24, "total_words must be between 1 and 24");
    ASSERT_OR_DIE(on_done, "on_done callback is required");
    ASSERT_OR_DIE(on_cancel, "on_cancel callback is required");

    bip39_wordlist_init();
    we_ctx_t* c = calloc(1, sizeof(*c));
    ASSERT_OR_DIE(c, "out of memory");
    c->total     = total_words;
    c->on_done   = on_done;
    c->on_cancel = on_cancel;

    lv_obj_t* s     = ui_make_screen();
    c->entry_screen = s;

    c->status = lv_label_create(s);
    lv_obj_set_style_text_color(c->status, lv_color_white(), 0);
    lv_obj_set_style_text_font(c->status, ui_font(20), 0);
    lv_obj_align(c->status, LV_ALIGN_TOP_MID, 0, ui_scale(5));

    lv_obj_t* ta = lv_textarea_create(s);
    lv_obj_set_size(ta, ui_scale(380), ui_scale(40));
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, ui_scale(30));
    lv_obj_set_style_text_font(ta, ui_font(24), 0);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0x222222), 0);
    lv_obj_set_style_text_color(ta, lv_color_white(), 0);
    lv_textarea_set_placeholder_text(ta, "Type to filter...");
    lv_group_focus_obj(ta);
    we_update_status(c);

    c->match_container = lv_obj_create(s);
    lv_obj_set_size(c->match_container, ui_scale(440), ui_scale(30));
    lv_obj_align(c->match_container, LV_ALIGN_TOP_MID, 0, ui_scale(78));
    lv_obj_set_style_bg_color(c->match_container, lv_color_hex(0x111111), 0);
    lv_obj_set_style_border_width(c->match_container, 0, 0);
    lv_obj_set_flex_flow(c->match_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(c->match_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(c->match_container, LV_DIR_HOR);

    lv_obj_t* kb = lv_keyboard_create(s);
    c->keyboard  = kb;
    lv_keyboard_set_textarea(kb, ta);

    // clang-format off
    static const char* kb_map[] = {
        "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "\n",
        "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
        "z", "x", "c", "v", "b", "n", "m", LV_SYMBOL_BACKSPACE, "\n",
        ""
    };
    // clang-format on
    lv_buttonmatrix_ctrl_t kb_ctrl[27] = {0};
    lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_LOWER, kb_map, kb_ctrl);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_add_event_cb(kb, we_keyboard_cb, LV_EVENT_ALL, c); // pass ctx as user_data
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);

    ui_add_btn_evt(s, "Back", we_go_back, c, UI_BTN_SIZE_SMALL, LV_ALIGN_TOP_RIGHT, -10, 5);

    ui_nav_build(s);
    lv_scr_load(s);
    return c;
}

bool ui_word_entry_next(word_entry_handle_t handle) {
    we_ctx_t* c = (we_ctx_t*)handle;
    ASSERT_OR_DIE(c, "null context");
    const char* matches[2048];
    size_t      n    = bip39_wordlist_lookup(c->prefix, matches);
    const char* word = (n > 0) ? matches[0] : c->prefix;
    if (!word[0]) return false;

    if (c->current > 0) strcat(c->result, " ");
    strcat(c->result, word);
    c->current++;
    c->prefix[0]   = '\0';
    c->selected[0] = '\0';

    if (c->keyboard) {
        lv_obj_t* ta = lv_keyboard_get_textarea(c->keyboard);
        if (ta) lv_textarea_set_text(ta, "");
    }

    if (c->current >= c->total) {
        c->on_done();
        return true;
    }

    we_update_status(c);
    we_refresh_matches(c);
    if (c->entry_screen) {
        ui_nav_build(c->entry_screen);
        lv_scr_load(c->entry_screen);
    }
    return false;
}

static void we_go_back(lv_event_t* e) {
    we_ctx_t* c = lv_event_get_user_data(e);
    ASSERT_OR_DIE(c, "null context");
    if (c->current == 0) {
        c->on_cancel();
        return;
    }
    char* last = strrchr(c->result, ' ');
    if (last) {
        secure_memzero(last, strlen(last) + 1);
    } else {
        secure_memzero(c->result, strlen(c->result) + 1);
    }
    c->current--;
    c->prefix[0] = '\0';

    if (c->keyboard) {
        lv_obj_t* ta = lv_keyboard_get_textarea(c->keyboard);
        if (ta) lv_textarea_set_text(ta, "");
    }

    we_update_status(c);
    we_refresh_matches(c);
    if (c->entry_screen) ui_nav_build(c->entry_screen);
}

const char* ui_word_entry_result(word_entry_handle_t handle) {
    we_ctx_t* c = (we_ctx_t*)handle;
    ASSERT_OR_DIE(c, "null context");
    return c->result;
}

static void delete_screen_async(void* ptr) { lv_obj_delete((lv_obj_t*)ptr); }

void ui_word_entry_discard(word_entry_handle_t handle) {
    we_ctx_t* c = (we_ctx_t*)handle;
    ASSERT_OR_DIE(c, "null context");
    if (c->entry_screen) lv_async_call(delete_screen_async, c->entry_screen);
    secure_memzero(c, sizeof(*c));
    free(c);
}
