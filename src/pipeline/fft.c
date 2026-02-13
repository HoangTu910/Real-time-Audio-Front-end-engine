/**
 * @file fft.c
 * @brief In-place iterative radix-2 DIT FFT — zero-alloc, fixed-point Q1.31.
 * @note Implemented by AI, need to be reviewed.
 *
 * Algorithm:
 *   1. Bit-reversal permutation of input arrays
 *   2. log2(N) butterfly stages with block scaling (right-shift by 1 per stage)
 *      to prevent overflow in Q1.31 accumulators
 *   3. Twiddle indices stride through the precomputed cos/sin tables
 *
 * Block scaling: each stage divides by 2, so after log2(N) stages the output
 * is scaled down by N. The caller must account for this (return value = number
 * of shifts applied).
 */

#include "fft.h"

/* ── Helpers ────────────────────────────────────────────────────────────── */

/** Count trailing zeros — gives log2(n) for power-of-2 n. */
static inline int log2_int(size_t n)
{
    int k = 0;
    while ((n >>= 1) != 0) k++;
    return k;
}

/** Bit-reverse an index of width 'bits'. */
static inline size_t bit_reverse(size_t x, int bits)
{
    size_t r = 0;
    for (int i = 0; i < bits; i++) {
        r = (r << 1) | (x & 1);
        x >>= 1;
    }
    return r;
}

/* ── FFT ────────────────────────────────────────────────────────────────── */

int fft_radix2_q31(q31_t *re, q31_t *im, size_t n,
                   const q31_t *tw_cos, const q31_t *tw_sin)
{
    const int stages = log2_int(n);
    int total_shifts = 0;

    /* ── 1. Bit-reversal permutation ────────────────────────────────────── */
    for (size_t i = 0; i < n; i++) {
        size_t j = bit_reverse(i, stages);
        if (j > i) {
            /* Swap re[i] <-> re[j] */
            q31_t tmp_re = re[i]; re[i] = re[j]; re[j] = tmp_re;
            q31_t tmp_im = im[i]; im[i] = im[j]; im[j] = tmp_im;
        }
    }

    /* ── 2. Butterfly stages ────────────────────────────────────────────── */
    for (int s = 0; s < stages; s++) {
        size_t half_size = (size_t)1 << s;          /* butterflies per group */
        size_t group_size = half_size << 1;          /* distance between groups */
        size_t tw_stride = n / group_size;           /* step through twiddle table */

        /* Block scaling: shift everything right by 1 to keep headroom */
        for (size_t i = 0; i < n; i++) {
            re[i] >>= 1;
            im[i] >>= 1;
        }
        total_shifts++;

        for (size_t k = 0; k < n; k += group_size) {
            for (size_t j = 0; j < half_size; j++) {
                size_t tw_idx = j * tw_stride;       /* index into twiddle LUT */

                size_t top = k + j;
                size_t bot = top + half_size;

                /*
                 * Butterfly:
                 *   T = W * X[bot]   where W = cos - j*sin  (DIT convention)
                 *   X[top] = X[top] + T
                 *   X[bot] = X[top] - T
                 *
                 * Complex multiply (Q1.31 × Q1.31 → keep Q1.31):
                 *   T_re = cos*re[bot] + sin*im[bot]
                 *   T_im = cos*im[bot] - sin*re[bot]
                 */
                q31_t wr = tw_cos[tw_idx];
                q31_t wi = tw_sin[tw_idx];

                q31_t br = re[bot];
                q31_t bi = im[bot];

                q31_t t_re = (q31_t)(((q63_t)wr * br + (q63_t)wi * bi) >> 31);
                q31_t t_im = (q31_t)(((q63_t)wr * bi - (q63_t)wi * br) >> 31);

                re[bot] = re[top] - t_re;
                im[bot] = im[top] - t_im;
                re[top] = re[top] + t_re;
                im[top] = im[top] + t_im;
            }
        }
    }

    return total_shifts;
}
