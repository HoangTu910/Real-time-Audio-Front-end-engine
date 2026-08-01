#include "dsp_pipeline.hpp"

DSPPipeline::DSPPipeline()
{
    buffer_pool_.InitMemoryPool(BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, NUM_BLOCKS);
}

DSPPipeline::~DSPPipeline()
{
}

void DSPPipeline::AddModule(IDSPModule *module)
{
    modules_.push_back(module);
}

void DSPPipeline::Process(sample_t *in_buf)
{
    /* just an example of four channels processing */
    DSPBlock *block_channel_lf = (DSPBlock*)buffer_pool_.Alloc();
    DSPBlock *block_channel_rf = (DSPBlock*)buffer_pool_.Alloc();
    DSPBlock *block_channel_lr = (DSPBlock*)buffer_pool_.Alloc();
    DSPBlock *block_channel_rr = (DSPBlock*)buffer_pool_.Alloc();

    block_channel_lf->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftFront);
    block_channel_rf->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightFront);
    block_channel_lr->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftRear);
    block_channel_rr->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightRear);

    for (auto dsp_module : modules_) {
        dsp_module->ProcessBlock(block_channel_lf);
        dsp_module->ProcessBlock(block_channel_rf);
        dsp_module->ProcessBlock(block_channel_lr);
        dsp_module->ProcessBlock(block_channel_rr);
    }
}

void DSPPipeline::ProcessFixed(sample_t *in_buf)
{
    DSPBlock *block_channel_lf = (DSPBlock*)buffer_pool_.Alloc();
    DSPBlock *block_channel_rf = (DSPBlock*)buffer_pool_.Alloc();
    DSPBlock *block_channel_lr = (DSPBlock*)buffer_pool_.Alloc();
    DSPBlock *block_channel_rr = (DSPBlock*)buffer_pool_.Alloc();

    block_channel_lf->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftFront);
    block_channel_rf->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightFront);
    block_channel_lr->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftRear);
    block_channel_rr->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightRear);

    for (auto dsp_module : modules_) {
        dsp_module->ProcessBlockFixed(block_channel_lf);
        dsp_module->ProcessBlockFixed(block_channel_rf);
        dsp_module->ProcessBlockFixed(block_channel_lr);
        dsp_module->ProcessBlockFixed(block_channel_rr);
    }
}
