#ifndef GFX_H
#define GFX_H
#include <tamtypes.h>
#include <gsKit.h>
#define RGB(r,g,b) GS_SETREG_RGBAQ((r),(g),(b),0x80,0x00)
extern int SW, SH;
void gfx_init(void);
void gfx_begin(int r, int g, int b);   /* limpia color + z-buffer */
void gfx_ztest(int on);                /* 1 = z-test activo (3D), 0 = dibujo 2D encima */
void gfx_end(void);                    /* envia el frame y espera vsync */
void gfx_viewport(int x, int y, int w, int h);
int  gfx_vw(void);
int  gfx_vh(void);
void gfx_rect(float x1, float y1, float x2, float y2, u64 c);   /* relativo al viewport, recortado */
void gfx_tri3d(float x1, float y1, int z1, float x2, float y2, int z2, float x3, float y3, int z3, int r, int g, int b);
void gfx_text(float x, float y, int scale, const char *s, u64 c);
int  gfx_text_w(const char *s, int scale);
void gfx_num(float x, float y, int scale, int n, u64 c);
#endif
