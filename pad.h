#ifndef PAD_H
#define PAD_H
#include <tamtypes.h>
typedef struct { u32 now, old, pressed; int lx, ly, rx, ry; int connected; } Pad;
void pad_init(void);
void pad_poll(int port, Pad *p);   /* port 0 = mando 1, port 1 = mando 2 */
#endif
