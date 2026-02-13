/**
 * @file fe_init.c
 * @brief Lifecycle helpers: state/scratch sizing, reset, free, version.
 *
 * fe_init() and fe_process_hop() live in fe_api.c.
 */

#include "fe_api.h"

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

    /* TODO: refine as more pipeline stages are added */
    const size_t n = cfg->frame_len;
    return 2 * n * sizeof(q31_t);  /* FFT complex buffer */
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
    state->frame_len = cfg->frame_len;
    state->num_channels = cfg->num_channels;
    state->scratch = (int32_t*)scratch;
    state->scratch_bytes = scratch_bytes;

    // allocate/init DC removal per channels
    for (uint8_t ch = 0; ch < state->num_channels; ch++) {
        dc_removal_init(&state->dc_block[ch], cfg->dc_rm_alpha);
        pre_emphasis_init(&state->pre_emphasis_block[ch], cfg->pre_emphasis_alpha);
    }

    // apply the same for: pre-emphasis, AGC, noise suppression

    return FE_OK;
}