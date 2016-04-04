#ifndef MOTION_H
#define MOTION_H

#include <stdint.h>

#include "average.h"
#include "vector.h"

typedef struct {

    int16_t *sf[4], *df[4];
    int nc, nr;

    av_ctx *av_l, *av_h;

    int *dx, *dy;
    int dn;

} mt_ctx;

mt_ctx *mt_init(int, int);
void mt_push(mt_ctx *, uint8_t *);
vc mt_motion(mt_ctx *, uint8_t *);
void mt_free(mt_ctx *);

#endif
