#include "BiquadFilter.hpp"

inline void BiquadLPF::vDesign(float frequency, float QFactor, float sampleRate)
{
    sincos w0 = fast_sine_cos(2.0f * M_PI * frequency / sampleRate);
    float alpha = w0.sin/(2*QFactor);
    float a0_inv = 1/(1 + alpha);
    m_biquadCoeffs.b0 = (1 - w0.cos) / 2;
    m_biquadCoeffs.b1 = 1 - w0.cos;
    m_biquadCoeffs.b2 = (1 - w0.cos) / 2;
    m_biquadCoeffs.a1 = -2 * w0.cos / (1 + alpha);
    m_biquadCoeffs.a2 = (1 - alpha) / (1 + alpha);

    m_biquadCoeffsFix.b0 = FLOAT_TO_Q2_14(m_biquadCoeffs.b0);
    m_biquadCoeffsFix.b1 = FLOAT_TO_Q2_14(m_biquadCoeffs.b1);
    m_biquadCoeffsFix.b2 = FLOAT_TO_Q2_14(m_biquadCoeffs.b2);
    m_biquadCoeffsFix.a1 = FLOAT_TO_Q2_14(m_biquadCoeffs.a1);
    m_biquadCoeffsFix.a2 = FLOAT_TO_Q2_14(m_biquadCoeffs.a2);
}

inline void BiquadHPF::vDesign(float frequency, float QFactor, float sampleRate)
{
    sincos w0 = fast_sine_cos(2.0f * M_PI * frequency / sampleRate);
    float alpha = w0.sin/(2*QFactor);
    float a0_inv = 1/(1 + alpha);
    m_biquadCoeffs.b0 = (1 + w0.cos) / 2;
    m_biquadCoeffs.b1 = -(1 + w0.cos);
    m_biquadCoeffs.b2 = (1 + w0.cos) / 2;
    m_biquadCoeffs.a1 = -2 * w0.cos / (1 + alpha);
    m_biquadCoeffs.a2 = (1 - alpha) / (1 + alpha);

    m_biquadCoeffsFix.a1 = FLOAT_TO_Q2_14(m_biquadCoeffs.a1);
    m_biquadCoeffsFix.a2 = FLOAT_TO_Q2_14(m_biquadCoeffs.a2);
    m_biquadCoeffsFix.b0 = FLOAT_TO_Q2_14(m_biquadCoeffs.b0);
    m_biquadCoeffsFix.b1 = FLOAT_TO_Q2_14(m_biquadCoeffs.b1);
    m_biquadCoeffsFix.b2 = FLOAT_TO_Q2_14(m_biquadCoeffs.b2);
}

inline void BiquadPeak::vDesign(float frequency, float QFactor, float sampleRate)
{
    sincos w0 = fast_sine_cos(2.0f * M_PI * frequency / sampleRate);
    float alpha = w0.sin/(2*QFactor);
    float a0_inv = 1/(1 + alpha);
    m_biquadCoeffs.b0 = w0.sin / (2 * QFactor);
    m_biquadCoeffs.b1 = 0;
    m_biquadCoeffs.b2 = -w0.sin / (2 * QFactor);
    m_biquadCoeffs.a1 = -2 * w0.cos / (1 + alpha);
    m_biquadCoeffs.a2 = (1 - alpha) / (1 + alpha);

    m_biquadCoeffsFix.b0 = FLOAT_TO_Q2_14(m_biquadCoeffs.b0);
    m_biquadCoeffsFix.b1 = FLOAT_TO_Q2_14(m_biquadCoeffs.b1);
    m_biquadCoeffsFix.b2 = FLOAT_TO_Q2_14(m_biquadCoeffs.b2);
    m_biquadCoeffsFix.a1 = FLOAT_TO_Q2_14(m_biquadCoeffs.a1);
    m_biquadCoeffsFix.a2 = FLOAT_TO_Q2_14(m_biquadCoeffs.a2);
}

