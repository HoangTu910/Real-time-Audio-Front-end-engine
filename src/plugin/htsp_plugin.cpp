#include "htsp_plugin.hpp"

HtspErrRet HTSPPlugin::ProcessDSPBlock(sample_t **in_buf, u16 num_channels)
{
    #ifdef FIXED_POINT
        return dsp_pipeline.ProcessFixed(in_buf, num_channels);
    #else
        return dsp_pipeline.Process(in_buf, num_channels);
    #endif
}
