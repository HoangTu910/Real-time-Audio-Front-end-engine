 #include "module/fft.h"
 #include "utils.h"
 #include <math.h>

void hamming_window(float *x, int N) {
    for (int n = 0; n < N; n++) {
        sincos sc = fast_sine_cos(2.0f * M_PI * n / (N - 1));
        x[n] *= 0.54f - 0.46f * sc.cos;
    }
 }

void hanning_window(float *x, int N) {
    for (int n = 0; n < N; n++) {
        sincos sc = fast_sine_cos(2.0f * M_PI * n / (N - 1));
        x[n] *= 0.5f * (1.0f - sc.cos);
    }
 }

static inline int bit_reverse(int n, int bits) {
    int reversed = 0;
    for (int i = 0; i < bits; i++) {
        reversed <<= 1;
        reversed |= (n & 1);
        n >>= 1;
    }
    return reversed;
}

static inline q2_14 q2_14_mul(q2_14 a, q2_14 b) {
    s32 prod = (s32)a * (s32)b;
    s32 shifted = prod >> Q2_14_SHIFT;
    return CLIP_S16(shifted);
}

static inline s32 abs_s16_to_s32(s16 v) {
    s32 out = (s32)v;
    return (out < 0) ? -out : out;
}

static inline s32 max_abs_complex_fixed(const complex_fixed_t *x, int N) {
    s32 max_abs = 0;
    for (int i = 0; i < N; i++) {
        s32 ar = abs_s16_to_s32(x[i].real);
        s32 ai = abs_s16_to_s32(x[i].imag);
        if (ar > max_abs) max_abs = ar;
        if (ai > max_abs) max_abs = ai;
    }
    return max_abs;
}

