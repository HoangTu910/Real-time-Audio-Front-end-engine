/**
 * @file fe_types.h
 * @brief Fixed-point type aliases and error codes for RTAFE.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

/** Q-format conversion principle
 * From Qm.n -> Qp.q
 * shift = q - n
 * shift > 0 -> left shift
 * shift < 0 -> right shift
 */

#define Q1_31_SHIFT 31
#define Q1_15_SHIFT 15

/* ── Fixed-point type aliases ───────────────────────────────────────────── */
typedef int16_t  q15_t;    /**< Q1.15  range: −1 … +0.999969         */
typedef int32_t  q31_t;    /**< Q1.31  accumulator / extended state   */
typedef int64_t  q63_t;    /**< Q1.63  double-width accumulator       */

/* ── Saturation helpers ─────────────────────────────────────────────────── */
static inline q15_t q15_sat(q31_t x)
{
    if (x > (q31_t)INT16_MAX) return (q15_t)INT16_MAX;
    if (x < (q31_t)INT16_MIN) return (q15_t)INT16_MIN;
    return (q15_t)x;
}

/* ── Error / status codes ───────────────────────────────────────────────── */
typedef enum {
    FE_OK                =  0,   /**< Success                            */
    FE_ERR_NULL_PTR      = -1,   /**< NULL pointer argument              */
    FE_ERR_BAD_CONFIG    = -2,   /**< Invalid configuration parameter    */
    FE_ERR_SCRATCH_SMALL = -3,   /**< Scratch buffer too small           */
    FE_ERR_NOT_INIT      = -4,   /**< State not initialised              */
} fe_status_t;
