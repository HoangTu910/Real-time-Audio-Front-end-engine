#include "module/fft.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>

static float magf(complex_t v) {
    return sqrtf(v.real * v.real + v.imag * v.imag);
}

static float magf_fixed(complex_fixed_t v) {
    float r = Q2_14_TO_FLOAT(v.real);
    float i = Q2_14_TO_FLOAT(v.imag);
    return sqrtf(r * r + i * i);
}

static inline q2_14 q2_14_mul(q2_14 a, q2_14 b) {
    s32 prod = (s32)a * (s32)b;
    s32 shifted = prod >> Q2_14_SHIFT;
    return CLIP_S16(shifted);
}

static void apply_hann_window_float(complex_t *x, int N) {
    for (int n = 0; n < N; n++) {
        float w = 0.5f * (1.0f - cosf(2.0f * M_PI * (float)n / (float)(N - 1)));
        x[n].real *= w;
        x[n].imag *= w;
    }
}

static void apply_hann_window_fixed(complex_fixed_t *x, int N) {
    for (int n = 0; n < N; n++) {
        float wf = 0.5f * (1.0f - cosf(2.0f * M_PI * (float)n / (float)(N - 1)));
        q2_14 w = FLOAT_TO_Q2_14(wf);
        x[n].real = q2_14_mul(x[n].real, w);
        x[n].imag = q2_14_mul(x[n].imag, w);
    }
}

static int test_tone_float(int bin) {
    complex_t x[512];
    for (int n = 0; n < 512; n++) {
        x[n].real = cosf(2.0f * M_PI * (float)bin * n / 512.0f);
        x[n].imag = 0.0f;
    }

    apply_hann_window_float(x, 512);

    fft(x, 512);

    float peak = magf(x[bin]);
    float mirror = magf(x[512 - bin]);
    float max_other = 0.0f;
    for (int k = 1; k < 512; k++) {
        int in_main_lobe = (k >= bin - 2 && k <= bin + 2) ||
                           (k >= (512 - bin) - 2 && k <= (512 - bin) + 2);
        if (in_main_lobe) {
            continue;
        }
        float m = magf(x[k]);
        if (m > max_other) {
            max_other = m;
        }
    }

    printf("Float tone bin %d: peak=%.6f mirror=%.6f max_other=%.6f\n",
           bin, peak, mirror, max_other);

    if (peak < 0.1f || mirror < 0.1f) {
        printf("FAIL: peak bins too small\n");
        return 1;
    }
    if (max_other > peak * 0.25f) {
        printf("FAIL: leakage too high\n");
        return 1;
    }
    return 0;
}

static int test_tone_fixed(int bin) {
    complex_fixed_t x[512];
    for (int n = 0; n < 512; n++) {
        x[n].real = FLOAT_TO_Q2_14(cosf(2.0f * M_PI * (float)bin * n / 512.0f));
        x[n].imag = 0;
    }

    apply_hann_window_fixed(x, 512);

    if (fft_fixed(x, 512) != 0) {
        printf("FAIL: fft_fixed returned error\n");
        return 1;
    }

    float peak = magf_fixed(x[bin]);
    float mirror = magf_fixed(x[512 - bin]);
    float max_other = 0.0f;
    for (int k = 1; k < 512; k++) {
        int in_main_lobe = (k >= bin - 2 && k <= bin + 2) ||
                           (k >= (512 - bin) - 2 && k <= (512 - bin) + 2);
        if (in_main_lobe) {
            continue;
        }
        float m = magf_fixed(x[k]);
        if (m > max_other) {
            max_other = m;
        }
    }

    printf("Fixed tone bin %d: peak=%.6f mirror=%.6f max_other=%.6f\n",
           bin, peak, mirror, max_other);

    if (peak < 0.0005f || mirror < 0.0005f) {
        printf("FAIL: fixed-point peak bins too small\n");
        return 1;
    }
    if (max_other > peak * 0.5f) {
        printf("FAIL: fixed-point leakage too high\n");
        return 1;
    }
    return 0;
}

static int test_ifft_roundtrip_float(void) {
    complex_t x[512];
    for (int n = 0; n < 512; n++) {
        float v = 0.5f * sinf(2.0f * M_PI * 7.0f * n / 512.0f) +
                  0.25f * cosf(2.0f * M_PI * 21.0f * n / 512.0f);
        x[n].real = v;
        x[n].imag = 0.0f;
    }

    fft(x, 512);
    ifft(x, 512);

    float max_err = 0.0f;
    for (int n = 0; n < 512; n++) {
        float v = 0.5f * sinf(2.0f * M_PI * 7.0f * n / 512.0f) +
                  0.25f * cosf(2.0f * M_PI * 21.0f * n / 512.0f);
        float err = fabsf(x[n].real - v);
        if (err > max_err) {
            max_err = err;
        }
    }

    printf("Float iFFT round-trip max_err=%.8f\n", max_err);
    if (max_err > 2e-2f) {
        printf("FAIL: float iFFT round-trip error too high\n");
        return 1;
    }
    return 0;
}

int main(void) {
    int failures = 0;

    failures += test_tone_float(50);
    failures += test_tone_fixed(50);
    failures += test_ifft_roundtrip_float();

    if (failures == 0) {
        printf("PASS: FFT tests OK\n");
        return 0;
    }

    printf("FAIL: %d FFT test(s) failed\n", failures);
    return 1;
}