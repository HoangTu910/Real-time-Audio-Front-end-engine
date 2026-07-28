/* RTAFE Audio Processing API */

#ifdef __cplusplus
extern "C" {
#endif

#include "rtafe_main_ap.hpp"

void dspRTAFE_AP_plugin_init_triple_buf(u16 numSampleInBuffer);
void dspRTAFE_AP_plugin_process_buffer();

#ifdef __cplusplus
}
#endif