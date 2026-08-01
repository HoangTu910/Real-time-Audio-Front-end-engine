#ifndef RTAFE_MAIN_SP_HPP
#define RTAFE_MAIN_SP_HPP

#include "idsp_module.hpp"
#include "dsp_pipeline.hpp"
#include "utils.hpp"
#include "pre_emphasis.hpp"
#include "dc_removal.hpp"
#include "noise_suppress.hpp"

#define REQUIRED_BLOCK_SIZE 512

typedef sample_t tSample;

class RTAFE_DSPMain {
public:
    RTAFE_DSPMain()  = default;
    ~RTAFE_DSPMain() = default;
    
    /* main process function with full dsp pipeline*/
    RtafeErrRet ProcessDSPBlock(sample_t *in_buf);
private:
    DCRemoval     dc_removal_module_{0.995f};
    PreEmphasis   pre_emphasis_module_{0.97f};
    NoiseSuppress noise_suppress_module_;

    DSPPipeline   dsp_pipeline{&dc_removal_module_, 
                               &pre_emphasis_module_, 
                               &noise_suppress_module_};
};

#endif /* RTAFE_MAIN_SP_HPP */