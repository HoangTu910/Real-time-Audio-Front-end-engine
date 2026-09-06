#include "htsp_plugin.hpp"

HTSPPlugin::HTSPPlugin()
{
    buffer_manager_.InitMemoryPool(BLOCK_SIZE * SIZE_OF_DSP_SAMPLE, NUM_BLOCKS);

    for (u16 channel = 0; channel < HTSP_MAX_CHANNELS; channel++) {
        channel_modules_[channel][0] = &dc_removal_modules_[channel];
        channel_modules_[channel][1] = &pre_emphasis_modules_[channel];
        channel_modules_[channel][2] = &noise_suppress_modules_[channel];
    }

    SetPipeline();
}

static int ChannelIndex(ChannelId channel_id)
{
    switch (channel_id) {
    case ChannelId::kLeftFront:  return 0;
    case ChannelId::kRightFront: return 1;
    case ChannelId::kLeftRear:   return 2;
    case ChannelId::kRightRear:  return 3;
    default:                     return -1;
    }
}

HtspErrRet HTSPPlugin::Process(sample_t **in_buf, u16 num_channels)
{
    if (in_buf == nullptr || (num_channels != 2 && num_channels != 4)) {
        return kErrorInvalidChannelCount;
    }

    const ChannelId channel_ids[4] = {
        ChannelId::kLeftFront,
        ChannelId::kRightFront,
        ChannelId::kLeftRear,
        ChannelId::kRightRear
    };
    DSPBlock *blocks[4] = {};

    for (u16 channel = 0; channel < num_channels; channel++) {
        blocks[channel] = static_cast<DSPBlock *>(buffer_manager_.Alloc());
        if (blocks[channel] == nullptr || in_buf[channel] == nullptr) {
            for (u16 allocated = 0; allocated <= channel; allocated++) {
                if (blocks[allocated] != nullptr) {
                    buffer_manager_.Free(blocks[allocated]);
                }
            }
            return blocks[channel] == nullptr ? kErrorMemoryAllocation
                                               : kErrorNullModuleParam;
        }
        blocks[channel]->SetDSPBlock(in_buf[channel],
                                     BLOCK_SIZE * SIZE_OF_DSP_SAMPLE,
                                     channel_ids[channel]);
    }

    HtspErrRet status = kOk;
    for (u16 channel = 0; channel < num_channels && status == kOk; channel++) {
        status = channel_pipelines_[channel].ProcessDSPPipeline(blocks[channel]);
    }

    for (u16 channel = 0; channel < num_channels; channel++) {
        buffer_manager_.Free(blocks[channel]);
    }

    return status;
}

HtspErrRet HTSPPlugin::ProcessFixed(sample_t **in_buf, u16 num_channels)
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

            HtspErrRet status = channel_pipelines_[i].ProcessDSPPipelineFixed(block);
            if (status != kOk) return status;
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

            HtspErrRet status = channel_pipelines_[i].ProcessDSPPipelineFixed(block);
            if (status != kOk) return status;
        }

        buffer_manager_.Free(block_channel_lf);
        buffer_manager_.Free(block_channel_rf);
        buffer_manager_.Free(block_channel_lr);
        buffer_manager_.Free(block_channel_rr);

        return kOk;
    }

    return kErrorInvalidChannelCount;
}

void HTSPPlugin::SetPipeline()
{
    IDSPModule *left_front_modules[] = {
        &dc_removal_modules_[0],
        &pre_emphasis_modules_[0],
        &noise_suppress_modules_[0]
    };
    SetChannelPipeline(ChannelId::kLeftFront, left_front_modules, 3);

    IDSPModule *right_front_modules[] = {
        &dc_removal_modules_[1],
        &pre_emphasis_modules_[1]
    };
    SetChannelPipeline(ChannelId::kRightFront, right_front_modules, 2);

    IDSPModule *left_rear_modules[] = {
        &dc_removal_modules_[2],
        &pre_emphasis_modules_[2],
        &noise_suppress_modules_[2]
    };
    SetChannelPipeline(ChannelId::kLeftRear, left_rear_modules, 3);

    IDSPModule *right_rear_modules[] = {
        &dc_removal_modules_[3],
        &noise_suppress_modules_[3]
    };
    SetChannelPipeline(ChannelId::kRightRear, right_rear_modules, 2);
}

HtspErrRet HTSPPlugin::SetChannelPipeline(ChannelId channel_id,
                                           IDSPModule *const *modules,
                                           u16 num_modules)
{
    int channel = ChannelIndex(channel_id);
    if (channel < 0 || modules == nullptr || num_modules > HTSP_MAX_MODULES) {
        return kErrorInvalidModuleParam;
    }

    for (u16 module = 0; module < num_modules; module++) {
        if (modules[module] == nullptr) return kErrorNullModuleParam;
        channel_modules_[channel][module] = modules[module];
    }

    for (u16 module = num_modules; module < HTSP_MAX_MODULES; module++) {
        channel_modules_[channel][module] = nullptr;
    }

    return channel_pipelines_[channel].ConfigDSPPipeline(channel_modules_[channel],
                                                         num_modules,
                                                         channel_id);
}

HtspErrRet HTSPPlugin::SetParams()
{
    /* I think this function should do some parsing from config file to get the parameters for each module */
    /* For now, let see manually */
    DSPModuleParams dc_params[1] = {0.995f};  /* DC removal alpha */
    DSPModuleParams pre_emphasis_params[1] = {0.97f};

    for (u16 channel = 0; channel < HTSP_MAX_CHANNELS; channel++) {
        HtspErrRet status = dc_removal_modules_[channel].SetParams(
            dc_params, DC_MODULE_PARAMS_COUNT);
        if (status != kOk) return status;

        status = pre_emphasis_modules_[channel].SetParams(
            pre_emphasis_params, PRE_EMPHASIS_MODULE_PARAMS_COUNT);
        if (status != kOk) return status;

        status = noise_suppress_modules_[channel].SetParams(nullptr, 0);
        if (status != kOk) return status;
    }

    return kOk;
}
