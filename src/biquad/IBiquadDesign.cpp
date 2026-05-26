#include "IBiquadDesign.hpp"

void IBiquadDesign::vProcessBlock(TrplBufferStr *pProcessBuf)
{
    for (size_t i = 0; i < pProcessBuf->bufferSize; ++i) {
        float in = pProcessBuf->pBufferRef[i];
        float out = m_biquadCoeffs.b0 * in + m_biquadCoeffs.b1 * m_biquadState.x1 + m_biquadCoeffs.b2 * m_biquadState.x2
                    - m_biquadCoeffs.a1 * m_biquadState.y1 - m_biquadCoeffs.a2 * m_biquadState.y2;
        pProcessBuf->pBufferRef[i] = out;

        m_biquadState.x2 = m_biquadState.x1;
        m_biquadState.x1 = in;
        m_biquadState.y2 = m_biquadState.y1;
        m_biquadState.y1 = out;
    }
    return;
}

void IBiquadDesign::vProcessBlockFix(TrplBufferStr *pProcessBuf)
{
    for (size_t i = 0; i < pProcessBuf->bufferSize; ++i) {
        s32 in = pProcessBuf->pBufferRef[i];
        s64 acc = (s64)m_biquadCoeffsFix.b0 * in + (s64)m_biquadCoeffsFix.b1 * m_biquadStateFix.x1 + (s64)m_biquadCoeffsFix.b2 * m_biquadStateFix.x2
                - (s64)m_biquadCoeffsFix.a1 * m_biquadStateFix.y1 - (s64)m_biquadCoeffsFix.a2 * m_biquadStateFix.y2;

        s32 outAcc = (s32)((acc + (1 << (Q2_14_SHIFT - 1))) /* half LSB round to nearest */ >> Q2_14_SHIFT /* / 2^14*/);
        s16 out = CLIP_S16(outAcc);
        pProcessBuf->pBufferRef[i] = out;

        m_biquadStateFix.x2 = m_biquadStateFix.x1;
        m_biquadStateFix.x1 = in;
        m_biquadStateFix.y2 = m_biquadStateFix.y1;
        m_biquadStateFix.y1 = out;
    }
    return;
}