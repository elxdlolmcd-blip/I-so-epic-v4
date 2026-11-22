#include <stdio.h>
#include <string.h>
#include <math.h>
#include "ui.h"
#include "gfx.h"
#include "engine.h"
#include "audio.h"
#include "weapons.h"
#include <libpad.h>

enum { S_MAIN, S_NEW, S_SERVERS, S_SHOP };
static int scr = S_MAIN, sel[4], mode, map, players = 1, shop_sel;
static float t_spin, msg_t;
static const char *msg = "";

static const int ACCENT[NMAPS][3] = {
    {120,170,200}, {215,190,120}, {150,150,170}, {235,150,120}, {220,110,60}, {120,170,120}, {110,120,150}, {110,90,80} };

#define C_TXT  RGB(25, 25, 25)
#define C_ORG  RGB(255, 150, 20)

static void bg(int light)
{
    /* degradado vertical: cada franja se pinta desde su inicio hasta el fondo (sin huecos) */
    for (int i = 0; i < 40; i++) {
        float t = i / 39.0f;
        int r = (int)((light ? 225 : 168) + ((light ? 190 : 88) - (light ? 225 : 168)) * t);
        int g = (int)((light ? 240 : 214) + ((light ? 222 : 172) - (light ? 240 : 214)) * t);
        int b = (int)((light ? 190 : 8) + ((light ? 120 : 22) - (light ? 190 : 8)) * t);
        gfx_rect(0, (float)(SH * i / 40), (float)SW, (float)SH, RGB(r, g, b));
    }
}

static void box(float x, float y, float w, float h, u64 fill, u64 border)
{
    gfx_rect(x - 2, y - 2, x + w + 2, y + h + 2, border);
    gfx_rect(x, y, x + w, y + h, fill);
}

static void button(float x, float y, float w, float h, const char *label, int selected, int disabled, int scale)
{
    box(x, y, w, h, selected ? RGB(245, 252, 225) : RGB(206, 232, 172), selected ? C_ORG : RGB(160, 196, 120));
    gfx_text(x + (w - gfx_text_w(label, scale)) / 2, y + (h - 5 * scale) / 2, scale, label, disabled ? RGB(140, 150, 125) : C_TXT);
}

static void draw_chicken(int x, int y, int w, int h, float yaw)
{
    ModelInst mi = { game_chicken(), 0, 0, 0, yaw, 1.0f };
    CamPose cp = { 0, 0.95f, -2.25f, 0, 0 };
    gfx_viewport(x, y, w, h);
    gfx_ztest(1);
    render_scene(&cp, w, h, NULL, &mi, 1, gfx_tri3d, NULL);
    gfx_ztest(0);
    gfx_viewport(0, 0, SW, SH);
}

static void back_button(void) { button(10, 10, 90, 28, "BACK", 0, 0, 2); }
static void title(const char *s) { gfx_text((SW - gfx_text_w(s, 3)) / 2.0f, 16, 3, s, C_TXT); }

void ui_init(void) { audio_music(1); }

static void go(int s) { scr = s; audio_play(SND_CLICK); }
static void say(const char *m) { msg = m; msg_t = 2.5f; }

int ui_update(float dt, const Pad *pads, GameParams *gp)
{
    const Pad *p = &pads[0];
    t_spin += dt; if (msg_t > 0) msg_t -= dt;
    u32 up = p->pressed & PAD_UP, dn = p->pressed & PAD_DOWN, lf = p->pressed & PAD_LEFT, rt = p->pressed & PAD_RIGHT;
    u32 ok = p->pressed & (PAD_CROSS | PAD_START), back = p->pressed & PAD_CIRCLE;
    if (p->pressed & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT)) audio_play(SND_CLICK);

    switch (scr) {
    case S_MAIN:
        if (lf) sel[S_MAIN] = (sel[S_MAIN] + 2) % 3;
        if (rt) sel[S_MAIN] = (sel[S_MAIN] + 1) % 3;
        if (ok) go(sel[S_MAIN] == 0 ? S_SHOP : (sel[S_MAIN] == 1 ? S_NEW : S_SERVERS));
        break;
    case S_NEW: {
        int *r = &sel[S_NEW];
        if (up) *r = (*r + 3) % 4;
        if (dn) *r = (*r + 1) % 4;
        if (*r == 0 && (lf || rt)) mode ^= 1;
        if (*r == 1 && lf) map = (map + NMAPS - 1) % NMAPS;
        if (*r == 1 && rt) map = (map + 1) % NMAPS;
        if (*r == 2 && (lf || rt)) players = players == 1 ? 2 : 1;
        if (back) go(S_MAIN);
        if (ok && *r == 3) {
            if (players == 2 && !pads[1].connected) { say("CONNECT CONTROLLER 2"); break; }
            gp->nplayers = players; gp->map = map; gp->mode = mode;
            audio_play(SND_PICKUP); audio_music(0);
            return 1;
        }
        break; }
    case S_SERVERS:
        if (up || dn) sel[S_SERVERS] ^= 1;
        if (back) go(S_MAIN);
        if (ok) { if (sel[S_SERVERS] == 0) go(S_NEW); else say("LAN GAMES: COMING SOON"); }
        break;
    case S_SHOP:
        if (up) shop_sel = (shop_sel + NWEAP - 1) % NWEAP;
        if (dn) shop_sel = (shop_sel + 1) % NWEAP;
        if (lf) shop_sel = (shop_sel + NWEAP - 8) % NWEAP;
        if (rt) shop_sel = (shop_sel + 8) % NWEAP;
        if (back) go(S_MAIN);
        break;
    }
    return 0;
}

