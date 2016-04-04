#ifndef UTILS_H
#define UTILS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int clamp(int, int, int);

double lanc(double, double);

void prepare_lanc_kernels(void);
int *select_lanc_kernel(double);

void draw_point(unsigned char *, int, int, int, int, int, int, int, int);
void draw_line(unsigned char *, int, int, int, int, int, int, int, int, int, int);

#endif
