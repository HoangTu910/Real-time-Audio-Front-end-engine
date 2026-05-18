#include "fe_api.h"

static void _read_riff_header(FILE *file, fe_audio_info_t *info) {
    char riff[4], wave[4];
    fread(riff, 1, 4, file);
    fread(&info->file_size, 4, 1, file);
    fread(wave, 1, 4, file);
    
    if (strncmp(riff, "RIFF", 4) != 0 || strncmp(wave, "WAVE", 4) != 0) {
        FE_ERROR("Not a valid WAV file\n");
        info->sample_rate = 0;
        info->num_channels = 0;
        info->bits_per_sample = 0;
    }
}

static void _find_fmt_chunk(FILE *file, fe_audio_info_t *info) {
    char chunk_id[4];
    while (fread(chunk_id, 1, 4, file) == 4) {
        uint32_t chunk_size;
        fread(&chunk_size, 4, 1, file);
        
        if (strncmp(chunk_id, "fmt ", 4) == 0) {
            fread(&info->audio_format, 2, 1, file);
            fread(&info->num_channels, 2, 1, file);
            fread(&info->sample_rate, 4, 1, file);
            fread(&info->byte_rate, 4, 1, file);
            fread(&info->block_align, 2, 1, file);
            fread(&info->bits_per_sample, 2, 1, file);
            
            if (info->audio_format != 1) {
                FE_ERROR("Only PCM format is supported\n");
                info->sample_rate = 0;
                info->num_channels = 0;
                info->bits_per_sample = 0;
            }
            
            // Skip remaining fmt chunk data if any
            if (chunk_size > 16) {
                fseek(file, chunk_size - 16, SEEK_CUR);
            }
            return;
        } else {
            // Skip this chunk
            fseek(file, chunk_size, SEEK_CUR);
        }
    }
}

static void _find_data_chunk(FILE *file, fe_audio_info_t *info) {
    char chunk_id[4];
    while (fread(chunk_id, 1, 4, file) == 4) {
        uint32_t chunk_size;
        fread(&chunk_size, 4, 1, file);
        
        if (strncmp(chunk_id, "data", 4) == 0) {
            info->data_size = chunk_size;
            return;
        } else {
            fseek(file, chunk_size, SEEK_CUR);
        }
    }
}

static void _get_wav_info(const char *filename, fe_manager_t *mng)
{
    mng->buffer_mng.file = fopen(filename, "rb");
    if (!mng->buffer_mng.file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        mng->audio_info.sample_rate = 0;
        mng->audio_info.num_channels = 0;
        mng->audio_info.bits_per_sample = 0;
        return;
    }

    _read_riff_header(mng->buffer_mng.file, &mng->audio_info);
    _find_fmt_chunk(mng->buffer_mng.file, &mng->audio_info);
    _find_data_chunk(mng->buffer_mng.file, &mng->audio_info);

    /* calculate number of samples */
    uint32_t num_bytes_per_sample = mng->audio_info.bits_per_sample / 8;
    uint32_t total_samples = mng->audio_info.data_size / (mng->audio_info.num_channels * num_bytes_per_sample);

    mng->config.num_samples = total_samples;
    mng->audio_info.sample_rate = mng->audio_info.sample_rate;
    mng->audio_info.num_channels = mng->audio_info.num_channels;
    mng->audio_info.bits_per_sample = mng->audio_info.bits_per_sample;

    /* Initialize buffer manager for frame reading */
    mng->buffer_mng.total_samples = total_samples;
    mng->buffer_mng.samples_read = 0;
    mng->buffer_mng.eof_reached = false;
    
    /* File stays OPEN for streaming frame-by-frame reading */
    /* (Will be closed when done or on error) */
}

