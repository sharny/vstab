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
// #include <values.h>
#include <limits.h>
#include "main.h"
#include "motion.h"
#include "utils.h"

mt_ctx *mt_init(int nc, int nr) {

    int i, x, y;

    if ((nc < 128) || (nr < 128) || (nc & 15) || (nr & 15))
        return NULL;

    mt_ctx *mt = (mt_ctx *) malloc(sizeof (mt_ctx));

    for (i = 0; i < 4; i++) {

        mt->sf[i] = (int16_t *) calloc((nc >> i) * (nr >> i), sizeof (int16_t));
        mt->df[i] = (int16_t *) calloc((nc >> i) * (nr >> i), sizeof (int16_t));
    }

    mt->nc = nc;
    mt->nr = nr;

    for (x = 64, mt->dn = 0; x < nc - 64; x += 16)
        for (y = 64; y < nr - 64; y += 16)
            mt->dn++;

    mt->dx = (int *) calloc(mt->dn, sizeof (int));
    mt->dy = (int *) calloc(mt->dn, sizeof (int));

    mt->av_l = av_init(1, AV_LOPASS);
    mt->av_h = av_init(12, AV_HIPASS);

    return mt;
}

static void grey(uint8_t *sf, int16_t *df, int nc, int nr) {

    int x, y;

    for (y = nr; y; y--)
        for (x = nc; x; x--, sf += 3, df++)
            *df = (sf[0] * 53 + sf[1] * 181 + sf[2] * 18) >> 8;
}

static void halve(int16_t *sf, int16_t *df, int nc, int nr) {

    int x, y;

    for (y = nr >> 1; y; y--, sf += nc)
        for (x = nc >> 1; x; x--, sf += 2, df++)
            *df = *sf;
}

void mt_push(mt_ctx *mt, uint8_t *f) {

    int i;

    for (i = 0; i < 4; i++) {

        int16_t *t = mt->sf[i];
        mt->sf[i] = mt->df[i];
        mt->df[i] = t;
    }

    grey(f, mt->df[0], mt->nc, mt->nr);

    // Make pyramid

    for (i = 0; i < 4; i++) {

        int nc = mt->nc >> i;
        int nr = mt->nr >> i;

        av_apply(mt->av_l, mt->df[i], nc, nr);

        if (i < 3)
            halve(mt->df[i], mt->df[i + 1], nc, nr);

        av_apply(mt->av_h, mt->df[i], nc, nr);
    }
}

static int sad(int16_t *sf, int16_t *df, int nc) {

    int d = 0;

    int x, y;

    for (y = 9; y; y--, sf += nc - 9, df += nc - 9)
        for (x = 9; x; x--, sf++, df++)
            d += abs(*sf - *df);

    return d;
}

static void search(mt_ctx *mt, int l, int sx, int sy, int *dx, int *dy) {

    int nc = mt->nc >> l;

    int16_t *sf = mt->sf[l] + ((sy >> l) - 4) * nc + ((sx >> l) - 4);
    int16_t *df = mt->df[l] + ((*dy >> l) - 8) * nc + ((*dx >> l) - 8);

    int bx = 0, by = 0;
    int be = INT_MAX;

    int x, y;

    for (y = -4; y <= 4; y++, df += nc - 9) {
        for (x = -4; x <= 4; x++, df++) {

            int ce = sad(sf, df, nc);

            if (ce < be) {

                bx = x;
                by = y;
                be = ce;
            }
        }
    }

    *dx += bx << l;
    *dy += by << l;
}

static int compare(const void *a, const void *b) {

    const int *da = (const int *) a;
    const int *db = (const int *) b;

    return *da > *db;
}

static double median(int *dv, int dn) {

    qsort(dv, dn, sizeof (int), compare);

	int rv = dv[dn / 2];
	double av = 0, an = 0;
	int i;
	
	for (i = 0; i < dn; i++) {
	
		if (abs(dv[i] - rv) < 2) {
		
			av += dv[i];
			an += 1.0;
		}
	}

    return av / an;
}

vc mt_motion(mt_ctx *mt, uint8_t *f) {

    int i, j, x, y;

    for (y = 64, i = 0; y < mt->nr - 64; y += 16) {
        for (x = 64; x < mt->nc - 64; x += 16, i++) {

            int dx = x, dy = y;

            for (j = 3; j >= 0; j--)
                search(mt, j, x, y, &dx, &dy);

            mt->dx[i] = dx - x;
            mt->dy[i] = dy - y;
        }
    }

    if (opt_debug_conv) {

        int i, x, y, z;

        for (i = 0; i < 4; i++)
            for (y = 0; y < (mt->nr >> i); y++)
                for (x = 0; x < (mt->nc >> i); x++)
                    for (z = 0; z < 3; z++)
                        f[(y * mt->nc + x) * 3 + z] =
                                clamp(mt->df[i][y * (mt->nc >> i) + x] + 128, 0, 255);

        for (y = 64, i = 0; y < mt->nr - 64; y += 16) {
            for (x = 64; x < mt->nc - 64; x += 16, i++) {

                draw_line(f, mt->nc, mt->nr,
                          x, y, x + mt->dx[i], y + mt->dy[i],
                          0, 255, 255, 128);

                draw_point(f, mt->nc, mt->nr,
                           x + mt->dx[i], y + mt->dy[i],
                           1, 128, 255, 128);
            }
        }
    }

    return vc_set(
            median(mt->dx, mt->dn),
            median(mt->dy, mt->dn)
            );
}

void mt_free(mt_ctx * mt) {

    int i;

    for (i = 0; i < 4; i++) {

        free(mt->sf[i]);
        free(mt->df[i]);
    }

    free(mt->dx);
    free(mt->dy);

    av_free(mt->av_l);
    av_free(mt->av_h);

    free(mt);
}
