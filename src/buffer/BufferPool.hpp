#include <cstddef>
#include "utils.h"

struct BufferPool {
    size_t poolSize;
    size_t bufferSize;
    sample_t** pBuffer;
};

class BufferPool {
public:
    BufferPool(size_t bufferSize, size_t poolSize);
    ~BufferPool();

}