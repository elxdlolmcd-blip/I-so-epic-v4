#include "mapdata.h"
typedef char chk_tri[sizeof(MapTri) == 24 ? 1 : -1];
typedef char chk_chunk[sizeof(MapChunk) == 24 ? 1 : -1];

int mapdata_load(MapData *m, const char *name, const unsigned char *blob)
{
    if (blob[0] != 'C' || blob[1] != 'G' || blob[2] != 'M' || blob[3] != 'P') return 0;
    const unsigned *h = (const unsigned *)blob;
    const float *f = (const float *)(blob + 16);
    m->name = name; m->ntris = (int)h[1]; m->nchunks = (int)h[2]; m->nspawns = (int)h[3];
    m->scale = f[0]; m->origin[0] = f[1]; m->origin[1] = f[2]; m->origin[2] = f[3];
    const unsigned char *p = blob + 32;
    m->spawns = (const float (*)[3])p; p += 12 * m->nspawns;
    m->chunks = (const MapChunk *)p;   p += sizeof(MapChunk) * m->nchunks;
    m->tris = (const MapTri *)p;
    return 1;
}
