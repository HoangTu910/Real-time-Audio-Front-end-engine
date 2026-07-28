#include "biquad_filter.hpp"

inline void BiquadLPF::Design(float frequency, float q_factor, float sample_rate)
{
    sincos_t w0 = fast_sine_cos(2.0f * M_PI * frequency / sample_rate);
    float alpha = w0.sin/(2*q_factor);
    biquad_coeffs_.b0 = (1 - w0.cos) / 2;
    biquad_coeffs_.b1 = 1 - w0.cos;
    biquad_coeffs_.b2 = (1 - w0.cos) / 2;
    biquad_coeffs_.a1 = -2 * w0.cos / (1 + alpha);
    biquad_coeffs_.a2 = (1 - alpha) / (1 + alpha);

    biquad_coeffs_fixed_.b0 = FLOAT_TO_Q2_14(biquad_coeffs_.b0);
    biquad_coeffs_fixed_.b1 = FLOAT_TO_Q2_14(biquad_coeffs_.b1);
    biquad_coeffs_fixed_.b2 = FLOAT_TO_Q2_14(biquad_coeffs_.b2);
    biquad_coeffs_fixed_.a1 = FLOAT_TO_Q2_14(biquad_coeffs_.a1);
    biquad_coeffs_fixed_.a2 = FLOAT_TO_Q2_14(biquad_coeffs_.a2);
}

inline void BiquadHPF::Design(float frequency, float q_factor, float sample_rate)
{
    sincos_t w0 = fast_sine_cos(2.0f * M_PI * frequency / sample_rate);
    float alpha = w0.sin/(2*q_factor);
    biquad_coeffs_.b0 = (1 + w0.cos) / 2;
    biquad_coeffs_.b1 = -(1 + w0.cos);
    biquad_coeffs_.b2 = (1 + w0.cos) / 2;
    biquad_coeffs_.a1 = -2 * w0.cos / (1 + alpha);
    biquad_coeffs_.a2 = (1 - alpha) / (1 + alpha);

    biquad_coeffs_fixed_.a1 = FLOAT_TO_Q2_14(biquad_coeffs_.a1);
    biquad_coeffs_fixed_.a2 = FLOAT_TO_Q2_14(biquad_coeffs_.a2);
    biquad_coeffs_fixed_.b0 = FLOAT_TO_Q2_14(biquad_coeffs_.b0);
    biquad_coeffs_fixed_.b1 = FLOAT_TO_Q2_14(biquad_coeffs_.b1);
    biquad_coeffs_fixed_.b2 = FLOAT_TO_Q2_14(biquad_coeffs_.b2);
}

inline void BiquadPeak::Design(float frequency, float q_factor, float sample_rate)
{
    sincos_t w0 = fast_sine_cos(2.0f * M_PI * frequency / sample_rate);
    float alpha = w0.sin/(2*q_factor);
    biquad_coeffs_.b0 = w0.sin / (2 * q_factor);
    biquad_coeffs_.b1 = 0;
    biquad_coeffs_.b2 = -w0.sin / (2 * q_factor);
    biquad_coeffs_.a1 = -2 * w0.cos / (1 + alpha);
    biquad_coeffs_.a2 = (1 - alpha) / (1 + alpha);

    biquad_coeffs_fixed_.b0 = FLOAT_TO_Q2_14(biquad_coeffs_.b0);
    biquad_coeffs_fixed_.b1 = FLOAT_TO_Q2_14(biquad_coeffs_.b1);
    biquad_coeffs_fixed_.b2 = FLOAT_TO_Q2_14(biquad_coeffs_.b2);
    biquad_coeffs_fixed_.a1 = FLOAT_TO_Q2_14(biquad_coeffs_.a1);
    biquad_coeffs_fixed_.a2 = FLOAT_TO_Q2_14(biquad_coeffs_.a2);
}

inline void BiquadBPF::Design(float frequency, float q_factor, float sample_rate)
{
    sincos_t w0 = fast_sine_cos(2.0f * M_PI * frequency / sample_rate);
    float alpha = w0.sin/(2*q_factor);
    biquad_coeffs_.b0 = w0.sin / (2 * q_factor);
    biquad_coeffs_.b1 = 0;
    biquad_coeffs_.b2 = -w0.sin / (2 * q_factor);
    biquad_coeffs_.a1 = -2 * w0.cos / (1 + alpha);
    biquad_coeffs_.a2 = (1 - alpha) / (1 + alpha);

    biquad_coeffs_fixed_.b0 = FLOAT_TO_Q2_14(biquad_coeffs_.b0);
    biquad_coeffs_fixed_.b1 = FLOAT_TO_Q2_14(biquad_coeffs_.b1);
    biquad_coeffs_fixed_.b2 = FLOAT_TO_Q2_14(biquad_coeffs_.b2);
    biquad_coeffs_fixed_.a1 = FLOAT_TO_Q2_14(biquad_coeffs_.a1);
    biquad_coeffs_fixed_.a2 = FLOAT_TO_Q2_14(biquad_coeffs_.a2);
}

inline void BiquadNotch::Design(float frequency, float q_factor, float sample_rate)
{
    sincos_t w0 = fast_sine_cos(2.0f * M_PI * frequency / sample_rate);
    float alpha = w0.sin/(2*q_factor);
    biquad_coeffs_.b0 = 1;
    biquad_coeffs_.b1 = -2 * w0.cos / (1 + alpha);
    biquad_coeffs_.b2 = 1;
    biquad_coeffs_.a1 = -2 * w0.cos / (1 + alpha);
    biquad_coeffs_.a2 = (1 - alpha) / (1 + alpha);

    biquad_coeffs_fixed_.b0 = FLOAT_TO_Q2_14(biquad_coeffs_.b0);
    biquad_coeffs_fixed_.b1 = FLOAT_TO_Q2_14(biquad_coeffs_.b1);
    biquad_coeffs_fixed_.b2 = FLOAT_TO_Q2_14(biquad_coeffs_.b2);
    biquad_coeffs_fixed_.a1 = FLOAT_TO_Q2_14(biquad_coeffs_.a1);
    biquad_coeffs_fixed_.a2 = FLOAT_TO_Q2_14(biquad_coeffs_.a2);
}

inline void BiquadAllpass::Design(float frequency, float q_factor, float sample_rate)
{
    sincos_t w0 = fast_sine_cos(2.0f * M_PI * frequency / sample_rate);
    float alpha = w0.sin/(2*q_factor);
    biquad_coeffs_.b0 = (1 - alpha) / (1 + alpha);
    biquad_coeffs_.b1 = -2 * w0.cos / (1 + alpha);
    biquad_coeffs_.b2 = 1;
    biquad_coeffs_.a1 = -2 * w0.cos / (1 + alpha);
    biquad_coeffs_.a2 = (1 - alpha) / (1 + alpha);

    biquad_coeffs_fixed_.b0 = FLOAT_TO_Q2_14(biquad_coeffs_.b0);
    biquad_coeffs_fixed_.b1 = FLOAT_TO_Q2_14(biquad_coeffs_.b1);
    biquad_coeffs_fixed_.b2 = FLOAT_TO_Q2_14(biquad_coeffs_.b2);
    biquad_coeffs_fixed_.a1 = FLOAT_TO_Q2_14(biquad_coeffs_.a1);
    biquad_coeffs_fixed_.a2 = FLOAT_TO_Q2_14(biquad_coeffs_.a2);
}


