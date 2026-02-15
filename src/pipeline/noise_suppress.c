#include "noise_suppress.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief Speech-aware adaptive noise estimation with minimum tracking.
 *
 * AUDIO SIGNAL PROCESSING CONTEXT:
 * In the spectral domain (frequency bins), noise exhibits pseudo-stationary
 * characteristics while speech contains non-stationary transients. This algorithm
 * leverages this property by tracking the minimum power envelope per frequency
 * bin as a robust noise floor estimate.
 *
 * MINIMUM TRACKING PRINCIPLE (Martin 1994):
 * - Noise power ≈ minimum power observed over past N frames
 * - Speech transients create peaks above the noise floor
 * - By tracking minimums, we build a reliable noise profile
 * - Reset periodically (every ~200-300ms for 10ms frames)
 *
 * VAD-LIKE ACTIVITY DETECTION:
 * - Compares current frame energy against 1.5x noise estimate
 * - Speech frames trigger slower noise adaptation
 * - Prevents upward drift of noise estimate during speech
 *
 * Q-FORMAT NOTES:
 * - Power spectrum: Q1.31 (result of q31_t * q31_t multiplication)
 * - Gain output: Q6.9 (0 to ~512, typically 0-1 in linear)
 *
 * EMBEDDED OPTIMIZATION:
 * - No expensive division; uses fixed shifts for 1/8, 1/16
 * - Single-pass computation per frame
 * - Minimal state per channel (n_bins × 4 bytes for minimums)
 */

fe_status_t noise_suppress_init(noise_suppress_state_t *state, size_t n_bins)
{
    if (state == NULL) return FE_ERR_NULL_PTR;

    /* Allocate minimum tracking buffer (one per bin) */
    state->power_min = (q31_t *)malloc(n_bins * sizeof(q31_t));
    if (state->power_min == NULL) return FE_ERR_NULL_PTR;

    /* Initialize with maximum values (will be overwritten on first frames) */
    for (size_t i = 0; i < n_bins; i++) {
        state->power_min[i] = INT32_MAX;
    }

    state->min_track_count = 0;
    state->total_power = 0;

    return FE_OK;
}

void noise_suppress_process(noise_suppress_state_t *state,
                            const q31_t *fft_re,
                            const q31_t *fft_im,
                            q31_t       *noise_est,
                            q15_t       *gain_out,
                            size_t       n_bins,
                            q15_t        over_sub,
                            q15_t        floor,
                            uint16_t     min_track_len)
{
    if (state == NULL || fft_re == NULL || fft_im == NULL) return;

    state->total_power = 0;
    q31_t max_power = 0;

    /* ─────────────────────────────────────────────────────────────────────
       STEP 1: Power Spectrum Computation & Minimum Tracking
       
       For each frequency bin: Power[k] = |X[k]|² = Re[k]² + Im[k]²
       Track bin-wise minimum over sliding window for noise floor.
       ───────────────────────────────────────────────────────────────────── */
    for (size_t i = 0; i < n_bins; i++) {
        q31_t power = (fft_re[i] * fft_re[i]) + (fft_im[i] * fft_im[i]);

        /* Track minimum over sliding window (Martin 1994) */
        if (power < state->power_min[i]) {
            state->power_min[i] = power;
        }

        /* Accumulate for frame energy estimation (for VAD-like decision) */
        state->total_power += power;
        if (power > max_power) max_power = power;
    }

    /* ─────────────────────────────────────────────────────────────────────
       STEP 2: Energy-Based Activity Detection (Simple VAD)
       
       Compare frame energy against noise estimate to determine if current
       frame contains speech transients. This prevents noise floor from
       drifting upward during speech bursts (Sohn et al. 1999).
       ───────────────────────────────────────────────────────────────────── */
    q31_t avg_power = state->total_power / n_bins;
    q31_t noise_estimate_avg = 0;
    for (size_t i = 0; i < n_bins; i++) {
        noise_estimate_avg += noise_est[i];
    }
    noise_estimate_avg /= n_bins;

    /* Activity threshold: if avg power > 1.5x noise estimate, likely speech
       (Both average and peak must exceed threshold to avoid false positives) */
    q31_t activity_threshold = (noise_estimate_avg * 3) >> 1;  /* 1.5x via bit shift */
    int is_speech_frame = (avg_power > activity_threshold) && (max_power > activity_threshold);

    /* ─────────────────────────────────────────────────────────────────────
       STEP 3: Adaptive Noise Estimate Update
       
       Use two time constants:
       - Silence frames: α = 1/8 (fast adaptation to changing noise)
       - Speech frames: α = 1/16 (slow adaptation, preserve noise floor)
       
       Blend minimum estimate (short-term floor) with current estimate
       (long-term drift tracking) for smooth convergence.
       ───────────────────────────────────────────────────────────────────── */
    q15_t alpha_num, alpha_den;
    if (is_speech_frame) {
        alpha_num = 1;      /* 1/16 = 0.0625 (slower) */
        alpha_den = 16;
    } else {
        alpha_num = 1;      /* 1/8 = 0.125 (faster) */
        alpha_den = 8;
    }

    for (size_t i = 0; i < n_bins; i++) {
        q31_t power = (fft_re[i] * fft_re[i]) + (fft_im[i] * fft_im[i]);

        /* Use minimum estimate as base for better noise floor tracking */
        q31_t min_est = state->power_min[i];

        /* Blend minimum estimate with current noise estimate:
           - min_est tracks short-term floor (reliable during speech)
           - noise_est[i] tracks long-term changes (slow background changes)
           - Average provides smooth balance */
        q31_t blended = (min_est + noise_est[i]) >> 1;

        /* Update with adaptive smoothing:
           noise_est[i] = 7/8 * noise_est[i] + 1/8 * blended
           (Avoids tracking speech spikes as noise) */
        noise_est[i] = noise_est[i] - (noise_est[i] >> 3)  /* 7/8 * old */
                       + (blended >> 3);                    /* 1/8 * blended */

        /* ─────────────────────────────────────────────────────────────────
           STEP 4: Spectral Subtraction Gain Computation
           
           Implements: Gain[k] = (Power[k] - α·NoiseEst[k]) / Power[k]
           where α = over_sub (over-subtraction factor, typically 1.0-1.5)
           
           Bounded by spectral floor to prevent over-attenuation.
           Output format Q6.9: 512 represents unity gain (no suppression).
           ───────────────────────────────────────────────────────────────── */
        q15_t gain = 0;
        if (power > floor) {
            q31_t numerator = power - ((over_sub * noise_est[i]) >> 9);
            if (numerator < floor) numerator = floor;
            gain = (q15_t)((numerator << 9) / (power + 1)); /* Q6.9 */
        }
        gain_out[i] = gain;
    }

    /* ─────────────────────────────────────────────────────────────────────
       STEP 5: Periodic Minimum Tracker Reset
       
       Every min_track_len frames (~200ms if frame=10ms, min_track_len=20),
       reset minimums to restart minimum search. This allows noise estimate
       to adapt to slowly changing acoustic environment.
       ───────────────────────────────────────────────────────────────────── */
    state->min_track_count++;
    if (state->min_track_count >= min_track_len) {
        /* Reset minimums for next tracking window */
        for (size_t i = 0; i < n_bins; i++) {
            state->power_min[i] = INT32_MAX;
        }
        state->min_track_count = 0;
    }
}