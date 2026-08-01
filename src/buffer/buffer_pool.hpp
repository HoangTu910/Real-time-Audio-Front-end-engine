#ifndef BUFFER_POOL_HPP
#define BUFFER_POOL_HPP

#include "utils.hpp"

#define BLOCK_SIZE 512
#define NUM_BLOCKS 10

typedef struct FreeBuffer {
    // Points to the next free buffer in the pool.
    FreeBuffer *next = nullptr;
} FreeBuffer;

typedef struct MemoryPool {
    void       *memory;
    FreeBuffer *free_list_buffer;
    u16         block_size;
    u16         num_blocks;
} MemoryPool;

class BufferPool {
public:
    BufferPool();
    ~BufferPool();

    void  InitMemoryPool(u16 block_size, u16 num_blocks);
    void* Alloc();
    void  Free(void *buffer);
private:
    MemoryPool *mem_pool_;
};

#endif /* BUFFER_POOL_HPP */