static bool _read_wav_frame(fe_buffer_manager_t *mng, sample_t *frame_buffer) {
    u16 num_channels = mng->info.num_channels;
    
    /* Try to read first sample to check EOF early */
    if (feof(mng->file)) {
        return false;  /* Already at EOF */
    }

    for(u16 i = 0; i < mng->frame_size * num_channels; i++) {
        size_t items_read = 0;
        
        if (mng->info.bits_per_sample == BIT_PCM_FORMAT_16) {
            int16_t val;
            items_read = fread(&val, 2, 1, mng->file);
            if (items_read == 0) return false;  /* Hit EOF */
            
            #ifdef FIXED_POINT
                frame_buffer[i] = (sample_t)(val >> 1);  /* Q1.15 -> Q2.14 */
            #else
                float f_sample = (float)val / 32768.0f;  /* Normalize to [-1.0, 1.0] */
                frame_buffer[i] = f_sample;
            #endif
            
        } else if (mng->info.bits_per_sample == BIT_PCM_FORMAT_24) {
            uint8_t bytes[3];
            items_read = fread(bytes, 1, 3, mng->file);
            if (items_read == 0) return false;  /* Hit EOF */
            
            /* Reconstruct 24-bit signed value */
            int32_t val_24 = (int32_t)((bytes[0]) | (bytes[1] << 8) | (bytes[2] << 16));
            if (val_24 & 0x800000) val_24 |= 0xFF000000;  /* Sign-extend if negative */
            
            #ifdef FIXED_POINT
                /* Scale from Q1.23 to Q2.14: shift right by 9 */
                frame_buffer[i] = (sample_t)(val_24 >> 9);
            #else
                float f_sample = (float)val_24 / 8388608.0f;  /* Normalize to [-1.0, 1.0] */
                frame_buffer[i] = f_sample;
            #endif
            
        } else if (mng->info.bits_per_sample == BIT_PCM_FORMAT_32) {
            int32_t val;
            items_read = fread(&val, 4, 1, mng->file);
            if (items_read == 0) return false;  /* Hit EOF */
            
            #ifdef FIXED_POINT
                /* Scale from Q1.31 to Q2.14: shift right by 17 */
                frame_buffer[i] = (sample_t)(val >> 17);
            #else
                float f_sample = (float)val / 2147483648.0f;  /* Normalize to [-1.0, 1.0] */
                frame_buffer[i] = f_sample;
            #endif
            
        } else {
            fprintf(stderr, "Unsupported bits per sample: %u\n", mng->info.bits_per_sample);
            return false;   
        }
    }    
    return true;  /* Frame read successfully */
}

#ifdef OPTIMIZATION_METHOD
static inline void _deinterleave_2ch(const sample_t *in, sample_t *restrict ch1, sample_t *restrict ch2, int num_samples) {
    int i = 0;
    int j = 0;

    for(; i <= num_samples - 8; i+=8, j+=4) {
        ch1[j]     = in[i];
        ch2[j]     = in[i + 1];

        ch1[j + 1] = in[i + 2];
        ch2[j + 1] = in[i + 3];

        ch1[j + 2] = in[i + 4];
        ch2[j + 2] = in[i + 5];

        ch1[j + 3] = in[i + 6];
        ch2[j + 3] = in[i + 7];
    }

    for (; i < num_samples; i += 2, j++) {
        ch1[j] = in[i];
        ch2[j] = in[i + 1];
    }
}

static inline void _interleave_2ch(const sample_t *restrict ch1,
                            const sample_t *restrict ch2,
                            sample_t *restrict out,
                            int frame_size)
{
    int i = 0;

    for (; i <= frame_size - 4; i += 4) {

        out[2*i]     = ch1[i];
        out[2*i + 1] = ch2[i];

        out[2*i + 2] = ch1[i + 1];
        out[2*i + 3] = ch2[i + 1];

        out[2*i + 4] = ch1[i + 2];
        out[2*i + 5] = ch2[i + 2];

        out[2*i + 6] = ch1[i + 3];
        out[2*i + 7] = ch2[i + 3];
    }

    for (; i < frame_size; i++) {
        out[2*i]     = ch1[i];
        out[2*i + 1] = ch2[i];
    }
}
#endif

