#ifndef RTAFE_MAIN_SP_HPP
#define RTAFE_MAIN_SP_HPP

#include "buffer_pool.hpp"
#include "biquad_filter.hpp"
#include "pre_emphasis.hpp"
#include "idsp_module.hpp"
#include "dc_removal.hpp"
#include "utils.hpp"

typedef sample_t tSample;

/* signal processing pipeline */
class BiquadLPF;
class BiquadHPF;
class BiquadPeak;
class BiquadBPF;
class BiquadNotch;
class BiquadAllpass;
class PreEmphasis;
class DCRemoval;
class NoiseSuppress;
class BufferMng;

class RTAFE_Main_SP {
public:
    RTAFE_Main_SP();
    ~RTAFE_Main_SP();

    void dspRTAFE_InitBuffer(u16 numSampleInBuffer);

    tSample* dspRTAFE_pGetInputBuffer();
    void dspRTAFE_vCompleteInput();
    tSample* dspRTAFE_pGetOutputBuffer();
    void dspRTAFE_vCompleteOutput();

    /* main process function with full dsp pipeline*/
    void dspRTAFE_vProcess();
private:
    BiquadLPF       *pBiquadFilterLPF     = nullptr;
    BiquadHPF       *pBiquadFilterHPF     = nullptr;
    BiquadPeak      *pBiquadFilterPeak    = nullptr;
    BiquadBPF       *pBiquadFilterBPF     = nullptr;
    BiquadNotch     *pBiquadFilterNotch   = nullptr;
    BiquadAllpass   *pBiquadFilterAllpass = nullptr;
    PreEmphasis     *pPreEmphasis         = nullptr;
    DCRemoval       *pDCRemoval           = nullptr;
    NoiseSuppress   *pNoiseSuppress       = nullptr;

    BufferMng       *pBufferMng           = nullptr;
};

#endif /* RTAFE_MAIN_SP_HPP */