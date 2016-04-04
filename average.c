/*
 * Video stabilizer
 *
 * Copyright (c) 2008 Lenny <leonardo.masoni@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "average.h"

av_ctx *av_init(int rd, int lh) {

    av_ctx *av = (av_ctx *) malloc(sizeof (av_ctx));

    av->rd = rd;
    av->dm = rd * 2 + 1;
	av->dv = (1 << 16) / av->dm;

	av->lh = lh;

    av->bs = (int *) calloc(av->dm, sizeof (int));
    av->sm = 0;

    av->be = av->bs + av->dm;
    av->bp = av->bs;

    return av;
}

static void reset(av_ctx *av) {

    memset(av->bs, 0, av->dm * sizeof (int));
    av->sm = 0;
}

static int16_t step(av_ctx *av, int16_t v) {

    av->sm -= *av->bp;
    av->sm += v;

    *av->bp = v;
    av->bp++;

    if (av->bp == av->be)
        av->bp = av->bs;

    return (av->sm * av->dv) >> 16;
}

static void av_apply_h(av_ctx *av, int16_t *sf, int nc, int nr) {

    int x, y;

    for (y = nr; y; y--, sf += nc) {

        int16_t *p1 = sf;
        int16_t *p2 = sf;

        reset(av);

        for (x = av->rd; x; x--, p2++)
            step(av, *p2);

        for (x = nc - av->rd; x; x--, p1++, p2++)
            *p1 = av->lh * *p1 + step(av, *p2);

        for (x = av->rd; x; x--, p1++)
            *p1 = av->lh * *p1 + step(av, 0);
    }
}

static void av_apply_v(av_ctx *av, int16_t *sf, int nc, int nr) {

    int x, y;

    for (x = nc; x; x--, sf++) {

        int16_t *p1 = sf;
        int16_t *p2 = sf;

        reset(av);

        for (y = av->rd; y; y--, p2 += nc)
            step(av, *p2);

        for (y = nr - av->rd; y; y--, p1 += nc, p2 += nc)
            *p1 = av->lh * *p1 + step(av, *p2);

        for (y = av->rd; y; y--, p1 += nc)
            *p1 = av->lh * *p1 + step(av, 0);
    }
}

void av_apply(av_ctx *av, int16_t *sf, int nc, int nr) {

        av_apply_h(av, sf, nc, nr);
        av_apply_v(av, sf, nc, nr);
}

void av_free(av_ctx *av) {

    free(av->bs);
    free(av);
}
