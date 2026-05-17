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
 
#ifndef NOISE_SUPPRESS_H
#define NOISE_SUPPRESS_H
#include "fft.h"

typedef struct {
    u32 sample_rate;
    u32 frame_size_millis;
} noise_suppress_t;

void process_noise_suppression(float *input, noise_suppress_t *ns);
void process_noise_suppression_fixed(s16 *input, noise_suppress_t *ns);

#endif