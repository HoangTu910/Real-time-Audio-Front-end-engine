/* fixedpoint.h — Fixed-point math utilities (saturating ops, Q-format shifts) */
#include <stdint.h>
#include <math.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef int32_t s32;
typedef int16_t s16;
typedef uint64_t u64;
typedef int64_t s64;

#define q15 int16_t
#define q31 int32_t

#define FLOAT_TO_Q15(x) ((q15)((x) * 32768.0f))
#define FLOAT_TO_Q31(x) ((q31)((x) * 2147483648.0f))
#define FLOAT_TO_Q2_14(x)((q31)((x) * 16384.0f))

#define SAMPLING_RATE (48000.0)

#define M_PI		3.14159265358979323846	/* pi */
#define M_PI_2		1.57079632679489661923	/* pi/2 */

#define FE_LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define FE_WARN(fmt, ...) fprintf(stderr, "WARN: " fmt, ##__VA_ARGS__)
#define FE_ERROR(fmt, ...) fprintf(stderr, "ERROR: " fmt, ##__VA_ARGS__)

#ifdef FIXED_POINT
typedef u16 sample_t;
#else
typedef float sample_t;
#endif

typedef struct sincos
{
    float sin, cos;
} sincos;

static inline float fast_sin_poly(float x)
{
    /* https://uli.rocks/p/polynomial-approximation/ */
    float x2 = x * x;
    return x * (0.98786f + x2 * (-0.15527f + x2 * 0.005643f));
}

static inline sincos _fast_sine_cos(float phase)
{
    /**
     * wrap to [-pi, pi]
     * Why? According to the reference: "This is the best fifth degree polynomial approximation for sin(x) over [-pi, pi]"
     */
    if (phase > M_PI) phase -= 2 * M_PI;
    if (phase < -M_PI) phase += 2 * M_PI;

    sincos out;

    out.sin = fast_sin_poly(phase);

    float phase_shift = phase + M_PI_2;
    if (phase_shift > M_PI) phase_shift -= 2 * M_PI;

    out.cos = fast_sin_poly(phase_shift);

    return out;
}

#define fast_sine_cos(phase) _fast_sine_cos(phase)