#ifdef OPTIMIZATION_METHOD
static inline void _fe_process_sample_vec_2ch(fe_manager_t *mng, sample_t in_ch1, sample_t in_ch2, sample_t *out_ch1, sample_t *out_ch2)
{
    *out_ch1 = in_ch1;
    *out_ch2 = in_ch2;
    if(mng->config.module_flags & FE_FLAG_FILTER) {
        /* TBD */
        #ifdef FIXED_POINT
            biquad_step_fixed(&mng->state.biquad_block, out_ch1);
            biquad_step_fixed(&mng->state.biquad_block, out_ch2);
        #else
            biquad_step_vec_2ch(&mng->state.biquad_block, out_ch1, out_ch2, out_ch1, out_ch2);
        #endif
    }
    if (mng->config.module_flags & FE_FLAG_DC_REMOVAL) {
        dc_removal_sample_process(&mng->state.dc_remov_block, out_ch1);
        dc_removal_sample_process(&mng->state.dc_remov_block, out_ch2);
    }
    if (mng->config.module_flags & FE_FLAG_PRE_EMPHASIS) {
        pre_emphasis_sample_process(&mng->state.pre_emphasis_block, out_ch1);
        pre_emphasis_sample_process(&mng->state.pre_emphasis_block, out_ch2);
    }
    if (mng->config.module_flags & FE_FLAG_NOISE_SUPPRESS) { 
        /* TBD */
    }
}
#endif

sample_t inline _fe_process_sample(fe_manager_t *mng, sample_t in)
{
    sample_t out = in;
    /* TBD */
    if(mng->config.module_flags & FE_FLAG_FILTER) {
        /* TBD */
        #ifdef FIXED_POINT
            biquad_step_fixed(&mng->state.biquad_block, &out);
        #else
            biquad_step(&mng->state.biquad_block, &out);
        #endif
    }
    if (mng->config.module_flags & FE_FLAG_DC_REMOVAL) {
        dc_removal_sample_process(&mng->state.dc_remov_block, &out);
    }
    if (mng->config.module_flags & FE_FLAG_PRE_EMPHASIS) {
        pre_emphasis_sample_process(&mng->state.pre_emphasis_block, &out);
    }
    if (mng->config.module_flags & FE_FLAG_NOISE_SUPPRESS) { 
        /* TBD */
    }
    if(mng->config.module_flags & FE_FLAG_FADER) {

    }
    return out;
}

