#include <math.h>
#include <stdio.h>

#include "biquad.h"

#include <string.h>

int main() {
    biquad my_filter;
    memset(&my_filter.state, 0, sizeof(biquad_state));

    biquad_lpf(&my_filter, 1000.0f, 0.707f);

    biquad_quantize(&my_filter.coeff, &my_filter.coeff_fixed);

    printf("Time(s),Input,Output\n");

    for (int i = 0; i < 500; i++) {
        float t = (float)i / SAMPLING_RATE;
        float input_float = sinf(2.0f * 3.14159f * 500.0f * t) + 
                            0.5f * sinf(2.0f * 3.14159f * 10000.0f * t);

    #ifdef FIXED_POINT
        s16 input_fixed = FLOAT_TO_Q2_14(input_float);
        s16 output_fixed = biquad_step_fixed(&my_filter, input_fixed);
        printf("%f,%f,%d\n", t, input_float, output_fixed);
    #else 
        float output_float = biquad_step(&my_filter, input_float);
        printf("%f,%f,%f\n", t, input_float, output_float);
    #endif
    }

    return 0;
}