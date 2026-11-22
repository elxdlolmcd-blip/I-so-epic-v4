#include <math.h>
#include <string.h>
#include "engine.h"

#define NEAR 0.15f
#define FAR 90.0f
#define ZMAX 16700000.0f
#define RADIUS 0.5f      /* CharacterController.radius */
#define STEP 0.35f       /* stepOffset */
#define GRAV 20.0f       /* CharacterMotor.gravity */
#define JUMPV 7.0f       /* CharacterMotor.jumpSpeed */
#define BODY_H 1.6f

static const MapData *M;
void world_set_map(const MapData *m) { M = m; }

static void tri_pts(const MapData *md, const MapTri *t, float p[3][3])
{
    for (int i = 0; i < 3; i++) {
        p[i][0] = md->origin[0] + t->v[i * 3 + 0] * md->scale;
        p[i][1] = md->origin[1] + t->v[i * 3 + 1] * md->scale;
        p[i][2] = md->origin[2] + t->v[i * 3 + 2] * md->scale;
    }
}

/* ---------- colision ---------- */
static float ground_at(float x, float z, float maxy)
{
    float best = -1e9f;
    for (int c = 0; c < M->nchunks; c++) {
        const MapChunk *ch = &M->chunks[c];
        float dx = x - ch->cx, dz = z - ch->cz;
        if (dx * dx + dz * dz > (ch->r + 0.5f) * (ch->r + 0.5f)) continue;
        for (unsigned i = ch->first; i < ch->first + ch->count; i++) {
            const MapTri *t = &M->tris[i];
            if (t->n[1] < 30) continue;                                 /* pendiente max ~75 grados */
            float p[3][3]; tri_pts(M, t, p);
            float d1 = (x - p[1][0]) * (p[0][2] - p[1][2]) - (p[0][0] - p[1][0]) * (z - p[1][2]);
            float d2 = (x - p[2][0]) * (p[1][2] - p[2][2]) - (p[1][0] - p[2][0]) * (z - p[2][2]);
            float d3 = (x - p[0][0]) * (p[2][2] - p[0][2]) - (p[2][0] - p[0][0]) * (z - p[0][2]);
            int neg = (d1 < 0) || (d2 < 0) || (d3 < 0), pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
            if (neg && pos) continue;
            float nx = (float)t->n[0], ny = (float)t->n[1], nz = (float)t->n[2];
            float h = p[0][1] - (nx * (x - p[0][0]) + nz * (z - p[0][2])) / ny;
            if (h <= maxy && h > best) best = h;
        }
    }
    return best;
}

static void push_walls(float *x, float *z, float y)
{
    for (int c = 0; c < M->nchunks; c++) {
        const MapChunk *ch = &M->chunks[c];
        float dx = *x - ch->cx, dz = *z - ch->cz;
        if (dx * dx + dz * dz > (ch->r + 1.0f) * (ch->r + 1.0f)) continue;
        for (unsigned i = ch->first; i < ch->first + ch->count; i++) {
            const MapTri *t = &M->tris[i];
            if (t->n[1] > 60 || t->n[1] < -60) continue;               /* solo paredes */
            float p[3][3]; tri_pts(M, t, p);
            float ymin = fminf(p[0][1], fminf(p[1][1], p[2][1])), ymax = fmaxf(p[0][1], fmaxf(p[1][1], p[2][1]));
            if (ymax < y + STEP || ymin > y + BODY_H) continue;
            int a = 0, b = 1; float best = -1;
            for (int k = 0; k < 3; k++) {
                int j = (k + 1) % 3; float ex = p[j][0] - p[k][0], ez = p[j][2] - p[k][2], l = ex * ex + ez * ez;
                if (l > best) { best = l; a = k; b = j; }
            }
            float sx = p[b][0] - p[a][0], sz = p[b][2] - p[a][2], l2 = sx * sx + sz * sz;
            float u = l2 > 1e-8f ? ((*x - p[a][0]) * sx + (*z - p[a][2]) * sz) / l2 : 0;
            u = u < 0 ? 0 : (u > 1 ? 1 : u);
            float qx = p[a][0] + sx * u, qz = p[a][2] + sz * u;
            float ox = *x - qx, oz = *z - qz, d = sqrtf(ox * ox + oz * oz);
            if (d >= RADIUS) continue;
            float nl = sqrtf((float)(t->n[0] * t->n[0] + t->n[2] * t->n[2]));
            float nx, nz;
            if (nl > 1e-3f) { nx = t->n[0] / nl; nz = t->n[2] / nl; if (nx * ox + nz * oz < 0) { nx = -nx; nz = -nz; } }
            else if (d > 1e-4f) { nx = ox / d; nz = oz / d; } else continue;
            *x += nx * (RADIUS - d); *z += nz * (RADIUS - d);
        }
    }
}

