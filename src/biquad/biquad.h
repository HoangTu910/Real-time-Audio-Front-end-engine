#include "utils.h"

typedef struct biquad_coeffs
{
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;
} biquad_coeffs;

typedef struct {
    s16 b0, b1, b2, a1, a2;
} biquad_coeffs_fixed;

typedef struct biquad_state {
#ifdef FIXED_POINT
    s64 d1, d2;
#else
    float d1, d2;
#endif
} biquad_state;

typedef struct biquad {
	biquad_coeffs coeff;
	biquad_state state;
    biquad_coeffs_fixed coeff_fixed;
} biquad;

static inline float biquad_df2t(biquad_coeffs *c, float x, float *d1, float *d2)
{
    float out = c->b0 * x + *d1;

    float temp_d1 = c->b1 * x - c->a1 * out + *d2;
    float temp_d2 = c->b2 * x - c->a2 * out;

    *d1 = temp_d1;
    *d2 = temp_d2;

    return out;
}

static inline s16 biquad_df2t_fixed(biquad_coeffs_fixed *c, s16 x, s64 *d1, s64 *d2)
{
    s64 acc = (s64)c->b0 * x + *d1;

    s16 out = (s16)(acc + 0x2000) >> 14;

    *d1 = (s64)c->b1 * x - (s64)c->a1 * out + *d2;
    *d2 = (s64)c->b2 * x - (s64)c->a2 * out;

    return out;
}

static inline void biquad_quantize(biquad_coeffs *c_float, biquad_coeffs_fixed *c_fixed)
{
    c_fixed->b0 = FLOAT_TO_Q2_14(c_float->b0);
    c_fixed->b1 = FLOAT_TO_Q2_14(c_float->b1);
    c_fixed->b2 = FLOAT_TO_Q2_14(c_float->b2);
    c_fixed->a1 = FLOAT_TO_Q2_14(c_float->a1);
    c_fixed->a2 = FLOAT_TO_Q2_14(c_float->a2);
}

static inline void _biquad_lpf(biquad_coeffs *res, float freq, float Q) 
{
    sincos w0 = fast_sine_cos(2.0f * M_PI * freq / SAMPLING_RATE);

    float alpha = w0.sin/(2*Q);
	float a0_inv = 1/(1 + alpha);
	float b1 = (1 - w0.cos) * a0_inv;

    res->b0 = b1 * 0.5f;
	res->b1 = b1;
	res->b2 = b1 * 0.5f;
	res->a1 = -2 * w0.cos * a0_inv;
	res->a2 = (1 - alpha) * a0_inv;
}

static inline void _biquad_hpf(biquad_coeffs *res, float freq, float Q)
{
	sincos w0 = fast_sine_cos(2.0f * M_PI * freq / SAMPLING_RATE);
	float alpha = w0.sin/(2*Q);
	float a0_inv = 1/(1 + alpha);
	float b1 = (1 + w0.cos) * a0_inv;

	res->b0 = b1 / 2;
	res->b1 = -b1;
	res->b2 = b1 / 2;
	res->a1 = -2*w0.cos	* a0_inv;
	res->a2 = (1 - alpha)	* a0_inv;
}

static inline void _biquad_notch_filter(biquad_coeffs *res, float freq, float Q)
{
	sincos w0 = fast_sine_cos(2.0f * M_PI * freq / SAMPLING_RATE);
	float alpha = w0.sin/(2*Q);
	float a0_inv = 1/(1 + alpha);

	res->b0 = 1 		* a0_inv;
	res->b1 = -2*w0.cos	* a0_inv;
	res->b2 = 1		* a0_inv;
	res->a1 = -2*w0.cos	* a0_inv;
	res->a2 = (1 - alpha)	* a0_inv;
}

static inline void _biquad_bpf_peak(biquad_coeffs *res, float freq, float Q)
{
	sincos w0 = fast_sine_cos(2.0f * M_PI * freq / SAMPLING_RATE);
	float alpha = w0.sin/(2*Q);
	float a0_inv = 1/(1 + alpha);

	res->b0 = Q*alpha	* a0_inv;
	res->b1 = 0;
	res->b2 = -Q*alpha	* a0_inv;
	res->a1 = -2*w0.cos	* a0_inv;
	res->a2 = (1 - alpha)	* a0_inv;
}

static inline void _biquad_bpf(biquad_coeffs *res, float freq, float Q)
{
	sincos w0 = fast_sine_cos(2.0f * M_PI * freq / SAMPLING_RATE);
	float alpha = w0.sin/(2*Q);
	float a0_inv = 1/(1 + alpha);

	res->b0 = alpha		* a0_inv;
	res->b1 = 0;
	res->b2 = -alpha	* a0_inv;
	res->a1 = -2*w0.cos	* a0_inv;
	res->a2 = (1 - alpha)	* a0_inv;
}

static inline void _biquad_allpass_filter(biquad_coeffs *res, float freq, float Q)
{
	sincos w0 = fast_sine_cos(2.0f * M_PI * freq / SAMPLING_RATE);
	float alpha = w0.sin/(2*Q);
	float a0_inv = 1/(1 + alpha);

	res->b0 = (1 - alpha)	* a0_inv;
	res->b1 = (-2*w0.cos)	* a0_inv;
	res->b2 = 1;
	res->a1 = res->b1;
	res->a2 = res->b0;
}

static inline float biquad_step(biquad *bq, float x)
{
	return biquad_df2t(&bq->coeff, x, &bq->state.d1, &bq->state.d2);
}

static inline s16 biquad_step_fixed(biquad *bq, s16 x)
{
	return biquad_df2t_fixed(&bq->coeff_fixed, x, &bq->state.d1, &bq->state.d2);
}

#define biquad_lpf(bq,f,Q) _biquad_lpf(&(bq)->coeff,f,Q)
#define biquad_hpf(bq,f,Q) _biquad_hpf(&(bq)->coeff,f,Q)
#define biquad_notch_filter(bq,f,Q) _biquad_notch_filter(&(bq)->coeff,f,Q)
#define biquad_bpf_peak(bq,f,Q) _biquad_bpf_peak(&(bq)->coeff,f,Q)
#define biquad_bpf(bq,f,Q) _biquad_bpf(&(bq)->coeff,f,Q)
#define biquad_allpass_filter(bq,f,Q) _biquad_allpass_filter(&(bq)->coeff,f,Q)