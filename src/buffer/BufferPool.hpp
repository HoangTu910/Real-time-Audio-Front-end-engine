#include <cstddef>
#include "utils.h"

/* how to create a buffer pool that allow both floating point and fixed point?*/
/* maybe we can use templates, first we need to define the sample type 
   and then, for each sample type, we can create a specialized buffer pool */

#define MAX_POOL_SIZE_KB 256
#define NUM_BUFFER_CONFIGS 8

uint32_t listOfSizeOfBufferPool[NUM_BUFFER_CONFIGS] = {128, 192, 256, 384, 512, 768, 1024, 1536};
/* now we need to populate the buffer pool with the specified buffer size and pool size */
/* each buffer will be of size bufferSize, bufferSize can be 128, 192, 256, 384, 512, 768, 1024, 1536 */

struct BufferPoolConfig {
    size_t bufferSize;
    size_t poolSize;
    sample_t** pBuffers; // array of pointers to buffers
    bool* bufferInUse; // array to track if a buffer is in use
};

struct BufferData {
    sample_t *pBuffer; // pointer to the actual buffer
    size_t numSamples; // size of the buffer
    size_t numChannels; // number of channels
    float sampleRate; // sample rate of the audio data
    bool inUse; // flag to indicate if the buffer is currently in use
};

class BufferPool {
public:
    BufferPool();
    ~BufferPool();
    BufferData* getBufferFromPool(BufferData &requestedBuffer);
    void releaseBuffer(BufferData* bufferData);
private:
    BufferPoolConfig bufferPoolConfigs[NUM_BUFFER_CONFIGS];
    void initializeBufferPool();
};