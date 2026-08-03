#ifndef DSP_BLOCK_HPP
#define DSP_BLOCK_HPP

#include "utils.hpp"
#include "buffer_manager.hpp"

class DSPBlock {
public:
    DSPBlock()  = default;
    ~DSPBlock() = default;
    void SetDSPBlock(sample_t *buffer, u16 block_size_bytes, u16 channel_id);

    sample_t *GetDSPBuffer();
    u16       GetBlockSizeBytes();
    u16       GetBlockSize();
    u16       GetChannelId();
private:
    sample_t    *dsp_buffer_;
    u16         block_size_;
    u16         block_size_bytes_;
    u16         channel_id_;
};

#endif /* DSP_BLOCK_HPP */