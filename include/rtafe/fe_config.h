/**
 * @file fe_config.h
 * @brief Configuration structures, feature-flag macros, and version info.
 */
#pragma once

#include "fe_types.h"

/* ── Feature-enable flags (bitfield for fe_config_t.flags) ──────────────── */
#define FE_FLAG_NOISE_SUPPRESS  (1U << 0)  /**< Enable spectral noise suppression */
#define FE_FLAG_AGC             (1U << 1)  /**< Enable automatic gain control     */
#define FE_FLAG_BEAMFORMER      (1U << 2)  /**< Enable basic beamforming          */
#define FE_FLAG_AEC_STUB        (1U << 3)  /**< Enable echo suppression stub      */
#define FE_FLAG_FEATURES        (1U << 4)  /**< Enable feature extraction output  */

/* ── Library version ────────────────────────────────────────────────────── */
#define FE_VERSION_MAJOR  0
#define FE_VERSION_MINOR  1
#define FE_VERSION_PATCH  0

/* ── Configuration ──────────────────────────────────────────────────────── */
typedef struct {
    uint32_t sample_rate;       /**< Sampling rate in Hz (16000 or 48000)     */
    uint16_t frame_len;         /**< FFT frame length in samples (e.g. 256)  */
    uint16_t hop_len;           /**< Hop / stride in samples (e.g. 128)      */
    uint8_t  num_mics;          /**< Number of microphone channels (1 or 2)  */
    uint8_t  flags;             /**< OR'd FE_FLAG_* bitmask                  */

    /* Noise suppression tuning */
    q15_t    ns_over_subtract;  /**< Over-subtraction factor Q6.9            */
    q15_t    ns_floor;          /**< Spectral floor Q6.9                     */

    /* AGC tuning */
    q15_t    agc_target_level;  /**< Target output RMS in Q1.15              */
    uint16_t agc_attack_ms;     /**< Attack time constant (ms)               */
    uint16_t agc_release_ms;    /**< Release time constant (ms)              */
} fe_config_t;

/* ── Opaque state handle (defined internally in src/) ───────────────────── */
typedef struct fe_state_t fe_state_t;
