#ifndef BIQUAD_H
#define BIQUAD_H

#include "utils.h"
#include "BufferMng.hpp"

typedef enum {
    BIQUAD_LPF,
    BIQUAD_HPF,
    BIQUAD_BPF_PEAK,
    BIQUAD_BPF,
    BIQUAD_NOTCH,
    BIQUAD_ALLPASS,
} BiquadType;

struct BiquadCoeffs {
    float b0, b1, b2;
    float a1, a2;
};

struct BiquadState {
    float x1, x2;
    float y1, y2;
};

struct BiquadCoeffsFix {
    s32 b0, b1, b2;
    s32 a1, a2;
};

struct BiquadStateFix {
    s32 x1, x2;
    s32 y1, y2;
};

class IBiquadDesign {
public:
    virtual ~IBiquadDesign() = default;
    virtual void vDesign(float frequency, float QFactor, float sampleRate) = 0;
    virtual void vProcessBlock(TrplBufferStr *pProcessBuf);
    virtual void vProcessBlockFix(TrplBufferStr *pProcessBuf);
protected:
    BiquadCoeffs m_biquadCoeffs;
    BiquadState m_biquadState;
    BiquadCoeffsFix m_biquadCoeffsFix;
    BiquadStateFix m_biquadStateFix;
}; 

#endif /* BIQUAD_H */
