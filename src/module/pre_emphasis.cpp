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

void PreEmphasis::vProcessBlock(DspBlock *dsp_block)
{
    /* using the pre-emphasis filter 1 - 0.68z^-1 [Bäckström et al., 2017]. */
    /* alpha = 0.68, set it in constructor please */
    for(int i = 0; i < dsp_block->block_size; i++) {
        float in = dsp_block->dsp_buffer[i];
        float out = in - m_alpha * m_state.x[0];
        m_state.x[0] = in;
        dsp_block->dsp_buffer[i] = out;
    }
}

void PreEmphasis::vProcessBlockFix(DspBlock *dsp_block)
{
    for(int i = 0; i < dsp_block->block_size; i++) {
        s16 in = (s16)dsp_block->dsp_buffer[i];
        s32 acc = (s32)m_alpha_fixed * m_state.x[0];
        s16 val = (s16)((acc + 0x2000) >> Q2_14_SHIFT);  /* Q2.14 to Q1.15 with rounding */
        s16 out = in - val;
        m_state.x[0] = in;
        dsp_block->dsp_buffer[i] = out;
    }
}