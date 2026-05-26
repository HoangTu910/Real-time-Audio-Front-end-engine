#include <cstddef>
#include "utils.h"

#define NUM_BUFFER_CONFIGS 8

uint32_t listOfSizeOfBuffer[NUM_BUFFER_CONFIGS] = {128, 192, 256, 384, 512, 768, 1024, 1536};
/* now we need to populate the buffer pool with the specified buffer size and pool size */
/* each buffer will be of size bufferSize, bufferSize can be 128, 192, 256, 384, 512, 768, 1024, 1536 */

#define TRPL_BUF_SIZE 4
#define TRPL_TYPE_INPUT_BUF 0
#define TRBL_TYPE_PROCESSING_BUF 1
#define TRBL_TYPE_OUTPUT_BUF 2

typedef struct TrplBufferStr {
    u8 bufferIdx;
    u8 bufferType;
    u16 bufferSize;
    sample_t *pBufferRef = nullptr;
    bool isCompleted = false;
} TrplBufferStr;

class BufferMng {
public:
    BufferMng(u16 numSampleInBuffer);
    ~BufferMng();

    TrplBufferStr *pBufferA[TRPL_BUF_SIZE];
    TrplBufferStr *pBufferB[TRPL_BUF_SIZE];
    TrplBufferStr *pBufferC[TRPL_BUF_SIZE];
private:
    void vInitTripleBuffer(u16 numSampleInBuffer);
    TrplBufferStr* pInitLocalBuffer(u8 bufferType, u8 bufferIdx, u16 numSampleInBuffer);
    void releaseTripleBuffer();
};