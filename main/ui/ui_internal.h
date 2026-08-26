/**
 * @file main/ui/ui_internal.h
 * @brief Internal helpers shared between ui.c and log.c.
 */

#ifndef UI_INTERNAL_H
#define UI_INTERNAL_H

#include "lvgl.h"
#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t* ui_make_screen(void);
lv_obj_t* ui_add_title(lv_obj_t* parent, const char* text);
void      ui_btn_invoke(lv_event_t* e);
void      ui_swap_screen(lv_obj_t* new_scr);

#ifdef __cplusplus
}
#endif

#endif /* UI_INTERNAL_H */
