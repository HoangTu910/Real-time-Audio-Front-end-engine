#include "preemphasis.h"
/* preemphasis.c */

void pre_emphasis_init(PreEmphasis *filt, q15_t alpha_q15)
{
    filt->x_prev = 0;
    filt->alpha_q15 = alpha_q15;
}

q15_t pre_emphasis_process(PreEmphasis *filt, q15_t x)
{
    /** Difference equation for Direct Form I
     * y[n] = x[n] - alpha * x[n - 1]
     */

    int32_t acc, mul;
    q15_t out;

    mul = (int32_t)filt->alpha_q15 * (int32_t)filt->x_prev;

    /** Q1.15 * Q1.15 = Q2.30 -> Shift back to Q1.15 */
    mul = mul >> Q1_15_SHIFT;

    acc = (int32_t)x - mul;

    if (acc > INT16_MAX) acc = INT16_MAX;
    else if (acc < INT16_MIN) acc = INT16_MIN;

    out = (q15_t)acc;

    filt->x_prev = x;

    return out;
}