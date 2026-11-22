/* Incrusta binarios en .rodata. Las rutas son relativas a la carpeta donde se ejecuta make. */
#define INCBIN(sym, file) \
    __asm__(".section .rodata\n.balign 16\n.global " #sym "\n" #sym ":\n.incbin \"" file "\"\n" \
            ".global " #sym "_end\n" #sym "_end:\n.balign 16\n.previous\n")

INCBIN(chicken_bin, "chicken.bin");
INCBIN(map_chalohouse_bin, "map_chalohouse.bin");
INCBIN(map_dust2_bin, "map_dust2.bin");
INCBIN(map_town_bin, "map_town.bin");
INCBIN(map_house_bin, "map_house.bin");
INCBIN(map_lava_bin, "map_lava.bin");
INCBIN(map_proto07_bin, "map_proto07.bin");
INCBIN(map_racetrack_bin, "map_racetrack.bin");
INCBIN(map_dungeons_bin, "map_dungeons.bin");
INCBIN(snd_music, "snd_music.raw");
INCBIN(snd_ui_click, "snd_ui_click.raw");
INCBIN(snd_shot_pistol, "snd_shot_pistol.raw");
INCBIN(snd_shot_auto, "snd_shot_auto.raw");
INCBIN(snd_shot_rifle, "snd_shot_rifle.raw");
INCBIN(snd_shot_shotgun, "snd_shot_shotgun.raw");
INCBIN(snd_shot_rocket, "snd_shot_rocket.raw");
INCBIN(snd_dry, "snd_dry.raw");
INCBIN(snd_reload, "snd_reload.raw");
INCBIN(snd_step, "snd_step.raw");
INCBIN(snd_hurt, "snd_hurt.raw");
INCBIN(snd_boom, "snd_boom.raw");
INCBIN(snd_pickup, "snd_pickup.raw");
#ifndef HOST
INCBIN(audsrv_irx, "audsrv.irx");   /* se busca con -Wa,-I$(PS2SDK)/iop/irx */
#endif