void fft(complex_t *x, int N) {
    int log2n = 0;
    for (int n = N; n > 1; n >>= 1) {
        log2n++;
    }

    static complex_t twiddle[256];
    static int twiddle_init = 0;
    if (!twiddle_init) {
        for (int k = 0; k < 256; k++) {
            float angle = -2.0f * (float)M_PI * (float)k / 512.0f;
            sincos sc = fast_sine_cos(angle);
            twiddle[k].real = sc.cos;
            twiddle[k].imag = sc.sin;
        }
        twiddle_init = 1;
    }

    for (int i = 0; i < N; i++) {
        int j = bit_reverse(i, log2n);
        if (j > i) {
            complex_t temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }

    for (int s = 1; s <= log2n; s++) {
        int m = 1 << s;
        int step = 512 / m;

        for (int k = 0; k < N; k += m) {
            for (int j = 0; j < m / 2; j++) {
                complex_t w = twiddle[j * step];
                complex_t t;
                complex_t u = x[k + j];

                float xr = x[k + j + m / 2].real;
                float xi = x[k + j + m / 2].imag;

                t.real = w.real * xr - w.imag * xi;
                t.imag = w.real * xi + w.imag * xr;

                x[k + j].real = u.real + t.real;
                x[k + j].imag = u.imag + t.imag;
                x[k + j + m / 2].real = u.real - t.real;
                x[k + j + m / 2].imag = u.imag - t.imag;
            }
        }
    }
}

void ifft(complex_t *x, int N) {
    for (int i = 0; i < N; i++) {
        x[i].imag = -x[i].imag;
    }

    fft(x, N);

    for (int i = 0; i < N; i++) {
        x[i].real /= N;
        x[i].imag = -x[i].imag / N;
    }
}

int fft_fixed(complex_fixed_t *x, int N) {
    if (N != 512) {
        return -1;
    }

    int log2n = 0;
    for (int n = N; n > 1; n >>= 1) {
        log2n++;
    }

    static complex_fixed_t twiddle[256];
    static int twiddle_init = 0;
    if (!twiddle_init) {
        for (int k = 0; k < 256; k++) {
            float angle = -2.0f * (float)M_PI * (float)k / 512.0f;
            sincos sc = fast_sine_cos(angle);
            twiddle[k].real = FLOAT_TO_Q2_14(sc.cos);
            twiddle[k].imag = FLOAT_TO_Q2_14(sc.sin);
        }
        twiddle_init = 1;
    }

    for (int i = 0; i < N; i++) {
        int j = bit_reverse(i, log2n);
        if (j > i) {
            complex_fixed_t temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }

    for (int s = 1; s <= log2n; s++) {
        int m = 1 << s;
        int step = 512 / m;

        for (int k = 0; k < N; k += m) {
            for (int j = 0; j < m / 2; j++) {
                complex_fixed_t w = twiddle[j * step];
                complex_fixed_t t;
                complex_fixed_t u = x[k + j];

                q2_14 xr = x[k + j + m / 2].real;
                q2_14 xi = x[k + j + m / 2].imag;

                t.real = q2_14_mul(w.real, xr) - q2_14_mul(w.imag, xi);
                t.imag = q2_14_mul(w.real, xi) + q2_14_mul(w.imag, xr);

                x[k + j].real = (u.real + t.real) >> 1;
                x[k + j].imag = (u.imag + t.imag) >> 1;
                x[k + j + m / 2].real = (u.real - t.real) >> 1;
                x[k + j + m / 2].imag = (u.imag - t.imag) >> 1;
            }
        }
    }

    return 0;
}

void ifft_fixed(complex_fixed_t *x, int N) {
    for (int i = 0; i < N; i++) {
        x[i].imag = -x[i].imag;
    }

    fft_fixed(x, N);

    for (int i = 0; i < N; i++) {
        x[i].real = x[i].real >> 9;
        x[i].imag = (-x[i].imag) >> 9;
    }
}

int fft_fixed_bfp(complex_fixed_t *x, int N, int *scale_shift) {
    if (N != 512) {
        return -1;
    }

    int log2n = 0;
    for (int n = N; n > 1; n >>= 1) {
        log2n++;
    }

    static complex_fixed_t twiddle[256];
    static int twiddle_init = 0;
    if (!twiddle_init) {
        for (int k = 0; k < 256; k++) {
            float angle = -2.0f * (float)M_PI * (float)k / 512.0f;
            sincos sc = fast_sine_cos(angle);
            twiddle[k].real = FLOAT_TO_Q2_14(sc.cos);
            twiddle[k].imag = FLOAT_TO_Q2_14(sc.sin);
        }
        twiddle_init = 1;
    }

    for (int i = 0; i < N; i++) {
        int j = bit_reverse(i, log2n);
        if (j > i) {
            complex_fixed_t temp = x[i];
            x[i] = x[j];
            x[j] = temp;
        }
    }

    int total_shift = 0;
    for (int s = 1; s <= log2n; s++) {
        int m = 1 << s;
        int step = 512 / m;

        s32 max_abs = max_abs_complex_fixed(x, N);
        int stage_shift = (max_abs > 16383) ? 1 : 0;
        total_shift += stage_shift;

        for (int k = 0; k < N; k += m) {
            for (int j = 0; j < m / 2; j++) {
                complex_fixed_t w = twiddle[j * step];
                complex_fixed_t t;
                complex_fixed_t u = x[k + j];

                q2_14 xr = x[k + j + m / 2].real;
                q2_14 xi = x[k + j + m / 2].imag;

                t.real = q2_14_mul(w.real, xr) - q2_14_mul(w.imag, xi);
                t.imag = q2_14_mul(w.real, xi) + q2_14_mul(w.imag, xr);

                s32 sum_r = (s32)u.real + (s32)t.real;
                s32 sum_i = (s32)u.imag + (s32)t.imag;
                s32 diff_r = (s32)u.real - (s32)t.real;
                s32 diff_i = (s32)u.imag - (s32)t.imag;

                if (stage_shift) {
                    sum_r >>= 1;
                    sum_i >>= 1;
                    diff_r >>= 1;
                    diff_i >>= 1;
                }

                x[k + j].real = CLIP_S16(sum_r);
                x[k + j].imag = CLIP_S16(sum_i);
                x[k + j + m / 2].real = CLIP_S16(diff_r);
                x[k + j + m / 2].imag = CLIP_S16(diff_i);
            }
        }
    }

    if (scale_shift) {
        *scale_shift = total_shift;
    }
    return 0;
}

int ifft_fixed_bfp(complex_fixed_t *x, int N, int *scale_shift) {
    for (int i = 0; i < N; i++) {
        x[i].imag = -x[i].imag;
    }

    int fft_shift = 0;
    int rc = fft_fixed_bfp(x, N, &fft_shift);
    if (rc != 0) {
        return rc;
    }

    for (int i = 0; i < N; i++) {
        x[i].real = x[i].real >> 9;
        x[i].imag = (-x[i].imag) >> 9;
    }

    if (scale_shift) {
        *scale_shift = fft_shift + 9;
    }
    return 0;
}