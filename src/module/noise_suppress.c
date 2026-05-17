#include "noise_suppress.h"

void process_noise_suppression(float *input, noise_suppress_t *ns)
{
    const int length = ns->frame_size_millis * ns->sample_rate / 1000; // Convert ms to samples
    if (length <= 0) {
        return;
    }

    /* Current FFT implementation supports N=512 only */
    if (length != 512) {
        return;
    }

    const int N = length;
    /* window */
    float windowed_input[N];
    for (int i = 0; i < N; i++) {
        windowed_input[i] = input[i];
    }
    hamming_window(windowed_input, N);

    /* Transform the noisy signal into the frequency domain using STFT */
    complex_t Y[N];
    for (int i = 0; i < N; i++) {
        Y[i].real = windowed_input[i];
        Y[i].imag = 0.0f;
    }
    fft(Y, N);

    /* Estimate the noise spectrum N(f) from the noisy signal Y(f) */
    /* Minimum statistics with smoothing and periodic reset */
    static float p_smooth[512];
    static float p_min[512];
    static int minimum_count = 0;
    static int noise_init = 0;
    static int noise_len = 0;

    if (!noise_init || noise_len != N) {
        for (int i = 0; i < N; i++) {
            p_smooth[i] = 0.0f;
            p_min[i] = 0.0f;
        }
        minimum_count = 0;
        noise_init = 1;
        noise_len = N;
    }

    const float alpha = 0.9f;
    const float beta = 1.0f;
    const int minimum_power_update_interval = 50;

    float noise_estimate[N];
    for (int i = 0; i < N; i++) {
        /* power of the noisy signal Y(f) */
        float p_Y = Y[i].real * Y[i].real + Y[i].imag * Y[i].imag;

        /* update the smoothed power estimate */
        p_smooth[i] = alpha * p_smooth[i] + (1.0f - alpha) * p_Y;

        /* update the minimum power estimate, why? Because it represents the lowest power level observed in the noise */
        if (p_min[i] == 0.0f || p_smooth[i] < p_min[i]) {
            p_min[i] = p_smooth[i];
        }
    }

    /* Update the minimum power estimate periodically */
    minimum_count++;
    if (minimum_count >= minimum_power_update_interval) {
        for (int i = 0; i < N; i++) {
            p_min[i] = p_smooth[i];
        }
        minimum_count = 0;
    }

    for (int i = 0; i < N; i++) {
        float noise_power = beta * p_min[i];
        noise_estimate[i] = sqrtf(noise_power);
    }

    /* Subtract the estimated noise spectrum from the noisy spectrum to get the clean spectrum S(f) */
    complex_t S[N];
    for (int i = 0; i < N; i++) {
        float Y_mag = sqrtf(Y[i].real * Y[i].real + Y[i].imag * Y[i].imag);
        float N_mag = noise_estimate[i];
        float S_mag = fmaxf(0.0f, Y_mag - N_mag);
        float gain = (Y_mag > 0.0f) ? (S_mag / Y_mag) : 0.0f;
        S[i].real = gain * Y[i].real;
        S[i].imag = gain * Y[i].imag;   
    }

    /* Transform the clean spectrum back to the time domain using inverse STFT */
    ifft(S, N);
    for (int i = 0; i < N; i++) {
        input[i] = S[i].real;
    }
}

void process_noise_suppression_fixed(s16 *input, noise_suppress_t *ns)
{
    const int length = ns->frame_size_millis * ns->sample_rate / 1000; // Convert ms to samples
    /* Similar to the floating-point version but using fixed-point arithmetic */
    /* This is a placeholder implementation and should be replaced with actual fixed-point processing */
    for (int i = 0; i < length; i++) {
        input[i] = input[i]; // No processing, just copy input to output
    }
}
