#ifndef PRE_EMPHASIS_HPP
#define PRE_EMPHASIS_HPP

#include "idsp_module.hpp"

struct PreEmState {
    float x[2];
    float y[2];
};

class PreEmphasis : public IDSPModule {
public:
    PreEmphasis(float preEmphasisFactor = 0.97f);
    ~PreEmphasis() override = default;
    void vProcessBlock(DspBlock *dsp_block) override;
    void vProcessBlockFix(DspBlock *dsp_block) override;
    void vSetCoeffs(float preEmphasisFactor);
private:
    PreEmState m_state;
    float m_alpha;
    s32 m_alpha_fixed;
};

#endif /* PRE_EMPHASIS_HPP */