static void draw_main(void)
{
    bg(0);
    /* perfil: nombre, nivel y barra de experiencia */
    box(48, 14, 170, 26, RGB(232, 246, 200), RGB(190, 220, 140));
    gfx_text(56, 22, 2, "PLAYER", C_TXT);
    box(48, 46, 170, 8, RGB(226, 240, 190), RGB(190, 220, 140));
    gfx_rect(48, 46, 48 + 170 * 0.4f, 54, RGB(240, 200, 60));
    box(10, 10, 38, 38, RGB(255, 255, 255), RGB(190, 220, 140));
    gfx_num(21, 18, 3, 2, C_ORG);
    gfx_text(18, 38, 1, "LVL", RGB(120, 120, 120));
    /* titulo */
    const char *l1 = "ROOSTER RUDY", *l2 = "FIREFIGHT";
    gfx_text(SW - 18 - gfx_text_w(l1, 5) + 2, 18 + 2, 5, l1, RGB(60, 100, 10));
    gfx_text(SW - 18 - gfx_text_w(l1, 5), 18, 5, l1, C_TXT);
    gfx_text(SW - 18 - gfx_text_w(l2, 5) + 2, 52 + 2, 5, l2, RGB(60, 100, 10));
    gfx_text(SW - 18 - gfx_text_w(l2, 5), 52, 5, l2, C_TXT);
    /* sombra y pollo (de espaldas, girando) */
    gfx_rect(SW / 2 - 60, 318, SW / 2 + 60, 322, RGB(120, 180, 20));
    gfx_rect(SW / 2 - 75, 322, SW / 2 + 75, 328, RGB(104, 168, 16));
    gfx_rect(SW / 2 - 60, 328, SW / 2 + 60, 332, RGB(120, 180, 20));
    draw_chicken(SW / 2 - 150, 40, 300, 300, t_spin * 0.7f);
    static const char *L[3] = { "SHOP", "PLAY", "SERVERS" };
    for (int i = 0; i < 3; i++) button(53 + i * 182, 356, 170, 54, L[i], sel[S_MAIN] == i, 0, 3);
    gfx_text(SW - 100, SH - 22, 2, "V 1.2.6", RGB(40, 90, 10));
    gfx_text(12, SH - 18, 1, "LEFT/RIGHT: MOVE   X: SELECT", RGB(40, 90, 10));
}

