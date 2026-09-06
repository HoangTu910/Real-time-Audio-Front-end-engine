#ifndef HTSP_PLUGIN_HPP
#define HTSP_PLUGIN_HPP

#include "idsp_module.hpp"
#include "dsp_pipeline.hpp"
#include "utils.hpp"
#include "pre_emphasis.hpp"
#include "dc_removal.hpp"
#include "noise_suppress.hpp"

#define REQUIRED_BLOCK_SIZE 512
#define HTSP_MAX_CHANNELS 4
#define HTSP_MAX_MODULES 3

typedef sample_t tSample;

struct DSPBlockChannel {
    DSPBlock *block;
    ChannelId channel_id;
};

class HTSPPlugin {
public:
    HTSPPlugin();
    ~HTSPPlugin() = default;
    
    /* main process function with full dsp pipeline*/
    void       SetPipeline();
    HtspErrRet SetChannelPipeline(ChannelId channel_id,
                                  IDSPModule *const *modules,
                                  u16 num_modules);
    HtspErrRet SetParams();
    HtspErrRet Process(sample_t **in_buf, u16 num_channels);
    HtspErrRet ProcessFixed(sample_t **in_buf, u16 num_channels);

private:
    DCRemoval     dc_removal_modules_[HTSP_MAX_CHANNELS];
    PreEmphasis   pre_emphasis_modules_[HTSP_MAX_CHANNELS];
    NoiseSuppress noise_suppress_modules_[HTSP_MAX_CHANNELS];
    IDSPModule   *channel_modules_[HTSP_MAX_CHANNELS][HTSP_MAX_MODULES];
    BufferManager buffer_manager_;
    DSPPipeline   channel_pipelines_[HTSP_MAX_CHANNELS];
};

#endif /* HTSP_PLUGIN_HPP */