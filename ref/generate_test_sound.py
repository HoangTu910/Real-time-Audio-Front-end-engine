import numpy as np
import wave
import os

def write_wav_stereo_int16(path, samples_l, samples_r, sample_rate):
    """Write stereo WAV file (interleaved L/R)."""
    samples_l = np.clip(samples_l, -1.0, 1.0)
    samples_r = np.clip(samples_r, -1.0, 1.0)
    pcm_l = (samples_l * 32767).astype(np.int16)
    pcm_r = (samples_r * 32767).astype(np.int16)
    
    # Interleave L/R
    interleaved = np.empty((len(pcm_l) * 2,), dtype=np.int16)
    interleaved[0::2] = pcm_l
    interleaved[1::2] = pcm_r

    with wave.open(path, "wb") as wf:
        wf.setnchannels(2)
        wf.setsampwidth(2)
        wf.setframerate(sample_rate)
        wf.writeframes(interleaved.tobytes())

def write_wav_mono_int16(path, samples, sample_rate):
    samples = np.clip(samples, -1.0, 1.0)
    pcm = (samples * 32767).astype(np.int16)

    with wave.open(path, "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)  # int16
        wf.setframerate(sample_rate)
        wf.writeframes(pcm.tobytes())

def generate_test_signal(sample_rate=16000, duration_s=2.0):
    """
    Generate comprehensive test signal for speech processing pipeline.
    
    Tests:
    - DC removal (DC-biased segment)
    - Pre-emphasis (low vs high freq boost)
    - FFT & windowing (sine waves at different frequencies)
    - Noise suppression (white noise + speech)
    - Adaptive noise floor (minimum tracking)
    """
    n_total = int(sample_rate * duration_s)
    t = np.arange(n_total) / sample_rate
    noise_floor = 0.08  # Background noise throughout
    
    segments = [
        # (start_s, end_s, description)
        (0.0,   0.2,   "silence_with_dc"),
        (0.2,   0.5,   "low_freq_50hz"),      # Tests pre-emphasis baseline (not boosted)
        (0.5,   0.8,   "mid_freq_440hz"),     # Speech formant region
        (0.8,   1.2,   "chirp_300_3000hz"),   # Speech-like sweep, high freq boosted
        (1.2,   1.6,   "high_freq_3khz"),     # Tests pre-emphasis boost
        (1.6,   1.9,   "bursts_with_noise"),  # Impulses + noise (VAD-like behavior)
        (1.9,   2.0,   "fadeout"),
    ]
    
    x = np.zeros(n_total, dtype=np.float32)
    
    for start_s, end_s, seg_type in segments:
        i0 = int(start_s * sample_rate)
        i1 = int(end_s * sample_rate)
        ts = t[i0:i1]
        
        if seg_type == "silence_with_dc":
            # DC offset (0.1 * range) + tiny noise to prevent complete silence
            x[i0:i1] = 0.08 + 0.02 * np.random.randn(i1 - i0)
            
        elif seg_type == "low_freq_50hz":
            # Low frequency: tests DC removal ripple, pre-emphasis minimal boost
            sig = 0.3 * np.sin(2 * np.pi * 50 * ts)
            x[i0:i1] = sig + noise_floor * np.random.randn(i1 - i0)
            
        elif seg_type == "mid_freq_440hz":
            # Speech formant: 440 Hz (A4 note) - reference for pre-emphasis
            sig = 0.35 * np.sin(2 * np.pi * 440 * ts)
            x[i0:i1] = sig + noise_floor * np.random.randn(i1 - i0)
            
        elif seg_type == "chirp_300_3000hz":
            # Linear chirp over speech frequency range
            # Demonstrates how pre-emphasis boosts high frequencies
            f0, f1 = 300.0, 3000.0
            duration = end_s - start_s
            k = (f1 - f0) / duration
            phase = 2 * np.pi * (f0 * (ts - start_s) + 0.5 * k * (ts - start_s) ** 2)
            sig = 0.4 * np.sin(phase)
            x[i0:i1] = sig + noise_floor * np.random.randn(i1 - i0)
            
        elif seg_type == "high_freq_3khz":
            # High frequency: 3 kHz (boosted by pre-emphasis)
            sig = 0.25 * np.sin(2 * np.pi * 3000 * ts)
            x[i0:i1] = sig + noise_floor * np.random.randn(i1 - i0)
            
        elif seg_type == "bursts_with_noise":
            # Speech-like bursts in noise: tests adaptive noise floor tracking
            # and VAD-like behavior
            noise = 0.15 * np.random.randn(i1 - i0)
            bursts = np.zeros(i1 - i0)
            
            # Add 3 short "phonemes" (30ms bursts)
            burst_len = int(0.03 * sample_rate)  # 30ms burst
            for burst_idx in range(3):
                burst_start = int(burst_idx * 0.13 * sample_rate)
                burst_end = min(burst_start + burst_len, i1 - i0)
                if burst_start < i1 - i0:
                    freq = 200 + burst_idx * 400  # Varying frequency
                    burst_ts = (np.arange(burst_end - burst_start) / sample_rate)
                    bursts[burst_start:burst_end] = 0.4 * np.sin(2 * np.pi * freq * burst_ts)
            
            x[i0:i1] = bursts + noise
            
        elif seg_type == "fadeout":
            # Fade to zero
            fade = np.linspace(1.0, 0.0, i1 - i0)
            x[i0:i1] = noise_floor * fade * np.random.randn(i1 - i0)
    
    # Smooth fade in/out at boundaries to avoid clicks from DC removal
    fade_len = int(0.005 * sample_rate)  # 5ms fade
    fade_in = np.linspace(0.0, 1.0, fade_len)
    fade_out = fade_in[::-1]
    x[:fade_len] *= fade_in
    x[-fade_len:] *= fade_out
    
    # Ensure stays within [-1, 1] after all processing
    x = np.clip(x, -0.95, 0.95)
    
    return x

if __name__ == "__main__":
    sr = 16000
    signal = generate_test_signal(sample_rate=sr, duration_s=2.0)
    
    # Generate stereo version (2 channels with slight delay for beamformer test)
    delay_samples = int(0.002 * sr)  # 2ms delay on channel 2
    signal_ch2 = np.concatenate([np.zeros(delay_samples), signal[:-delay_samples]])
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    tests_dir = os.path.join(script_dir, "..", "tests")
    os.makedirs(tests_dir, exist_ok=True)
    
    # Mono version
    mono_path = os.path.join(tests_dir, "test_signal_mono.wav")
    write_wav_mono_int16(mono_path, signal, sr)
    print(f"Wrote {mono_path}")
    
    # Stereo version (for beamformer testing)
    stereo_path = os.path.join(tests_dir, "test_signal_stereo.wav")
    write_wav_stereo_int16(stereo_path, signal, signal_ch2, sr)
    print(f"Wrote {stereo_path}")