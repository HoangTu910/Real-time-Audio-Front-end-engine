#include "pre_emphasis.hpp"

PreEmphasis::PreEmphasis(float preEmphasisFactor)
    : m_state{}
    , m_alpha(0.0f)
    , m_alpha_fixed(0)
{
    vSetCoeffs(preEmphasisFactor);
}

void PreEmphasis::vSetCoeffs(float preEmphasisFactor)
{
    m_alpha = preEmphasisFactor;
    m_alpha_fixed = FLOAT_TO_Q2_14(preEmphasisFactor);
}

void PreEmphasis::vProcessBlock(TrplBufferStr *pProcessBuf)
{
    /* using the pre-emphasis filter 1 - 0.68z^-1 [Bäckström et al., 2017]. */
    /* alpha = 0.68, set it in constructor please */
    for(int i = 0; i < pProcessBuf->bufferSize; i++) {
        float in = pProcessBuf->pBufferRef[i];
        float out = in - m_alpha * m_state.x[0];
        m_state.x[0] = in;
        pProcessBuf->pBufferRef[i] = out;
    }
}

void PreEmphasis::vProcessBlockFix(TrplBufferStr *pProcessBuf)
{
    for(int i = 0; i < pProcessBuf->bufferSize; i++) {
        s16 in = (s16)pProcessBuf->pBufferRef[i];
        s32 acc = (s32)m_alpha_fixed * m_state.x[0];
        s16 val = (s16)((acc + 0x2000) >> Q2_14_SHIFT);  /* Q2.14 to Q1.15 with rounding */
        s16 out = in - val;
        m_state.x[0] = in;
        pProcessBuf->pBufferRef[i] = out;
    }
}