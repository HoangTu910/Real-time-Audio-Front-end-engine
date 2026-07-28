#include "rtafe_main_sp.hpp"
#include "wav_file_mgr.hpp"
#include <cstdio>

/* ── Process WAV file through DSP pipeline ── */
static void processWavFile(const char *inputFile, const char *outputFile)
{
    const u16 frameSize = 256;

    printf("\n=== WAV Processing ===\n");
    printf("Input:  %s\n", inputFile);
    printf("Output: %s\n\n", outputFile);

    /* Setup WAV manager */
    WavFileMgr wav;
    if (!wav.bOpenRead(inputFile, frameSize)) {
        printf("Failed to open input WAV file\n");
        return;
    }

    u16 numChannels = wav.uGetChannels();
    if (numChannels > WavFileMgr::MAX_CHANNELS) {
        printf("Error: %d channels exceeds max %d\n",
               numChannels, WavFileMgr::MAX_CHANNELS);
        wav.vCloseRead();
        return;
    }

    /* Create one DSP pipeline per channel */
    RTAFE_Main_SP *dsp[WavFileMgr::MAX_CHANNELS];
    for (u16 ch = 0; ch < WavFileMgr::MAX_CHANNELS; ch++) {
        dsp[ch] = nullptr;
    }
    for (u16 ch = 0; ch < numChannels; ch++) {
        dsp[ch] = new RTAFE_Main_SP();
        dsp[ch]->dspRTAFE_InitBuffer(frameSize);
    }

    if (!wav.bOpenWrite(outputFile)) {
        printf("Failed to create output WAV file\n");
        for (u16 ch = 0; ch < numChannels; ch++) delete dsp[ch];
        wav.vCloseRead();
        return;
    }

    /* Process frame by frame */
    u32 frameCount = 0;
    while (wav.bHasNextFrame()) {
        sample_t **ppCh = wav.ppReadFrame();
        if (!ppCh) break;

        /* Process each channel independently */
        for (u16 ch = 0; ch < numChannels; ch++) {
            sample_t *pIn = dsp[ch]->dspRTAFE_pGetInputBuffer();
            for (u16 i = 0; i < frameSize; i++) {
                pIn[i] = ppCh[ch][i];
            }
            dsp[ch]->dspRTAFE_vCompleteInput();
            dsp[ch]->dspRTAFE_vProcess();
        }

        /* Collect processed output per channel */
        sample_t *pOutBuf[WavFileMgr::MAX_CHANNELS];
        for (u16 ch = 0; ch < numChannels; ch++) {
            pOutBuf[ch] = dsp[ch]->dspRTAFE_pGetOutputBuffer();
        }
        wav.vWriteFrame(pOutBuf, frameSize);

        for (u16 ch = 0; ch < numChannels; ch++) {
            dsp[ch]->dspRTAFE_vCompleteOutput();
        }

        frameCount++;
        if (frameCount % 10 == 0) {
            printf("Processed %lu frames...\n", (unsigned long)frameCount);
        }
    }

    /* Finalize */
    wav.vCloseWrite();
    wav.vCloseRead();

    printf("\nDone! Processed %lu frames (%d channels).\n",
           (unsigned long)frameCount, numChannels);
    printf("Output: %s\n", outputFile);

    for (u16 ch = 0; ch < numChannels; ch++) delete dsp[ch];
}

/* ── Triple buffer test ── */
static void testTripleBuffer()
{
    printf("\n=== Triple Buffer Test ===\n");

    RTAFE_Main_SP *rtafeMainSP = new RTAFE_Main_SP();
    u16 numSampleInBuffer = 512;
    rtafeMainSP->dspRTAFE_InitBuffer(numSampleInBuffer);

    /* output starts as "consumed" (empty buffer), so
     * first rotation happens after input is filled and processing is done. */
    for (int cycle = 0; cycle < 10; cycle++) {
        /* Producer: fill input buffer */
        sample_t *pIn = rtafeMainSP->dspRTAFE_pGetInputBuffer();
        for (u16 i = 0; i < numSampleInBuffer; i++) {
            pIn[i] = (sample_t)(cycle * numSampleInBuffer + i);
        }
        rtafeMainSP->dspRTAFE_vCompleteInput();

        /* DSP: process + rotate if all stages done */
        rtafeMainSP->dspRTAFE_vProcess();

        /* Consumer: read output buffer */
        sample_t *pOut = rtafeMainSP->dspRTAFE_pGetOutputBuffer();
        printf("Cycle %d: output[0]=%d, output[1]=%d\n", cycle, pOut[0], pOut[1]);
        rtafeMainSP->dspRTAFE_vCompleteOutput();
    }

    delete rtafeMainSP;
}

int main(int argc, char *argv[])
{
    if (argc >= 3) {
        /* WAV processing mode: test_rtafe input.wav output.wav */
        processWavFile(argv[1], argv[2]);
    } else {
        /* Default: triple buffer test */
        testTripleBuffer();
    }

    return 0;
}