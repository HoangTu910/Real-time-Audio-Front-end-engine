#include "fe_init.h"

#define TEST_WAV_FILE "tests/test_signal_stereo.wav"

int main() {
    fe_manager_t mng;
    fe_init_buffer(&mng, TEST_WAV_FILE);
    if (mng.audio_info.sample_rate == 0) {
        fprintf(stderr, "Failed to initialize buffer with valid WAV file\n");
        return 1;
    }
    FE_LOG("Audio Info: Sample Rate: %u, Channels: %u, Bits per Sample: %u, Number of Samples: %zu\n", 
    mng.audio_info.sample_rate, mng.audio_info.num_channels, mng.audio_info.bits_per_sample, mng.config.num_samples);
    
    return 0;
}