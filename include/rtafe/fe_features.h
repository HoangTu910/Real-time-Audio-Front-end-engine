/**
 * @file fe_features.h
 * @brief Optional feature-extraction API (log-mel filterbank, frame energy).
 */
#pragma once

#include "fe_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Number of mel-frequency bins produced per frame. */
#define FE_MEL_BINS  40

/**
 * Feature frame exported when FE_FLAG_FEATURES is enabled.
 */
typedef struct {
    int16_t log_mel[FE_MEL_BINS];   /**< Log-mel energies in Q7.8        */
    int16_t frame_energy;            /**< Total frame energy in Q7.8      */
} fe_feature_frame_t;

/**
 * Standalone feature extraction from the current spectral frame.
 *
 * May be called after fe_process_hop() regardless of FE_FLAG_FEATURES.
 * When FE_FLAG_FEATURES is set AND feature_out != NULL in fe_process_hop(),
 * this is called automatically.
 *
 * @param state  Initialised state with a valid spectral frame.
 * @param out    Destination for extracted features.
 * @return FE_OK on success; negative fe_status_t on error.
 */
fe_status_t fe_extract_features(const fe_state_t    *state,
                                fe_feature_frame_t  *out);

#ifdef __cplusplus
}
#endif
