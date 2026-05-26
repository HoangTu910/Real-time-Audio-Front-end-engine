#include "RTAFE_main_sp.hpp"

RTAFE_Main_SP::RTAFE_Main_SP()
{
    pBiquadFilterLPF     = new BiquadLPF();
    pBiquadFilterHPF     = new BiquadHPF();
    pBiquadFilterPeak    = new BiquadPeak();
    pBiquadFilterBPF     = new BiquadBPF();
    pBiquadFilterNotch   = new BiquadNotch();
    pBiquadFilterAllpass = new BiquadAllpass();
}

RTAFE_Main_SP::~RTAFE_Main_SP()
{
    delete pBiquadFilterLPF;
    delete pBiquadFilterHPF;
    delete pBiquadFilterPeak;
    delete pBiquadFilterBPF;
    delete pBiquadFilterNotch;
    delete pBiquadFilterAllpass;
}

void RTAFE_Main_SP::dspRTAFE_Process(TrplBufferStr *pProcessBuf)
{
}

void RTAFE_Main_SP::dspRTAFE_InitBuffer(u16 numSampleInBuffer)
{
}
