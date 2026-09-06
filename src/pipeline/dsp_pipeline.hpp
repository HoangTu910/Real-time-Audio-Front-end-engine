#ifndef DSP_PIPELINE_HPP
#define DSP_PIPELINE_HPP

#include "idsp_module.hpp"
#include "dc_removal.hpp"
#include "pre_emphasis.hpp"
#include "noise_suppress.hpp"
#include "errors_code.hpp"

class DSPPipeline {
public:
    DSPPipeline();
    ~DSPPipeline();
    
    HtspErrRet ConfigDSPPipeline(IDSPModule **list_of_modules,
                                 u16 num_modules,
                                 ChannelId channel_id);
    HtspErrRet ProcessDSPPipeline(DSPBlock *dsp_block);
    HtspErrRet ProcessDSPPipelineFixed(DSPBlock *dsp_block);

private:
    IDSPModule**  processing_chain_; /* array of pointers to DSP modules */
    u16           num_modules_;       /* number of modules in the processing chain */
    ChannelId     channel_id_;        /* channel ID for this pipeline */
};

#endif /* DSP_PIPELINE_HPP */