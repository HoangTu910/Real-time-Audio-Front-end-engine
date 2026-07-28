#ifndef BIQUAD_H
#define BIQUAD_H

#include "utils.h"
#include "buffer_mng.hpp"

typedef enum {
    BIQUAD_LPF,
    BIQUAD_HPF,
    BIQUAD_BPF_PEAK,
    BIQUAD_BPF,
    BIQUAD_NOTCH,
    BIQUAD_ALLPASS,
} BiquadType;

struct BiquadCoeffs {
    tFloat b0, b1, b2;
    tFloat a1, a2;
};

struct BiquadState {
    tFloat x1, x2;
    tFloat y1, y2;
};

struct BiquadCoeffsFixed {
    tFixed b0, b1, b2;
    tFixed a1, a2;
};

struct BiquadStateFixed {
    tFixed x1, x2;
    tFixed y1, y2;
};

class IBiquadDesign {
public:
    virtual ~IBiquadDesign() = default;

    virtual void Design(float frequency, float QFactor, float sampleRate) = 0;
    virtual void ProcessBlock(TrplBufferStr *pProcessBuf);
    virtual void ProcessBlockFixed(TrplBufferStr *pProcessBuf);
protected:
    BiquadCoeffs      biquad_coeffs_;
    BiquadState       biquad_state_;
    BiquadCoeffsFixed biquad_coeffs_fixed_;
    BiquadStateFixed  biquad_state_fixed_;
}; 

#endif /* BIQUAD_H */
