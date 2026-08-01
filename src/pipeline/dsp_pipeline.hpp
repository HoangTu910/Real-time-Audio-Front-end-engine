#ifndef DSP_PIPELINE_HPP
#define DSP_PIPELINE_HPP

#include <vector>
#include "idsp_module.hpp"

class DSPPipeline {
public:
    DSPPipeline() = default;
    ~DSPPipeline() = default;

    void AddModule(IDSPModule *module);
    void Process(DSPBlock *dsp_block);
    void ProcessFixed(DSPBlock *dsp_block);

private:
    std::vector<IDSPModule*> modules_;
};

#endif /* DSP_PIPELINE_HPP */