void fe_process_frame(fe_manager_t *mng)
{
    if (mng->buffer_mng.eof_reached)
        return;

    sample_t *input = mng->audio_buffer.input_buffer;
    sample_t *output = mng->audio_buffer.output_buffer;

    uint8_t num_channels = mng->audio_info.num_channels;
    int frame_size = mng->buffer_mng.frame_size;
    int num_samples = frame_size * num_channels;

    bool has_data = get_frame_buffer(mng, input);

    if (!has_data) {
        mng->buffer_mng.eof_reached = true;
        return;
    }

    #ifdef OPTIMIZATION_METHOD
    if (num_channels == 2) {
        sample_t *ch1 = mng->deinterleave_buffer.buffer_channel_1;
        sample_t *ch2 = mng->deinterleave_buffer.buffer_channel_2;

        _deinterleave_2ch(input, ch1, ch2, num_samples);

        for (int i = 0; i < frame_size; i++) {
            _fe_process_sample_vec_2ch(mng, ch1[i], ch2[i], &ch1[i], &ch2[i]);
        }

        _interleave_2ch(ch1, ch2, output, frame_size);
    }
    else {
        for (int i = 0; i < num_samples; i++) {
            output[i] = _fe_process_sample(mng, input[i]);
        }
    }
    #else
    for (int i = 0; i < num_samples; i++) {
        output[i] = _fe_process_sample(mng, input[i]);
    }
    #endif

    if(mng->config.module_flags & FE_FLAG_NOISE_SUPPRESS) {
        #ifdef FIXED_POINT
        process_noise_suppression_fixed((s16 *)output, &mng->state.noise_suppress_block);
        #else
        process_noise_suppression((float *)output, &mng->state.noise_suppress_block);
        #endif
    }
    
    /* Apply overlap-add if enabled */
    if (mng->config.overlap_percentage > 0 && mng->ola_initialized) {
        int overlap_size = (frame_size * mng->config.overlap_percentage) / 100;
        
        /* Apply synthesis window to output */
        /**
         * Synthesis window is a method to smooth the transitions between frames in overlap-add processing.
         * Currently, the synthesis window is Hann window.
         */
        #ifdef FIXED_POINT
        /* for now, this fixed-point is not working properly */
        for (int i = 0; i < frame_size; i++) {
            output[i] = (sample_t)((output[i] * mng->synthesis_window[i]) >> 14);
        }
        #else
        for (int i = 0; i < frame_size; i++) {
            output[i] = (sample_t)(output[i] * mng->synthesis_window[i]);
        }
        #endif
        
        /* Overlap-add: combine overlap buffer with first part of output */
        /* OPT_MARK: can we vectorize this to make it run faster?*/
        for (int ch = 0; ch < num_channels; ch++) {
            for (int i = 0; i < overlap_size; i++) {
                int out_idx = i * num_channels + ch;
                int overlap_idx = i * num_channels + ch;

                /**
                 * in case you wonder, two samples are added together in the overlapping region to create a smooth transition between frames.
                 * The output sample is the sum of the current frame's sample and the overlapping tail from the previous frame.
                 */
                output[out_idx] = (sample_t)(output[out_idx] + mng->overlap_buffer[overlap_idx]);
            }
        }
        
        /* Save tail (last overlap_size samples) to overlap buffer for next frame */
        for (int i = 0; i < overlap_size * num_channels; i++) {
            mng->overlap_buffer[i] = output[(frame_size - overlap_size) * num_channels + i];
        }
    }

    mng->buffer_mng.samples_read += frame_size;
}

void fe_init_audio_info(fe_manager_t *mng, const char *filename)
{
    _get_wav_info(filename, mng);
    if(mng->audio_info.sample_rate != (uint32_t)SAMPLING_RATE) {
        FE_WARN("Current sampling rate is %u, we expect it to be %u\n", mng->audio_info.sample_rate, (uint32_t)SAMPLING_RATE);
    }
}

void fe_init_frame_size(fe_manager_t *mng, u32 frame_size_samples)
{
    if (frame_size_samples == 0) {
        FE_ERROR("Frame size must be greater than 0\n");
        return;
    }
    mng->buffer_mng.frame_size = frame_size_samples;
    
    /* Calculate hop size from overlap percentage */
    u32 hop_size = (frame_size_samples * (100 - mng->config.overlap_percentage)) / 100;
    if (hop_size == 0) hop_size = frame_size_samples;  /* Fallback if overlap is 100% */
    mng->buffer_mng.hop_size = hop_size;
    
    FE_LOG("Frame size: %u samples, Overlap: %u%%, Hop size: %u samples\n",
            frame_size_samples, mng->config.overlap_percentage, hop_size);
}

