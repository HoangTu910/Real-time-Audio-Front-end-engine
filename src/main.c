#define _POSIX_C_SOURCE 199309L
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "biquad.h"
#define NUM_SAMPLES 1000000

#ifdef ARM_TARGET
#define DWT_CYCCNT   (*(volatile uint32_t*)0xE0001004)
#define DWT_CONTROL  (*(volatile uint32_t*)0xE0001000)
#define SCB_DEMCR    (*(volatile uint32_t*)0xE000EDFC)
#define TRCENA       (1 << 24)

static inline void dwt_init(void)
{
    SCB_DEMCR |= TRCENA;
    DWT_CYCCNT = 0;
    DWT_CONTROL |= 1;
}
#endif

uint64_t get_time_ns() {
#ifndef ARM_TARGET
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#else
    return (uint64_t)DWT_CYCCNT;
#endif
}

int main() {
    #ifdef ARM_TARGET
        dwt_init();
    #endif

    biquad my_filter;
    memset(&my_filter.state, 0, sizeof(biquad_state));
    biquad_lpf(&my_filter, 1000.0f, 0.707f);
    biquad_quantize(&my_filter.coeff, &my_filter.coeff_fixed);

    uint64_t start = get_time_ns();

    for (int i = 0; i < NUM_SAMPLES; i++) {
        float input_float = 0.5f; // Tín hiệu đơn giản để benchmark

    #ifdef FIXED_POINT
        s16 input_fixed = (s16)(input_float * 16384.0f);
        biquad_step_fixed(&my_filter, input_fixed);
    #else 
        biquad_step(&my_filter, input_float);
    #endif
    }

    /**
     * The benchmark only make sense if your hardware doesn't have FPU support
     * And counting the clock pulse would give a better result...
     */
    uint64_t end = get_time_ns();
    uint64_t total_ns = end - start;

    #ifdef ARM_TARGET
    volatile uint64_t result = total_ns;
    #else
    printf("Time: %llu ns\n", total_ns);
    #endif

    return 0;
}