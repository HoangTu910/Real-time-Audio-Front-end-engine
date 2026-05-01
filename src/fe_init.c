#include "fe_init.h"

void _read_riff_header(FILE *file, fe_audio_info_t *info) {
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

void _find_fmt_chunk(FILE *file, fe_audio_info_t *info) {
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

void _find_data_chunk(FILE *file, fe_audio_info_t *info) {
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

void _wav_to_buffer(const char *filename, fe_manager_t *mng, fe_audio_info_t *info)
{
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        info->sample_rate = 0;
        info->num_channels = 0;
        info->bits_per_sample = 0;
        return;
    }

    _read_riff_header(file, info);
    _find_fmt_chunk(file, info);
    _find_data_chunk(file, info);

    /* calculate number of samples */
    uint32_t num_bytes_per_sample = info->bits_per_sample / 8;
    uint32_t total_samples = info->data_size / (info->num_channels * num_bytes_per_sample);

    mng->config.num_samples = total_samples;
    
    info->sample_rate = info->sample_rate;
    info->num_channels = info->num_channels;
    info->bits_per_sample = info->bits_per_sample;
    mng->audio_buffer.input_buffer = (sample_t *)malloc(total_samples * info->num_channels * sizeof(sample_t));
    if (!mng->audio_buffer.input_buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        info->sample_rate = 0;
        info->num_channels = 0;
        info->bits_per_sample = 0;
        return;
    }

    /* read PCM data and convert to float if required */
    float scale_factor = 1.0f / (1 << (info->bits_per_sample - 1));
    
    for (uint32_t i = 0; i < total_samples; i++) {
        for (uint16_t ch = 0; ch < info->num_channels; ch++) {
            int32_t sample = 0;
            
            if (info->bits_per_sample == BIT_PCM_FORMAT_8) {
                uint8_t val;
                fread(&val, 1, 1, file);
                sample = (int32_t)(val - 128) << 24;  // Convert unsigned 8-bit to signed
            } else if (info->bits_per_sample == BIT_PCM_FORMAT_16) {
                int16_t val;
                fread(&val, 2, 1, file);
                sample = (int32_t)val << 16;
            } else if (info->bits_per_sample == BIT_PCM_FORMAT_24) {
                uint8_t bytes[3];
                fread(bytes, 1, 3, file);
                sample = (int32_t)(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16));
                if (sample & 0x800000) sample |= 0xFF000000;  // Sign extend
                sample <<= 8;
            } else if (info->bits_per_sample == BIT_PCM_FORMAT_32) {
                int32_t val;
                fread(&val, 4, 1, file);
                sample = val;
            }
            
            float f_sample = (sample_t)sample * scale_factor / (1 << 24);
            #ifdef FIXED_POINT
                mng->audio_buffer.input_buffer[i * info->num_channels + ch] = FLOAT_TO_Q2_14(f_sample);
            #else 
                mng->audio_buffer.input_buffer[i * info->num_channels + ch] = f_sample;
            #endif
        }
    }

    fclose(file);
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
    return out;
}

void fe_process(fe_manager_t *mng)
{
    sample_t *input_buffer = mng->audio_buffer.input_buffer;
    sample_t *output_buffer = mng->audio_buffer.output_buffer;
    size_t num_samples = mng->config.num_samples;
    uint8_t num_channels = mng->config.num_channels;

    for(int i = 0; i < num_samples * num_channels; i++) {
        output_buffer[i] = _fe_process_sample(mng, input_buffer[i]);
    }
}

void fe_init_buffer(fe_manager_t *mng, const char *filename)
{
    _wav_to_buffer(filename, mng, &mng->audio_info);
    if(mng->audio_info.sample_rate != SAMPLING_RATE) {
        FE_WARN("Current sampling rate is %u, we expect it to be %f\n", mng->audio_info.sample_rate, SAMPLING_RATE);
    }
    mng->audio_buffer.output_buffer = (sample_t*)malloc(mng->config.num_samples * mng->audio_info.num_channels * sizeof(sample_t));
}
