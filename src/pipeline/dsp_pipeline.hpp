#ifndef DSP_PIPELINE_HPP
#define DSP_PIPELINE_HPP

#include <vector>
#include "idsp_module.hpp"

class DSPPipeline {
public:
    DSPPipeline();
    ~DSPPipeline();

    void AddModule(IDSPModule *module);
    void Process(sample_t *in_buf);
    void ProcessFixed(sample_t *in_buf);

private:
    std::vector<IDSPModule*> modules_;
    BufferPool               buffer_pool_;
};

#endif /* DSP_PIPELINE_HPP */