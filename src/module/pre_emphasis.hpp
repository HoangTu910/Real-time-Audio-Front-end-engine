#ifndef PRE_EMPHASIS_HPP
#define PRE_EMPHASIS_HPP

#include "idsp_module.hpp"

struct PreEmState {
    float x[2];
    float y[2];
};

class PreEmphasis : public IDSPModule {
public:
    PreEmphasis(float pre_emphasis_factor = 0.97f);
    ~PreEmphasis() override = default;

    void ProcessBlock(DSPBlock *dsp_block) override;
    void ProcessBlockFixed(DSPBlock *dsp_block) override;

    void SetCoeffs(float pre_emphasis_factor);
private:
    PreEmState state_;
    float      alpha_;
    s32        alpha_fixed_;
};

#endif /* PRE_EMPHASIS_HPP */