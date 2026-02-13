#include "window.h"
/* window.c */

void window_apply(const q15_t *window, q15_t *frame, size_t frame_len)
{
    /** Windowed signal
     *  x_w[n] = x[n] * w[n]
     */
    for (size_t n = 0; n < frame_len; n++) {
        /** Q1.15 * Q1.15 = Q2.30 -> Shift back to Q1.15 */
        int32_t acc = (int32_t)frame[n] * (int32_t)window[n];
        acc = acc >> Q1_15_SHIFT;

        if (acc > INT16_MAX) acc = INT16_MAX;
        else if (acc < INT16_MIN) acc = INT16_MIN;

        frame[n] = (q15_t)acc;
    }
}