void world_spawn(Player3D *p, int idx)
{
    const float *s = M->spawns[idx % M->nspawns];
    memset(p, 0, sizeof *p);
    p->x = s[0]; p->z = s[2]; p->y = s[1] + 0.5f;
}

void world_move(Player3D *p, float fwd, float strafe, float speed, float dt, int jump, int crouch)
{
    float sy = sinf(p->yaw), cy = cosf(p->yaw), k = speed * (crouch ? 0.5f : 1.0f) * dt;
    p->x += (sy * fwd + cy * strafe) * k;
    p->z += (cy * fwd - sy * strafe) * k;
    push_walls(&p->x, &p->z, p->y); push_walls(&p->x, &p->z, p->y);
    if (jump && p->grounded) { p->vy = JUMPV; p->grounded = 0; }
    p->vy -= GRAV * dt;
    p->y += p->vy * dt;
    float g = ground_at(p->x, p->z, p->y + STEP);
    if (g > -1e8f && p->y <= g + 0.001f) { p->y = g; p->vy = 0; p->grounded = 1; }
    else p->grounded = (g > -1e8f && p->y <= g + 0.05f);
    if (p->y < -300.0f) world_spawn(p, 0);
}

/* ---------- render ---------- */
typedef struct { float x, y, z; } V3;
typedef struct {
    float eye[3], fwd[3], rgt[3], upv[3], tx, ty, fx, fy; int W, H; TriFn fn; int count;
} Cam;

static int clip_plane(const V3 *in, int n, V3 *out, int plane, float tx, float ty)
{
    int m = 0;
    for (int i = 0; i < n; i++) {
        const V3 *a = &in[i], *b = &in[(i + 1) % n];
        float da, db;
        switch (plane) {
            case 0: da = a->z - NEAR; db = b->z - NEAR; break;
            case 1: da = a->x + tx * a->z; db = b->x + tx * b->z; break;
            case 2: da = tx * a->z - a->x; db = tx * b->z - b->x; break;
            case 3: da = ty * a->z - a->y; db = ty * b->z - b->y; break;
            default: da = a->y + ty * a->z; db = b->y + ty * b->z; break;
        }
        if (da >= 0) out[m++] = *a;
        if ((da >= 0) != (db >= 0)) {
            float t = da / (da - db);
            out[m].x = a->x + (b->x - a->x) * t; out[m].y = a->y + (b->y - a->y) * t; out[m].z = a->z + (b->z - a->z) * t; m++;
        }
        if (m > 10) break;
    }
    return m;
}

static void draw_range(Cam *c, const MapData *md, unsigned first, unsigned count, const ModelInst *mi)
{
    const float sun[3] = { 0.40f, 0.80f, 0.35f };
    float cy = 1, sy = 0, sc = 1, px = 0, py = 0, pz = 0;
    if (mi) { cy = cosf(mi->yaw); sy = sinf(mi->yaw); sc = mi->scale; px = mi->x; py = mi->y; pz = mi->z; }
    for (unsigned i = first; i < first + count; i++) {
        const MapTri *t = &md->tris[i];
        float w[3][3]; tri_pts(md, t, w);
        float nx = t->n[0] / 127.0f, ny = t->n[1] / 127.0f, nz = t->n[2] / 127.0f;
        if (mi) {
            for (int k = 0; k < 3; k++) {
                float x = w[k][0] * sc, z = w[k][2] * sc;
                w[k][0] = px + x * cy + z * sy; w[k][2] = pz - x * sy + z * cy; w[k][1] = py + w[k][1] * sc;
            }
            float nx2 = nx * cy + nz * sy; nz = -nx * sy + nz * cy; nx = nx2;
        }
        if (nx * (c->eye[0] - w[0][0]) + ny * (c->eye[1] - w[0][1]) + nz * (c->eye[2] - w[0][2]) <= 0) continue;
        V3 v[3]; int allin = 1; float zavg = 0;
        for (int k = 0; k < 3; k++) {
            float q[3] = { w[k][0] - c->eye[0], w[k][1] - c->eye[1], w[k][2] - c->eye[2] };
            v[k].x = q[0] * c->rgt[0] + q[1] * c->rgt[1] + q[2] * c->rgt[2];
            v[k].y = q[0] * c->upv[0] + q[1] * c->upv[1] + q[2] * c->upv[2];
            v[k].z = q[0] * c->fwd[0] + q[1] * c->fwd[1] + q[2] * c->fwd[2];
            zavg += v[k].z;
            if (v[k].z < NEAR || fabsf(v[k].x) > c->tx * v[k].z || fabsf(v[k].y) > c->ty * v[k].z) allin = 0;
        }
        zavg /= 3.0f;
        if (zavg > FAR + 5.0f) continue;
        float lit = nx * sun[0] + ny * sun[1] + nz * sun[2];
        float sh = 0.60f + 0.55f * (lit > 0 ? lit : 0);
        float fog = (zavg - 50.0f) / (FAR - 50.0f); fog = fog < 0 ? 0 : (fog > 1 ? 1 : fog);
        int R = (int)((t->col[0] * sh) * (1 - fog) + 110 * fog), G = (int)((t->col[1] * sh) * (1 - fog) + 170 * fog), B = (int)((t->col[2] * sh) * (1 - fog) + 230 * fog);
        if (R > 255) R = 255;
        if (G > 255) G = 255;
        if (B > 255) B = 255;

        V3 poly[12], tmp[12]; int n = 3;
        memcpy(poly, v, sizeof v);
        if (!allin) {
            for (int pl = 0; pl < 5 && n >= 3; pl++) { n = clip_plane(poly, n, tmp, pl, c->tx, c->ty); memcpy(poly, tmp, sizeof(V3) * (n > 12 ? 12 : n)); }
            if (n < 3) continue;
        }
        float sxs[12], sys[12]; int zs[12];
        for (int k = 0; k < n; k++) {
            float iz = 1.0f / poly[k].z;
            sxs[k] = c->W * 0.5f + poly[k].x * iz * c->fx;
            sys[k] = c->H * 0.5f - poly[k].y * iz * c->fy;
            zs[k] = (int)(ZMAX * NEAR * iz);
        }
        for (int k = 1; k + 1 < n; k++) {
            c->fn(sxs[0], sys[0], zs[0], sxs[k], sys[k], zs[k], sxs[k + 1], sys[k + 1], zs[k + 1], R, G, B);
            c->count++;
        }
    }
}

