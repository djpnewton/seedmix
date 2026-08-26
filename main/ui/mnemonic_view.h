/**
 * @file main/ui/mnemonic_view.h
 * @brief Reusable numbered-grid view for displaying a BIP39 mnemonic.
 */

#ifndef MNEMONIC_VIEW_H
#define MNEMONIC_VIEW_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a mnemonic grid container.
 *
 * The returned object is empty until ui_mnemonic_view_set_words() is called.
 * It is sized and styled ready to be placed on a screen.
 *
 * @param parent Parent object (usually a screen).
 * @return       The grid container.
 */
lv_obj_t* ui_mnemonic_view_create(lv_obj_t* parent);

/**
 * @brief Populate the grid from a space-separated mnemonic string.
 *
 * Each word is shown in a numbered cell.  The grid lays itself out in
 * 4 columns (3 rows for 12 words, 6 rows for 24 words).
 *
 * @param view  Grid returned by ui_mnemonic_view_create().
 * @param words Space-separated mnemonic words (e.g. "abandon ability ...").
 */
void ui_mnemonic_view_set_words(lv_obj_t* view, const char* words);

#ifdef __cplusplus
}
#endif

#endif /* MNEMONIC_VIEW_H */
