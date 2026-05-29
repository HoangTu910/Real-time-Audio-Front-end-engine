#include "RTAFE_AP_plugin.hpp"

static RTAFE_Main_AP gRTAFE_Main_AP_Instance;
extern "C" {
void dspRTAFE_AP_plugin_init_triple_buf(u16 numSampleInBuffer)
{
    gRTAFE_Main_AP_Instance.dspRTAFE_InitBuffer(numSampleInBuffer);
}

void dspRTAFE_AP_plugin_process_buffer()
{
    gRTAFE_Main_AP_Instance.dspRTAFE_Process();
}
}
