#include "i_biquad_design.hpp"

void IBiquad::ProcessBlock(DSPBlock *dsp_block)
{
    for (size_t i = 0; i < dsp_block->block_size; ++i) {
        float in = dsp_block->dsp_buffer[i];
        float out = biquad_coeffs_.b0 * in + biquad_coeffs_.b1 * biquad_state_.x1 + biquad_coeffs_.b2 * biquad_state_.x2
                    - biquad_coeffs_.a1 * biquad_state_.y1 - biquad_coeffs_.a2 * biquad_state_.y2;
        dsp_block->dsp_buffer[i] = out;

        biquad_state_.x2 = biquad_state_.x1;
        biquad_state_.x1 = in;
        biquad_state_.y2 = biquad_state_.y1;
        biquad_state_.y1 = out;
    }
    return;
}

void IBiquad::ProcessBlockFixed(DSPBlock *dsp_block)
{
    for (size_t i = 0; i < dsp_block->block_size; ++i) {
        tFixed in = dsp_block->dsp_buffer[i];
        s64 acc = (s64)biquad_coeffs_fixed_.b0 * in + (s64)biquad_coeffs_fixed_.b1 * biquad_state_fixed_.x1 + (s64)biquad_coeffs_fixed_.b2 * biquad_state_fixed_.x2
                - (s64)biquad_coeffs_fixed_.a1 * biquad_state_fixed_.y1 - (s64)biquad_coeffs_fixed_.a2 * biquad_state_fixed_.y2;

        tFixed outAcc = (tFixed)((acc + (1 << (Q2_14_SHIFT - 1))) /* half LSB round to nearest */ >> Q2_14_SHIFT /* / 2^14*/);
        tFixed out = CLIP_S16(outAcc);
        dsp_block->dsp_buffer[i] = out;

        biquad_state_fixed_.x2 = biquad_state_fixed_.x1;
        biquad_state_fixed_.x1 = in;
        biquad_state_fixed_.y2 = biquad_state_fixed_.y1;
        biquad_state_fixed_.y1 = out;
    }
    return;
}