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
#include <math.h>

#include "utils.h"

static int lanc_kernels[256][8];

int clamp(int x, int l, int h) {

    if (x < l)
        return l;

    if (x > h)
        return h;

    return x;
}

double lanc(double x, double r) {

    double t = x * M_PI;

    if (x == 0.0)
        return 1.0;

    if (x <= -r || x >= r)
        return 0.0;

    return r * sin(t) * sin(t / r) / (t * t);
}

void prepare_lanc_kernels(void) {

    int i, j;

    for (i = 0; i < 256; i++)
        for (j = -3; j < 5; j++)
            lanc_kernels[i][j + 3] = lanc(j - i / 256.0, 4.0) * (1 << 16);
}

int *select_lanc_kernel(double x) {

    return lanc_kernels[(int) ((x - floor(x)) * 256.0)] + 3;
}

void draw_point(unsigned char *fr, int nc, int nr, int vx, int vy, int d, int r, int g, int b) {

    int x, y;

    for (y = vy - d; y <= vy + d; y++) {

        if (y >= 0 && y < nr) {

            int yp = y * nc;

            for (x = vx - d; x <= vx + d; x++) {

                if (x >= 0 && x < nc) {

                    int p = (yp + x) * 3;

                    fr[p + 0] = r;
                    fr[p + 1] = g;
                    fr[p + 2] = b;
                }
            }
        }
    }
}

void draw_line(unsigned char *fr, int nc, int nr, int v1x, int v1y, int v2x, int v2y, int d, int r, int g, int b) {

    int t, deltax, deltay, error, ystep, x, y;

    int steep = abs(v2y - v1y) > abs(v2x - v1x);

    if (steep) {

        t = v1x;
        v1x = v1y;
        v1y = t;

        t = v2x;
        v2x = v2y;
        v2y = t;
    }

    if (v1x > v2x) {

        t = v1x;
        v1x = v2x;
        v2x = t;

        t = v1y;
        v1y = v2y;
        v2y = t;
    }

    deltax = v2x - v1x;
    deltay = abs(v2y - v1y);
    error = deltax / 2;

    y = v1y;

    if (v1y < v2y)
        ystep = 1;
    else
        ystep = -1;

    for (x = v1x; x < v2x; x++) {

        if (steep)
            draw_point(fr, nc, nr, y, x, d, r, g, b);
        else
            draw_point(fr, nc, nr, x, y, d, r, g, b);

        error = error - deltay;

        if (error < 0) {

            y = y + ystep;
            error = error + deltax;
        }
    }
}
