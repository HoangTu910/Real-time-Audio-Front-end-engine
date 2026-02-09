/**
 * @file host_file_process.c
 * @brief Example: offline file-based audio processing using the RTAFE API.
 *
 * Reads raw 16 kHz stereo PCM from a file, runs the full front-end pipeline
 * hop-by-hop, and writes the enhanced mono output to another file.
 *
 * Usage:
 *   ./host_file_process  input_stereo.raw  output_mono.raw
 *
 * Input format : int16_t interleaved stereo, 16 kHz
 * Output format: int16_t mono, 16 kHz
 *
 * Build (from project root):
 *   cmake --build build --target host_file_process
 */

#include "rtafe/fe_api.h"
#include "rtafe/fe_features.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Helper: print version ──────────────────────────────────────────────── */

static void print_version(void)
{
    uint32_t v = fe_version();
    printf("RTAFE v%u.%u.%u\n",
           (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_stereo.raw> <output_mono.raw>\n", argv[0]);
        return 1;
    }

    print_version();

    /* ── 1. Open files ──────────────────────────────────────────────────── */

    FILE *fin = fopen(argv[1], "rb");
    if (!fin) { perror("fopen input"); return 1; }

    FILE *fout = fopen(argv[2], "wb");
    if (!fout) { perror("fopen output"); fclose(fin); return 1; }

    /* ── 2. Configure ───────────────────────────────────────────────────── */

    fe_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.sample_rate        = 16000;
    cfg.frame_len          = 256;     /* 16 ms @ 16 kHz              */
    cfg.hop_len            = 128;     /* 8 ms  (50% overlap)         */
    cfg.num_channels       = 2;       /* stereo input                */
    cfg.flags              = FE_FLAG_NOISE_SUPPRESS
                           | FE_FLAG_AGC
                           | FE_FLAG_FEATURES;
    /* Alpha coeffs config */
    cfg.dc_rm_alpha        = 32414; /* Q1.31 format */
    cfg.pre_emphasis_alpha = 1234;  /* Q1.15 format */

    /* Noise suppression tuning */
    cfg.ns_over_subtract   = 512;     /* ~1.0  in Q6.9               */
    cfg.ns_floor           = 26;      /* ~0.05 in Q6.9               */

    /* AGC tuning */
    cfg.agc_target_level   = 16384;   /* ~0.5  in Q1.15              */
    cfg.agc_attack_ms      = 10;
    cfg.agc_release_ms     = 100;

    /* ── 3. Allocate state + scratch (no malloc at runtime) ─────────────── */

    size_t state_sz   = fe_state_bytes(&cfg);
    size_t scratch_sz = fe_scratch_bytes(&cfg);

    printf("State  : %zu bytes\n", state_sz);
    printf("Scratch: %zu bytes\n", scratch_sz);

    void *state_mem   = malloc(state_sz);
    void *scratch_mem = malloc(scratch_sz);
    if (!state_mem || !scratch_mem) {
        fprintf(stderr, "Out of memory\n");
        free(state_mem); free(scratch_mem);
        fclose(fin); fclose(fout);
        return 1;
    }

    fe_state_t *state = (fe_state_t *)state_mem;

    /* ── 4. Initialise ──────────────────────────────────────────────────── */

    fe_status_t rc = fe_init(&cfg, state, scratch_mem, scratch_sz);
    if (rc != FE_OK) {
        fprintf(stderr, "fe_init failed: %d\n", rc);
        free(state_mem); free(scratch_mem);
        fclose(fin); fclose(fout);
        return 1;
    }

    /* ── 5. Process loop — one hop at a time ────────────────────────────── */

    const size_t hop          = cfg.hop_len;
    const size_t num_channels = cfg.num_channels;
    const size_t in_samples   = hop * num_channels;   /* interleaved stereo  */
    const size_t out_samples  = hop;               /* mono output         */

    int16_t *pcm_in  = (int16_t *)malloc(in_samples  * sizeof(int16_t));
    int16_t *pcm_out = (int16_t *)malloc(out_samples * sizeof(int16_t));
    if (!pcm_in || !pcm_out) {
        fprintf(stderr, "Out of memory (buffers)\n");
        free(pcm_in); free(pcm_out);
        free(state_mem); free(scratch_mem);
        fclose(fin); fclose(fout);
        return 1;
    }

    fe_feature_frame_t features;
    size_t total_hops = 0;

    while (fread(pcm_in, sizeof(int16_t), in_samples, fin) == in_samples) {

        rc = fe_process_hop(state,
                            pcm_in,
                            pcm_out,
                            &features,                   /* receive features  */
                            sizeof(features));

        if (rc != FE_OK) {
            fprintf(stderr, "fe_process_hop failed at hop %zu: %d\n", total_hops, rc);
            break;
        }

        fwrite(pcm_out, sizeof(int16_t), out_samples, fout);
        total_hops++;

        /* Example: print feature energy every 100 hops (~0.8 s) */
        if (total_hops % 100 == 0) {
            printf("  hop %6zu  frame_energy = %d (Q7.8)\n",
                   total_hops, features.frame_energy);
        }
    }

    printf("Processed %zu hops (%.2f s of audio)\n",
           total_hops,
           (double)(total_hops * hop) / (double)cfg.sample_rate);

    /* ── 6. Cleanup ─────────────────────────────────────────────────────── */

    fe_free(state);

    free(pcm_in);
    free(pcm_out);
    free(state_mem);
    free(scratch_mem);
    fclose(fin);
    fclose(fout);

    return 0;
}
