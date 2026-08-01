#include "rtafe_main.hpp"

RtafeErrRet RTAFE_DSPMain::ProcessDSPBlock(sample_t *in_buf)
{
    #ifdef FIXED_POINT
        dsp_pipeline.ProcessFixed(in_buf);
    #else
        dsp_pipeline.Process(in_buf);
    #endif
}