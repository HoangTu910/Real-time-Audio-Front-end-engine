/**
 * @file noise_suppress.h
 * @brief Spectral noise suppression (spectral subtraction / Wiener filter).
 */
#pragma once

#include "rtafe/fe_types.h"

/**
 * Update noise estimate and compute spectral suppression gain.
 *
 * @param power_spec  Input power spectrum (n_bins, Q-format TBD).
 * @param noise_est   Running noise estimate (n_bins, updated in-place).
 * @param gain_out    Output suppression gain per bin (Q6.9).
 * @param n_bins      Number of frequency bins (frame_len/2 + 1).
 * @param over_sub    Over-subtraction factor (Q6.9).
 * @param floor       Spectral floor (Q6.9).
 */
void noise_suppress_process(const q31_t *power_spec,
                            q31_t       *noise_est,
                            q15_t       *gain_out,
                            size_t       n_bins,
                            q15_t        over_sub,
                            q15_t        floor);
