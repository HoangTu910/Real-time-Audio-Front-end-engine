#ifndef DC_REMOVAL_HPP
#define DC_REMOVAL_HPP

#include "idsp_module.hpp"
#include "utils.hpp"
#include "dsp_block.hpp"
#include "errors_code.hpp"

/** Direct form I from Richard Lyons */
/** Difference equation for Direct Form I
 * y[n] = x[n] - x[n - 1] + alpha * y[n - 1]
 */

typedef struct {
    float x[2];
    float y[2];
} DCRemovalState;

class DCRemoval : public IDSPModule {
public:
    DCRemoval(float alpha = 0.995f);
    ~DCRemoval();

    HtspErrRet SetParams(const DSPModuleParams *params,
                         u16 param_count) override;
    void ProcessBlock(DSPBlock *dsp_block) override;
    void ProcessBlockFixed(DSPBlock *dsp_block) override;

    void SetCoeffs(float alpha);

private:
    float          alpha_;
    s32            alpha_fixed_;
    DCRemovalState state_;
};

#endif /* DC_REMOVAL_HPP */