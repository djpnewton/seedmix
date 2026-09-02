/**
 * @file main/ui/mnemonic_view.c
 * @brief Reusable numbered-grid view for displaying a BIP39 mnemonic.
 */

#include "mnemonic_view.h"
#include "ui_internal.h"
#include "util/error.h"
#include "util/utils.h"
#include <stdio.h>
#include <string.h>

#define MNEMONIC_VIEW_MAX_WORDS 24
#define MNEMONIC_VIEW_COLS 4
#define MNEMONIC_VIEW_ROW_H 30

/* -- Public API ------------------------------------------------------- */
lv_obj_t* ui_mnemonic_view_create(lv_obj_t* parent) {
    ASSERT_OR_DIE(parent, "null parent");

    lv_obj_t* view = lv_obj_create(parent);
    lv_obj_set_size(view, ui_scale(460), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(view, lv_color_hex(0x111111), 0);
    lv_obj_set_style_border_width(view, 0, 0);
    lv_obj_set_style_pad_all(view, 4, 0);
    lv_obj_set_style_pad_row(view, 2, 0);
    lv_obj_set_style_pad_column(view, 2, 0);
    lv_obj_clear_flag(view, LV_OBJ_FLAG_SCROLLABLE);
    return view;
}

void ui_mnemonic_view_set_words(lv_obj_t* view, const char* words) {
    ASSERT_OR_DIE(view, "null view");
    ASSERT_OR_DIE(words, "null words");

    lv_obj_clean(view);

    /* Tokenize into a local array.  lv_label_set_text() copies the text, so
     * pointers into this buffer are only needed while building the cells. */
    char buf[512];
    strncpy(buf, words, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    const char* word[MNEMONIC_VIEW_MAX_WORDS];
    size_t      count   = 0;
    char*       saveptr = NULL;
    for (char* tok = strtok_r(buf, " \t\n", &saveptr); tok && count < MNEMONIC_VIEW_MAX_WORDS;
         tok       = strtok_r(NULL, " \t\n", &saveptr)) {
        word[count++] = tok;
    }

    if (count == 0) return;

    unsigned cols = MNEMONIC_VIEW_COLS;
    unsigned rows = (unsigned)((count + cols - 1) / cols);

    /* Grid template: equal-width columns, fixed-height rows. */
    int32_t col_dsc[MNEMONIC_VIEW_COLS + 1];
    int32_t row_dsc[7]; /* up to 6 rows (24 words / 4 cols) + terminator */
    for (unsigned c = 0; c < cols; c++) col_dsc[c] = LV_GRID_FR(1);
    col_dsc[cols] = LV_GRID_TEMPLATE_LAST;
    for (unsigned r = 0; r < rows; r++) row_dsc[r] = ui_scale(MNEMONIC_VIEW_ROW_H);
    row_dsc[rows] = LV_GRID_TEMPLATE_LAST;

    lv_obj_set_grid_dsc_array(view, col_dsc, row_dsc);

    for (size_t i = 0; i < count; i++) {
        unsigned col = (unsigned)(i % cols);
        unsigned row = (unsigned)(i / cols);

        lv_obj_t* cell = lv_obj_create(view);
        lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(cell, lv_color_hex(0x1c1c1c), 0);
        lv_obj_set_style_border_width(cell, 0, 0);
        lv_obj_set_style_radius(cell, 2, 0);
        lv_obj_set_style_pad_hor(cell, 3, 0);
        lv_obj_set_style_pad_ver(cell, 1, 0);
        lv_obj_set_style_pad_column(cell, 3, 0);
        lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER);
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, (int32_t)col, 1, LV_GRID_ALIGN_STRETCH,
                             (int32_t)row, 1);

        char num[8];
        int  res = snprintf(num, sizeof(num), "%u.", (unsigned)(i + 1));
        ASSERT_OR_DIE(res > 0 && (size_t)res < sizeof(num), "number too long");
        lv_obj_t* num_lbl = lv_label_create(cell);
        lv_label_set_text(num_lbl, num);
        lv_obj_set_style_text_color(num_lbl, lv_color_hex(0x888888), 0);
        lv_obj_set_style_text_font(num_lbl, ui_font(14), 0);
        lv_obj_set_style_min_width(num_lbl, ui_scale(20), 0);

        lv_obj_t* word_lbl = lv_label_create(cell);
        lv_label_set_text(word_lbl, word[i]);
        lv_obj_set_style_text_color(word_lbl, lv_color_white(), 0);
        lv_obj_set_style_text_font(word_lbl, ui_font(14), 0);
        lv_label_set_long_mode(word_lbl, LV_LABEL_LONG_DOT);
    }

    secure_memzero(buf, sizeof(buf));
}
