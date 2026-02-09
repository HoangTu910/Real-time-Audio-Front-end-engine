#include "fe_api.h"
/* fe_api.c — Top-level pipeline orchestration */

fe_status_t fe_process_hop(fe_state_t *state, const q15_t *pcm_in, q15_t *pcm_out, void *feature_out, size_t feature_sz)
{
    uint16_t frame_len = state->frame_len;
    uint8_t num_channels = state->num_channels;

    for(uint8_t ch = 0; ch < num_channels; ch++) {
        /** Loop through each channels */
        DCRemoval *dc = &state->dc_block[ch];
        PreEmphasis *pre_emphasis = &state->pre_emphasis_block[ch];

        for(uint16_t n = 0; n < frame_len; n++) {
            uint16_t idx = n * num_channels + ch;
            q15_t sample = pcm_in[idx];

            /** 1. DC Removal */
            /** Convert Q15 -> Q31 */
            q31_t sample_q31 = ((q31_t)sample) << 16;
            sample_q31 = dc_removal_process(dc, sample_q31);

            /** 2. Pre-emphasis */ 
            q15_t sample_q15 = (q15_t)((sample_q31 + (1 << 15)) >> 16);
            sample_q15 = pre_emphasis_process(pre_emphasis, sample_q15);

            /** 3. AGC (optional) */ 
            /** sample = agc_process_sample(&state->agc_blocks[ch], sample); */ 

            pcm_out[idx] = sample_q15; // overwrite or copy to output buffer
        }
    }

    /** 4. Noise suppression (frame-based) - optional*/ 
    /** ns_process_frame(&state->ns_blocks[0], pcm_out, frame_len, num_mics); */ 

    /** 5. Feature extraction (optional) */ 
    /** if (feature_out) fe_extract_features(state, pcm_out, feature_out, feature_sz); */ 

    return FE_OK;
}

fe_status_t fe_init(const fe_config_t *cfg, fe_state_t *state, void *scratch, size_t scratch_bytes)
{
    state->frame_len = cfg->frame_len;
    state->num_channels = cfg->num_channels;
    state->scratch = (int32_t*)scratch;
    state->scratch_bytes = scratch_bytes;

    // allocate/init DC removal per channels
    for (uint8_t ch = 0; ch < state->num_channels; ch++) {
        dc_init(&state->dc_block[ch], cfg->dc_rm_alpha);
        pre_emphasis_init(&state->pre_emphasis_block, cfg->pre_emphasis_alpha);
    }

    // apply the same for: pre-emphasis, AGC, noise suppression

    return FE_OK;
}