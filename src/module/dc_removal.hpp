#ifndef DC_REMOVAL_HPP
#define DC_REMOVAL_HPP

#include "IDSPModule.hpp"
#include "utils.h"
#include "BufferMng.hpp"

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

    void vProcessBlock(TrplBufferStr *pBuf) override;
    void vProcessBlockFix(TrplBufferStr *pBuf) override;
    void vSetCoeffs(float alpha);

private:
    float m_alpha;
    s32 m_alpha_fixed;
    DCRemovalState m_state;
};

#endif /* DC_REMOVAL_HPP */