#ifndef RTAFE_MAIN_SP_HPP
#define RTAFE_MAIN_SP_HPP

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

class RTAFE_Main_SP {
private:
    BiquadLPF       *pBiquadFilterLPF     = nullptr;
    BiquadHPF       *pBiquadFilterHPF     = nullptr;
    BiquadPeak      *pBiquadFilterPeak    = nullptr;
    BiquadBPF       *pBiquadFilterBPF     = nullptr;
    BiquadNotch     *pBiquadFilterNotch   = nullptr;
    BiquadAllpass   *pBiquadFilterAllpass = nullptr;

public:
    RTAFE_Main_SP();
    ~RTAFE_Main_SP();

    /* signal processing functions, containing dsp pipeline */
    void dspRTAFE_Process(TrplBufferStr *pProcessBuf);

    /* list of init functions */
    void dspRTAFE_InitBuffer(u16 numSampleInBuffer);
};

#endif /* RTAFE_MAIN_SP_HPP */