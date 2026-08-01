#include "dsp_pipeline.hpp"

void DSPPipeline::AddModule(IDSPModule *module)
{
    modules_.push_back(module);
}

void DSPPipeline::Process(DSPBlock *dsp_block)
{
    for (auto dsp_module : modules_) {
        dsp_module->ProcessBlock(dsp_block);
    }
}

void DSPPipeline::ProcessFixed(DSPBlock *dsp_block)
{
    for (auto dsp_module : modules_) {
        dsp_module->ProcessBlockFixed(dsp_block);
    }
}
