#include "RTAFE_main_ap.hpp"

RTAFE_Main_AP::RTAFE_Main_AP()
{
    pBiquadFilterLPF     = new BiquadLPF();
    pBiquadFilterHPF     = new BiquadHPF();
    pBiquadFilterPeak    = new BiquadPeak();
    pBiquadFilterBPF     = new BiquadBPF();
    pBiquadFilterNotch   = new BiquadNotch();
    pBiquadFilterAllpass = new BiquadAllpass();
}

RTAFE_Main_AP::~RTAFE_Main_AP()
{
    delete pBiquadFilterLPF;
    delete pBiquadFilterHPF;
    delete pBiquadFilterPeak;
    delete pBiquadFilterBPF;
    delete pBiquadFilterNotch;
    delete pBiquadFilterAllpass;
}
void RTAFE_Main_AP::dspRTAFE_Process(TrplBufferStr *pProcessBuf)
{
}

void RTAFE_Main_AP::dspRTAFE_InitBuffer(u16 numSampleInBuffer)
{
}
