/* RTAFE Speech Processing API */

#ifdef __cplusplus
extern "C" {
#endif

#include "rtafe_main_sp.hpp"

void dspRTAFE_SP_plugin_init_triple_buf(u16 numSampleInBuffer);
void dspRTAFE_SP_plugin_process_buffer();

#ifdef __cplusplus
}
#endif