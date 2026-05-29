#ifndef RTAFE_MAIN_AP_HPP
#define RTAFE_MAIN_AP_HPP

#include "BufferMng.hpp"
#include "BiquadFilter.hpp"
#include "utils.h"

/* signal processing pipeline */
class BiquadLPF;
class BiquadHPF;
class BiquadPeak;
class BiquadBPF;
class BiquadNotch;
class BiquadAllpass;

class RTAFE_Main_AP {
private:
    BiquadLPF       *pBiquadFilterLPF       = nullptr;
    BiquadHPF       *pBiquadFilterHPF       = nullptr;
    BiquadPeak      *pBiquadFilterPeak      = nullptr;
    BiquadBPF       *pBiquadFilterBPF       = nullptr;
    BiquadNotch     *pBiquadFilterNotch     = nullptr;
    BiquadAllpass   *pBiquadFilterAllpass   = nullptr;

public:
    RTAFE_Main_AP();
    ~RTAFE_Main_AP();
    void dspRTAFE_Process();
    void dspRTAFE_InitBuffer(u16 numSampleInBuffer);
};

#endif /* RTAFE_MAIN_AP_HPP */