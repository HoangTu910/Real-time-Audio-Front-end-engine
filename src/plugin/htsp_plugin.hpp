#ifndef HTSP_PLUGIN_HPP
#define HTSP_PLUGIN_HPP

#include "idsp_module.hpp"
#include "dsp_pipeline.hpp"
#include "utils.hpp"
#include "pre_emphasis.hpp"
#include "dc_removal.hpp"
#include "noise_suppress.hpp"

#define REQUIRED_BLOCK_SIZE 512

typedef sample_t tSample;

class HTSPPlugin {
public:
    HTSPPlugin()  = default;
    ~HTSPPlugin() = default;
    
    /* main process function with full dsp pipeline*/

    HtspErrRet ProcessDSPBlock(sample_t **in_buf, u16 num_channels);
private:
    DCRemoval     dc_removal_module_{0.995f};
    PreEmphasis   pre_emphasis_module_{0.97f};
    NoiseSuppress noise_suppress_module_;

    DSPPipeline   dsp_pipeline{&dc_removal_module_, 
                               &pre_emphasis_module_, 
                               &noise_suppress_module_};
};

#endif /* HTSP_PLUGIN_HPP */