void fe_start_frame_streaming(fe_manager_t *mng)
{
    if (!mng->buffer_mng.file) {
        FE_ERROR("Audio file not opened. Call fe_init_audio_info() first.\n");
        return;
    }

    /* Copy audio info to buffer manager, kinda dumb structure, need to think about how to 
       clean this redundant part*/
    mng->buffer_mng.info = mng->audio_info;

    /* Allocate input/output buffers (one-time allocation) */
    u32 num_channels = mng->audio_info.num_channels;
    allocate_frame_buffer(mng, mng->audio_buffer.input_buffer);
    allocate_frame_buffer(mng, mng->audio_buffer.output_buffer);

    if (!mng->audio_buffer.input_buffer || !mng->audio_buffer.output_buffer) {
        FE_ERROR("Failed to allocate frame buffers\n");
        return;
    }

    if(num_channels == 2) {
        allocate_deinterleave_buffer(mng, mng->deinterleave_buffer.buffer_channel_1);
        allocate_deinterleave_buffer(mng, mng->deinterleave_buffer.buffer_channel_2);
    }
    else if(num_channels == 4) {
        allocate_deinterleave_buffer(mng, mng->deinterleave_buffer.buffer_channel_1);
        allocate_deinterleave_buffer(mng, mng->deinterleave_buffer.buffer_channel_2);
        allocate_deinterleave_buffer(mng, mng->deinterleave_buffer.buffer_channel_3);
        allocate_deinterleave_buffer(mng, mng->deinterleave_buffer.buffer_channel_4);
    }
    
    /* Initialize overlap-add buffers and windows if overlap is enabled */
    if (mng->config.overlap_percentage > 0) {
        u32 frame_size = mng->buffer_mng.frame_size;
        mng->buffer_mng.overlap_size = (frame_size * mng->config.overlap_percentage) / 100;
        
        /* Allocate overlap buffer (stores tail from previous frame) */
        allocate_overlap_buffer(mng, mng->overlap_buffer);
        
        /* Initialize overlap buffer to zero */
        for (u32 i = 0; i < mng->buffer_mng.overlap_size; i++) {
            mng->overlap_buffer[i] = 0.0f;
        }
        
        /* Allocate analysis and synthesis windows */
        allocate_frame_buffer(mng, mng->analysis_window);
        allocate_frame_buffer(mng, mng->synthesis_window);
        
        if (!mng->analysis_window || !mng->synthesis_window) {
            FE_ERROR("Failed to allocate window buffers\n");
            return;
        }
        
        /* Generate Hann window for both analysis and synthesis (ideal for 50% overlap) */
        for (u32 n = 0; n < frame_size; n++) {
            sincos sc = fast_sine_cos(2.0f * M_PI * n / (frame_size - 1));
            float hann = 0.5f * (1.0f - sc.cos);
            mng->analysis_window[n] = (sample_t)hann;
            mng->synthesis_window[n] = (sample_t)hann;
        }
        
        mng->ola_initialized = 1;
        
        FE_LOG("OLA initialized: overlap_size=%u, window type=Hann\n", mng->buffer_mng.overlap_size);
    }

    /* Track memory usage */
    mng->mem_stats.input_buffer_bytes = mng->buffer_mng.frame_size * num_channels * sizeof(sample_t);
    mng->mem_stats.output_buffer_bytes = mng->buffer_mng.frame_size * num_channels * sizeof(sample_t);
    mng->mem_stats.total_allocated = mng->mem_stats.input_buffer_bytes + mng->mem_stats.output_buffer_bytes;
    
    if (mng->config.overlap_percentage > 0) {
        u32 overlap_size = (mng->buffer_mng.frame_size * mng->config.overlap_percentage) / 100;
        mng->mem_stats.total_allocated += overlap_size * sizeof(sample_t);  /* overlap buffer */
        mng->mem_stats.total_allocated += 2 * mng->buffer_mng.frame_size * sizeof(sample_t);  /* windows */
    }
    
    mng->mem_stats.peak_usage = mng->mem_stats.total_allocated;

    FE_LOG("Frame streaming initialized: %u samples/frame, %u channels\n", 
            mng->buffer_mng.frame_size, num_channels);
}

void fe_stop_frame_streaming(fe_manager_t *mng)
{
    if (mng->buffer_mng.file) {
        fclose(mng->buffer_mng.file);
        mng->buffer_mng.file = NULL;
    }
    if (mng->audio_buffer.input_buffer) {
        free_processed_buffer(mng->audio_buffer.input_buffer);
    }
    if (mng->audio_buffer.output_buffer) {
        free_processed_buffer(mng->audio_buffer.output_buffer);
    }
    if (mng->deinterleave_buffer.buffer_channel_1) {
        free_processed_buffer(mng->deinterleave_buffer.buffer_channel_1);
    }
    if (mng->deinterleave_buffer.buffer_channel_2) {
        free_processed_buffer(mng->deinterleave_buffer.buffer_channel_2);
    }
    if (mng->deinterleave_buffer.buffer_channel_3) {
        free_processed_buffer(mng->deinterleave_buffer.buffer_channel_3);
    }
    if (mng->deinterleave_buffer.buffer_channel_4) {
        free_processed_buffer(mng->deinterleave_buffer.buffer_channel_4);
    }
    if (mng->overlap_buffer) {
        free_processed_buffer(mng->overlap_buffer);
    }
    if (mng->analysis_window) {
       free_processed_buffer(mng->analysis_window);
    }
    if (mng->synthesis_window) {
        free_processed_buffer(mng->synthesis_window);
    }
    
    FE_LOG("Frame streaming stopped and buffers freed\n");
}

