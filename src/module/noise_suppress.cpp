#include "noise_suppress.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>

NoiseSuppress::NoiseSuppress()
    : m_minStatState{}
{
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

    const int N = (int)pProcessBuf->bufferSize;
    if (N <= 0 || N > MAX_FRAME_SIZE) return;

    if (N != 512) {
#ifndef RELEASE_BUILD
        printf("Noise suppression: only 512 samples supported, got %d. Skipping.\n", N);
#endif
        return;
    }

    vInitNoiseState(N);

    /* 1. STFT: y[n] -> Y(f) */
    complex_t Y[N];
    for (int i = 0; i < N; i++) {
        Y[i].real = pProcessBuf->pBufferRef[i];
        Y[i].imag = 0.0f;
    }
    fft(Y, N);

    /* 2. Estimate noise spectrum N(f) via minimum statistics */
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
        noiseMag[i] = sqrtf(BETA * m_minStatState.pMin[i]);
    }

    /* 3. Spectral subtraction: S(f) = G(f) * Y(f) */
    complex_t S[N];
    for (int i = 0; i < N; i++) {
        float Y_mag = sqrtf(Y[i].real * Y[i].real + Y[i].imag * Y[i].imag);
        float S_mag = fmaxf(0.0f, Y_mag - noiseMag[i]);
        float gain  = (Y_mag > 0.0f) ? (S_mag / Y_mag) : 0.0f;

        S[i].real = gain * Y[i].real;
        S[i].imag = gain * Y[i].imag;
    }

    /* 4. iSTFT: S(f) -> s^[n] */
    ifft(S, N);
    for (int i = 0; i < N; i++) {
        pProcessBuf->pBufferRef[i] = (sample_t)S[i].real;
    }
}

void NoiseSuppress::vProcessBlockFix(TrplBufferStr *pProcessBuf)
{
    /* TODO: implement fixed-point spectral subtraction */
    (void)pProcessBuf;
}
