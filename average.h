#ifndef AVERAGE_H
#define AVERAGE_H

#include <stdint.h>

#define AV_LOPASS (0)
#define AV_HIPASS (-1)

typedef struct {
    int *bs, *be, *bp, sm, dv;
    int rd, dm, lh;
} av_ctx;

av_ctx *av_init(int, int);
void av_apply(av_ctx *, int16_t *, int, int);
void av_free(av_ctx *);

#endif