static void _write_wav_header(FILE *out_file, const fe_audio_info_t *info, uint32_t data_size) {
    /* RIFF header */
    fwrite("RIFF", 1, 4, out_file);
    uint32_t file_size = 36 + data_size;  /* 36 = RIFF header + fmt chunk */
    fwrite(&file_size, 4, 1, out_file);
    fwrite("WAVE", 1, 4, out_file);
    
    /* fmt sub-chunk */
    fwrite("fmt ", 1, 4, out_file);
    uint32_t fmt_size = 16;  /* Standard PCM fmt chunk size */
    fwrite(&fmt_size, 4, 1, out_file);
    
    uint16_t audio_format = 1;  /* PCM */
    fwrite(&audio_format, 2, 1, out_file);
    fwrite(&info->num_channels, 2, 1, out_file);
    fwrite(&info->sample_rate, 4, 1, out_file);
    fwrite(&info->byte_rate, 4, 1, out_file);
    fwrite(&info->block_align, 2, 1, out_file);
    fwrite(&info->bits_per_sample, 2, 1, out_file);
    
    /* data sub-chunk */
    fwrite("data", 1, 4, out_file);
    fwrite(&data_size, 4, 1, out_file);
}

static void _write_wav_frame(fe_audio_file_ptr_t out_file, const sample_t *frame_buffer, u32 frame_size, 
                              u16 num_channels, u16 bits_per_sample) {
    for (u32 i = 0; i < frame_size * num_channels; i++) {
        sample_t sample = frame_buffer[i];
        
        if (bits_per_sample == BIT_PCM_FORMAT_16) {
            /* Convert from sample_t back to int16_t */
            #ifdef FIXED_POINT
                s16 val = (s16)sample;  /* sample_t is Q2.14 fixed-point */
            #else
                s16 val = (s16)(sample * 32767.0f);  /* Convert float to int16 */
            #endif
            fwrite(&val, 2, 1, out_file);
            
        } else if (bits_per_sample == BIT_PCM_FORMAT_24) {
            #ifdef FIXED_POINT
                s32 val_32 = ((s32)sample) << 16;  /* Q2.14 to 32-bit */
            #else
                s32 val_32 = (s32)(sample * 8388607.0f);  /* float, 24-bit scaled */
            #endif
            uint8_t bytes[3] = {
                (uint8_t)(val_32 & 0xFF),
                (uint8_t)((val_32 >> 8) & 0xFF),
                (uint8_t)((val_32 >> 16) & 0xFF)
            };
            fwrite(bytes, 1, 3, out_file);
            
        } else if (bits_per_sample == BIT_PCM_FORMAT_32) {
            #ifdef FIXED_POINT
                s32 val = ((s32)sample) << 16;
            #else
                s32 val = (s32)(sample * 2147483647.0f);
            #endif
            fwrite(&val, 4, 1, out_file);
        }
    }
}

FILE* fe_open_output_wav(const char *filename, const fe_audio_info_t *info, uint32_t total_samples) {
    FILE *out_file = fopen(filename, "wb");
    if (!out_file) {
        FE_ERROR("Cannot open output file %s for writing\n", filename);
        return NULL;
    }
    
    uint32_t num_bytes_per_sample = info->bits_per_sample / 8;
    uint32_t data_size = total_samples * info->num_channels * num_bytes_per_sample;
    
    _write_wav_header(out_file, info, data_size);
    return out_file;
}

