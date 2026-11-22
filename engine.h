#ifndef ENGINE_H
#define ENGINE_H
#include "mapdata.h"
/* Motor 3D portable (sin dependencias de PS2): render con clipping, modelos, colision y gravedad.
 * Valores del jugador tomados de P_Player.prefab de Unity (CharacterController y CharacterMotor). */
typedef void (*TriFn)(float x1, float y1, int z1, float x2, float y2, int z2, float x3, float y3, int z3, int r, int g, int b);
typedef struct { float x, y, z, yaw, pitch, vy; int grounded; } Player3D;       /* x,y,z = pies */
typedef struct { float x, y, z, yaw, pitch; } CamPose;                          /* posicion del ojo */
typedef struct { const MapData *model; float x, y, z, yaw, scale; } ModelInst;  /* modelo colocado en el mundo */

#define EYE_HEIGHT    1.55f   /* capsula de 1.67 m */
#define PLAYER_SPEED  5.5f    /* SpeedValue */

void world_set_map(const MapData *m);
void world_spawn(Player3D *p, int idx);
void world_move(Player3D *p, float fwd, float strafe, float speed, float dt, int jump, int crouch);
void render_scene(const CamPose *cam, int W, int H, const MapData *map, const ModelInst *inst, int ninst, TriFn fn, int *tri_count);
void world_render(const Player3D *p, int crouch, int W, int H, const ModelInst *inst, int ninst, TriFn fn, int *tri_count);
#endif
