/* ifft.h — Inverse FFT and overlap-add reconstruction */

#pragma once
#include <stdint.h>
#include "rtafe/fe_types.h"
void ifft_radix2_q31(const q31_t *input, q31_t *output, size_t n);
void overlap_add(q15_t *output, const q15_t *frame, size_t frame_len, size_t hop_len);