/**
 * @file lv_conf.h
 * @brief LVGL configuration - shared between Linux and ESP32 builds.
 *
 * This file is generic.  Platform-specific overrides (e.g. color depth,
 * memory size) go in lv_conf_<platform>.h and are included conditionally.
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------
 * General
 *----------------------------------------------------------------------*/
#define LV_COLOR_DEPTH          16
#define LV_COLOR_16_SWAP        0
#define LV_COLOR_SCREEN_TRANSP  0

/*----------------------------------------------------------------------
 * Memory
 *----------------------------------------------------------------------*/
#define LV_MEM_SIZE             (128U * 1024U)   /* 128 kB */
#define LV_MEM_ADR              0
#define LV_MEM_BUF_MAX_NUM      16
#define LV_MEMCPY_MEMSET_STD    1

/*----------------------------------------------------------------------
 * HAL
 *----------------------------------------------------------------------*/

/*----------------------------------------------------------------------
 * Display buffer
 *----------------------------------------------------------------------*/
#define LV_HOR_RES_MAX          (480)
#define LV_VER_RES_MAX          (320)
#define LV_DPI                  130

/*----------------------------------------------------------------------
 * GPU
 *----------------------------------------------------------------------*/
#define LV_USE_GPU              0
#define LV_GPU_DMA2D_FLUSH      0

/*----------------------------------------------------------------------
 * SDL (Linux desktop driver)
 *----------------------------------------------------------------------*/
#define LV_USE_SDL              1

/*----------------------------------------------------------------------
 * Logging
 *----------------------------------------------------------------------*/
#define LV_USE_LOG              1
#if LV_USE_LOG
    #define LV_LOG_LEVEL         LV_LOG_LEVEL_WARN
    #define LV_LOG_PRINTF        1
#endif

/*----------------------------------------------------------------------
 * Assets / Fonts
 *----------------------------------------------------------------------*/
#define LV_FONT_MONTSERRAT_10   1
#define LV_FONT_MONTSERRAT_12   1
#define LV_FONT_MONTSERRAT_14   1
#define LV_FONT_MONTSERRAT_18   1
#define LV_FONT_MONTSERRAT_20   1
#define LV_FONT_MONTSERRAT_24   1
#define LV_FONT_MONTSERRAT_28   1
#define LV_FONT_MONTSERRAT_48   1
#define LV_USE_FONT_COMPRESSED  1
#define LV_USE_FONT_SUBPX       1

/*----------------------------------------------------------------------
 * Features - enable what you need
 *----------------------------------------------------------------------*/
#define LV_USE_ANIMATION        1
#define LV_USE_SHADOW           0
#define LV_USE_OUTLINE          0
#define LV_USE_PATTERN          0
#define LV_USE_VALUE_STR        0
#define LV_USE_BLEND_MODES      0
#define LV_USE_OPA_SCALE        0
#define LV_USE_IMG_TRANSFORM    0

#define LV_USE_GROUP            1
#define LV_USE_GPU_DRAW         0
#define LV_USE_SKELETON         0

#define LV_USE_SYSMON           0
#define LV_USE_PERF_MONITOR     0

#define LV_USE_SNAPSHOT         0
#define LV_USE_MONKEY           0
#define LV_USE_GRIDNAV          0

/*----------------------------------------------------------------------
 * Widgets
 *----------------------------------------------------------------------*/
#define LV_USE_ARC              1
#define LV_USE_BAR              1
#define LV_USE_BTN              1
#define LV_USE_BTNMATRIX        1
#define LV_USE_CANVAS           0
#define LV_USE_CHECKBOX         0
#define LV_USE_DROPDOWN         0
#define LV_USE_IMG              1
#define LV_USE_LABEL            1
#define LV_USE_LINE             1
#define LV_USE_ROLLER           0
#define LV_USE_SLIDER           0
#define LV_USE_SWITCH           0
#define LV_USE_TEXTAREA         1
#define LV_USE_TABLE            0
#define LV_USE_TABVIEW          0
#define LV_USE_TILEVIEW         0
#define LV_USE_WIN              0

#define LV_USE_SPINNER          1
#define LV_USE_CALENDAR         0
#define LV_USE_CHART            0
#define LV_USE_COLORWHEEL       0
#define LV_USE_IMGBTN           0
#define LV_USE_KEYBOARD         1
#define LV_USE_LED              0
#define LV_USE_LIST             0
#define LV_USE_MENU             0
#define LV_USE_METER            0
#define LV_USE_MSGBOX           1
#define LV_USE_SPAN             0
#define LV_USE_SPINBOX          0
#define LV_USE_TABVIEW          0
#define LV_USE_TILEVIEW         0
#define LV_USE_WIN              0

/*----------------------------------------------------------------------
 * Themes
 *----------------------------------------------------------------------*/
#define LV_USE_THEME_DEFAULT    1
#define LV_USE_THEME_MONO       0

#ifdef __cplusplus
}
#endif

#endif /* LV_CONF_H */
