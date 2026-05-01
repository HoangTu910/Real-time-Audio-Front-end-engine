#include "fe_init.h"

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

    // Read WAV header
    char riff[4], fmt[4], data[4];
    uint32_t file_size, fmt_size, byte_rate, sample_rate, data_size;
    uint16_t audio_format, num_channels, block_align, bits_per_sample;

    // Read RIFF header
    fread(riff, 1, 4, file);
    if (strncmp(riff, "RIFF", 4) != 0) {
        fprintf(stderr, "Error: Not a valid WAV file\n");
        fclose(file);
        info->sample_rate = 0;
        info->num_channels = 0;
        info->bits_per_sample = 0;
        return;
    }

    fread(&file_size, 4, 1, file);
    fread(riff, 1, 4, file);  // Read "WAVE"
    if (strncmp(riff, "WAVE", 4) != 0) {
        fprintf(stderr, "Error: Not a valid WAV file (missing WAVE)\n");
        fclose(file);
        info->sample_rate = 0;
        info->num_channels = 0;
        info->bits_per_sample = 0;
        return;
    }

    // Find fmt chunk
    while (fread(fmt, 1, 4, file) == 4) {
        fread(&fmt_size, 4, 1, file);
        
        if (strncmp(fmt, "fmt ", 4) == 0) {
            fread(&audio_format, 2, 1, file);
            fread(&num_channels, 2, 1, file);
            fread(&sample_rate, 4, 1, file);
            fread(&byte_rate, 4, 1, file);
            fread(&block_align, 2, 1, file);
            fread(&bits_per_sample, 2, 1, file);
            
            if (audio_format != 1) {
                fprintf(stderr, "Error: Only PCM format is supported\n");
                fclose(file);
                info->sample_rate = 0;
                info->num_channels = 0;
                info->bits_per_sample = 0;
                return;
            }
            
            // Skip remaining fmt chunk data if any
            if (fmt_size > 16) {
                fseek(file, fmt_size - 16, SEEK_CUR);
            }
            break;
        } else {
            // Skip this chunk
            fseek(file, fmt_size, SEEK_CUR);
        }
    }

    // Find data chunk
    while (fread(data, 1, 4, file) == 4) {
        fread(&data_size, 4, 1, file);
        
        if (strncmp(data, "data", 4) == 0) {
            break;
        } else {
            // Skip this chunk
            fseek(file, data_size, SEEK_CUR);
        }
    }

    // Calculate number of samples
    uint32_t num_bytes_per_sample = bits_per_sample / 8;
    uint32_t total_samples = data_size / (num_channels * num_bytes_per_sample);
    
    info->sample_rate = sample_rate;
    info->num_channels = num_channels;
    info->bits_per_sample = bits_per_sample;
    mng->input_buffer = (sample_t *)malloc(total_samples * num_channels * sizeof(sample_t));
    if (!mng->input_buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        info->sample_rate = 0;
        info->num_channels = 0;
        info->bits_per_sample = 0;
        return;
    }

    // Read PCM data and convert to float
    float scale_factor = 1.0f / (1 << (bits_per_sample - 1));
    
    for (uint32_t i = 0; i < total_samples; i++) {
        for (uint16_t ch = 0; ch < num_channels; ch++) {
            int32_t sample = 0;
            
            if (bits_per_sample == BIT_PCM_FORMAT_8) {
                uint8_t val;
                fread(&val, 1, 1, file);
                sample = (int32_t)(val - 128) << 24;  // Convert unsigned 8-bit to signed
            } else if (bits_per_sample == BIT_PCM_FORMAT_16) {
                int16_t val;
                fread(&val, 2, 1, file);
                sample = (int32_t)val << 16;
            } else if (bits_per_sample == BIT_PCM_FORMAT_24) {
                uint8_t bytes[3];
                fread(bytes, 1, 3, file);
                sample = (int32_t)(bytes[0] | (bytes[1] << 8) | (bytes[2] << 16));
                if (sample & 0x800000) sample |= 0xFF000000;  // Sign extend
                sample <<= 8;
            } else if (bits_per_sample == BIT_PCM_FORMAT_32) {
                int32_t val;
                fread(&val, 4, 1, file);
                sample = val;
            }
            
            // Convert to float [-1.0, 1.0]
            mng->input_buffer[i * num_channels + ch] = (sample_t)sample * scale_factor / (1 << 24);
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
    float *input_buffer = mng->input_buffer;
    float *output_buffer = mng->output_buffer;
    size_t num_samples = mng->config.num_samples;
    uint8_t num_channels = mng->config.num_channels;

    for(int i = 0; i < num_samples * num_channels; i++) {
        output_buffer[i] = _fe_process_sample(mng, input_buffer[i]);
    }
}

void fe_init_buffer(fe_manager_t *mng, const char *filename)
{
    size_t num_samples;
    fe_audio_info_t info;
    _wav_to_buffer(filename, mng, &info);
    mng->config.num_samples = num_samples;
    mng->output_buffer = (float *)malloc(num_samples * info.num_channels * sizeof(sample_t));
}
