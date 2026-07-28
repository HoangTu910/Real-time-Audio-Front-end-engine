#ifndef FFT_H
#define FFT_H

#include <stdint.h>
#include <math.h>
#include "utils.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float real;
    float imag;
} complex_t;

typedef s16 q2_14;

typedef struct {
    q2_14 real;
    q2_14 imag;
} complex_fixed_t;

/* floating point */
void fft(complex_t *x, int N);
void ifft(complex_t *x, int N);

/* fixed point */
int fft_fixed(complex_fixed_t *x, int N);
void ifft_fixed(complex_fixed_t *x, int N);


/* fixed point but use block floating point */
int fft_fixed_bfp(complex_fixed_t *x, int N, int *scale_shift);
int ifft_fixed_bfp(complex_fixed_t *x, int N, int *scale_shift);

/* window */
void hamming_window(float *x, int N);
void hanning_window(float *x, int N);

#ifdef __cplusplus
}
#endif

#endif /* FFT_H */