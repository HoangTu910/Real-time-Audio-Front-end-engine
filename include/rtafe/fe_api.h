/**
 * @file fe_api.h
 * @brief Primary streaming API for the Real-Time Audio Front-End engine.
 *
 * Lifecycle:
 *   1. Fill fe_config_t
 *   2. Allocate state  (fe_state_bytes)  and scratch (fe_scratch_bytes)
 *   3. fe_init(...)
 *   4. Loop: fe_process_hop(...)
 *   5. fe_reset(...) or fe_free(...)
 */
#pragma once

#include "fe_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t frame_len;
    uint8_t num_channels;

    DCRemoval *dc_block; /** array size = num channels */
    PreEmphasis *pre_emphasis_block;
    /** More block... */

    int32_t *scratch; /** scratch buffer: FFT temp, intermediate */
    size_t scratch_bytes;
} fe_state_t;

/* ── Query helpers ──────────────────────────────────────────────────────── */

/**
 * Return the size (in bytes) required for fe_state_t given @p cfg.
 * The caller must allocate at least this many bytes for the state pointer
 * passed to fe_init().
 */
size_t      fe_state_bytes(const fe_config_t *cfg);

/**
 * Return the size (in bytes) required for the scratch buffer given @p cfg.
 */
size_t      fe_scratch_bytes(const fe_config_t *cfg);

/* ── Lifecycle ──────────────────────────────────────────────────────────── */

/**
 * Initialise a front-end instance.
 *
 * @param cfg        Immutable configuration (caller retains ownership).
 * @param state      Pre-allocated buffer of >= fe_state_bytes(cfg) bytes.
 * @param scratch    Pre-allocated scratch of >= fe_scratch_bytes(cfg) bytes.
 * @param scratch_sz Actual size of the scratch buffer (validated internally).
 * @return FE_OK on success; negative fe_status_t on error.
 */
fe_status_t fe_init(const fe_config_t *cfg,
                    fe_state_t        *state,
                    void              *scratch,
                    size_t             scratch_sz);

/**
 * Process one hop of audio.
 *
 * @param state       Initialised state object.
 * @param pcm_in      Input: num_mics × hop_len interleaved Q1.15 samples.
 * @param pcm_out     Output: hop_len mono Q1.15 samples (enhanced).
 * @param feature_out Optional (may be NULL). Receives feature data when
 *                    FE_FLAG_FEATURES is set.
 * @param feature_sz  Size of feature_out buffer in bytes.
 * @return FE_OK on success; negative fe_status_t on error.
 */
fe_status_t fe_process_hop(fe_state_t     *state,
                           const q15_t    *pcm_in,
                           q15_t          *pcm_out,
                           void           *feature_out,
                           size_t          feature_sz);

/**
 * Reset internal state to initial conditions (keeps config & scratch).
 */
fe_status_t fe_reset(fe_state_t *state);

/**
 * Release any platform-specific resources.
 * No-op on bare-metal targets.
 */
fe_status_t fe_free(fe_state_t *state);

/* ── Introspection ──────────────────────────────────────────────────────── */

/**
 * Return library version packed as (major << 16 | minor << 8 | patch).
 */
uint32_t    fe_version(void);

#ifdef __cplusplus
}
#endif
