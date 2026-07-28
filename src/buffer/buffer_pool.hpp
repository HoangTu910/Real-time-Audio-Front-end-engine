#ifndef BUFFER_POOL_HPP
#define BUFFER_POOL_HPP

#include "utils.hpp"

typedef struct FreeBuffer{
    FreeBuffer *next;
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

    void InitMemoryPool(u16 block_size, u16 num_blocks);
private:
    MemoryPool *mem_pool_;
};

#endif /* BUFFER_POOL_HPP */