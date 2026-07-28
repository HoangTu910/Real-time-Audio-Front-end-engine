#include "buffer_pool.hpp"

BufferPool::BufferPool()
{
    mem_pool_ = new MemoryPool();
}

void BufferPool::InitMemoryPool(u16 block_size, u16 num_blocks)
{
    mem_pool_->block_size = block_size;
    mem_pool_->num_blocks = num_blocks;

    mem_pool_->memory = malloc(block_size * num_blocks);
    mem_pool_->free_list_buffer = nullptr;

    for(int i = 0; i < num_blocks; ++i) {
        FreeBuffer *buffer = (FreeBuffer*)((u8*)mem_pool_->memory + i * block_size);
        buffer->next = mem_pool_->free_list_buffer;
        mem_pool_->free_list_buffer = buffer;
    }
}