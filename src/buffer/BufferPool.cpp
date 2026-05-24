#include "BufferPool.hpp"

BufferPool::BufferPool()
{
    initializeBufferPool();
}

BufferPool::~BufferPool()
{
    for (size_t i = 0; i < NUM_BUFFER_CONFIGS; ++i) {
        for (size_t j = 0; j < bufferPoolConfigs[i].poolSize; ++j) {
            delete[] bufferPoolConfigs[i].pBuffers[j];
        }
        delete[] bufferPoolConfigs[i].pBuffers;
        delete[] bufferPoolConfigs[i].bufferInUse;
    }
}

BufferData *BufferPool::getBufferFromPool(BufferData &requestedBuffer)
{
    for (size_t i = 0; i < NUM_BUFFER_CONFIGS; ++i) {
        if (bufferPoolConfigs[i].bufferSize >= requestedBuffer.numSamples) {
            for (size_t j = 0; j < bufferPoolConfigs[i].poolSize; ++j) {
                if (!bufferPoolConfigs[i].bufferInUse[j]) {
                    bufferPoolConfigs[i].bufferInUse[j] = true;
                    requestedBuffer.pBuffer = bufferPoolConfigs[i].pBuffers[j];
                    requestedBuffer.numSamples = bufferPoolConfigs[i].bufferSize;
                    return &requestedBuffer;
                }
            }
        }
    }
    return nullptr; // No available buffer found
}

void BufferPool::releaseBuffer(BufferData *bufferData)
{
    for (size_t i = 0; i < NUM_BUFFER_CONFIGS; ++i) {
        for (size_t j = 0; j < bufferPoolConfigs[i].poolSize; ++j) {
            if (bufferPoolConfigs[i].pBuffers[j] == bufferData->pBuffer) {
                bufferPoolConfigs[i].bufferInUse[j] = false;
                return;
            }
        }
    }
}

void BufferPool::initializeBufferPool()
{
    for (size_t i = 0; i < NUM_BUFFER_CONFIGS; ++i) {
        bufferPoolConfigs[i].bufferSize = listOfSizeOfBufferPool[i];
        bufferPoolConfigs[i].poolSize = (MAX_POOL_SIZE_KB * 1024) / (bufferPoolConfigs[i].bufferSize * sizeof(sample_t));
        bufferPoolConfigs[i].pBuffers = new sample_t*[bufferPoolConfigs[i].poolSize];
        bufferPoolConfigs[i].bufferInUse = new bool[bufferPoolConfigs[i].poolSize]();
        for (size_t j = 0; j < bufferPoolConfigs[i].poolSize; ++j) {
            bufferPoolConfigs[i].pBuffers[j] = new sample_t[bufferPoolConfigs[i].bufferSize];
            bufferPoolConfigs[i].bufferInUse[j] = false;
        }
    }
}