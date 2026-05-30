#include "noise_suppress.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>

NoiseSuppress::NoiseSuppress()
    : m_minStatState{}
{
    memset(m_overlapIn, 0, sizeof(m_overlapIn));
    memset(m_overlapOut, 0, sizeof(m_overlapOut));
    memset(m_minStatState.pSmooth, 0, sizeof(m_minStatState.pSmooth));
    memset(m_minStatState.pMin, 0, sizeof(m_minStatState.pMin));
}

void NoiseSuppress::vInitNoiseState(int frameSize)
{
    if (!m_minStatState.bInitialized || m_minStatState.lastFrameSize != frameSize) {
        memset(m_minStatState.pSmooth, 0, sizeof(float) * frameSize);
        memset(m_minStatState.pMin, 0, sizeof(float) * frameSize);
        m_minStatState.minimumCount = 0;
        m_minStatState.bInitialized = true;
        m_minStatState.lastFrameSize = frameSize;
    }
}

void NoiseSuppress::vProcessBlock(TrplBufferStr *pProcessBuf)
{
    if (pProcessBuf == nullptr || pProcessBuf->pBufferRef == nullptr) return;

    const int hop = (int)pProcessBuf->bufferSize;  /* caller provides hop-sized buffer (N/2) */
    if (hop != MAX_FRAME_SIZE / 2) {
#ifndef RELEASE_BUILD
        printf("Noise suppression: only hop=256 supported (N=512 FFT), got %d. Skipping.\n", hop);
#endif
        return;
    }
    const int N = hop * 2;  /* FFT size = 512 */

    vInitNoiseState(N);

    /* 1. Build N-sample frame with 50% overlap input:
     *    frame = [overlap_in (N/2) | new_input (N/2)] */
    float windowed[MAX_FRAME_SIZE];
    for (int i = 0; i < hop; i++) {
        windowed[i] = m_overlapIn[i];                            /* first half: previous tail */
    }
    for (int i = 0; i < hop; i++) {
        windowed[hop + i] = (float)pProcessBuf->pBufferRef[i];   /* second half: new input */
    }
    for (int i = 0; i < hop; i++) {
        m_overlapIn[i] = (float)pProcessBuf->pBufferRef[i];      /* save input for next frame */
    }

    /* 2. Analysis Hann window to reduce spectral leakage */
    hanning_window(windowed, N);

    /* 3. STFT: y[n] -> Y(f) */
    complex_t Y[N];
    for (int i = 0; i < N; i++) {
        Y[i].real = windowed[i];
        Y[i].imag = 0.0f;
    }
    fft(Y, N);

    /* 4. Estimate noise spectrum N(f) via minimum statistics */
    for (int i = 0; i < N; i++) {
        float p_Y = Y[i].real * Y[i].real + Y[i].imag * Y[i].imag;

        /* smoothed power estimate */
        m_minStatState.pSmooth[i] = ALPHA * m_minStatState.pSmooth[i] + (1.0f - ALPHA) * p_Y;

        /* track minimum power */
        if (m_minStatState.pMin[i] == 0.0f || m_minStatState.pSmooth[i] < m_minStatState.pMin[i]) {
            m_minStatState.pMin[i] = m_minStatState.pSmooth[i];
        }
    }

    /* periodic minimum reset */
    m_minStatState.minimumCount++;
    if (m_minStatState.minimumCount >= MIN_POWER_UPDATE_INTERVAL) {
        for (int i = 0; i < N; i++) {
            m_minStatState.pMin[i] = m_minStatState.pSmooth[i];
        }
        m_minStatState.minimumCount = 0;
    }

    /* noise magnitude estimate */
    float noiseMag[N];
    for (int i = 0; i < N; i++) {
        noiseMag[i] = sqrtf(m_minStatState.pMin[i]);
    }

    /* 5. Spectral subtraction: S(f) = G(f) * Y(f)
     *    G(f) = max(BETA, 1 - |N(f)| / |Y(f)|)
     *    BETA is the spectral floor to prevent musical noise */
    complex_t S[N];
    for (int i = 0; i < N; i++) {
        float Y_mag = sqrtf(Y[i].real * Y[i].real + Y[i].imag * Y[i].imag);
        float gain  = (Y_mag > 0.0f) ? fmaxf(BETA, 1.0f - noiseMag[i] / Y_mag) : BETA;

        S[i].real = gain * Y[i].real;
        S[i].imag = gain * Y[i].imag;
    }

    /* 6. iSTFT: S(f) -> s^[n] */
    ifft(S, N);

    /* 7. Synthesis Hann window for perfect reconstruction with 50% OLA */
    float synth[MAX_FRAME_SIZE];
    for (int i = 0; i < N; i++) {
        synth[i] = S[i].real;
    }
    hanning_window(synth, N);

    /* 8. Overlap-add reconstruction:
     *    - Add first N/2 of synthesis output to previous overlap tail (m_overlapOut)
     *    - Save last N/2 of synthesis output for next frame
     *    - Output N/2 reconstructed samples */
    for (int i = 0; i < hop; i++) {
        float ola_sample = synth[i] + m_overlapOut[i];
        pProcessBuf->pBufferRef[i] = (sample_t)ola_sample;
    }
    for (int i = 0; i < hop; i++) {
        m_overlapOut[i] = synth[hop + i];  /* save tail for next OLA */
    }
}

void NoiseSuppress::vProcessBlockFix(TrplBufferStr *pProcessBuf)
{
    /* TODO: implement fixed-point spectral subtraction */
    (void)pProcessBuf;
}
