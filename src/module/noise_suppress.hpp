#ifndef NOISE_SUPPRESS_HPP
#define NOISE_SUPPRESS_HPP

/**
 * basic model of noise suppresion
 * x[n] = s[n] + noise[n]
 * x[n]: noisy signal
 * s[n]: clean signal
 * noise[n]: noise signal
 * 
 * We want to find s^[n] = estimate of s[n]
 * 
 * Linear filtering is a common approach to noise suppresion:
 * s^[n] = h[k] * x[n-k]  (convolution)
 * -> find h[k] that minimizes the mean squared error between s^[n] and s[n]
 * 
 * Spectral subtraction is another approach:
 * Given noisy signal as y[n], we have
 * y[n] = s[n] + d[n]
 * After applying STFT, we have
 * Y(f) = S(f) + D(f)
 * if we can estimate D(f), we can get S(f) = Y(f) - D(f) -> this is the basic idea of spectral subtraction
 * 
 * In STFT we have
 * |S(f)|^2 = |X(f)|^2 - |N(f)|^2
 * where S(f) is the clean signal spectrum, X(f) is the noisy signal spectrum, and N(f) is the noise spectrum
 * or
 * |S(f)| = max(0, |X(f)| - |N(f)|)
 * -> we have to estimate the noise spectrum N(f) and subtract it from the noisy spectrum X(f) to get the clean spectrum S(f)
 * 
 * Step to implement noise suppression:
 * 1. Transform the noisy signal into the frequency domain using STFT
 * -> y[n] -> Y(f)
 * 2. Estimate the noise spectrum N(f) from the noisy signal Y(f)
 * -> There are various methods to estimate the noise spectrum
 * 3. Subtract the estimated noise spectrum from the noisy spectrum to get the clean spectrum S(f)
 * -> S(f) = Y(f) - N(f)
 * -> We have 2 options: magnitude subtraction or power subtraction
 * -> Magnitude subtraction: |S(f)| = max(0, |Y(f)| - |N(f)|)
 * -> Power subtraction: |S(f)|^2 = max(0, |Y(f)|^2 - |N(f)|^2)
 * 4. Half-wave rectify the result to ensure non-negativity
 * -> S(f) = max(|S(f)|, beta|Y(f)|) where beta is a small positive constant to prevent musical noise
 * 5. Transform the clean spectrum back to the time domain using inverse STFT
 * -> S(f) -> s^[n]
 * 
 * Spectral subtraction can be rewrite as:
 * S(f) = G(f) * Y(f)
 * where G(f) is the gain function defined as:
 * G(f) = max(beta, 1 - alpha (|N(f)|^2 / |Y(f)|^2)) for power subtraction
 * G(f) = max(beta, 1 - alpha (|N(f)| / |Y(f)|)) for magnitude subtraction
 * N(f) is the estimated noise spectrum, Y(f) is the noisy spectrum, 
 * alpha is the over-subtraction factor, and beta is the spectral floor to prevent musical noise */

#include "idsp_module.hpp"
#include "utils.h"
#include "buffer_mng.hpp"
#include "fft.h"

/**
 * Spectral Subtraction Noise Suppression
 *
 * Model: x[n] = s[n] + noise[n]
 *
 * Gain function (magnitude subtraction):
 *   G(f) = max(beta, 1 - alpha * |N(f)| / |Y(f)|)
 *
 * Steps:
 *   1. STFT: y[n] -> Y(f)
 *   2. Estimate noise spectrum N(f) via minimum statistics
 *   3. Compute gain G(f) and apply: S(f) = G(f) * Y(f)
 *   4. iSTFT: S(f) -> s^[n]
 */
#define MINSTAT_WINDOW 30 
#define MAX_FRAME_SIZE 512
#define ALPHA_SMOOTH   0.8f      // smoothing
#define BETA_FLOOR     0.02f     // spectral floor
#define BIAS_CORR      1.5f      // bias compensation (approx)

typedef struct MinStatState {
    float pSmooth[512];
    bool bInitialized;
    int lastFrameSize;
    float minBuffer[MINSTAT_WINDOW][MAX_FRAME_SIZE];
    int   minBufIdx = 0;
    int   minBufCount = 0;
} MinStatState;

class NoiseSuppress : public IDSPModule {
public:
    NoiseSuppress();
    ~NoiseSuppress() override = default;

    void vProcessBlock(TrplBufferStr *pProcessBuf) override;
    void vProcessBlockFix(TrplBufferStr *pProcessBuf) override;

private:
    /* 50% overlap-add state */
    float m_overlapIn[MAX_FRAME_SIZE / 2];   /* input overlap: last N/2 of previous input */
    float m_overlapOut[MAX_FRAME_SIZE / 2];  /* output OLA: last N/2 of previous iFFT */

    /* minimum statistics noise estimation state */
    MinStatState m_minStatState;

    void vInitNoiseState(int frameSize);
};

#endif /* NOISE_SUPPRESS_HPP */
