#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "errors_code.h"

static inline en_fe _malloc_audio_buffer(sample_t **buffer, u32 frame_size, u16 num_channels) {
    *buffer = (sample_t *)malloc(frame_size * num_channels * sizeof(sample_t));
    if (*buffer == NULL) {
        fprintf(stderr, "Failed to allocate audio buffer\n");
        return FE_ERROR_MEMORY_ALLOCATION;
    }
    return FE_ERROR_NONE;
}

static inline en_fe _malloc_deinterleave_buffer(sample_t **ch, u32 frame_size) {
    *ch = (sample_t *)malloc(frame_size * sizeof(sample_t));
    if (*ch == NULL) {
        fprintf(stderr, "Failed to allocate deinterleave buffer\n");
        return FE_ERROR_MEMORY_ALLOCATION;
    }
    return FE_ERROR_NONE;
}

static inline en_fe _malloc_overlap_buffer(sample_t **buffer, u32 overlap_size) {
    *buffer = (sample_t *)malloc(overlap_size * sizeof(sample_t));
    if (*buffer == NULL) {
        fprintf(stderr, "Failed to allocate overlap buffer\n");
        return FE_ERROR_MEMORY_ALLOCATION;
    }
    return FE_ERROR_NONE;
}

static inline void _free_buffer(sample_t **buffer) {
    if (*buffer) {
        free(*buffer);
        *buffer = NULL;
    }
}

#define allocate_frame_buffer(mng, frame_buf) _malloc_audio_buffer(&frame_buf, mng->buffer_mng.frame_size, mng->audio_info.num_channels)
#define allocate_deinterleave_buffer(mng, ch_buf) _malloc_deinterleave_buffer(&ch_buf, mng->buffer_mng.frame_size)
#define allocate_overlap_buffer(mng, overlap_buf) _malloc_overlap_buffer(&overlap_buf, mng->buffer_mng.overlap_size)
#define free_processed_buffer(frame_buf) _free_buffer(&frame_buf)