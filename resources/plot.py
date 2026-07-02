import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.signal import spectrogram

sr_in,  audio_in  = wavfile.read("C:/EXTRAS/Projects/JUCENoiseReduction/NewProject/resources/input_raw.wav")
sr_out, audio_out = wavfile.read("C:/EXTRAS/Projects/JUCENoiseReduction/NewProject/resources/output_processed.wav")


audio_in  = audio_in.astype(np.float32)  
audio_out = audio_out.astype(np.float32) 

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 8), sharex=True)

for ax, audio, sr, title in [
    (ax1, audio_in,  sr_in,  "Input (raw mic)"),
    (ax2, audio_out, sr_out, "Output (noise reduced)")
]:
    f, t, Sxx = spectrogram(audio, sr, nperseg=512, noverlap=256, window='hann')
    Sxx_dB = 10 * np.log10(Sxx + 1e-10)

    img = ax.pcolormesh(t, f, Sxx_dB, shading='gouraud', cmap='binary')
    ax.set_facecolor('white')
    ax.set_ylabel("Frequency (Hz)", color='black')
    ax.set_title(title, color='black')
    ax.set_ylim(0, 8000)
    ax.tick_params(colors='black')
    for spine in ax.spines.values():
        spine.set_edgecolor('black')
    fig.colorbar(img, ax=ax, label="dB")

ax2.set_xlabel("Time (s)", color='black')
fig.patch.set_facecolor('white')
plt.tight_layout()
plt.savefig("C:/EXTRAS/Projects/JUCENoiseReduction/NewProject/resources/city_noise_spectrogram.png", dpi=150, facecolor='white')
plt.show()

f_in, t_in, Sxx_in = spectrogram(audio_in, sr_in, nperseg=2048)
f_out, t_out, Sxx_out = spectrogram(audio_out, sr_out, nperseg=2048)

# 2. Average across the time axis to get the steady-state spectrum
avg_mag_in = np.mean(Sxx_in, axis=1)
avg_mag_out = np.mean(Sxx_out, axis=1)

# 3. Calculate dB reduction (avoiding division by zero)
reduction_db = 10 * np.log10(avg_mag_in / (avg_mag_out + 1e-10))

# 4. Plot
plt.figure(figsize=(10, 4))
plt.plot(f_in, reduction_db, color='black')
plt.title("Noise Reduction Performance (dB per Frequency)")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Reduction (dB)")
plt.grid(True, alpha=0.3)
plt.savefig("C:/EXTRAS/Projects/JUCENoiseReduction/NewProject/resources/city_noise_db_reduction.png", dpi=150, facecolor='white')
plt.show()