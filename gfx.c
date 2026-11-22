#include <gsKit.h>
#include <dmaKit.h>
#include "gfx.h"

int SW = 640, SH = 448;
static GSGLOBAL *gs;
static int vx, vy, vw = 640, vh = 448;

void gfx_init(void)
{
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);
    gs = gsKit_init_global();
    gs->PSM = GS_PSM_CT24;
    gs->PSMZ = GS_PSMZ_24;
    gs->ZBuffering = GS_SETTING_ON;
    gs->DoubleBuffering = GS_SETTING_ON;
    gsKit_init_screen(gs);
    gsKit_mode_switch(gs, GS_ONESHOT);
    SW = gs->Width; SH = gs->Height;
    gfx_viewport(0, 0, SW, SH);
}

void gfx_begin(int r, int g, int b) { gsKit_clear(gs, RGB(r, g, b)); gsKit_set_test(gs, GS_ZTEST_ON); }
void gfx_ztest(int on) { gsKit_set_test(gs, on ? GS_ZTEST_ON : GS_ZTEST_OFF); }
void gfx_end(void) { gsKit_queue_exec(gs); gsKit_sync_flip(gs); }
void gfx_viewport(int x, int y, int w, int h) { vx = x; vy = y; vw = w; vh = h; }
int gfx_vw(void) { return vw; }
int gfx_vh(void) { return vh; }

void gfx_rect(float x1, float y1, float x2, float y2, u64 c)
{
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > vw) x2 = (float)vw;
    if (y2 > vh) y2 = (float)vh;
    if (x2 <= x1 || y2 <= y1) return;
    gsKit_prim_sprite(gs, vx + x1, vy + y1, vx + x2, vy + y2, 1, c);
}

void gfx_tri3d(float x1, float y1, int z1, float x2, float y2, int z2, float x3, float y3, int z3, int r, int g, int b)
{
    gsKit_prim_triangle_3d(gs, vx + x1, vy + y1, z1, vx + x2, vy + y2, z2, vx + x3, vy + y3, z3, RGB(r, g, b));
}
