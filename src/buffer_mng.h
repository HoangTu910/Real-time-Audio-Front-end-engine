#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

static inline void _malloc_audio_buffer(sample_t **buffer, u32 frame_size, u16 num_channels) {
    *buffer = (sample_t *)malloc(frame_size * num_channels * sizeof(sample_t));
    if (*buffer == NULL) {
        fprintf(stderr, "Failed to allocate audio buffer\n");
        exit(1);
    }
}

#define allocate_frame_buffer(mng, frame_buf) _malloc_audio_buffer(&frame_buf, mng->buffer_mng.frame_size, mng->audio_info.num_channels)