static int sphere_visible(const Cam *c, float x, float y, float z, float r)
{
    float d[3] = { x - c->eye[0], y - c->eye[1], z - c->eye[2] };
    float vz = d[0] * c->fwd[0] + d[1] * c->fwd[1] + d[2] * c->fwd[2];
    float vx = d[0] * c->rgt[0] + d[1] * c->rgt[1] + d[2] * c->rgt[2];
    float vy = d[0] * c->upv[0] + d[1] * c->upv[1] + d[2] * c->upv[2];
    if (vz + r < NEAR || vz - r > FAR) return 0;
    if (vx > vz * c->tx + r * 1.5f || vx < -(vz * c->tx + r * 1.5f)) return 0;
    if (vy > vz * c->ty + r * 1.5f || vy < -(vz * c->ty + r * 1.5f)) return 0;
    return 1;
}

void render_scene(const CamPose *cp, int W, int H, const MapData *map, const ModelInst *inst, int ninst, TriFn fn, int *tri_count)
{
    Cam c; memset(&c, 0, sizeof c);
    c.tx = 0.78f; c.ty = c.tx * H / W; c.fx = W * 0.5f / c.tx; c.fy = H * 0.5f / c.ty; c.W = W; c.H = H; c.fn = fn;
    float cyw = cosf(cp->yaw), syw = sinf(cp->yaw), cpi = cosf(cp->pitch), spi = sinf(cp->pitch);
    c.fwd[0] = syw * cpi; c.fwd[1] = spi; c.fwd[2] = cyw * cpi;
    c.rgt[0] = cyw; c.rgt[1] = 0; c.rgt[2] = -syw;
    c.upv[0] = -syw * spi; c.upv[1] = cpi; c.upv[2] = -cyw * spi;
    c.eye[0] = cp->x; c.eye[1] = cp->y; c.eye[2] = cp->z;
    if (map)
        for (int i = 0; i < map->nchunks; i++) {
            const MapChunk *ch = &map->chunks[i];
            if (sphere_visible(&c, ch->cx, ch->cy, ch->cz, ch->r)) draw_range(&c, map, ch->first, ch->count, NULL);
        }
    for (int i = 0; i < ninst; i++) {
        const ModelInst *m = &inst[i];
        if (sphere_visible(&c, m->x, m->y + m->scale, m->z, m->scale * 1.5f)) draw_range(&c, m->model, 0, (unsigned)m->model->ntris, m);
    }
    if (tri_count) *tri_count = c.count;
}

void world_render(const Player3D *p, int crouch, int W, int H, const ModelInst *inst, int ninst, TriFn fn, int *tri_count)
{
    CamPose cp = { p->x, p->y + (crouch ? EYE_HEIGHT * 0.65f : EYE_HEIGHT), p->z, p->yaw, p->pitch };
    render_scene(&cp, W, H, M, inst, ninst, fn, tri_count);
}
