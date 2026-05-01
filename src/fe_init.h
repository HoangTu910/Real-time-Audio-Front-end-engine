#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "module/dc_removal.h"

#define BIT_PCM_FORMAT_8 8
#define BIT_PCM_FORMAT_16 16
#define BIT_PCM_FORMAT_24 24
#define BIT_PCM_FORMAT_32 32

#define FE_FLAG_DC_REMOVAL      0x01
#define FE_FLAG_PRE_EMPHASIS    0x02
#define FE_FLAG_NOISE_SUPPRESS  0x04
#define FE_FLAG_AGC              0x08

typedef struct fe_config_t {
    uint16_t frame_len;          /**< Frame length in samples (e.g., 512) */
    uint8_t num_channels;        /**< Number of input channels (e.g., 1 for mono) */
    uint32_t sample_rate;        /**< Sampling rate in Hz (e.g., 16000) */
    uint8_t module_flags;        /**< Bitfield for module enable/disable (e.g., noise suppression) */
    size_t num_samples;          /**< Total number of samples in the input buffer */
} fe_config_t;

typedef struct fe_state_t {
    dc_remov dc_remov_block;          /**< DC removal state (per channel) */
    /*...*/
} fe_state_t;

typedef struct fe_audio_info_t {
    uint32_t file_size, fmt_size, byte_rate, sample_rate, data_size;
    uint16_t audio_format, num_channels, block_align, bits_per_sample;
} fe_audio_info_t;

typedef struct fe_audio_buffer_t {
    sample_t *input_buffer;   
    sample_t *output_buffer;
} fe_audio_buffer_t;

typedef struct fe_manager_t {
    fe_config_t config;          /**< Configuration parameters */
    fe_state_t state;            /**< Internal state for processing */
    fe_audio_info_t audio_info;  /**< Audio format information */
    fe_audio_buffer_t audio_buffer; /**< Buffers for input and output audio data */
} fe_manager_t;


void _wav_to_buffer(const char *filename, fe_manager_t *mng, fe_audio_info_t *info);
sample_t _fe_process_sample(fe_manager_t *mng, sample_t in);
void fe_process(fe_manager_t *mng);
void fe_init_buffer(fe_manager_t *mng, const char *filename);