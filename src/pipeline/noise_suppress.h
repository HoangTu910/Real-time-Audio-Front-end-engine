/**
 * @file noise_suppress.h
 * @brief Spectral noise suppression with speech-aware adaptive noise estimation.
 *
 * AUDIO APPLICATION: Speech enhancement via spectral subtraction with adaptive
 * noise floor estimation. Designed for real-time embedded audio DSP systems.
 *
 * ALGORITHM BASIS:
 * - Minimum-tracking noise estimation (Martin 1994, Sohn et al. 1999)
 * - Speech-aware smoothing to avoid tracking speech transients
 * - Energy-based voice activity detection (VAD-like)
 * - Fixed-point arithmetic for ARM Cortex-M, RISC-V, and DSP cores
 *
 * REFERENCES:
 * [1] Martin, R. (1994). "Noise power spectral density estimation based on
 *     optimal smoothing and minimum statistics." IEEE Trans. Speech Audio Process.
 * [2] Sohn, J., Kim, N. S., & Legall, W. (1999). "A statistical model-based voice
 *     activity detection." IEEE Trans. Speech Audio Process., 7(4), 467-474.
 * [3] Dabov, K., Foi, A., & Katkovnik, V. (2011). "Audio denoising by time-
 *     frequency block thresholding." In Audio Signal Processing for Next-Generation
 *     Multimedia Communication (pp. 297-326).
 *
 * SUITABLE FOR:
 * - Real-time audio processing on embedded devices (< 50ms latency)
 * - Microphone array beamforming post-processing
 * - Voice communication (VoIP, telephony) noise reduction
 * - Audio recording cleanup
 */
#pragma once

#include "rtafe/fe_types.h"

/**
 * Noise suppression state for speech-aware adaptive estimation.
 * Maintains minimal state for embedded devices.
 */
typedef struct {
    q31_t *power_min;         /**< Minimum power estimate per bin */
    uint16_t min_track_count; /**< Frame counter for minimum tracking */
    q31_t total_power;        /**< Total frame power for VAD-like decision */
} noise_suppress_state_t;

/**
 * Initialize noise suppression state.
 * @param state       State structure to initialize
 * @param n_bins      Number of frequency bins
 * @return FE_OK on success, FE_ERR_NULL_PTR if state is NULL
 */
fe_status_t noise_suppress_init(noise_suppress_state_t *state, size_t n_bins);

/**
 * Update noise estimate and compute spectral suppression gain.
 * Uses speech-aware adaptive tracking for better embedded performance.
 *
 * @param state       Noise suppression state (maintains tracking statistics)
 * @param fft_re      Real FFT values
 * @param fft_im      Imaginary FFT values
 * @param noise_est   Running noise estimate (n_bins, updated in-place)
 * @param gain_out    Output suppression gain per bin (Q6.9)
 * @param n_bins      Number of frequency bins
 * @param over_sub    Over-subtraction factor (Q6.9)
 * @param floor       Spectral floor minimum
 * @param min_track_len  Minimum tracking window length (frames) - suggest 15-25
 */
void noise_suppress_process(noise_suppress_state_t *state,
                            const q31_t *fft_re,
                            const q31_t *fft_im,
                            q31_t       *noise_est,
                            q15_t       *gain_out,
                            size_t       n_bins,
                            q15_t        over_sub,
                            q15_t        floor,
                            uint16_t     min_track_len);