static void draw_new(const Pad *pads)
{
    bg(1);
    back_button(); title("NEW GAME");
    box(30, 48, 580, 352, RGB(238, 246, 222), RGB(190, 215, 150));
    int r = sel[S_NEW];
    #define MARK(row, y) if (r == (row)) gfx_rect(36, (y), 42, (y) + 12, C_ORG)
    MARK(0, 60); gfx_text(48, 62, 2, "SELECT GAME MODE", C_TXT);
    static const char *M[2] = { "DEATHMATCH", "TEAM FIGHT" };
    for (int i = 0; i < 2; i++) button(120 + i * 210, 82, 190, 40, M[i], mode == i, 0, 2);
    MARK(1, 138); gfx_text(48, 140, 2, "SELECT MAP", C_TXT);
    int first = map - 1; if (first < 0) first = 0; if (first > NMAPS - 4) first = NMAPS - 4;
    gfx_text(46, 214, 3, "<", C_TXT); gfx_text(588, 214, 3, ">", C_TXT);
    for (int i = 0; i < 4; i++) {
        int m = first + i; float x = 66 + i * 126;
        box(x, 160, 116, 96, m == map ? RGB(255, 255, 255) : RGB(232, 240, 215), m == map ? C_ORG : RGB(190, 215, 150));
        gfx_rect(x + 6, 166, x + 110, 220, RGB(ACCENT[m][0], ACCENT[m][1], ACCENT[m][2]));
        gfx_rect(x + 6, 190, x + 110, 220, RGB(ACCENT[m][0] * 3 / 4, ACCENT[m][1] * 3 / 4, ACCENT[m][2] * 3 / 4));
        const char *n = game_map_name(m);
        gfx_text(x + (116 - gfx_text_w(n, 2)) / 2.0f, 232, 2, n, C_TXT);
    }
    MARK(2, 274); gfx_text(48, 276, 2, "PLAYERS COUNT", C_TXT);
    char b[40]; snprintf(b, sizeof b, "< %d >", players);
    gfx_text(280, 272, 3, b, C_TXT);
    gfx_text(350, 276, 2, players == 2 ? "SPLIT SCREEN" : "OFFLINE", RGB(90, 130, 40));
    button(210, 330, 220, 44, "CREATE GAME", r == 3, 0, 3);
    if (players == 2 && !pads[1].connected) gfx_text(48, 382, 2, "CONTROLLER 2 NOT CONNECTED", RGB(190, 40, 30));
    gfx_text(12, SH - 18, 1, "UP/DOWN: ROW   LEFT/RIGHT: CHANGE   X: OK   O: BACK", RGB(70, 100, 30));
}

static void draw_servers(void)
{
    bg(1);
    back_button(); title("CONNECT TO GAME");
    box(20, 56, 230, 300, RGB(238, 246, 222), RGB(190, 215, 150));
    draw_chicken(22, 58, 226, 296, t_spin * 0.7f);
    box(270, 56, 350, 300, RGB(238, 246, 222), RGB(190, 215, 150));
    gfx_text(286, 70, 2, "LAN GAMES", C_TXT);
    gfx_text(286, 120, 2, "NO GAMES FOUND", RGB(120, 130, 100));
    gfx_text(286, 146, 2, "LAN: COMING SOON", RGB(120, 130, 100));
    button(270, 372, 168, 40, "CREATE GAME", sel[S_SERVERS] == 0, 0, 2);
    button(452, 372, 168, 40, "JOIN RANDOM", sel[S_SERVERS] == 1, 1, 2);
}

static void draw_shop(void)
{
    bg(1);
    back_button(); title("SHOP");
    box(30, 48, 580, 352, RGB(238, 246, 222), RGB(190, 215, 150));
    int top = shop_sel - 4; if (top < 0) top = 0; if (top > NWEAP - 9) top = NWEAP - 9;
    for (int i = 0; i < 9; i++) {
        int k = top + i; float y = 56 + i * 38;
        if (k >= NWEAP) break;
        box(40, y, 560, 32, k == shop_sel ? RGB(255, 255, 255) : RGB(226, 238, 205), k == shop_sel ? C_ORG : RGB(206, 226, 176));
        gfx_num(50, y + 11, 2, k + 1, RGB(120, 130, 100));
        gfx_text(90, y + 11, 2, WEAPONS[k].id, C_TXT);
        char b[24]; snprintf(b, sizeof b, "DMG %d", WEAPONS[k].dmg);
        gfx_text(330, y + 11, 2, b, RGB(70, 100, 30));
        if (WEAPONS[k].free_item) gfx_text(500, y + 11, 2, "FREE", RGB(40, 150, 40));
        else { snprintf(b, sizeof b, "$%d", WEAPONS[k].price); gfx_text(500, y + 11, 2, b, RGB(190, 130, 10)); }
    }
    gfx_text(12, SH - 18, 1, "UP/DOWN: SELECT   LEFT/RIGHT: PAGE   O: BACK", RGB(70, 100, 30));
}

void ui_draw(const Pad *pads)
{
    gfx_viewport(0, 0, SW, SH);
    gfx_begin(168, 214, 8);
    gfx_ztest(0);
    switch (scr) {
    case S_MAIN: draw_main(); break;
    case S_NEW: draw_new(pads); break;
    case S_SERVERS: draw_servers(); break;
    case S_SHOP: draw_shop(); break;
    }
    if (msg_t > 0) {
        box(SW / 2 - 170, SH - 70, 340, 34, RGB(40, 40, 40), RGB(255, 150, 20));
        gfx_text((SW - gfx_text_w(msg, 2)) / 2.0f, SH - 60, 2, msg, RGB(255, 255, 255));
    }
}
