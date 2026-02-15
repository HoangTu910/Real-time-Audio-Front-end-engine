
#include "rtafe/fe_api.h"
#include "rtafe/fe_features.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INFINITE_LOOP 1 /* just want to make the code not appear any magic number :) */
#define FRAME_BUFFER_LEN 256
#define NUM_CHANNELS 2

typedef struct {
    char riff[4];
    int overall_size;
    char wave[4];
    char fmt_chunk_marker[4];
    int length_of_fmt;
    short format_type;
    short channels;
    int sample_rate;
    int byterate;
    short block_align;
    short bits_per_sample;
    char data_chunk_header[4];
    int data_size;
} wav_header_t;


void print_version(void)
{
    uint32_t v = fe_version();
    printf("RTAFE v%u.%u.%u\n",
           (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
}

int main() {
    print_version();

    fe_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.sample_rate        = 16000;
    cfg.frame_len          = 256;     /* 16 ms @ 16 kHz              */
    cfg.hop_len            = 128;     /* 8 ms  (50% overlap)         */
    cfg.num_channels       = 2;       /* stereo input                */
    cfg.flags              = FE_FLAG_NOISE_SUPPRESS
                           | FE_FLAG_AGC
                           | FE_FLAG_FEATURES;

    size_t state_sz   = fe_state_bytes(&cfg);
    size_t scratch_sz = fe_scratch_bytes(&cfg);

    printf("State: %zu bytes\n", state_sz);
    printf("Scratch: %zu bytes\n", scratch_sz);

    FILE *pcm_in = fopen("test_signal_stereo.wav", "rb");
    FILE *pcm_out = fopen("output.raw", "wb");

    wav_header_t header;
    fread(&header, sizeof(wav_header_t), 1, pcm_in);
    printf("Input WAV: %d channels, %d Hz, %d bits per sample\n",
           header.channels, header.sample_rate, header.bits_per_sample);
    fwrite(&header, sizeof(wav_header_t), 1, pcm_out); /* write header to output */

    q15_t frame_buffer[FRAME_BUFFER_LEN * NUM_CHANNELS];        /* 256 × 2 samples for processing */
    q15_t overlap_buffer[FRAME_BUFFER_LEN/2 * NUM_CHANNELS];    /* 128 × 2 samples overlap carry-over */
    q15_t output_buffer[FRAME_BUFFER_LEN * NUM_CHANNELS];       /* processed frame output */
    int first_frame = 1;                                        /* flag for first iteration */

    while(INFINITE_LOOP) {
        size_t read;
        
        if (first_frame) {
            /* First iteration: read full 256 samples */
            read = fread(frame_buffer, sizeof(q15_t), FRAME_BUFFER_LEN * NUM_CHANNELS, pcm_in);
            first_frame = 0;
        } else {
            /* Subsequent iterations: copy overlap, then read 128 new samples */
            memcpy(frame_buffer, overlap_buffer, (FRAME_BUFFER_LEN/2) * NUM_CHANNELS * sizeof(q15_t));
            read = fread(frame_buffer + (FRAME_BUFFER_LEN/2) * NUM_CHANNELS, sizeof(q15_t),
                        (FRAME_BUFFER_LEN/2) * NUM_CHANNELS, pcm_in);
        }
        
        /* Check end-of-file */
        if (read == 0) {
            printf("End of file reached\n");
            break;
        }

        /* Pad with zeros if last frame is short */
        size_t total_read = first_frame ? read : (FRAME_BUFFER_LEN/2) + read;
        if (total_read < FRAME_BUFFER_LEN) {
            memset(frame_buffer + total_read * NUM_CHANNELS, 0,
                   (FRAME_BUFFER_LEN - total_read) * NUM_CHANNELS * sizeof(q15_t));
        }

        /* Process full frame (core dump fck it, not gonna run until I fix it!)*/
        /* fe_process_hop(state, frame_buffer, output_buffer, NULL, 0); */

        /* Step 6: Save tail (last 128 × 2 samples) for next iteration's overlap */
        memcpy(overlap_buffer, frame_buffer + (FRAME_BUFFER_LEN/2) * NUM_CHANNELS,
               (FRAME_BUFFER_LEN/2) * NUM_CHANNELS * sizeof(q15_t));

        /* Step 7: Write processed output (mono, hop_len samples) to file */
        /* fwrite(output_buffer, sizeof(q15_t), FRAME_BUFFER_LEN/2, pcm_out); */
    }
}