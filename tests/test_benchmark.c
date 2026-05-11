#include "biquad/biquad.h"
#include "fe_api.h"

#define TEST_WAV_FILE "tests/chirp_stereo.wav"
#define OUTPUT_WAV_FILE "tests/chirp_processed.wav"
#define FRAME_SIZE_MS 5           /* Process 5ms frames */

#include <stdio.h>

int main() {
    fe_init_t init = {
        .input_wav_file = TEST_WAV_FILE,
        .output_wav_file = OUTPUT_WAV_FILE,
        .frame_size_millis = FRAME_SIZE_MS,
        .module_flags = FE_FLAG_FILTER  /* Enable the processing modules you want to benchmark */
    };
    en_fe result = fe_init(&init);
    if (result != FE_ERROR_NONE) {
        FE_ERROR("Failed to initialize frontend\n");
        return 1;
    }
    fe_manager_t *mng = init.mng;
    
    biquad bq = {0};
    biquad_lpf(&bq, 300.0f, 0.707f, (float)mng->audio_info.sample_rate);
    mng->state.biquad_block.coeff = bq.coeff;
    biquad_quantize(&mng->state.biquad_block.coeff, &mng->state.biquad_block.coeff_fixed);

    int frame_count = 0;
    /* loop until all the wav data is processed */
    while (!fe_is_processing_done(mng)) {
        fe_process_frame(mng);
        
        /* I don't want to write processed frames to a WAV file to keep the benchmark data precise */
        #ifndef BENCHMARK_MODE
        /* Write processed frame to output WAV file */
        fe_write_frame_to_wav(mng->output_wav_file, mng->audio_buffer.output_buffer, mng->buffer_mng.frame_size,
                               mng->audio_info.num_channels, mng->audio_info.bits_per_sample);
        
        /* print some stuffs to make sure it works */
        frame_count++;
        
        if (frame_count % 40 == 0) {  /* Print progress every 200ms */
            FE_LOG("  Processed %d frames (%u samples, %.1f%%)\n",
                   frame_count,
                   mng->buffer_mng.samples_read,
                   100.0f * mng->buffer_mng.samples_read / mng->buffer_mng.total_samples);
        }
        #endif
    }

    /* cleanup, close file and free buffers */
    fe_stop_frame_streaming(mng);

    free(init.mng);

    return 0;
}