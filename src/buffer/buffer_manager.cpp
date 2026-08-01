#include "buffer_manager.hpp"

BufferManager::BufferManager()
{
    mem_pool_ = new MemoryPool();
}

BufferManager::~BufferManager()
{
    delete mem_pool_;
}

void BufferManager::InitMemoryPool(u16 block_size, u16 num_blocks)
{
    mem_pool_->block_size = block_size;
    mem_pool_->num_blocks = num_blocks;

    mem_pool_->memory = malloc(block_size * num_blocks);
    mem_pool_->free_list_buffer = nullptr;

    for(int i = 0; i < num_blocks; ++i) {
        FreeBuffer *buffer = (FreeBuffer*)((u8*)mem_pool_->memory + i * block_size);

        /* push_front(buffer) */
        buffer->next = mem_pool_->free_list_buffer;
        mem_pool_->free_list_buffer = buffer;
    }
}

void *BufferManager::Alloc()
{
    if(mem_pool_->free_list_buffer == nullptr) {
        return nullptr;
    }

    FreeBuffer *buffer = mem_pool_->free_list_buffer;
    mem_pool_->free_list_buffer = buffer->next;

    return (void*)buffer;
}

void BufferManager::Free(void *buffer)
{
    if(buffer == nullptr) {
        return;
    }

    FreeBuffer *free_buffer = (FreeBuffer*)buffer;

    /* push_front(free_buffer) */
    free_buffer->next = mem_pool_->free_list_buffer;
    mem_pool_->free_list_buffer = free_buffer;
}
