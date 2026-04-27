/* preemphasis.h — Pre-emphasis FIR filter (per channel) */
#pragma once

#include <stdint.h>
#include "fe_types.h"

typedef struct {
    /** Alpha need to be recomputed -> Choose appropriate alpha depend on your application */
    q15_t alpha_q15;   // Q1.15
    q15_t x_prev;      // Q1.15
} PreEmphasis;

/** Direct form I from Richard Lyons */
q15_t pre_emphasis_process(PreEmphasis *filt, q15_t x);
void pre_emphasis_init(PreEmphasis *filt, q15_t alpha_q15);