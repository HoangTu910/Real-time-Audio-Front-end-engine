#include "fe_init.h"

#define TEST_WAV_FILE "tests/test_signal_mono.wav"
#define OUTPUT_WAV_FILE "tests/test_signal_mono_processed.wav"
#define FRAME_SIZE_MS 5           /* Process 5ms frames */

/* This would be a good skeleton for anyone who wants to use the frame processing functions */

int main() {
    /* everytime you started to use the frame processing functions, config this init struct first */
    fe_init_t init = {
        .mng = (fe_manager_t *)malloc(sizeof(fe_manager_t)),
        .filename = TEST_WAV_FILE,
        .frame_size_millis = FRAME_SIZE_MS
    };
    
    /* and call below init function */
    fe_init(&init);

    fe_manager_t *mng = init.mng;
    
    /* Open output WAV file for writing processed frames */
    FILE *out_wav = fe_open_output_wav(OUTPUT_WAV_FILE, &mng->audio_info, mng->buffer_mng.total_samples);
    if (!out_wav) {
        FE_ERROR("Failed to create output WAV file %s\n", OUTPUT_WAV_FILE);
        fe_stop_frame_streaming(mng);
        free(init.mng);
        return 1;
    }
    FE_LOG("Output file: %s\n\n", OUTPUT_WAV_FILE);

    int frame_count = 0;

    /* loop until all the wav data is processed */
    while (!fe_is_processing_done(mng)) {
        fe_process_frame(mng);
        
        /* Write processed frame to output WAV file */
        fe_write_frame_to_wav(out_wav, mng->audio_buffer.output_buffer, mng->buffer_mng.frame_size,
                               mng->audio_info.num_channels, mng->audio_info.bits_per_sample);
        
        /* print some stuffs to make sure it works */
        frame_count++;
        
        if (frame_count % 40 == 0) {  /* Print progress every 200ms */
            FE_LOG("  Processed %d frames (%u samples, %.1f%%)\n",
                   frame_count,
                   mng->buffer_mng.samples_read,
                   100.0f * mng->buffer_mng.samples_read / mng->buffer_mng.total_samples);
        }
    }
    
    FE_LOG("\nCompleted! Total frames processed: %d\n", frame_count);
    
    fe_close_output_wav(out_wav);
    FE_LOG("Output WAV file written: %s\n", OUTPUT_WAV_FILE);
    
    /**
     * Report memory usage statistics, if we use bigger frame sizes, the memory usage will be higher
     * 
     * I haven't test the speed performance with different frame sizes 
     * so I'm not sure which frame size is the best for processing. 
     * 
     * In theory, smaller frame size means lower latency but higher CPU usage, 
     * while bigger frame size means higher latency but lower CPU usage. 
     * 
     * You can try different frame sizes and see how it affects the memory usage and processing speed.
     */
    fe_report_memory_usage(mng);
    
    /* cleanup, close file and free buffers */
    fe_stop_frame_streaming(mng);

    free(init.mng);
    
    return 0;
}