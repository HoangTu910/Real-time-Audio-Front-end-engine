#include "rtafe_main.hpp"

RTAFE_DSPMain::RTAFE_DSPMain()
{
    dsp_pipeline = new DSPPipeline();
}

RTAFE_DSPMain::~RTAFE_DSPMain()
{
    delete dsp_pipeline;
}

void RTAFE_DSPMain::ProcessDSPBlock(DSPBlock *dsp_block)
{
    #ifdef FIXED_POINT
        dsp_pipeline->ProcessFixed(dsp_block);
    #else
        dsp_pipeline->Process(dsp_block);
    #endif
}