#include "RTAFE_main_sp.hpp"

RTAFE_Main_SP::RTAFE_Main_SP()
{
    pBiquadFilterLPF     = new BiquadLPF();
    pBiquadFilterHPF     = new BiquadHPF();
    pBiquadFilterPeak    = new BiquadPeak();
    pBiquadFilterBPF     = new BiquadBPF();
    pBiquadFilterNotch   = new BiquadNotch();
    pBiquadFilterAllpass = new BiquadAllpass();
    pPreEmphasis         = new PreEmphasis();
    pDCRemoval           = new DCRemoval();
    pNoiseSuppress       = new NoiseSuppress();
}

RTAFE_Main_SP::~RTAFE_Main_SP()
{
    delete pBiquadFilterLPF;
    delete pBiquadFilterHPF;
    delete pBiquadFilterPeak;
    delete pBiquadFilterBPF;
    delete pBiquadFilterNotch;
    delete pBiquadFilterAllpass;
    delete pPreEmphasis;
    delete pDCRemoval;
    delete pNoiseSuppress;
    delete pBufferMng;
}

void RTAFE_Main_SP::dspRTAFE_vProcess()
{
    /* Always process the processing buffer — it was filled in a previous cycle.
     * This runs regardless of whether current input is ready. */
    TrplBufferStr *pProc = pBufferMng->ptrGetCurProcessingBuf();
    if (!pProc->isCompleted) {
        pPreEmphasis->vProcessBlock(pProc);
        pDCRemoval->vProcessBlock(pProc);
        pNoiseSuppress->vProcessBlock(pProc);
        pProc->isCompleted = true;
    }

    /* Only rotate when ALL three stages are done: */
    bool bInputReady     = pBufferMng->ptrGetCurInBuf()->isCompleted;
    bool bProcessedReady = pProc->isCompleted;
    bool bOutputConsumed = pBufferMng->ptrGetCurOutBuf()->isCompleted;

    if (bInputReady && bProcessedReady && bOutputConsumed) {
        pBufferMng->vRotateBuffers();
    }
}

void RTAFE_Main_SP::dspRTAFE_InitBuffer(u16 numSampleInBuffer)
{
    pBufferMng = new BufferMng(numSampleInBuffer);
}

tSample* RTAFE_Main_SP::dspRTAFE_pGetInputBuffer()
{
    return pBufferMng->ptrGetCurInBuf()->pBufferRef;
}

void RTAFE_Main_SP::dspRTAFE_vCompleteInput()
{
    pBufferMng->ptrGetCurInBuf()->isCompleted = true;
}

tSample* RTAFE_Main_SP::dspRTAFE_pGetOutputBuffer()
{
    return pBufferMng->ptrGetCurOutBuf()->pBufferRef;
}

void RTAFE_Main_SP::dspRTAFE_vCompleteOutput()
{
    pBufferMng->ptrGetCurOutBuf()->isCompleted = true;
}
