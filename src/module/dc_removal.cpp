#include "dc_removal.hpp"
/* dc_removal.c */

/** Difference equation for Direct Form I
 * y[n] = x[n] - x[n - 1] + alpha * y[n - 1]
 */

DCRemoval::DCRemoval(float alpha)
    : alpha_(0.0f)
    , alpha_fixed_(0)
    , state_{}
{
    SetCoeffs(alpha);
}

DCRemoval::~DCRemoval() = default;

HtspErrRet DCRemoval::SetParams(const DSPModuleParams *params, u16 param_count)
{
    if (params == nullptr) return kErrorNullModuleParam;

    if (param_count != DC_MODULE_PARAMS_COUNT) {
        return kErrorInvalidModuleParam;
    }

    SetCoeffs(*params);
    return kOk;
}

void DCRemoval::ProcessBlock(DSPBlock *dsp_block)
{
    sample_t *in_buf = dsp_block->GetDSPBuffer();
    for(int i = 0; i < dsp_block->GetBlockSize(); i++) {
        float in = in_buf[i];
        float out = in - state_.x[0] + alpha_ * state_.y[0];
        state_.x[0] = in;
        state_.y[0] = out;
        in_buf[i]   = out;
    }
}

void DCRemoval::ProcessBlockFixed(DSPBlock *dsp_block)
{
    sample_t *in_buf = dsp_block->GetDSPBuffer();
    for(int i = 0; i < dsp_block->GetBlockSize(); i++) {
        s16 in = (s16)in_buf[i];
        s32 acc = (s32)in - (s32)state_.x[0] + ((((s32)alpha_fixed_) * ((s32)state_.y[0])) >> Q2_14_SHIFT);
        s16 out = CLIP_S16(acc);
        state_.x[0] = in;
        state_.y[0] = out;
        in_buf[i]   = out;
    }
}

void DCRemoval::SetCoeffs(float alpha)
{
    alpha_       = alpha;
    alpha_fixed_ = FLOAT_TO_Q2_14(alpha);
}