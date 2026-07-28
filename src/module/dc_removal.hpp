#ifndef DC_REMOVAL_HPP
#define DC_REMOVAL_HPP

#include "idsp_module.hpp"
#include "utils.hpp"
#include "buffer_pool.hpp"

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

    void vProcessBlock(DspBlock *dsp_block) override;
    void vProcessBlockFix(DspBlock *dsp_block) override;
    void vSetCoeffs(float alpha);

private:
    float m_alpha;
    s32 m_alpha_fixed;
    DCRemovalState m_state;
};

#endif /* DC_REMOVAL_HPP */