#include "wav_file_mgr.hpp"
#include "htsp_plugin.hpp"
#include "utils.hpp"

#include <cstdio>

static void processWavFile(const char *inputFile, const char *outputFile)
{
    /* should we integrate the frame size (block size) getter from htsp_plugin instead of hardcoding it? */
    const u16 frameSize = 512;

    WavFileMgr wav;
    if (!wav.bOpenRead(inputFile, frameSize)) {
        std::printf("Failed to open input WAV file\n");
        return;
    }

    const u16 numChannels = wav.uGetChannels();
    if (numChannels == 0 || numChannels > WavFileMgr::MAX_CHANNELS) {
        std::printf("Unsupported channel count: %u\n", numChannels);
        wav.vCloseRead();
        return;
    }

    if (!wav.bOpenWrite(outputFile)) {
        std::printf("Failed to open output WAV file\n");
        wav.vCloseRead();
        return;
    }

    HTSPPlugin htsp_plugin;
    while (wav.bHasNextFrame()) {
        sample_t **channels = wav.ppReadFrame();
        if (!channels) {
            break;
        }

        if(frameSize != REQUIRED_BLOCK_SIZE) {
            std::printf("Frame size mismatch: expected %u, got %u\n", REQUIRED_BLOCK_SIZE, frameSize);
            break;
        }
        /* still need to clarify again, only specific channel as input is allowed */
        HtspErrRet ret = htsp_plugin.ProcessDSPBlock(channels, numChannels);
        if (ret != kOk) {
            std::printf("DSP processing error: %d\n", ret);
            break;
        }

        wav.vWriteFrame(channels, frameSize);
    }

    wav.vCloseWrite();
    wav.vCloseRead();
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::printf("Usage: %s input.wav output.wav\n", argv[0]);
        return 1;
    }

    processWavFile(argv[1], argv[2]);
    return 0;
}