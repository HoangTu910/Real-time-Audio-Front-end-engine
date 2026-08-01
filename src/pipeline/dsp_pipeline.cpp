#include "dsp_pipeline.hpp"

DSPPipeline::DSPPipeline(IDSPModule *dc_removal, IDSPModule *pre_emphasis, IDSPModule *noise_suppress)
{
    this->dc_removal_module_     = dc_removal;
    this->pre_emphasis_module_   = pre_emphasis;
    this->noise_suppress_module_ = noise_suppress;

    buffer_manager_.InitMemoryPool(BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, NUM_BLOCKS);
}

DSPPipeline::~DSPPipeline()
{
}

RtafeErrRet DSPPipeline::Process(sample_t *in_buf)
{
    // #ifdef DSP_PIPELINE_AUDIO_FOUR_CHANNELS
    DSPBlock *block_channel_lf = (DSPBlock*)buffer_manager_.Alloc();
    DSPBlock *block_channel_rf = (DSPBlock*)buffer_manager_.Alloc();
    DSPBlock *block_channel_lr = (DSPBlock*)buffer_manager_.Alloc();
    DSPBlock *block_channel_rr = (DSPBlock*)buffer_manager_.Alloc();

    if( block_channel_lf == nullptr || block_channel_rf == nullptr || 
        block_channel_lr == nullptr || block_channel_rr == nullptr) {
        return kErrorMemoryAllocation;
    }

    block_channel_lf->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftFront);
    block_channel_rf->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightFront);
    block_channel_lr->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftRear);
    block_channel_rr->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightRear);

    DSPBlock *block_container[4] ={block_channel_lf, 
                                   block_channel_rf, 
                                   block_channel_lr, 
                                   block_channel_rr};
    for(int i = 0; i < 4; i++) {
        DSPBlock *block = block_container[i];
        dc_removal_module_->ProcessBlock(block);
        pre_emphasis_module_->ProcessBlock(block);
        noise_suppress_module_->ProcessBlock(block);
    }

    buffer_manager_.Free(block_channel_lf);
    buffer_manager_.Free(block_channel_rf);
    buffer_manager_.Free(block_channel_lr);
    buffer_manager_.Free(block_channel_rr);
    // #endif
    
    // #ifdef DSP_PIPELINE_SPEECH_PRE_PROCESSING
    DSPBlock *block_channel_mono = (DSPBlock*)buffer_manager_.Alloc();
    if( block_channel_mono == nullptr) return kErrorMemoryAllocation;
    
    block_channel_mono->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kMono);

    dc_removal_module_->ProcessBlock(block_channel_mono);
    pre_emphasis_module_->ProcessBlock(block_channel_mono);
    noise_suppress_module_->ProcessBlock(block_channel_mono);

    buffer_manager_.Free(block_channel_mono);
    // #endif
}

RtafeErrRet DSPPipeline::ProcessFixed(sample_t *in_buf)
{
    DSPBlock *block_channel_lf = (DSPBlock*)buffer_manager_.Alloc();
    DSPBlock *block_channel_rf = (DSPBlock*)buffer_manager_.Alloc();
    DSPBlock *block_channel_lr = (DSPBlock*)buffer_manager_.Alloc();
    DSPBlock *block_channel_rr = (DSPBlock*)buffer_manager_.Alloc();

    if( block_channel_lf == nullptr || block_channel_rf == nullptr || 
        block_channel_lr == nullptr || block_channel_rr == nullptr) {
        return kErrorMemoryAllocation;
    }

    block_channel_lf->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftFront);
    block_channel_rf->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightFront);
    block_channel_lr->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftRear);
    block_channel_rr->SetDSPBlock(in_buf, BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightRear);

    DSPBlock *block_container[4] ={block_channel_lf, 
                                   block_channel_rf, 
                                   block_channel_lr, 
                                   block_channel_rr};
    for(int i = 0; i < 4; i++) {
        DSPBlock *block = block_container[i];

        dc_removal_module_->ProcessBlockFixed(block);
        pre_emphasis_module_->ProcessBlockFixed(block);
        noise_suppress_module_->ProcessBlockFixed(block);
    }

    buffer_manager_.Free(block_channel_lf);
    buffer_manager_.Free(block_channel_rf);
    buffer_manager_.Free(block_channel_lr);
    buffer_manager_.Free(block_channel_rr);

    return kOk;
}