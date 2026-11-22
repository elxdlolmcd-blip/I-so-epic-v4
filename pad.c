#include <stdlib.h>
#include <string.h>
#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include "pad.h"
#include "controls.h"

static char padBuf[2][256] __attribute__((aligned(64)));
static int mode_set[2];

static int usable(int st) { return st == PAD_STATE_STABLE || st == PAD_STATE_FINDCTP1; }

void pad_init(void)
{
    SifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, NULL);
    SifLoadModule("rom0:PADMAN", 0, NULL);
    padInit(0);
    for (int port = 0; port < 2; port++) padPortOpen(port, 0, padBuf[port]);
    for (int i = 0; i < 200000; i++) if (usable(padGetState(0, 0))) break; /* espera acotada al mando 1 */
}

static int dz(int v) { return abs(v) < STICK_DEADZONE ? 0 : v; }

void pad_poll(int port, Pad *p)
{
    int st = padGetState(port, 0);
    p->old = p->now;
    if (!usable(st)) {
        p->connected = 0; p->now = 0; p->pressed = 0;
        p->lx = p->ly = p->rx = p->ry = 0; mode_set[port] = 0;
        return;
    }
    p->connected = 1;
    if (!mode_set[port]) { padSetMainMode(port, 0, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK); mode_set[port] = 1; }
    struct padButtonStatus pb;
    if (padRead(port, 0, &pb)) {
        p->now = 0xffff ^ pb.btns;
        int t = pb.mode >> 4;
        if (t == 7 || t == 5) {
            p->lx = dz((int)pb.ljoy_h - 128); p->ly = dz((int)pb.ljoy_v - 128);
            p->rx = dz((int)pb.rjoy_h - 128); p->ry = dz((int)pb.rjoy_v - 128);
        } else p->lx = p->ly = p->rx = p->ry = 0;
    }
    p->pressed = p->now & ~p->old;
}
