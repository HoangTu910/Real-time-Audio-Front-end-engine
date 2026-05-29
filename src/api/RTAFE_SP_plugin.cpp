#include "RTAFE_main_sp.hpp"
#include "RTAFE_SP_plugin.hpp"

static RTAFE_Main_SP gRTAFE_Main_SP_Instance;

extern "C" {

void dspRTAFE_SP_plugin_init_triple_buf(u16 numSampleInBuffer)
{
    gRTAFE_Main_SP_Instance.dspRTAFE_InitBuffer(numSampleInBuffer);
}

void dspRTAFE_SP_plugin_process_buffer()
{
    gRTAFE_Main_SP_Instance.dspRTAFE_Process();
}

}
