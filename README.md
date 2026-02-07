# Specs

## Real-time Audio Front-end engine for SoC
### Overview
Design and implement a fully C library for audio signal processing (audio front-end) run real-time, streaming, fixed-point. This library need to be constraint to the memory of SoC (every SoC) and has embedded API.

Supporting feature:
- Sampling rate: 16kHz (support 48kHz extension)
- Number of mic: 2 (stereo/dual-mic)
- Frame length: 256 samples (16 ms), hop = 128 samples (8 ms) (50% overlap)
- Memory target: 256 KB RAM
- Latency target end-to-end <= 20 ms
- Data format core: int16_t input/output (Q1.15), accumulators int32/int64

### Requirements
### 1. Functional
- DC removal (per channel)
- Pre-emphasis 
- STFT (fixed-point) or uniform filterbank
- Spectral noise suppression (DSP-based: spectral substraction / Wiener)
- Gain smoothing and AGC
- iFFT + overlap-add reconstruction
- Optional: beamforming (basic), echo suppresion stub
- Optional: Feature extraction (log-mel, energy) with INT8/INT16 export

### 2. Non-functional
- Deterministic streaming API (no malloc in runtime)
- Bit-extract fixed-point behavior where specified
- Thread safe if each stream has own state object
- Minimal heap usage; caller supplies scratch buffer
- Portable platform abstraction for SIMD and cycle counting

### 3. High-level
Input (per-frame, interleaved or per-channel buffer) -> DC removal -> Pre-emphasis -> Windowing + STFT -> Spectral processing (noise estimate, gain compute, smoothing) -> Apply gain -> iFFT -> Overlap-add -> AGC -> Output.

### Block details & Q-format mapping (suggestion)
- Input PCM: int16_t (Q1.15)
- DC Removal IIR: state int32_t (Q1.31 scaled for safe substraction)
- Pre-emphasis FIR coeffs: int16_t (Q1.15), accumulators int32_t (Q1.31)
- Window samples: precomputed in Q1.15
- FFT twiddles: int32_t Q1.31
- FFT internal stages: use int32_t accumulators with block scaling (keep headroom)
- Spectral magnitudes: int32_t energies (Q-format depends, document per implementation)
- Gain values (applies to spectrum): int16_t Q6.9 (range 0..1 represented)
- iFFT accumulators: int32_t with scaling matched to FFT
- AGC envelope: int32 Q1.31 or Q6.25 depending dynamics
- Output PCM: convert back to Q1.15 (int16_t) with saturation

### Deliverables & artifacts (what to hand over)
ref/python pipeline with notebook showing metrics and plots
src/ C library (buildable via CMake) with headers and docs
tests/ unit tests + regression vectors, automated in CI
docs/:
design.md: architecture description
q_formats.md: mapping & arithmetic rules
performance.md: profiling snapshots, bottlenecks, optimization log
readme_interview.md: 1–2 page summary: why choices, tradeoffs, migration to another SoC
One page “integration note”: how to embed library into firmware (mem layout, ISR hints)

### API public (C, minimal streaming pattern)
```
// audio_fe.h (skeleton)
#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t sample_rate;   // e.g., 16000
    uint16_t frame_len;     // e.g., 256
    uint16_t hop_len;       // e.g., 128
    uint8_t  num_mics;      // e.g., 2
    // flags: enable_noise_suppress, enable_agc, enable_features
    uint8_t  flags;
    // Q-format / tuning params can be here or loaded separately
} fe_config_t;

typedef struct fe_state_t fe_state_t;

// scratch_bytes = function to compute required scratch size
size_t fe_scratch_bytes(const fe_config_t *cfg);

// init: caller provides preallocated state and scratch memory
int fe_init(const fe_config_t *cfg, fe_state_t *state, void *scratch, size_t scratch_bytes);

// process one hop (hop_len samples per channel). Input interleaved or planar per config
int fe_process_hop(fe_state_t *state,
                   const int16_t *pcm_in,    // num_mics * hop_len samples interleaved/planar
                   int16_t *pcm_out,         // hop_len samples (mono) or per config
                   void *feature_out,        // nullable
                   size_t feature_bytes);

int fe_reset(fe_state_t *state);
int fe_free(fe_state_t *state); // optional, if any dynamic resources in platform
```