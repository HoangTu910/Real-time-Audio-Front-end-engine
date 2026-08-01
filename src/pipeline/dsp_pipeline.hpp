#ifndef DSP_PIPELINE_HPP
#define DSP_PIPELINE_HPP

#include "idsp_module.hpp"
#include "dc_removal.hpp"
#include "pre_emphasis.hpp"
#include "noise_suppress.hpp"
#include "errors_code.hpp"

class DSPPipeline {
public:
    DSPPipeline(IDSPModule *dc_removal,
                IDSPModule *pre_emphasis,
                IDSPModule *noise_suppress);

    ~DSPPipeline();

    RtafeErrRet Process(sample_t *in_buf);
    RtafeErrRet ProcessFixed(sample_t *in_buf);

private:
    BufferManager  buffer_manager_;

    IDSPModule*    dc_removal_module_;
    IDSPModule*    pre_emphasis_module_;
    IDSPModule*    noise_suppress_module_;
};

#endif /* DSP_PIPELINE_HPP */