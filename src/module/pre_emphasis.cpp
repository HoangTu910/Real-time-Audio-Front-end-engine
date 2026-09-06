#include "pre_emphasis.hpp"

PreEmphasis::PreEmphasis(float pre_emphasis_factor)
    : state_{}
    , alpha_(0.0f)
    , alpha_fixed_(0)
{
    SetCoeffs(pre_emphasis_factor);
}

void PreEmphasis::SetCoeffs(float pre_emphasis_factor)
{
    alpha_ = pre_emphasis_factor;
    alpha_fixed_ = FLOAT_TO_Q2_14(pre_emphasis_factor);
}

HtspErrRet PreEmphasis::SetParams(const DSPModuleParams *params, u16 param_count)
{
    if (params == nullptr) return kErrorNullModuleParam;

    if (param_count != PRE_EMPHASIS_MODULE_PARAMS_COUNT) {
        return kErrorInvalidModuleParam;
    }
    SetCoeffs(*params);
    return kOk;
}

void PreEmphasis::ProcessBlock(DSPBlock *dsp_block)
{
    /* using the pre-emphasis filter 1 - 0.68z^-1 [Bäckström et al., 2017]. */
    /* alpha = 0.68, set it in constructor please */
    if(dsp_block == nullptr) return;

    sample_t *in_buf = dsp_block->GetDSPBuffer();

    if(in_buf == nullptr) return;

    for(int i = 0; i < dsp_block->GetBlockSize(); i++) {
        float in = in_buf[i];
        float out = in - alpha_ * state_.x[0];
        state_.x[0] = in;
        in_buf[i] = out;
    }
}

void PreEmphasis::ProcessBlockFixed(DSPBlock *dsp_block)
{
    if(dsp_block == nullptr) return;

    sample_t *in_buf = dsp_block->GetDSPBuffer();

    if(in_buf == nullptr) return;

    for(int i = 0; i < dsp_block->GetBlockSize(); i++) {
        s16 in  = (s16)in_buf[i];
        s32 acc = (s32)alpha_fixed_ * state_.x[0];
        s16 val = (s16)((acc + 0x2000) >> Q2_14_SHIFT);  /* Q2.14 to Q1.15 with rounding */
        s16 out = in - val;
        state_.x[0] = in;
        in_buf[i]   = out;
    }
}