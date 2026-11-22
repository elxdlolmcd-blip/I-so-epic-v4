#include <stdio.h>
#include <string.h>
#include <math.h>
#include "game.h"
#include "gfx.h"
#include "engine.h"
#include "controls.h"
#include "weapons.h"
#include "audio.h"
#include "data.h"

static MapData maps[NMAPS], chicken;
static const char *MAP_NAMES[NMAPS] = { "CHALOHOUSE", "DUST", "TOWN", "HOUSE", "LAVA", "PROTO07", "RACETRACK1", "DUNGEONS" };
static const unsigned char *const MAP_BLOB[NMAPS] = {
    map_chalohouse_bin, map_dust2_bin, map_town_bin, map_house_bin, map_lava_bin, map_proto07_bin, map_racetrack_bin, map_dungeons_bin };

typedef struct {
    int w, holstered, crouch, moving, ammo[NWEAP];
    float shoot_cd, reload_t, flash, step_t, gren_cd;
} PState;

static Player3D pl[2];
static PState ps[2];
static int np = 1, cur_map = 0, tris[2];

void game_init(void)
{
    for (int i = 0; i < NMAPS; i++) mapdata_load(&maps[i], MAP_NAMES[i], MAP_BLOB[i]);
    mapdata_load(&chicken, "CHICKEN", chicken_bin);
}
const MapData *game_map(int i) { return &maps[i % NMAPS]; }
const char *game_map_name(int i) { return MAP_NAMES[i % NMAPS]; }
const MapData *game_chicken(void) { return &chicken; }

static void init_pstate(PState *s)
{
    memset(s, 0, sizeof *s);
    for (int k = 0; k < NWEAP; k++) { s->ammo[k] = WEAPONS[k].mag; if (!strcmp(WEAPONS[k].id, "DesertEagle")) s->w = k; }
}

void game_start(const GameParams *gp)
{
    np = gp->nplayers < 1 ? 1 : (gp->nplayers > 2 ? 2 : gp->nplayers);
    cur_map = gp->map % NMAPS;
    world_set_map(&maps[cur_map]);
    for (int i = 0; i < np; i++) { world_spawn(&pl[i], i); init_pstate(&ps[i]); }
}

static int shot_sound(const Weapon *w)
{
    if (w->kind == WK_EXPLOSIVE) return SND_ROCKET;
    if (w->kind == WK_MELEE) return SND_CLICK;
    if (w->cd < 0.2f) return SND_AUTO;
    if (w->dmg >= 50) return SND_SHOTGUN;
    return w->cd < 0.35f ? SND_RIFLE : SND_PISTOL;
}

int game_update(float dt, const Pad *pads)
{
    if (pads[0].pressed & BTN_MENU) return 1;
    for (int i = 0; i < np; i++) {
        const Pad *pd = &pads[i];
        Player3D *p = &pl[i];
        PState *s = &ps[i];
        const Weapon *w = &WEAPONS[s->w];
        if (pd->pressed & BTN_RESTART) world_spawn(p, i);

        float sens = (pd->now & BTN_AIM) ? 1.2f : 2.4f;
        p->yaw += (pd->rx / 128.0f) * sens * dt;
        p->pitch -= (pd->ry / 128.0f) * 1.6f * dt * (INVERT_Y ? -1.0f : 1.0f);
        if (p->pitch > 1.2f) p->pitch = 1.2f;
        if (p->pitch < -1.2f) p->pitch = -1.2f;

        float fwd = -pd->ly / 128.0f, str = pd->lx / 128.0f, m = sqrtf(fwd * fwd + str * str);
        if (m > 1.0f) { fwd /= m; str /= m; m = 1.0f; }
        s->crouch = (pd->now & BTN_CROUCH) != 0;
        float speed = (pd->now & BTN_WALK) ? PLAYER_SPEED * 0.5f : PLAYER_SPEED;
        world_move(p, fwd, str, speed, dt, (pd->pressed & BTN_JUMP) != 0, s->crouch);

        s->step_t -= dt;
        if (p->grounded && m > 0.3f && s->step_t <= 0) { audio_play(SND_STEP); s->step_t = (pd->now & BTN_WALK) ? 0.6f : 0.38f; }

        if (pd->pressed & BTN_PREV_WEAPON) { s->w = (s->w + NWEAP - 1) % NWEAP; s->reload_t = 0; audio_play(SND_CLICK); }
        if (pd->pressed & BTN_NEXT_WEAPON) { s->w = (s->w + 1) % NWEAP; s->reload_t = 0; audio_play(SND_CLICK); }
        if (pd->pressed & BTN_HOLSTER) { s->holstered ^= 1; audio_play(SND_CLICK); }
        w = &WEAPONS[s->w];

        s->shoot_cd -= dt; s->gren_cd -= dt; if (s->flash > 0) s->flash -= dt;
        if (w->kind != WK_MELEE) {
            if (s->reload_t > 0) { s->reload_t -= dt; if (s->reload_t <= 0) s->ammo[s->w] = w->mag; }
            else if (((pd->pressed & BTN_RELOAD) || s->ammo[s->w] == 0) && s->ammo[s->w] < w->mag && !s->holstered) {
                s->reload_t = w->reload; audio_play(SND_RELOAD);
            }
        }
        int autof = w->cd < 0.2f;
        int fire = autof ? (pd->now & (BTN_FIRE | BTN_FIRE2)) != 0 : (pd->pressed & (BTN_FIRE | BTN_FIRE2)) != 0;
        if (fire && !s->holstered && s->reload_t <= 0 && s->shoot_cd <= 0) {
            if (w->kind == WK_MELEE || s->ammo[s->w] > 0) {
                if (w->kind != WK_MELEE) s->ammo[s->w]--;
                s->shoot_cd = w->cd; s->flash = 0.06f; audio_play(shot_sound(w));
            } else { s->shoot_cd = 0.3f; audio_play(SND_DRY); }
        }
        if ((pd->pressed & BTN_GRENADE) && s->gren_cd <= 0) { s->gren_cd = 3.0f; audio_play(SND_BOOM); }
        if ((pd->pressed & BTN_ACTION) && s->shoot_cd <= 0) { s->shoot_cd = 0.5f; audio_play(SND_CLICK); }
    }
    return 0;
}

