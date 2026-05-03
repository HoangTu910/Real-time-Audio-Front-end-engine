#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "module/dc_removal.h"
#include "buffer_mng.h"
#include "utils.h"

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

typedef struct fe_buffer_manager_t {
    FILE *file;           /**< File pointer for the audio file (stays open during processing) */
    fe_audio_info_t info; /**< Audio format information */
    u32 frame_size;       /**< Number of samples per frame (e.g., 128 for 5ms @ 16kHz) */
    u32 total_samples;    /**< Total number of samples in the audio file */
    u32 samples_read;     /**< Samples already read/processed (track progress) */
    bool eof_reached;     /**< Flag: true when all samples processed */
} fe_buffer_manager_t;

typedef struct fe_memory_stats_t {
    size_t input_buffer_bytes;   /**< Input frame buffer size */
    size_t output_buffer_bytes;  /**< Output frame buffer size */
    size_t state_bytes;          /**< Internal DSP state */
    size_t total_allocated;      /**< Total bytes allocated */
    size_t peak_usage;           /**< Peak memory usage */
} fe_memory_stats_t;

typedef struct fe_manager_t {
    fe_config_t config;          /**< Configuration parameters */
    fe_state_t state;            /**< Internal state for processing */
    fe_audio_info_t audio_info;  /**< Audio format information */
    fe_audio_buffer_t audio_buffer; /**< Buffers for input and output audio data */
    fe_buffer_manager_t buffer_mng; /**< Manager for reading audio frames from file */
    fe_memory_stats_t mem_stats;    /**< Memory usage tracking */
} fe_manager_t;

typedef struct fe_init_t {
    fe_manager_t *mng;
    const char *filename;
    u32 frame_size_millis;  /**< Desired frame size in milliseconds (e.g., 5 for 5ms frames) */
} fe_init_t;


/* function declarations look like a mess lmao, need to cleanup this later :D */
sample_t _fe_process_sample(fe_manager_t *mng, sample_t in);
void fe_process_frame(fe_manager_t *mng);
void fe_init_audio_info(fe_manager_t *mng, const char *filename);
void fe_init_frame_size(fe_manager_t *mng, u32 frame_size_samples);
void fe_start_frame_streaming(fe_manager_t *mng);
void fe_stop_frame_streaming(fe_manager_t *mng);
void fe_init(fe_init_t *init);

/* WAV output functions */
FILE* fe_open_output_wav(const char *filename, const fe_audio_info_t *info, uint32_t total_samples);
void fe_write_frame_to_wav(FILE *out_file, const sample_t *frame_buffer, u32 frame_size,
                            u16 num_channels, u16 bits_per_sample);
void fe_close_output_wav(FILE *out_file);
bool fe_is_processing_done(fe_manager_t *mng);

/* Memory profiling */
void fe_report_memory_usage(const fe_manager_t *mng);

#define get_frame_buffer(mng, frame_buf) _read_wav_frame(&mng->buffer_mng, frame_buf)