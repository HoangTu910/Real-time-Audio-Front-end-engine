#include <cstddef>
#include "utils.h"

#define TRPL_NUM_BUFFERS 3  /* A, B, C */

typedef struct TrplBufferStr {
    u16 bufferSize = 0;
    sample_t *pBufferRef = nullptr;
    bool isCompleted = false;
} TrplBufferStr;

/*
 * Triple Buffer (Section 11.4.1)
 *
 * Cycle 0:  A = INPUT    B = PROCESSING   C = OUTPUT
 * Cycle 1:  C = INPUT    A = PROCESSING   B = OUTPUT
 * Cycle 2:  B = INPUT    C = PROCESSING   A = OUTPUT
 * Cycle 3:  A = INPUT    B = PROCESSING   C = OUTPUT  (repeat)
 *
 * Rotation: when all three stages complete, roles shift:
 * INPUT - PROCESSING - OUTPUT - INPUT
 *
 * Data never moves — only pointers rotate.
 */
class BufferMng {
public:
    BufferMng(u16 numSampleInBuffer);
    ~BufferMng();

    TrplBufferStr* ptrGetCurInBuf();
    TrplBufferStr* ptrGetCurProcessingBuf();
    TrplBufferStr* ptrGetCurOutBuf();

    /*
     * Rotate buffer roles after all stages complete.
     * Resets isCompleted flags for the new cycle.
     */
    void vRotateBuffers();

private:
    TrplBufferStr trTripleBuffer[TRPL_NUM_BUFFERS];

    TrplBufferStr* pCurrentInputBuf      = nullptr;
    TrplBufferStr* pCurrentProcessingBuf = nullptr;
    TrplBufferStr* pCurrentOutputBuf     = nullptr;

    void vInitTripleBuffer(u16 numSampleInBuffer);
    void vReleaseTripleBuffer();
};