inline void BiquadBPF::vDesign(float frequency, float QFactor, float sampleRate)
{
    sincos w0 = fast_sine_cos(2.0f * M_PI * frequency / sampleRate);
    float alpha = w0.sin/(2*QFactor);
    float a0_inv = 1/(1 + alpha);
    m_biquadCoeffs.b0 = w0.sin / (2 * QFactor);
    m_biquadCoeffs.b1 = 0;
    m_biquadCoeffs.b2 = -w0.sin / (2 * QFactor);
    m_biquadCoeffs.a1 = -2 * w0.cos / (1 + alpha);
    m_biquadCoeffs.a2 = (1 - alpha) / (1 + alpha);

    m_biquadCoeffsFix.b0 = FLOAT_TO_Q2_14(m_biquadCoeffs.b0);
    m_biquadCoeffsFix.b1 = FLOAT_TO_Q2_14(m_biquadCoeffs.b1);
    m_biquadCoeffsFix.b2 = FLOAT_TO_Q2_14(m_biquadCoeffs.b2);
    m_biquadCoeffsFix.a1 = FLOAT_TO_Q2_14(m_biquadCoeffs.a1);
    m_biquadCoeffsFix.a2 = FLOAT_TO_Q2_14(m_biquadCoeffs.a2);
}

inline void BiquadNotch::vDesign(float frequency, float QFactor, float sampleRate)
{
    sincos w0 = fast_sine_cos(2.0f * M_PI * frequency / sampleRate);
    float alpha = w0.sin/(2*QFactor);
    float a0_inv = 1/(1 + alpha);
    m_biquadCoeffs.b0 = 1;
    m_biquadCoeffs.b1 = -2 * w0.cos / (1 + alpha);
    m_biquadCoeffs.b2 = 1;
    m_biquadCoeffs.a1 = -2 * w0.cos / (1 + alpha);
    m_biquadCoeffs.a2 = (1 - alpha) / (1 + alpha);

    m_biquadCoeffsFix.b0 = FLOAT_TO_Q2_14(m_biquadCoeffs.b0);
    m_biquadCoeffsFix.b1 = FLOAT_TO_Q2_14(m_biquadCoeffs.b1);
    m_biquadCoeffsFix.b2 = FLOAT_TO_Q2_14(m_biquadCoeffs.b2);
    m_biquadCoeffsFix.a1 = FLOAT_TO_Q2_14(m_biquadCoeffs.a1);
    m_biquadCoeffsFix.a2 = FLOAT_TO_Q2_14(m_biquadCoeffs.a2);
}

inline void BiquadAllpass::vDesign(float frequency, float QFactor, float sampleRate)
{
    sincos w0 = fast_sine_cos(2.0f * M_PI * frequency / sampleRate);
    float alpha = w0.sin/(2*QFactor);
    float a0_inv = 1/(1 + alpha);
    m_biquadCoeffs.b0 = (1 - w0.sin / (2 * QFactor)) 
                / (1 + w0.sin / (2 * QFactor));
    m_biquadCoeffs.b1 = -2 * w0.cos / (1 + w0.sin / (2 * QFactor));
    m_biquadCoeffs.b2 = 1;
    m_biquadCoeffs.a1 = -2 * w0.cos / (1 + w0.sin / (2 * QFactor));
    m_biquadCoeffs.a2 = (1 - w0.sin / (2 * QFactor)) 
                / (1 + w0.sin / (2 * QFactor));

    m_biquadCoeffsFix.b0 = FLOAT_TO_Q2_14(m_biquadCoeffs.b0);
    m_biquadCoeffsFix.b1 = FLOAT_TO_Q2_14(m_biquadCoeffs.b1);
    m_biquadCoeffsFix.b2 = FLOAT_TO_Q2_14(m_biquadCoeffs.b2);
    m_biquadCoeffsFix.a1 = FLOAT_TO_Q2_14(m_biquadCoeffs.a1);
    m_biquadCoeffsFix.a2 = FLOAT_TO_Q2_14(m_biquadCoeffs.a2);
}


