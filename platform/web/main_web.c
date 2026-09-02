/**
 * @file platform/web/main_web.c
 * @brief Web (Emscripten) entry point - LVGL with the SDL backend, driven by
 *        the browser's requestAnimationFrame loop.
 */

#include "app.h"
#include "lv_sdl_keyboard.h"
#include "lv_sdl_mouse.h"
#include "lv_sdl_mousewheel.h"
#include "lv_sdl_window.h"
#include "lvgl.h"
#include <SDL2/SDL.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#define DISPLAY_WIDTH 480
#define DISPLAY_HEIGHT 320

static uint32_t sdl_tick_cb(void) { return SDL_GetTicks(); }

static void web_loop(void) { lv_timer_handler(); }

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    lv_init();
    lv_tick_set_cb(sdl_tick_cb);

    lv_display_t* disp = lv_sdl_window_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_sdl_window_set_title(disp, "seedmix");

    lv_sdl_mouse_create();
    lv_sdl_keyboard_create();
    lv_sdl_mousewheel_create();

    app_init();

    // fps = 0 -> requestAnimationFrame.  simulate_infinite_loop must be 0 when
    // ASYNCIFY is enabled; with EXIT_RUNTIME=0 the loop keeps running after
    // main() returns.
    emscripten_set_main_loop(web_loop, 0, 0);
    return 0;
}
