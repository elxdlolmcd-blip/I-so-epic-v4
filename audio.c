/* Audio: mezclador por software (8 voces + musica) enviado con audsrv (PCM 16 bit mono 22050 Hz). */
#include <string.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <sbv_patches.h>
#include <audsrv.h>
#include "audio.h"
#include "data.h"

#define RATE 22050
#define NVOICE 8
#define MAXCHUNK (RATE / 8)

typedef struct { const short *d; int len, pos; } Voice;
static Voice vo[NVOICE];
static int ok, music_on, mpos, nextv;
static float acc;
static short mixbuf[MAXCHUNK];

static const struct { const unsigned char *s, *e; } TAB[SND_COUNT] = {
    { snd_ui_click, snd_ui_click_end }, { snd_shot_pistol, snd_shot_pistol_end }, { snd_shot_auto, snd_shot_auto_end },
    { snd_shot_rifle, snd_shot_rifle_end }, { snd_shot_shotgun, snd_shot_shotgun_end }, { snd_shot_rocket, snd_shot_rocket_end },
    { snd_dry, snd_dry_end }, { snd_reload, snd_reload_end }, { snd_step, snd_step_end },
    { snd_hurt, snd_hurt_end }, { snd_boom, snd_boom_end }, { snd_pickup, snd_pickup_end },
};
static const int VOL[SND_COUNT] = { 200, 256, 220, 240, 256, 256, 200, 230, 110, 256, 256, 230 };

void audio_init(void)
{
    int ret = 0;
    sbv_patch_enable_lmb();
    SifLoadModule("rom0:LIBSD", 0, NULL);
    SifExecModuleBuffer((void *)audsrv_irx, (u32)(audsrv_irx_end - audsrv_irx), 0, NULL, &ret);
    if (audsrv_init() != 0) { ok = 0; return; }
    audsrv_fmt_t f; f.bits = 16; f.freq = RATE; f.channels = 1;
    audsrv_set_format(&f);
    audsrv_set_volume(MAX_VOLUME);
    ok = 1;
    memset(mixbuf, 0, sizeof mixbuf);
    audsrv_play_audio((const char *)mixbuf, RATE / 10 * 2);   /* colchon inicial de silencio */
}

void audio_play(int snd)
{
    if (!ok || snd < 0 || snd >= SND_COUNT) return;
    Voice *v = &vo[nextv]; nextv = (nextv + 1) % NVOICE;
    v->d = (const short *)TAB[snd].s; v->len = (int)((TAB[snd].e - TAB[snd].s) / 2); v->pos = 0;
    (void)VOL;
}

void audio_music(int on) { music_on = on; if (on) mpos = 0; }

static void mix(short *out, int n)
{
    const short *mus = (const short *)snd_music;
    int mlen = (int)((snd_music_end - snd_music) / 2);
    for (int i = 0; i < n; i++) {
        int a = 0;
        if (music_on) { a += mus[mpos] * 120 / 256; if (++mpos >= mlen) mpos = 0; }
        for (int k = 0; k < NVOICE; k++) {
            if (!vo[k].d) continue;
            a += vo[k].d[vo[k].pos] * 230 / 256;
            if (++vo[k].pos >= vo[k].len) vo[k].d = 0;
        }
        out[i] = (short)(a > 32767 ? 32767 : (a < -32768 ? -32768 : a));
    }
}

void audio_update(float dt)
{
    if (!ok) return;
    acc += dt * RATE;
    int n = (int)acc; acc -= (float)n;
    if (n > MAXCHUNK) n = MAXCHUNK;
    if (n <= 0) return;
    mix(mixbuf, n);
    audsrv_wait_audio(n * 2);
    audsrv_play_audio((const char *)mixbuf, n * 2);
}