void fe_write_frame_to_wav( fe_audio_file_ptr_t out_file, const sample_t *frame_buffer, u32 frame_size,
                            u16 num_channels, u16 bits_per_sample) {
    if (!out_file) return;
    _write_wav_frame(out_file, frame_buffer, frame_size, num_channels, bits_per_sample);
}

void inline fe_close_output_wav(fe_audio_file_ptr_t out_file) {
    if (out_file) {
        fclose(out_file);
    }
}

bool inline fe_is_processing_done(fe_manager_t *mng)
{
    return mng->buffer_mng.eof_reached;
}

void inline fe_report_memory_usage(const fe_manager_t *mng)
{
    if (!mng) return;
    
    printf("\n========== Memory Usage Report ==========\n");
    printf("Input Frame Buffer:   %zu bytes\n", mng->mem_stats.input_buffer_bytes);
    printf("Output Frame Buffer:  %zu bytes\n", mng->mem_stats.output_buffer_bytes);
    printf("─────────────────────────────────────\n");
    printf("Total Allocated:      %zu bytes (%.2f KB)\n", 
           mng->mem_stats.total_allocated,
           (float)mng->mem_stats.total_allocated / 1024.0f);
    printf("Peak Usage:           %zu bytes (%.2f KB)\n", 
           mng->mem_stats.peak_usage,
           (float)mng->mem_stats.peak_usage / 1024.0f);
    printf("========================================\n\n");
}

en_fe fe_init(fe_init_t *init)
{
    init->mng = (fe_manager_t *)malloc(sizeof(fe_manager_t));
    if (!init || !init->input_wav_file || !init->output_wav_file || init->frame_size_samples == 0) {
        FE_ERROR("Invalid initialization parameters\n");
        return FE_ERROR_INVALID_PARAM;
    }
    
    /* Initialize new OLA fields to NULL/0 */
    init->mng->overlap_buffer = NULL;
    init->mng->analysis_window = NULL;
    init->mng->synthesis_window = NULL;
    init->mng->ola_initialized = 0;
    
    /* Validate overlap percentage */
    if (init->overlap_percentage > 100) {
        FE_WARN("Overlap percentage clamped to 100%% (unstable above ~75%%)\n");
        init->overlap_percentage = 100;
    }
    
    FE_LOG("Initializing frontend with input: %s, output: %s, frame size: %u samples, overlap: %u%%\n", 
            init->input_wav_file, init->output_wav_file, init->frame_size_samples, init->overlap_percentage);

    /* Step 1: Parse WAV header and initialize audio info */
    fe_init_audio_info(init->mng, init->input_wav_file);
    if (init->mng->audio_info.sample_rate == 0) {
        FE_ERROR("Failed to initialize audio info from file: %s\n", init->input_wav_file);
        return FE_ERROR_INIT_AUDIO_INPUT_FAILED;
    }
    FE_LOG("Initializing audio input - Sample Rate: %u Hz, Channels: %u, Bits/Sample: %u\n", 
            init->mng->audio_info.sample_rate, init->mng->audio_info.num_channels, init->mng->audio_info.bits_per_sample);

    init->mng->output_wav_file = fe_open_output_wav(init->output_wav_file, &init->mng->audio_info, init->mng->config.num_samples);
    if(!init->mng->output_wav_file) {
        FE_ERROR("Failed to open output WAV file: %s\n", init->output_wav_file);
        return FE_ERROR_INIT_AUDIO_OUTPUT_FAILED;
    }
    FE_LOG("Output WAV file initialized: %s\n", init->output_wav_file);

    init->mng->config.module_flags = init->module_flags;
    init->mng->config.overlap_percentage = init->overlap_percentage;

    /* Step 2: Set frame size (which calculates hop_size based on overlap) */
    fe_init_frame_size(init->mng, init->frame_size_samples);

    /* Step 3: Start frame streaming (open file, allocate buffers) */
    fe_start_frame_streaming(init->mng);
    return FE_ERROR_NONE;
}