static void draw_hud(int idx)
{
    int vw = gfx_vw(), vh = gfx_vh();
    const PState *s = &ps[idx];
    const Weapon *w = &WEAPONS[s->w];
    float cx = vw * 0.5f, cy = vh * 0.5f;
    u64 white = RGB(255, 255, 255);
    gfx_rect(cx - 10, cy - 1, cx - 3, cy + 1, white); gfx_rect(cx + 3, cy - 1, cx + 10, cy + 1, white);
    gfx_rect(cx - 1, cy - 10, cx + 1, cy - 3, white); gfx_rect(cx - 1, cy + 3, cx + 1, cy + 10, white);
    gfx_text(10, 10, 2, MAP_NAMES[cur_map], RGB(255, 160, 40));
    if (np > 1) gfx_text(10, 26, 2, idx == 0 ? "P1" : "P2", white);
    gfx_text(vw - 10 - gfx_text_w(w->id, 2), 10, 2, w->id, RGB(255, 160, 40));
    if (w->kind != WK_MELEE) {
        float pw = w->mag > 12 ? 4.0f : 10.0f;
        for (int i = 0; i < s->ammo[s->w]; i++)
            gfx_rect(10 + i * (pw + 2), vh - 24, 10 + i * (pw + 2) + pw, vh - 12, s->reload_t > 0 ? RGB(120, 120, 120) : RGB(250, 210, 50));
        if (s->reload_t > 0) gfx_rect(10, vh - 8, 10 + 120 * (1.0f - s->reload_t / w->reload), vh - 5, white);
    }
    gfx_rect((float)vw - 26, (float)vh - 26, (float)vw - 10, (float)vh - 10, s->gren_cd <= 0 ? RGB(80, 200, 80) : RGB(120, 60, 60));
    if (s->flash > 0) gfx_rect(cx - 18, cy + vh * 0.18f, cx + 18, cy + vh * 0.26f, RGB(255, 230, 80));
}

void game_render(void)
{
    gfx_begin(110, 170, 230);
    int hh = SH / 2;
    for (int i = 0; i < np; i++) {
        int vy = np == 1 ? 0 : (i == 0 ? 0 : hh + 1), vh = np == 1 ? SH : hh - 1;
        gfx_viewport(0, vy, SW, vh);
        ModelInst other = {0}; int n = 0;
        if (np > 1) {   /* el otro jugador se ve como el pollo real */
            const Player3D *o = &pl[1 - i];
            other.model = &chicken; other.x = o->x; other.y = o->y; other.z = o->z; other.yaw = o->yaw; other.scale = 1.0f; n = 1;
        }
        gfx_ztest(1);
        world_render(&pl[i], ps[i].crouch, SW, vh, &other, n, gfx_tri3d, &tris[i]);
        gfx_ztest(0);
        draw_hud(i);
    }
    gfx_viewport(0, 0, SW, SH);
    if (np > 1) gfx_rect(0, (float)hh - 1, (float)SW, (float)hh + 1, RGB(0, 0, 0));
}
