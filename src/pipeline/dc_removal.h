/* dc_removal.h — DC offset removal (high-pass IIR, per channel) */
#pragma once

#include <stdint.h>
#include "fe_types.h"

typedef struct {
    /** Alpha need to be recomputed -> alpha = exp(-2.0 * M_PI * fc / fs); */
    q31_t alpha_q31;   // Q1.31
    q31_t x_prev;      // Q1.31
    q31_t y_prev;      // Q1.31
} DCRemoval;

/** Direct form I from Richard Lyons */
q31_t dc_removal_process(DCRemoval *filt, q31_t x);
void dc_removal_init(DCRemoval *filt, q31_t alpha_q31);