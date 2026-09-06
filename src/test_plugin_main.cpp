#include "wav_file_mgr.hpp"
#include "htsp_plugin.hpp"
#include "utils.hpp"

#include <cstdio>

static bool processWavFile(const char *inputFile, const char *outputFile)
{
    const u16 frameSize = BLOCK_SIZE;

    WavFileMgr wav;
    if (!wav.bOpenRead(inputFile, frameSize)) {
        std::printf("Failed to open input WAV file\n");
        return false;
    }

    const u16 numChannels = wav.uGetChannels();
    if (numChannels != 2 && numChannels != 4) {
        std::printf("Unsupported channel count: %u; plugin requires 2 or 4\n",
                    numChannels);
        wav.vCloseRead();
        return false;
    }

    if (!wav.bOpenWrite(outputFile)) {
        std::printf("Failed to open output WAV file\n");
        wav.vCloseRead();
        return false;
    }

    HTSPPlugin htsp_plugin;
    HtspErrRet ret = htsp_plugin.SetParams();
    if (ret != kOk) {
        std::printf("DSP parameter configuration error: %d\n", ret);
        wav.vCloseWrite();
        wav.vCloseRead();
        return false;
    }

    while (wav.bHasNextFrame()) {
        sample_t **channels = wav.ppReadFrame();
        if (!channels) {
            std::printf("Failed to read WAV frame\n");
            wav.vCloseWrite();
            wav.vCloseRead();
            return false;
        }

        #ifdef FIXED_POINT
        ret = htsp_plugin.ProcessFixed(channels, numChannels);
        #else
        ret = htsp_plugin.Process(channels, numChannels);
        #endif
        if (ret != kOk) {
            std::printf("DSP processing error: %d\n", ret);
            wav.vCloseWrite();
            wav.vCloseRead();
            return false;
        }

        wav.vWriteFrame(channels, frameSize);
    }

    wav.vCloseWrite();
    wav.vCloseRead();
    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::printf("Usage: %s input.wav output.wav\n", argv[0]);
        return 1;
    }

    return processWavFile(argv[1], argv[2]) ? 0 : 1;
}