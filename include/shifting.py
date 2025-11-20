import numpy as np
import matplotlib.pyplot as plt

# --- Parameters ---
fs = 150000          # Sample rate
f_sine = 1500       # Input sine frequency
duration = 0.001     # seconds
N = int(fs * duration)

# --- Generate test sine wave ---
t = np.arange(N) / fs
audio = (16000 * np.sin(2 * np.pi * f_sine * t)).astype(np.int16)

# --- SSB demod phasing variables ---
ssb_phase = 0

# --- Storage ---
I_samples = np.zeros(N, dtype=np.int16)
Q_samples = np.zeros(N, dtype=np.int16)

# --- Simple decode_iq: just store i and q ---
def decode_iq(i, q):
    return i, q

# --- decode_audio equivalent ---
for n in range(N):
    audio_sample = audio[n] >> 1
    ssb_phase = (ssb_phase + 1) & 3

    audio_i = np.array([audio_sample, 0, -audio_sample, 0], dtype=np.int16)
    audio_q = np.array([0, -audio_sample, 0, audio_sample], dtype=np.int16)

    ii = audio_i[ssb_phase]
    qq = audio_q[ssb_phase]

    sample_i = np.array([-qq, -ii, qq, ii], dtype=np.int16)
    sample_q = np.array([ii, -qq, -ii, qq], dtype=np.int16)

    i = sample_i[ssb_phase]
    q = sample_q[ssb_phase]

    I_samples[n], Q_samples[n] = decode_iq(i, q)

# --- Normalize to -1 to 1 ---
max_val = max(np.max(np.abs(audio)), np.max(np.abs(I_samples)), np.max(np.abs(Q_samples)))
audio_norm = audio / max_val
I_norm = I_samples / max_val
Q_norm = Q_samples / max_val

# --- Plot normalized signals ---
plt.figure(figsize=(12, 6))
plt.plot(t, audio_norm, label='Input', linewidth=1, alpha=0.7)
plt.plot(t, I_norm, label='I (in-phase)', linewidth=1)
plt.plot(t, Q_norm, label='Q (quadrature)', linewidth=1)
plt.xlabel('Time (s)')
plt.ylabel('Normalized Amplitude')
plt.title('Normalized Input Sine Wave and SSB Demodulated I/Q Signals')
plt.grid(True, alpha=0.3)
plt.legend()
plt.tight_layout()
plt.show()
