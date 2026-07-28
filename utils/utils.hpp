#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdint.h>
#include <math.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint16_t u16;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;
typedef uint64_t u64;
typedef int64_t  s64;
typedef s32      tFixed;
typedef float    tFloat;

#define FLOAT_TO_Q15(x) ((s16)((x) * 32768.0f))
#define FLOAT_TO_Q31(x) ((s32)((x) * 2147483648.0f))
#define FLOAT_TO_Q2_14(x) ((s32)((x) * 16384.0f))
#define Q2_14_TO_FLOAT(x) ((float)(x) / 16384.0f)

#define Q2_14_SHIFT 14

#define CLIP_S16(x) ((x) > INT16_MAX ? INT16_MAX : ((x) < INT16_MIN ? INT16_MIN : (s16)(x)))
#define CLIP_S32(x) ((x) > INT32_MAX ? INT32_MAX : ((x) < INT32_MIN ? INT32_MIN : (s32)(x)))
#define SAMPLING_RATE (48000.0)

#define M_PI		3.14159265358979323846	/* pi */
#define M_PI_2		1.57079632679489661923	/* pi/2 */

#define FE_LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define FE_WARN(fmt, ...) fprintf(stderr, "WARN: " fmt, ##__VA_ARGS__)
#define FE_ERROR(fmt, ...) fprintf(stderr, "ERROR: " fmt, ##__VA_ARGS__)

#ifdef FIXED_POINT
typedef tFixed sample_t;
#else
typedef tFloat sample_t;
#endif

typedef struct {
    sample_t *dsp_buffer;
    u16       block_size;
    u16       num_channels;
} DspBlock;

typedef struct sincos_t
{
    float sin, cos;
} sincos_t;

static inline float fast_sin_poly(float x)
{
    /* https://uli.rocks/p/polynomial-approximation/ */
    float x2 = x * x;
    return x * (0.98786f + x2 * (-0.15527f + x2 * 0.005643f));
}

static inline sincos_t fast_sine_cos(float x)
{
    // Range reduce to [-pi, pi]
    const float inv_two_pi = 0.15915494309189535f; // 1 / (2*pi)
    const float two_pi = 6.283185307179586f;

    float k = roundf(x * inv_two_pi);
    x = x - k * two_pi;

    // Map to [-pi/2, pi/2]
    float sign_sin = 1.0f;
    float sign_cos = 1.0f;

    if (x > M_PI_2) {
        x = M_PI - x;
        sign_cos = -1.0f;
    } else if (x < -M_PI_2) {
        x = -M_PI - x;
        sign_cos = -1.0f;
    }

    float x2 = x * x;

    // Minimax polynomial (degree 5)
    float sin_poly = x * (1.0f + x2 * (-0.16666667f + x2 * 0.0083333310f));
    float cos_poly = 1.0f + x2 * (-0.5f + x2 * 0.041666638f);

    sincos_t out;
    out.sin = sign_sin * sin_poly;
    out.cos = sign_cos * cos_poly;

    return out;
}

#endif  /* UTILS_HPP */