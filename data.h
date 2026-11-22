#ifndef DATA_H
#define DATA_H
/* Recursos incrustados en el ELF (.incbin en data.c) */
#define DATA(n) extern const unsigned char n[], n##_end[];
DATA(chicken_bin)
DATA(map_chalohouse_bin) DATA(map_dust2_bin) DATA(map_town_bin) DATA(map_house_bin)
DATA(map_lava_bin) DATA(map_proto07_bin) DATA(map_racetrack_bin) DATA(map_dungeons_bin)
DATA(snd_music) DATA(snd_ui_click) DATA(snd_shot_pistol) DATA(snd_shot_auto) DATA(snd_shot_rifle)
DATA(snd_shot_shotgun) DATA(snd_shot_rocket) DATA(snd_dry) DATA(snd_reload) DATA(snd_step)
DATA(snd_hurt) DATA(snd_boom) DATA(snd_pickup)
DATA(audsrv_irx)
#endif
