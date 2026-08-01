#include "api/wav_manager/wav_file_mgr.hpp"
#include "module/dc_removal.hpp"
#include "module/noise_suppress.hpp"
#include "module/pre_emphasis.hpp"
#include "pipeline/dsp_pipeline.hpp"
#include "utils.hpp"

#include <cstdio>

static void processWavFile(const char *inputFile, const char *outputFile)
{
    const u16 frameSize = 512;
    const u16 hopSize = frameSize / 2;

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

    DSPPipeline pipelines[WavFileMgr::MAX_CHANNELS];
    DCRemoval dcRemoval[WavFileMgr::MAX_CHANNELS];
    PreEmphasis preEmphasis[WavFileMgr::MAX_CHANNELS];
    NoiseSuppress noiseSuppress[WavFileMgr::MAX_CHANNELS];

    for (u16 ch = 0; ch < numChannels; ++ch) {
        pipelines[ch].AddModule(&dcRemoval[ch]);
        pipelines[ch].AddModule(&preEmphasis[ch]);
        pipelines[ch].AddModule(&noiseSuppress[ch]);
    }

    if (!wav.bOpenWrite(outputFile)) {
        std::printf("Failed to open output WAV file\n");
        wav.vCloseRead();
        return;
    }

    while (wav.bHasNextFrame()) {
        sample_t **channels = wav.ppReadFrame();
        if (!channels) {
            break;
        }

        for (u16 ch = 0; ch < numChannels; ++ch) {
            DSPBlock firstHalf;
            firstHalf.dsp_buffer = channels[ch];
            firstHalf.block_size = hopSize;
            firstHalf.num_channels = 1;
            pipelines[ch].Process(&firstHalf);

            DSPBlock secondHalf;
            secondHalf.dsp_buffer = channels[ch] + hopSize;
            secondHalf.block_size = hopSize;
            secondHalf.num_channels = 1;
            pipelines[ch].Process(&secondHalf);
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