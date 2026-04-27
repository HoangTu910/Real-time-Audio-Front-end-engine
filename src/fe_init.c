/**
 * @file fe_init.c
 * @brief Lifecycle helpers: state/scratch sizing, reset, free, version.
 *
 * fe_init() and fe_process_hop() live in fe_api.c.
 */

#include <stdio.h>
#include "fe_api.h"
#include "pipeline/noise_suppress.h"
#include "pipeline/dc_removal.h"
#include "pipeline/preemphasis.h"

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
    if (!cfg) return 0;

    /* State structure + all per-channel processing blocks */
    size_t state_sz = sizeof(fe_state_t);
    size_t dc_sz = cfg->num_channels * sizeof(DCRemoval);
    size_t pre_emp_sz = cfg->num_channels * sizeof(PreEmphasis);
    size_t ns_sz = 0;
    size_t ns_power_min_sz = 0;
    
    if (cfg->flags & FE_FLAG_NOISE_SUPPRESS) {
        size_t n_bins = cfg->frame_len / 2 + 1;
        ns_sz = cfg->num_channels * sizeof(noise_suppress_state_t);
        ns_power_min_sz = cfg->num_channels * n_bins * sizeof(q31_t);
    }
    
    return state_sz + dc_sz + pre_emp_sz + ns_sz + ns_power_min_sz;
}

size_t fe_scratch_bytes(const fe_config_t *cfg)
{
    if (!cfg) return 0;

    const size_t n = cfg->frame_len;
    const size_t n_bins = n / 2 + 1;

    /* Scratch layout (temporary buffers for FFT processing):
     *   frame_q15            [n]                      — Q1.15 frame buffer
     *   fft_re               [n]                      — Q1.31 FFT real
     *   fft_im               [n]                      — Q1.31 FFT imag (reused for gain)
     *   noise_est            [n_bins * channels]      — Q1.31 noise estimate per bin per channel (if enabled)
     */
    size_t frame_sz = n * sizeof(q15_t);
    size_t fft_sz = 2 * n * sizeof(q31_t);  /* fft_re + fft_im */
    size_t noise_est_sz = 0;
    
    if (cfg->flags & FE_FLAG_NOISE_SUPPRESS) {
        noise_est_sz = n_bins * cfg->num_channels * sizeof(q31_t);
    }

    return frame_sz + fft_sz + noise_est_sz;
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
    RTAFE_LOG("Initializing RTAFE with config: sample_rate=%u, frame_len=%u, hop_len=%u, num_channels=%u, flags=0x%02X\n",
              cfg->sample_rate, cfg->frame_len, cfg->hop_len, cfg->num_channels, cfg->flags);
    if (!cfg || !state) {
        RTAFE_LOG("Invalid argument: cfg=%p, state=%p\n", (void*)cfg, (void*)state);
        return FE_ERR_NULL_PTR;
    }

    state->frame_len = cfg->frame_len;
    state->num_channels = cfg->num_channels;
    state->flags = cfg->flags;
    state->scratch = (int32_t*)scratch;
    state->scratch_bytes = scratch_bytes;

    size_t n_bins = cfg->frame_len / 2 + 1;

    /* Carve state blocks from state memory (immediately after state structure) */
    uint8_t *state_ptr = ((uint8_t *)state) + sizeof(fe_state_t);

    /* DC removal blocks (one per channel) */
    state->dc_block = (DCRemoval *)state_ptr;
    state_ptr += cfg->num_channels * sizeof(DCRemoval);

    /* Pre-emphasis blocks (one per channel) */
    state->pre_emphasis_block = (PreEmphasis *)state_ptr;
    state_ptr += cfg->num_channels * sizeof(PreEmphasis);

    /* Noise suppression blocks and power_min arrays (one per channel, if enabled) */
    if (cfg->flags & FE_FLAG_NOISE_SUPPRESS) {
        state->noise_suppress_block = (NoiseSuppress *)state_ptr;
        state_ptr += cfg->num_channels * sizeof(noise_suppress_state_t);

        /* Power minimum tracking arrays (pre-allocated for each channel) */
        q31_t *power_min_base = (q31_t *)state_ptr;

        /* Set up power_min pointers for each channel and initialize */
        for (uint8_t ch = 0; ch < cfg->num_channels; ch++) {
            state->noise_suppress_block[ch].power_min = power_min_base + ch * n_bins;
            for (size_t i = 0; i < n_bins; i++) {
                state->noise_suppress_block[ch].power_min[i] = INT32_MAX;
            }
            state->noise_suppress_block[ch].min_track_count = 0;
            state->noise_suppress_block[ch].total_power = 0;
        }
    } else {
        state->noise_suppress_block = NULL;
    }

    /* Carve noise estimates from scratch buffer */
    if (cfg->flags & FE_FLAG_NOISE_SUPPRESS) {
        state->noise_est = (q31_t *)scratch;
        /* Initialize all noise estimates to a small value */
        for (size_t i = 0; i < n_bins * cfg->num_channels; i++) {
            state->noise_est[i] = (q31_t)((1 << 20) + (1 << 19));  /* ~0.0015 in Q1.31 */
        }
    } else {
        state->noise_est = NULL;
    }

    /* Initialize per-channel processing blocks */
    for (uint8_t ch = 0; ch < state->num_channels; ch++) {
        dc_removal_init(&state->dc_block[ch], cfg->dc_rm_alpha);
        pre_emphasis_init(&state->pre_emphasis_block[ch], cfg->pre_emphasis_alpha);
    }

    return FE_OK;
}