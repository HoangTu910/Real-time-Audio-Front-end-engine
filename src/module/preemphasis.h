#ifndef PREEMPHASIS_H
#define PREEMPHASIS_H
#include "utils.h"

typedef struct {
    float alpha;
} pre_emphasis_coeffs;

typedef struct {
    s16 alpha;
} pre_emphasis_coeffs_fixed;

typedef struct {
    #ifdef FIXED_POINT
        s16 x[2];
        s16 y[2];
    #else 
        float x[2];
        float y[2];
    #endif
} pre_emphasis_state;

typedef struct {
    pre_emphasis_coeffs_fixed coeffs_fixed;
    pre_emphasis_coeffs coeffs;
    pre_emphasis_state state;
} pre_emphasis;

/**
 * Difference equation for pre-emphasis filter:
 * y[n] = x[n] - alpha * x[n - 1]
 */

static inline float _pre_emphasis_sample_proc(pre_emphasis *c, float in)
{
    c->state.x[1] = in;
    float out = c->state.x[1] - c->coeffs.alpha * c->state.x[0];
    c->state.x[0] = c->state.x[1];

    return out;
}

static inline s16 _pre_emphasis_sample_proc_fixed(pre_emphasis *c, s16 in)
{
    c->state.x[1] = in;

    s32 acc = (s32)c->coeffs_fixed.alpha * c->state.x[0];
    s16 val = (s16)((acc + 0x2000) >> Q2_14_SHIFT);  /* Q2.14 to Q1.15 with rounding */
    s16 out = (s16)c->state.x[1] - val;
    c->state.x[0] = c->state.x[1];

    return out;
}

static inline void pre_emphasis_quantize(pre_emphasis_coeffs *c_float, pre_emphasis_coeffs_fixed *c_fixed) 
{
    c_fixed->alpha = FLOAT_TO_Q2_14(c_float->alpha);
}

static inline void pre_emphasis_sample_process(pre_emphasis *c, sample_t *in) 
{
    #ifdef FIXED_POINT
        *in = _pre_emphasis_sample_proc_fixed(c, *in);
    #else
        *in = _pre_emphasis_sample_proc(c, *in);
    #endif
}

#endif