#include "dsp_block.hpp"

void DSPBlock::SetDSPBlock(sample_t *buffer, u16 block_size, u16 channel_id)
{
    this->dsp_buffer_  = buffer;
    this->block_size_  = block_size;
    this->channel_id_  = channel_id;
}

sample_t *DSPBlock::GetDSPBuffer()
{
    return this->dsp_buffer_;
}

u16 DSPBlock::GetBlockSize()
{
    return this->block_size_;
}

u16 DSPBlock::GetChannelId()
{
    return this->channel_id_;
}
