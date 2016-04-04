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
 #include <values.h>
#include <math.h>

#include "vector.h"
#include "utils.h"

vc vc_zero(void) {

    vc v;

    v.x = 0.0;
    v.y = 0.0;

    return v;
}

vc vc_set(double x, double y) {

    vc v;

    v.x = x;
    v.y = y;

    return v;
}

vc vc_add(vc v1, vc v2) {

    return vc_set(v1.x + v2.x, v1.y + v2.y);
}

vc vc_sub(vc v1, vc v2) {

    return vc_set(v1.x - v2.x, v1.y - v2.y);
}

vc vc_mul(vc v1, double v2) {

    return vc_set(v1.x * v2, v1.y * v2);
}

void vc_mul_acc(vc *v1, vc v2, double v3) {

    v1->x += v2.x * v3;
    v1->y += v2.y * v3;
}

/*
 * vc_filter()
 * vi - Input array
 * vo - Output array
 * l  - Array length
 * r  - Filter radius
 * z  - Number of poles
 * h  - 1 = hpf, 0 = lpf
 */

void vc_filter(vc *vi, vc *vo, int l, int r, int z, int h) {

    int rz = r * z;
    int cl = rz * 2 + 1;
    double *ck;

    int i, j;

	ck = (double *) malloc(cl * sizeof (double));

    for (i = 0; i < cl; i++)
        ck[i] = lanc((double) (i - rz) / r, z) / r;

    if (h) {

        // Spectral inversion

        for (i = 0; i < cl; i++)
            ck[i] = -ck[i];

        ck[rz] += 1.0;
    }

    for (i = 0; i < l; i++) {

        vc a = vc_zero();

        for (j = 0; j < cl; j++) {

            int p = clamp(i + j - rz, 0, l - 1);

            vc_mul_acc(&a, vi[p], ck[j]);
        }

        vo[i] = a;
    }
    
    free(ck);
}

vc vc_interp(vc *vi, int l, double x) {

    int *lk = select_lanc_kernel(x);
    vc a = vc_zero();

    int i;

    for (i = -3; i < 5; i++) {

        int ic = clamp(floor(x) + i, 0, l - 1);

        vc_mul_acc(&a, vi[ic], lk[i]);
    }

    return vc_mul(a, 1.0 / (1 << 16));
}
