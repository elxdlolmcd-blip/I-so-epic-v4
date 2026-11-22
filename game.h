#ifndef GAME_H
#define GAME_H
#include "pad.h"
#include "mapdata.h"
#define NMAPS 8
typedef struct { int nplayers, map, mode; } GameParams;   /* mode: 0 = Deathmatch, 1 = Team fight */
void game_init(void);                                      /* carga mapas y modelo del pollo desde los datos incrustados */
const MapData *game_map(int i);
const char *game_map_name(int i);
const MapData *game_chicken(void);
void game_start(const GameParams *gp);
int  game_update(float dt, const Pad *pads);               /* devuelve 1 si hay que volver al menu */
void game_render(void);
#endif
