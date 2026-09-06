#include "dsp_pipeline.hpp"

DSPPipeline::DSPPipeline()
    : processing_chain_(nullptr)
    , num_modules_(0)
    , channel_id_(ChannelId::kMono)
{
}

DSPPipeline::~DSPPipeline()
{
}

HtspErrRet DSPPipeline::ConfigDSPPipeline(IDSPModule **list_of_modules,
                                          u16 num_modules,
                                          ChannelId channel_id)
{
    if(list_of_modules == nullptr || num_modules == 0) {
        return kErrorNullListOfModules;
    }

    this->processing_chain_ = list_of_modules;
    this->num_modules_      = num_modules;
    this->channel_id_       = channel_id;

    return kOk;
}

HtspErrRet DSPPipeline::ProcessDSPPipeline(DSPBlock *dsp_block)
{
    if(dsp_block == nullptr) {
        return kErrorNullModuleParam;
    }

    for(u16 i = 0; i < num_modules_; i++) {
        IDSPModule *module = processing_chain_[i];
        if(module == nullptr) {
            return kErrorNullModuleParam;
        }
        module->ProcessBlock(dsp_block);
    }

    return kOk;
}

HtspErrRet DSPPipeline::ProcessDSPPipelineFixed(DSPBlock *dsp_block)
{
    if (dsp_block == nullptr) {
        return kErrorNullModuleParam;
    }

    for (u16 i = 0; i < num_modules_; i++) {
        IDSPModule *module = processing_chain_[i];
        if (module == nullptr) {
            return kErrorNullModuleParam;
        }
        module->ProcessBlockFixed(dsp_block);
    }

    return kOk;
}
