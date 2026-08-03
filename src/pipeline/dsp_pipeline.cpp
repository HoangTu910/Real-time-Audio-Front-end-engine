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

HtspErrRet DSPPipeline::Process(sample_t **in_buf, u16 num_channels)
{
    if(num_channels == 2) {
        DSPBlock *block_channel_lf = (DSPBlock*)buffer_manager_.Alloc();
        DSPBlock *block_channel_rf = (DSPBlock*)buffer_manager_.Alloc();

        DSPBlock *block_container[2] ={block_channel_lf, 
                                       block_channel_rf};

        if( block_channel_lf == nullptr || block_channel_rf == nullptr) {
            return kErrorMemoryAllocation;
        }

        block_channel_lf->SetDSPBlock(in_buf[0], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftFront);
        block_channel_rf->SetDSPBlock(in_buf[1], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightFront);

        for(int i = 0; i < 2; i++) {
            DSPBlock *block = block_container[i];

            dc_removal_module_->ProcessBlock(block);
            pre_emphasis_module_->ProcessBlock(block);
            noise_suppress_module_->ProcessBlock(block);
        }

        buffer_manager_.Free(block_channel_lf);
        buffer_manager_.Free(block_channel_rf);

        return kOk;
    }

    if(num_channels == 4) {
        DSPBlock *block_channel_lf = (DSPBlock*)buffer_manager_.Alloc();
        DSPBlock *block_channel_rf = (DSPBlock*)buffer_manager_.Alloc();
        DSPBlock *block_channel_lr = (DSPBlock*)buffer_manager_.Alloc();
        DSPBlock *block_channel_rr = (DSPBlock*)buffer_manager_.Alloc();

        DSPBlock *block_container[4] ={block_channel_lf, 
                                       block_channel_rf, 
                                       block_channel_lr, 
                                       block_channel_rr};

        if( block_channel_lf == nullptr || block_channel_rf == nullptr || 
            block_channel_lr == nullptr || block_channel_rr == nullptr) {
            return kErrorMemoryAllocation;
        }

        block_channel_lf->SetDSPBlock(in_buf[0], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftFront);
        block_channel_rf->SetDSPBlock(in_buf[1], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightFront);
        block_channel_lr->SetDSPBlock(in_buf[2], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftRear);
        block_channel_rr->SetDSPBlock(in_buf[3], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightRear);

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

        return kOk;
    }

    return kErrorInvalidChannelCount;
}

HtspErrRet DSPPipeline::ProcessFixed(sample_t **in_buf, u16 num_channels)
{
    if(num_channels == 2) {
        DSPBlock *block_channel_lf = (DSPBlock*)buffer_manager_.Alloc();
        DSPBlock *block_channel_rf = (DSPBlock*)buffer_manager_.Alloc();

        DSPBlock *block_container[2] ={block_channel_lf, 
                                       block_channel_rf};

        if( block_channel_lf == nullptr || block_channel_rf == nullptr) {
            return kErrorMemoryAllocation;
        }

        block_channel_lf->SetDSPBlock(in_buf[0], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftFront);
        block_channel_rf->SetDSPBlock(in_buf[1], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightFront);

        for(int i = 0; i < 2; i++) {
            DSPBlock *block = block_container[i];

            dc_removal_module_->ProcessBlockFixed(block);
            pre_emphasis_module_->ProcessBlockFixed(block);
            noise_suppress_module_->ProcessBlockFixed(block);
        }

        buffer_manager_.Free(block_channel_lf);
        buffer_manager_.Free(block_channel_rf);

        return kOk;
    }

    if(num_channels == 4) {
        DSPBlock *block_channel_lf = (DSPBlock*)buffer_manager_.Alloc();
        DSPBlock *block_channel_rf = (DSPBlock*)buffer_manager_.Alloc();
        DSPBlock *block_channel_lr = (DSPBlock*)buffer_manager_.Alloc();
        DSPBlock *block_channel_rr = (DSPBlock*)buffer_manager_.Alloc();

        DSPBlock *block_container[4] ={block_channel_lf, 
                                       block_channel_rf, 
                                       block_channel_lr, 
                                       block_channel_rr};

        if( block_channel_lf == nullptr || block_channel_rf == nullptr || 
            block_channel_lr == nullptr || block_channel_rr == nullptr) {
            return kErrorMemoryAllocation;
        }

        block_channel_lf->SetDSPBlock(in_buf[0], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftFront);
        block_channel_rf->SetDSPBlock(in_buf[1], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightFront);
        block_channel_lr->SetDSPBlock(in_buf[2], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kLeftRear);
        block_channel_rr->SetDSPBlock(in_buf[3], BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, ChannelId::kRightRear);

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

    return kErrorInvalidChannelCount;
}
