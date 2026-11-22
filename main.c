/* Chicken Gun PS2 v4: menu original (Shop/Play/Servers), mapas reales de Unity, pollo Rooster Rudy y sonidos. */
#include <stdio.h>
#include <stdlib.h>
#include <tamtypes.h>
#include <libpad.h>
#include "gfx.h"
#include "pad.h"
#include "game.h"
#include "ui.h"
#include "audio.h"

#ifdef _EE
#define TICKS_PER_SEC 294912000.0f
static unsigned cpu_count(void) { unsigned c; __asm__ volatile("mfc0 %0, $9" : "=r"(c)); return c; }
#else
#include <time.h>
#define TICKS_PER_SEC ((float)CLOCKS_PER_SEC)
static unsigned cpu_count(void) { return (unsigned)clock(); }
#endif

enum { S_MENU, S_GAME };

int main(void)
{
    gfx_init();
    pad_init();
    audio_init();
    game_init();
    ui_init();

    Pad pads[2] = {{0}};
    GameParams gp = { 1, 0, 0 };
    int state = S_MENU;
    unsigned last = cpu_count();

    for (;;) {
        unsigned now = cpu_count();
        float dt = (float)(unsigned)(now - last) / TICKS_PER_SEC;
        last = now;
        if (dt <= 0.0f || dt > 0.1f) dt = 1.0f / 30.0f;
        if (dt < 0.004f) dt = 0.004f;

        pad_poll(0, &pads[0]);
        pad_poll(1, &pads[1]);

        if (state == S_MENU) {
            if (ui_update(dt, pads, &gp)) { game_start(&gp); state = S_GAME; }
            else ui_draw(pads);
        }
        if (state == S_GAME) {
            if (game_update(dt, pads)) { state = S_MENU; ui_init(); ui_draw(pads); }
            else game_render();
        }
        audio_update(dt);
        gfx_end();
    }
    return 0;
}
