#ifndef DSP_BLOCK_HPP
#define DSP_BLOCK_HPP

#include "utils.hpp"
#include "buffer_pool.hpp"

class DSPBlock {
public:
    DSPBlock();
    ~DSPBlock();
    void SetDSPBlock(sample_t *buffer, u16 block_size, u16 channel_id);

    sample_t *GetDSPBuffer();
    u16       GetBlockSize();
    u16       GetChannelId();
private:
    sample_t    *dsp_buffer_;
    u16         block_size_;
    u16         channel_id_;
};

#endif /* DSP_BLOCK_HPP */