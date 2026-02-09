#include "dc_removal.h"
/* dc_removal.c */

void dc_removal_init(DCRemoval *filt, q31_t alpha_q31)
{
    filt->x_prev = 0;
    filt->y_prev = 0;
    filt->alpha_q31 = alpha_q31;
}

q31_t dc_removal_process(DCRemoval *filt, q31_t x)
{
    /** Difference equation for Direct Form I
     * y[n] = x[n] - x[n - 1] + alpha * y[n - 1]
     */

    int64_t acc;
    q31_t out;

    acc = (int64_t)x - (int64_t)filt->x_prev;

    /** Q1.31 * Q1.31 = Q2.62 -> Shift back to Q1.31 */
    acc += ((int64_t)filt->alpha_q31 * (int64_t)filt->y_prev) >> Q1_31_SHIFT;

    if (acc > INT32_MAX) acc = INT32_MAX;
    else if (acc < INT32_MIN) acc = INT32_MIN;

    out = (q31_t)acc;
    filt->x_prev = x;
    filt->y_prev = out;

    return out;
}