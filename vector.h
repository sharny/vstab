#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    double x, y;
} vc;

vc vc_zero(void);
vc vc_set(double, double);

vc vc_add(vc, vc);
vc vc_sub(vc, vc);

vc vc_mul(vc, double);
void vc_mul_acc(vc *, vc, double);

void vc_filter(vc *, vc *, int, int, int, int);
vc vc_interp(vc *, int, double);

#endif
