#include "fe_api.h"
#include "pipeline/window.h"
#include "pipeline/fft.h"
#include "pipeline/noise_suppress.h"
#include "math/tables.h"

/* fe_api.c — Top-level pipeline orchestration */

/* Helper to compute magnitude spectrum from complex FFT output */
static inline q31_t magnitude_q31(q31_t re, q31_t im)
{
    /* Simplified: |X| ≈ max(|re|, |im|) + 0.5*min(|re|, |im|)
       More accurate approximations available but this is efficient */
    q31_t a = re < 0 ? -re : re;
    q31_t b = im < 0 ? -im : im;
    q31_t max_val = a > b ? a : b;
    q31_t min_val = a > b ? b : a;
    return max_val + (min_val >> 1);
}

fe_status_t fe_process_hop(fe_state_t *state, const q15_t *pcm_in, q15_t *pcm_out, void *feature_out, size_t feature_sz)
{
    uint16_t frame_len = state->frame_len;
    uint8_t num_channels = state->num_channels;

    /*
     * Scratch layout (all carved from state->scratch):
     *   frame_q15 [frame_len]           — Q1.15 frame buffer per channel
     *   fft_re    [frame_len]           — Q1.31 real part for FFT
     *   fft_im    [frame_len]           — Q1.31 imaginary part for FFT
     *   gain_spec [frame_len/2 + 1]    — Q6.9 spectral gain (noise suppression)
     */
    q15_t *frame_q15 = (q15_t *)state->scratch;
    q31_t *fft_re    = (q31_t *)(frame_q15 + frame_len);
    q31_t *fft_im    = fft_re + frame_len;
    /* Note: gain_spec carved from remaining scratch or reuse fft_im after processing */
    
    size_t n_bins = frame_len / 2 + 1;

    for (uint8_t ch = 0; ch < num_channels; ch++) {
        DCRemoval           *dc  = &state->dc_block[ch];
        PreEmphasis         *pre = &state->pre_emphasis_block[ch];
        NoiseSuppress       *ns  = NULL;
        if (state->flags & FE_FLAG_NOISE_SUPPRESS) {
            RTAFE_LOG("Noise suppression enabled for channel %d\n", ch);
            ns = &state->noise_suppress_block[ch];
        }

        /* ── Stage 1 & 2: DC removal + pre-emphasis (per sample) ───────── */
        for (uint16_t n = 0; n < frame_len; n++) {
            uint16_t idx = (uint16_t)(n * num_channels + ch);
            q15_t sample = pcm_in[idx];

            /* 1. DC Removal (Q15 → Q31 → process → Q15) */
            q31_t sample_q31 = ((q31_t)sample) << 16;
            sample_q31 = dc_removal_process(dc, sample_q31);
            q15_t sample_q15 = (q15_t)((sample_q31 + (1 << 15)) >> 16);

            /* 2. Pre-emphasis */
            sample_q15 = pre_emphasis_process(pre, sample_q15);

            /* Store into contiguous frame buffer */
            frame_q15[n] = sample_q15;
        }

        /* ── Stage 3: Windowing (whole frame at once) ──────────────────── */
        window_apply(window_hann_256, frame_q15, frame_len);

        /* ── Stage 4: Promote Q1.15 → Q1.31 and run FFT ───────────────── */
        for (uint16_t n = 0; n < frame_len; n++) {
            fft_re[n] = ((q31_t)frame_q15[n]) << 16;   /* Q1.15 → Q1.31 */
            fft_im[n] = 0;                               /* real input     */
        }

        int fft_shifts = fft_radix2_q31(fft_re, fft_im, frame_len,
                                         twiddle_cos_256, twiddle_sin_256);
        (void)fft_shifts; /* TODO: pass to downstream stages for scaling */

        /* ── Stage 5: Spectral processing (Noise Suppression) ────────── */
        if (state->flags & FE_FLAG_NOISE_SUPPRESS) {
            /* Allocate gain buffer (reuse fft_im after FFT processing) */
            q15_t *gain_out = (q15_t *)fft_im; /* n_bins elements, Q6.9 */

            /* Call adaptive noise suppression with minimum tracking */
            noise_suppress_process(
                ns,
                fft_re, fft_im,
                &state->noise_est[ch * n_bins], /* noise estimate per channel */
                gain_out,
                n_bins,
                (state->flags & FE_FLAG_NOISE_SUPPRESS) ? /* use config defaults */
                ((q15_t)512) : 0,      /* over_subtract (1.0x = 512 in Q6.9) */
                ((q15_t)1),            /* floor (minimal threshold) */
                20                     /* min_track_len: 20 frames ≈ 200ms */
            );

            /* Apply spectral gain to each bin: X[k] *= Gain[k] */
            for (size_t k = 0; k < n_bins; k++) {
                q15_t gain = gain_out[k];  /* Q6.9 */

                /* Multiply complex bin by gain: keep magnitude, preserve phase */
                /* X_out[k] = X_in[k] * (gain / 512) where 512 ≡ unity in Q6.9 */
                fft_re[k] = (q31_t)((fft_re[k] * gain) >> 9);
                fft_im[k] = (q31_t)((fft_im[k] * gain) >> 9);
            }
            /* Happy new year - wish this year I can achieve more goals, gain more experience and get promotion with better salary!*/
        }

        /* ── Stage 6: iFFT + overlap-add (TODO) ──────────────────────── */
        /* - ifft_radix2_q31(fft_re, fft_im, ...)
         * - Demote Q1.31 → Q1.15
         * - Overlap-add into output buffer
         */

        /* ── Stage 7: AGC (TODO) ──────────────────────────────────────── */

        /* Temporary: copy windowed frame to output for testing */
        for (uint16_t n = 0; n < frame_len; n++) {
            uint16_t idx = (uint16_t)(n * num_channels + ch);
            pcm_out[idx] = frame_q15[n];
        }
    }

    /* ── Stage 8: Feature extraction (optional) ───────────────────────── */
    /* if (feature_out) fe_extract_features(state, feature_out, feature_sz); */

    (void)feature_out;
    (void)feature_sz;

    return FE_OK;
}