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
}

void NoiseSuppress::vInitNoiseState(int frameSize)
{
    if (!m_minStatState.bInitialized || m_minStatState.lastFrameSize != frameSize) {
        memset(m_minStatState.pSmooth, 0, sizeof(float) * frameSize);
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
    float windowed[N];
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
    /* “Spectral Subtraction Based on Minimum Statistics” - Rainer Martin */
    float Pyy[N];
    for (int i = 0; i < N; i++) {
        float power = Y[i].real * Y[i].real + Y[i].imag * Y[i].imag;

        /* smoothed power estimate */
        m_minStatState.pSmooth[i] = 
            ALPHA_SMOOTH * m_minStatState.pSmooth[i] + (1.0f - ALPHA_SMOOTH) * power;

        /* track minimum power */
        Pyy[i] = m_minStatState.pSmooth[i];
    }

    memcpy(m_minStatState.minBuffer[m_minStatState.minBufIdx], Pyy, sizeof(float) * N);
    m_minStatState.minBufIdx = (m_minStatState.minBufIdx + 1) % MINSTAT_WINDOW;
    if (m_minStatState.minBufCount < MINSTAT_WINDOW)
        m_minStatState.minBufCount++;
    
    /* noise magnitude estimate */
    float Pnoise[N];
    /* TODO: Optimize this loop*/
    for (int i = 0; i < N; i++) {
        /**
         * Theory:
         * According to Martin, speech and noise are uncorrelated, so E[|Y(f)|^2] = E[|S(f)|^2] + E[|N(f)|^2].
         * Also, noise is assumed to be STATIONARY or SLOWLY VARYING compared to the speech. 
         * So we can simply track the minimum power in power spectrum (has been smoothed) over a window of frames.
         * 
         * Instead of knowing when speech is appearing to update noise estimate, Martin assumes that in a long enough frame
         * IT WILL ALWAYS has some frames with LOW speech or SILENT, that frame will have minimum power, which can be used as noise estimate.
         * 
         * Why minumum power is used to estimate noise? Assume we have
         * Frame 1: Speech = 8 + Noise = 2 -> Power = 10
         * Frame 2: Speech = 4 + Noise = 2 -> Power = 6
         * Frame 3: Speech = 0 + Noise = 2 -> Power = 2
         * Frame 4: Speech = 6 + Noise = 2 -> Power = 8
         * So again, according to Martin, there will be some frames with low speech energy (frame 3 in this example), 
         * and the minimum power across frames will be close to the noise power (2 in this example).
         * 
         * Noise is STABLE, so that's why I give it 2 for all frames, but speech is VARYING, so it has different values across frames.
         */
        float minVal = m_minStatState.minBuffer[0][i];
        /* find minimum value in the buffer */
        for (int j = 1; j < m_minStatState.minBufCount; j++) {
            if (m_minStatState.minBuffer[j][i] < minVal) {
                minVal = m_minStatState.minBuffer[j][i];
            }
        }
        Pnoise[i] = minVal * BIAS_CORR;  /* bias compensation */
    }

    /* 5. Compute gain G(f) and apply: S(f) = G(f) * Y(f) */
    complex_t S[N];
    for (int i = 0; i < N; i++) {
        float P_y = Y[i].real * Y[i].real + Y[i].imag * Y[i].imag;
        float gain = 0.0f;
        if (P_y > 0.0f) {
            float ratio = Pnoise[i] / P_y;
            gain = fmaxf(BETA_FLOOR, 1.0f - ratio);
        }
        else {
            gain = BETA_FLOOR;  /* if no signal, use floor gain */
        }
        S[i].real = gain * Y[i].real;
        S[i].imag = gain * Y[i].imag;
    }

    /* 6. iSTFT: S(f) -> s^[n] */
    ifft(S, N);

    /* 7. Overlap-add reconstruction:
     * COLA: Hann[n] + Hann[n + N/2] = 1.0, so OLA reconstructs perfectly.
     * - Add first N/2 of iFFT output to previous overlap tail (m_overlapOut)
     * - Save last N/2 of iFFT output for next frame
     * - Output N/2 reconstructed samples */
    for (int i = 0; i < hop; i++) {
        float ola_sample = S[i].real + m_overlapOut[i];
        pProcessBuf->pBufferRef[i] = (sample_t)ola_sample;
    }
    for (int i = 0; i < hop; i++) {
        m_overlapOut[i] = S[hop + i].real;  /* save tail for next OLA */
    }
}

void NoiseSuppress::vProcessBlockFix(TrplBufferStr *pProcessBuf)
{
    /* TODO: implement fixed-point spectral subtraction */
    (void)pProcessBuf;
}
