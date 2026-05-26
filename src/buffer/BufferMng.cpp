#include "BufferMng.hpp"

BufferMng::BufferMng(u16 numSampleInBuffer)
{
    vInitTripleBuffer(numSampleInBuffer);
}

BufferMng::~BufferMng()
{
    vReleaseTripleBuffer();
}

TrplBufferStr *BufferMng::ptrGetCurInBuf()
{
    return pCurrentInputBuf;
}

TrplBufferStr *BufferMng::ptrGetCurProcessingBuf()
{
    return pCurrentProcessingBuf;
}

TrplBufferStr *BufferMng::ptrGetCurOutBuf()
{
    return pCurrentOutputBuf;
}

void BufferMng::vInitTripleBuffer(u16 numSampleInBuffer)
{
    for (int i = 0; i < TRPL_NUM_BUFFERS; i++) {
        trTripleBuffer[i].bufferSize = numSampleInBuffer;
        trTripleBuffer[i].pBufferRef = new sample_t[numSampleInBuffer];
        trTripleBuffer[i].isCompleted = false;
    }

    pCurrentInputBuf      = &trTripleBuffer[0];  /* A = input */
    pCurrentProcessingBuf = &trTripleBuffer[1];  /* B = processing */
    pCurrentOutputBuf     = &trTripleBuffer[2];  /* C = output */
}

void BufferMng::vReleaseTripleBuffer()
{
    for (int i = 0; i < TRPL_NUM_BUFFERS; i++) {
        delete[] trTripleBuffer[i].pBufferRef;
        trTripleBuffer[i].pBufferRef = nullptr;
    }
}

void BufferMng::vRotateBuffers()
{
    TrplBufferStr *pTmp = pCurrentInputBuf;

    pCurrentInputBuf      = pCurrentOutputBuf;       /* output to input (ready to fill) */
    pCurrentOutputBuf     = pCurrentProcessingBuf;   /* processing to output (ready to read) */
    pCurrentProcessingBuf = pTmp;                    /* input to processing (ready to process) */

    /* Reset flags for new cycle */
    pCurrentInputBuf->isCompleted      = false;  /* producer must fill */
    pCurrentProcessingBuf->isCompleted = false;  /* DSP must process */
    /* pCurrentOutputBuf keeps its state — consumer reads, then signals done */
}