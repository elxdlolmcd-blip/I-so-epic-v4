#ifndef UI_H
#define UI_H
#include "pad.h"
#include "game.h"
/* Menu replicado del Chicken Gun original: Shop / Play / Servers, "New game" y "Connect to game". */
void ui_init(void);
int  ui_update(float dt, const Pad *pads, GameParams *gp);   /* 1 = iniciar partida con gp */
void ui_draw(const Pad *pads);
#endif
