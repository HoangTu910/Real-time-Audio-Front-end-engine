#include "fe_init.h"

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

void _get_wav_info(const char *filename, fe_manager_t *mng)
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
        float f_sample = 0.0f;
        
        if (mng->info.bits_per_sample == BIT_PCM_FORMAT_16) {
            int16_t val;
            items_read = fread(&val, 2, 1, mng->file);
            if (items_read == 0) return false;  /* Hit EOF */
            
            #ifdef FIXED_POINT
                frame_buffer[i] = val;  /* int16_t is already Q2.14 format */
            #else
                f_sample = (float)val / 32768.0f;  /* Normalize to [-1.0, 1.0] */
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
                f_sample = (float)val_24 / 8388608.0f;  /* Normalize to [-1.0, 1.0] */
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
                f_sample = (float)val / 2147483648.0f;  /* Normalize to [-1.0, 1.0] */
                frame_buffer[i] = f_sample;
            #endif
            
        } else {
            fprintf(stderr, "Unsupported bits per sample: %u\n", mng->info.bits_per_sample);
            return false;   
        }
    }
    
    return true;  /* Frame read successfully */
}

sample_t _fe_process_sample(fe_manager_t *mng, sample_t in)
{
    sample_t out = in;

    if (mng->config.module_flags & FE_FLAG_DC_REMOVAL) {
        dc_removal_sample_process(&mng->state.dc_remov_block, &out);
    }
    if(mng->config.module_flags & FE_FLAG_PRE_EMPHASIS) {
        /* TBD */
    }
    if(mng->config.module_flags & FE_FLAG_NOISE_SUPPRESS) { 
        /* TBD */
    }
    FE_LOG("Processed sample: %d\n", out);
    return out;
}

void fe_process_frame(fe_manager_t *mng)
{
    /* Check if already at EOF */
    if (mng->buffer_mng.eof_reached) {
        return;
    }

    sample_t *input_buffer = mng->audio_buffer.input_buffer;
    sample_t *output_buffer = mng->audio_buffer.output_buffer;
    uint8_t num_channels = mng->audio_info.num_channels;

    /* Read frame from file */
    bool has_data = get_frame_buffer(mng, input_buffer);
    
    if (!has_data) {
        mng->buffer_mng.eof_reached = true;
        return;
    }

    /* Process each sample in the frame */
    for(u32 i = 0; i < mng->buffer_mng.frame_size * num_channels; i++) {
        output_buffer[i] = _fe_process_sample(mng, input_buffer[i]);
    }

    /* tracking how many samples we've processed */
    mng->buffer_mng.samples_read += mng->buffer_mng.frame_size;
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
        free(mng->audio_buffer.input_buffer);
        mng->audio_buffer.input_buffer = NULL;
    }
    if (mng->audio_buffer.output_buffer) {
        free(mng->audio_buffer.output_buffer);
        mng->audio_buffer.output_buffer = NULL;
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

static void _write_wav_frame(FILE *out_file, const sample_t *frame_buffer, u32 frame_size, 
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

void fe_write_frame_to_wav(FILE *out_file, const sample_t *frame_buffer, u32 frame_size,
                            u16 num_channels, u16 bits_per_sample) {
    if (!out_file) return;
    _write_wav_frame(out_file, frame_buffer, frame_size, num_channels, bits_per_sample);
}

void fe_close_output_wav(FILE *out_file) {
    if (out_file) {
        fclose(out_file);
    }
}

void fe_init(fe_init_t *init)
{
    if (!init || !init->filename) {
        FE_ERROR("Invalid initialization parameters\n");
        return;
    }

    fe_manager_t *mng = init->mng;

    /* Step 1: Parse WAV header and initialize audio info */
    fe_init_audio_info(mng, init->filename);
    if (mng->audio_info.sample_rate == 0) {
        FE_ERROR("Failed to initialize audio info from file: %s\n", init->filename);
        return;
    }

    /* Step 2: Set default frame size (can be overridden later) */
    u32 default_frame_size = (mng->audio_info.sample_rate * init->frame_size_millis) / 1000;
    fe_init_frame_size(mng, default_frame_size);

    /* Step 3: Start frame streaming (open file, allocate buffers) */
    fe_start_frame_streaming(mng);
}
