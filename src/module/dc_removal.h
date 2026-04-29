/* dc_removal.h — DC offset removal (high-pass IIR, per channel) */
#pragma once

#include "utils.h"

typedef struct {
    float alpha;
} dc_remov_coeffs;

typedef struct {
    s16 alpha;
} dc_remov_coeffs_fixed;

typedef struct {
    #ifdef FIXED_POINT
        s16 x[2];
        s16 y[2];
    #else 
        float x[2];
        float y[2];
    #endif
} dc_remov_state;

typedef struct {
    dc_remov_coeffs_fixed coeffs_fixed;
    dc_remov_coeffs coeffs;
    dc_remov_state state;
} dc_remov;

/** Direct form I from Richard Lyons */
/** Difference equation for Direct Form I
 * y[n] = x[n] - x[n - 1] + alpha * y[n - 1]
 */

static inline float dc_remov_sample_proc(dc_remov *c, float in)
{
    c->state.x[1] = in;
    float out = c->state.x[1] - c->state.x[0] + c->coeffs.alpha * c->state.y[0];
    c->state.x[0] = c->state.x[1];
    c->state.y[0] = out;

    return out;
}

static inline s16 dc_remov_sample_proc_fixed(dc_remov *c, s16 in)
{
    c->state.x[1] = in;

    s32 acc = (s32)c->coeffs_fixed.alpha * c->state.y[0];
    s16 val = (s16)((acc + 0x2000) >> 14);
    
    s16 out = (s16)c->state.x[1] - (s16)c->state.x[0] + val;
    c->state.x[0] = c->state.x[1];
    c->state.y[0] = out;

    return out;
}

static inline void dc_remov_quantize(dc_remov_coeffs *c_float, dc_remov_coeffs_fixed *c_fixed) 
{
    c_fixed->alpha = FLOAT_TO_Q2_14(c_float->alpha);
}