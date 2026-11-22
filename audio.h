#ifndef AUDIO_H
#define AUDIO_H
enum { SND_CLICK, SND_PISTOL, SND_AUTO, SND_RIFLE, SND_SHOTGUN, SND_ROCKET, SND_DRY, SND_RELOAD, SND_STEP, SND_HURT, SND_BOOM, SND_PICKUP, SND_COUNT };
void audio_init(void);
void audio_update(float dt);     /* mezcla y envia audio; llamar una vez por frame */
void audio_play(int snd);        /* efecto de sonido */
void audio_music(int on);        /* musica del menu (ChickenSong) en bucle */
#endif
