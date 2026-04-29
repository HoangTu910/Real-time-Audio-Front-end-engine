#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include "module/dc_removal.h"

#define NUM_SAMPLES 1000000

int main() {
    dc_remov module;
    memset(&module.state, 0, sizeof(dc_remov_state));

    float alpha_val = 0.99f; 

#ifdef FIXED_POINT
    module.coeffs_fixed.alpha = FLOAT_TO_Q2_14(alpha_val);
    printf("Mode: FIXED POINT (Alpha: %d)\n", module.coeffs_fixed.alpha);
#else
    module.coeffs.alpha = alpha_val;
    printf("Mode: FLOATING POINT (Alpha: %.3f)\n", module.coeffs.alpha);
#endif

    printf("\n--- Verification (First 20 samples) ---\n");
    printf("Sample, Input_with_DC, Output_Cleaned\n");
    
    for (int i = 0; i < 20; i++) {
        /* Sine 1kHz and DC Bias 0.5 */
        float t = (float)i / SAMPLING_RATE;
        float sig = 0.2f * sinf(2.0f * M_PI * 1000.0f * t);
        float input_float = sig + 0.5f; // DC Bias rất nặng

#ifdef FIXED_POINT
        s16 input_fixed = (s16)(input_float * 16384.0f);
        s16 out_fixed = dc_remov_sample_proc_fixed(&module, input_fixed);
        printf("%d, %.4f, %d\n", i, input_float, out_fixed);
#else
        float out_display = dc_remov_sample_proc(&module, input_float);
        printf("%d, %.4f, %.4f\n", i, input_float, out_display);
#endif
    }

    return 0;
}