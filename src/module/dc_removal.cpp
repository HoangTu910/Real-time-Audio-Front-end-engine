#include "dc_removal.hpp"
/* dc_removal.c */

/** Difference equation for Direct Form I
 * y[n] = x[n] - x[n - 1] + alpha * y[n - 1]
 */

DCRemoval::DCRemoval(float alpha)
    : m_alpha(0.0f)
    , m_alpha_fixed(0)
    , m_state{}
{
    vSetCoeffs(alpha);
}

DCRemoval::~DCRemoval() = default;

void DCRemoval::vProcessBlock(TrplBufferStr *pBuf)
{
    for(int i = 0; i < pBuf->bufferSize; i++) {
        float in = pBuf->pBufferRef[i];
        float out = in - m_state.x[0] + m_alpha * m_state.y[0];
        m_state.x[0] = in;
        m_state.y[0] = out;
        pBuf->pBufferRef[i] = out;
    }
}

void DCRemoval::vProcessBlockFix(TrplBufferStr *pBuf)
{
    for(int i = 0; i < pBuf->bufferSize; i++) {
        s16 in = (s16)pBuf->pBufferRef[i];
        s32 acc = (s32)in - (s32)m_state.x[0] + ((((s32)m_alpha_fixed) * ((s32)m_state.y[0])) >> Q2_14_SHIFT);
        s16 out = CLIP_S16(acc);
        m_state.x[0] = in;
        m_state.y[0] = out;
        pBuf->pBufferRef[i] = out;
    }
}

void DCRemoval::vSetCoeffs(float alpha)
{
    m_alpha = alpha;
    m_alpha_fixed = FLOAT_TO_Q2_14(alpha);
}
