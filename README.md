# Real-Time Noise Reducer

This is a real-time noise reducer for live microphone input. It uses spectral subtraction to decrease stationary background noise, for example fan noise and air conditioner noise. I made it in C++ with the JUCE framework.

## Purpose

A noise gate mutes the signal when the level is below a threshold. When speech starts, the gate opens and you can hear the background noise again. This noise reducer measures the noise spectrum of the room. Then it subtracts that spectrum from each frame, also during speech.

## Formats

- **Standalone application:** Uses the microphone and audio drivers of the computer.
- **VST3 plugin:** Operates in a Digital Audio Workstation (DAW).
- **AU plugin:** The project enables AU. To build it, add an Xcode exporter to `NewProject.jucer`.

## Function

- **Frame analysis:** A 512-point FFT with a Hann window and a 256-sample hop (50% overlap).
- **Voice activity detection (VAD):** An energy-based detector identifies speech frames. During speech, the noise estimate does not change.
- **Digital silence threshold:** Frames with almost zero energy do not change the noise estimate. Thus, the startup silence of a microphone does not set the estimate to zero.
- **Asymmetric noise update:** Between words, the noise estimate increases quickly when the noise becomes louder. It decreases slowly when the noise becomes quieter.
- **Overlap-add reconstruction:** The output frames overlap by 50% and add together to make a continuous signal.

## Controls

| Control | Range | Default | Function |
| :--- | :--- | :--- | :--- |
| Subtraction Strength (α) | 0.0 to 9.0 | 2.0 | Sets how much of the noise estimate the plugin subtracts. |
| Spectral Floor (β) | 0.01 to 0.50 | 0.01 | Sets the minimum output level of each frequency bin, as a fraction of the input level. |
| Start Recording | — | — | Writes the input and the output to WAV files for tests. |

## Results

I measured the decrease in total signal power at each frequency:

- **White noise:** Approximately 19 to 20 dB from 200 Hz to 16 kHz.
- **Fan noise:** 15 to 20 dB from 4 kHz to 20 kHz. Below 3.5 kHz, the decrease is smaller because most speech energy is in that band.
- **City noise:** 6 to 17 dB.


## Limitations

- **Stationary noise:** The noise reducer is best for constant noise. It decreases noise that changes quickly less.
- **Musical noise:** High α values cause short, unwanted tones in the output.
- **Quiet speech:** The VAD uses one energy value for each frame. Very quiet speech can go into the noise estimate.
- **Startup:** The noise reducer needs approximately 0.1 s of room noise without speech. If you speak during this period, the noise estimate includes speech.
- **Slow decrease:** When the noise becomes quieter, the noise estimate needs approximately 1 s to decrease. During this time, the plugin can remove too much signal.
- **Latency:** The delay is 512 samples (10.7 ms at 48 kHz). The plugin does not report this delay to the host. Thus, a DAW does not correct it.
- **Mono:** The plugin uses only the left input channel. It sends the result to the left and right outputs.
- **Test recorder:** The recorder uses fixed Windows file paths. It operates only on my development computer.
- **Session state:** The plugin does not save α and β with a DAW session.

## Technical Documentation

For the signal path, the parameters, the development notes, and the full test results, read the **[technical documentation](https://delali-adanuty.github.io/NoiseReductionJUCE/)**.

---

**Author:** Van-Dyck Adanuty
