#include "BufferMng.hpp"

BufferMng::BufferMng(u16 numSampleInBuffer)
{
    vInitTripleBuffer(numSampleInBuffer);
}

BufferMng::~BufferMng()
{
    releaseTripleBuffer();
}

void BufferMng::vInitTripleBuffer(u16 numSampleInBuffer)
{
    TrplBufferStr* inputBufA     = pInitLocalBuffer(TRPL_TYPE_INPUT_BUF, 0, numSampleInBuffer);
    TrplBufferStr* processBufA   = pInitLocalBuffer(TRBL_TYPE_PROCESSING_BUF, 1, numSampleInBuffer);
    TrplBufferStr* outputBufA    = pInitLocalBuffer(TRBL_TYPE_OUTPUT_BUF, 2, numSampleInBuffer);
    TrplBufferStr* inputBufA_2   = pInitLocalBuffer(TRPL_TYPE_INPUT_BUF, 3, numSampleInBuffer);

    TrplBufferStr* processBufB   = pInitLocalBuffer(TRBL_TYPE_PROCESSING_BUF, 0, numSampleInBuffer);
    TrplBufferStr* inputBufB     = pInitLocalBuffer(TRPL_TYPE_INPUT_BUF, 1, numSampleInBuffer);
    TrplBufferStr* outputBufB    = pInitLocalBuffer(TRBL_TYPE_OUTPUT_BUF, 2, numSampleInBuffer);
    TrplBufferStr* processBufB_2 = pInitLocalBuffer(TRBL_TYPE_PROCESSING_BUF, 3, numSampleInBuffer);

    TrplBufferStr* outputBufC    = pInitLocalBuffer(TRBL_TYPE_PROCESSING_BUF, 0, numSampleInBuffer);
    TrplBufferStr* processBufC   = pInitLocalBuffer(TRPL_TYPE_INPUT_BUF, 1, numSampleInBuffer);
    TrplBufferStr* inputBufC     = pInitLocalBuffer(TRBL_TYPE_OUTPUT_BUF, 2, numSampleInBuffer);
    TrplBufferStr* outputBufC_2  = pInitLocalBuffer(TRBL_TYPE_PROCESSING_BUF, 3, numSampleInBuffer);

    pBufferA[0] = inputBufA;
    pBufferA[1] = processBufA;
    pBufferA[2] = outputBufA;
    pBufferA[3] = inputBufA_2;

    pBufferB[0] = processBufB;
    pBufferB[1] = inputBufB;
    pBufferB[2] = outputBufB;
    pBufferB[3] = processBufB_2;

    pBufferC[0] = outputBufC;
    pBufferC[1] = processBufC;
    pBufferC[2] = inputBufC;
    pBufferC[3] = outputBufC_2;
}

TrplBufferStr *BufferMng::pInitLocalBuffer(u8 bufferType, u8 bufferIdx, u16 numSampleInBuffer)
{
    TrplBufferStr* trBuf = new TrplBufferStr();
    trBuf->bufferIdx = bufferIdx;
    trBuf->bufferType = bufferType;
    trBuf->bufferSize = numSampleInBuffer;

    return trBuf;
}

void BufferMng::releaseTripleBuffer()
{
    for(int i = 0; i < TRPL_BUF_SIZE; i++) {
        delete pBufferA[i];
        delete pBufferB[i];
        delete pBufferC[i];
    }
}