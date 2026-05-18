#include "biquad/biquad.h"
#include "fe_api.h"

#define TEST_WAV_FILE "tests/noise.wav"
#define OUTPUT_WAV_FILE "tests/noise_processed.wav"
#define FRAME_SIZE_SAMPLES 512 /* Process 512-sample frames */

#include <stdio.h>

int main() {
    fe_init_t init = {
        .input_wav_file = TEST_WAV_FILE,
        .output_wav_file = OUTPUT_WAV_FILE,
        .frame_size_samples = FRAME_SIZE_SAMPLES,
        .module_flags =  FE_FLAG_FILTER | FE_FLAG_NOISE_SUPPRESS, /* Enable the processing modules you want to benchmark */
        .overlap_percentage = 0  /* 0 = no overlap, 50 = 50% overlap, etc. */
    };
    en_fe result = fe_init(&init);
    if (result != FE_ERROR_NONE) {
        FE_ERROR("Failed to initialize frontend\n");
        return 1;
    }
    fe_manager_t *mng = init.mng;
    
    /* init filter param */
    biquad bq = {0};
    biquad_hpf(&bq, 500.0f, 0.707f, (float)mng->audio_info.sample_rate);
    mng->state.biquad_block.coeff = bq.coeff;
    biquad_quantize(&mng->state.biquad_block.coeff, &mng->state.biquad_block.coeff_fixed);

    /* init dc removal param */
    dc_remov dc = {0};
    dc.coeffs.alpha = 0.9990f;
    dc_remov_quantize(&dc.coeffs, &dc.coeffs_fixed);
    mng->state.dc_remov_block = dc;

    /* init pre-emphasis param */
    pre_emphasis pe = {0};
    pe.coeffs.alpha = 0.68f;
    pre_emphasis_quantize(&pe.coeffs, &mng->state.pre_emphasis_block.coeffs_fixed);
    mng->state.pre_emphasis_block.coeffs = pe.coeffs;
    mng->state.pre_emphasis_block = pe;

    /* init noise suppression param */
    noise_suppress_t ns = {0};
    ns.frame_size_samples = FRAME_SIZE_SAMPLES;
    ns.sample_rate = mng->audio_info.sample_rate;
    mng->state.noise_suppress_block = ns;

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