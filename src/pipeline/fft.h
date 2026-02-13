/* fft.h — Fixed-point radix-2 DIT FFT with block scaling */

#pragma once
#include <stdint.h>
#include "rtafe/fe_types.h"

/**
 * In-place radix-2 decimation-in-time FFT (fixed-point Q1.31).
 *
 * @param re        Real part array, length @p n. Modified in-place.
 * @param im        Imaginary part array, length @p n. Modified in-place.
 *                  (zero-fill for real-only input)
 * @param n         FFT length (must be power of 2, e.g. 256).
 * @param tw_cos    Precomputed cosine twiddles (Q1.31), length n/2.
 * @param tw_sin    Precomputed sine twiddles (Q1.31), length n/2.
 * @return          Number of block-scaling right-shifts applied (for
 *                  headroom tracking in downstream stages).
 *
 * No dynamic allocation. Operates entirely in the provided arrays.
 */
int fft_radix2_q31(q31_t *re, q31_t *im, size_t n,
                   const q31_t *tw_cos, const q31_t *tw_sin);
