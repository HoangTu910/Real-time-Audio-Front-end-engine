/**
 * @file fe_init.c
 * @brief Lifecycle helpers: state/scratch sizing, reset, free, version.
 *
 * fe_init() and fe_process_hop() live in fe_api.c.
 */

#include "fe_api.h"
#include "pipeline/noise_suppress.h"

/* ── Version ────────────────────────────────────────────────────────────── */

uint32_t fe_version(void)
{
    return ((uint32_t)FE_VERSION_MAJOR << 16) |
           ((uint32_t)FE_VERSION_MINOR << 8)  |
           ((uint32_t)FE_VERSION_PATCH);
}

/* ── Query helpers ──────────────────────────────────────────────────────── */

size_t fe_state_bytes(const fe_config_t *cfg)
{
    (void)cfg;
    return sizeof(fe_state_t);
}

size_t fe_scratch_bytes(const fe_config_t *cfg)
{
    if (!cfg) return 0;

    const size_t n = cfg->frame_len;
    const size_t n_bins = n / 2 + 1;

    /* Scratch layout:
     *   frame_q15    [n]           — Q1.15 interleaved input frame
     *   fft_re       [n]           — Q1.31 FFT real
     *   fft_im       [n]           — Q1.31 FFT imag (reused for gain_out)
     *   noise_est    [n_bins * ch] — Q1.31 noise estimate per channel
     */
    size_t frame_q15_sz = n * sizeof(q15_t);
    size_t fft_sz = 2 * n * sizeof(q31_t);          /* fft_re + fft_im */
    size_t noise_est_sz = 0;
    if (cfg->flags & FE_FLAG_NOISE_SUPPRESS) {
        noise_est_sz = n_bins * cfg->num_channels * sizeof(q31_t);
    }

    return frame_q15_sz + fft_sz + noise_est_sz;
}

/* ── Reset ──────────────────────────────────────────────────────────────── */

fe_status_t fe_reset(fe_state_t *state)
{
    if (!state) return FE_ERR_NULL_PTR;
    /* TODO: zero internal buffers, re-init pipeline blocks */
    return FE_OK;
}

/* ── Free ───────────────────────────────────────────────────────────────── */

fe_status_t fe_free(fe_state_t *state)
{
    if (!state) return FE_ERR_NULL_PTR;
    /* No dynamic allocation — nothing to release on bare-metal */
    return FE_OK;
}

fe_status_t fe_init(const fe_config_t *cfg, fe_state_t *state, void *scratch, size_t scratch_bytes)
{
    if (!cfg || !state) return FE_ERR_NULL_PTR;

    state->frame_len = cfg->frame_len;
    state->num_channels = cfg->num_channels;
    state->flags = cfg->flags;
    state->scratch = (int32_t*)scratch;
    state->scratch_bytes = scratch_bytes;

    size_t n_bins = cfg->frame_len / 2 + 1;

    /* Carve scratch for noise estimates if noise suppression is enabled */
    if (cfg->flags & FE_FLAG_NOISE_SUPPRESS) {
        q15_t *frame_q15 = (q15_t *)scratch;
        q31_t *fft_re = (q31_t *)(frame_q15 + cfg->frame_len);
        state->noise_est = (q31_t *)(fft_re + 2 * cfg->frame_len);

        /* Initialize all noise estimates to a small value */
        for (size_t i = 0; i < n_bins * cfg->num_channels; i++) {
            state->noise_est[i] = (q31_t)((1 << 20) + (1 << 19));  /* ~0.0015 in Q1.31 */
        }
    }

    /* Initialize per-channel processing blocks */
    for (uint8_t ch = 0; ch < state->num_channels; ch++) {
        dc_removal_init(&state->dc_block[ch], cfg->dc_rm_alpha);
        pre_emphasis_init(&state->pre_emphasis_block[ch], cfg->pre_emphasis_alpha);

        /* Initialize noise suppression if enabled */
        if (cfg->flags & FE_FLAG_NOISE_SUPPRESS) {
            noise_suppress_init(&state->noise_suppress_block[ch], n_bins);
        }
    }

    return FE_OK;
}