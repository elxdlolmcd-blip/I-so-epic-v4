#ifndef MAPDATA_H
#define MAPDATA_H
/* Mapas y modelos de Chicken Gun convertidos desde Unity (formato CGMP, ver tools/cgbin.py).
 * Unidades de Unity (metros); ejes de Unity: x derecha, y arriba, z adelante. Posicion = origin + v * scale. */
typedef struct { short v[9]; unsigned char col[3]; signed char n[3]; } MapTri;
typedef struct { float cx, cy, cz, r; unsigned first, count; } MapChunk;
typedef struct {
    const char *name; float scale; float origin[3];
    int ntris; const MapTri *tris;
    int nchunks; const MapChunk *chunks;
    int nspawns; const float (*spawns)[3];
} MapData;
int mapdata_load(MapData *m, const char *name, const unsigned char *blob);   /* 1 = ok */
#endif
