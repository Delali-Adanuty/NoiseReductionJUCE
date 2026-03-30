# Demystifying DSP: From MATLAB Prototype to Real-Time C++ Plugin

Building a real-time digital signal processing (DSP) engine from scratch is a humbling experience. Recently, I set out to build a **Spectral Subtraction Noise Reducer** and this is designed to take live, noisy microphone input and intelligently subtract background room hiss without destroying the primary voice.

To tackle this, I adopted a two-phase approach: prototyping the math in MATLAB, and then translating it into a real-time C++ architecture using the JUCE framework. Throughout the project, I also utilized an AI assistant as a pair-programmer to help trace memory leaks, bounce diagnostic theories off of, and accelerate the debugging process.

Here is a breakdown of the development pipeline, the core challenges of moving from high-level math to low-level memory, and the engineering philosophy required to make it work.

---

## Phase 1: Proving the Math in MATLAB

In MATLAB, to build a spectral subtraction algorithm I load a `.wav` file, pass it through a Short-Time Fourier Transform (`stft`), and I got a matrix of frequency bins over time.After spending the last month unpacking the heavy theory behind the Fast Fourier Transform in my DSP class, finally getting to pull that math off the whiteboard and apply it to a real audio file was incredibly exciting. We did do matlab scripts on wave forms but applying them outside of that is refreshing in a different way.

The core logic was prototyped here:

1. **Estimate the Noise Floor:** Look at the first 0.5 seconds of audio (assuming it's just room tone) and average the magnitude of the frequency bins.
2. **Spectral Subtraction:** Subtract that noise magnitude from the rest of the audio signal.
3. **Floor Clamping:** Add a "spectral floor" (a beta multiplier) so that heavily subtracted bins don't drop to absolute zero, which causes a robotic "musical noise" artifact.
4. **Inverse Transform:** Run the `istft` to reconstruct the time-domain audio.

Inside MATLAB, tweaking the subtraction multiplier (Alpha) and regenerating a spectrogram took seconds. It wasn't too hard.

Then came the hard part.

---

## Phase 2: The C++ / JUCE Translation

Translating a pristine MATLAB script into a real-time C++ audio plugin is never a 1-to-1 process. MATLAB holds the entire audio file in memory and processes it at its leisure. A JUCE C++ plugin operates in real-time, grabbing chunks of audio (usually 512 samples at a time) and racing against a hard CPU deadline to process them before the speaker clicks.

To bridge this gap, I had to build custom plumbing:

- **Ring Buffers (FIFOs):** To decouple the DAW's unpredictable block sizes from my algorithm's strict `FFT_SIZE` requirements.
- **Overlap-Add (OLA) Architecture:** To stitch the processed frequency frames back together smoothly using a Hann window.
- **Asymmetric Tracking:** Because I couldn't just "look at the first 0.5 seconds" like in MATLAB, I had to build a real-time tracker that learned background noise quickly when the room was quiet, but paused learning when the user spoke.

With the architecture built and the AI helping to format the JUCE buffer logic, I hit compile. I expected clean audio. Instead, I got chaos.

---

## Phase 3: The Debugging War

When dealing with audio buffers, you can't _see_ the data failing. When the plugin loaded, it bounced between a speaker-blowing mechanical screech and absolute dead silence. To fix it, we had to adopt a strict engineering philosophy: **strip the complex system down to its absolute bare bones, and verify each piece one by one.**

### Challenge 1: The Infinite Screech (The Transform Trap)

**The Symptom:** Immediately upon loading the plugin, a constant, high-pitched mechanical tone blasted through my headphones.

**The Debugging Process (Binary Bypass Isolation):**
We stopped tweaking math variables and stripped the audio pipeline down to nothing.

1. We started by clearing the audio buffer completely (`buffer.clear()`). The screech stopped, proving the issue was inside the plugin, not the DAW routing.
2. We tested a direct passthrough (copying input straight to output). No screech.
3. We bypassed all the spectral math, running the audio purely through the Forward FFT, then immediately through the Inverse FFT. **The screech returned.**

**The Root Cause:**
By isolating the transform, I learned again the math wasn't the problem. But there were a few confusions with memory. I was feeding the same array pointer into the FFT for both input and output. While some libraries support in-place transforms, `juce::dsp::FFT::perform` on the MSVC compiler silently corrupted the frequency-domain data before the Inverse FFT could read it.

**The Fix:**
I separated the memory pipeline, creating a dedicated `fftOutput` buffer. The Forward FFT writes to the new buffer, the spectral math modifies it, and the Inverse FFT reads from it back into the complex buffer.

### Challenge 2: The VLA Memory Trap

**The Symptom:** Random audio dropouts and undefined runtime behavior, despite the code compiling perfectly.

**The Root Cause:**
In my initial block processing, I was sizing my arrays using runtime constants (`float frameIn[fftSize];`). These are Variable-Length Arrays (VLAs). While GCC and Clang often allow this C99 extension, MSVC strictly forbids it. Instead of throwing a hard compiler error, it was silently allocating uninitialized memory, feeding garbage data into the audio thread.

**The Fix:**
I replaced all VLAs with `std::array` using `static constexpr` sizes (`std::array<float, FFT_SIZE> frameIn;`). Pre-allocating fixed memory at compile time completely stabilized the runtime.

### Challenge 3: Dead Silence (The Architecture Pivot)

**The Symptom:** After fixing the screech, the output dropped to near-silence.

**The Root Cause:**

1. **The OLA Attenuation:** My Overlap-Add accumulator was halving the gain because it was zeroing out the tail end of the shift buffer before the second frame's contribution could be added.

**The Fix:**
I flipped the tracker rates. But more importantly, I applied the simplification philosophy to the architecture. Instead of endlessly tweaking OLA scale multipliers, I bypassed the accumulator entirely. Because I was using a Hann window at 50% overlap, I simply extracted the center `HOP_SIZE` samples directly from the Inverse FFT result and this is the exact region where the window function has full amplitude.

Audio immediately punched through at full volume.

## Current Constraints & Next Steps

Getting the memory and the math to finally play nicely together was the biggest hurdle, but the plugin isn't completely flawless yet. DSP is an iterative process, and here is what is on the roadmap for the next version:

- **Residual Static:** While the fatal screeching is entirely gone, there is still a faint baseline of "musical noise" (robotic static artifacts) that makes it through the spectral floor. We need to refine the smoothing across the frequency bins to reduce this.
- **Parameter Transitions:** Currently, adjusting the Alpha and Beta sliders in the UI causes slight audio glitches. The parameter transitions aren't exactly "clean" yet, so the next step is to implement linear interpolation (smoothing) for the parameters over the audio block to prevent "zipper noise."
- **Voice Activity Detection (VAD):** Our asymmetric tracker does a decent job of not muting the speaker by slowing down the learning rate when the signal is loud. However, building a dedicated VAD algorithm is the ultimate next step. This would allow the plugin to completely freeze the background noise profile updates the exact millisecond someone starts talking, preserving the voice perfectly.

---

## Key Takeaways

1. **MATLAB is for Math, C++ is for Memory:** Prototyping in MATLAB is essential for proving your algorithm, but 90% of C++ audio development is spent perfectly managing the memory pipeline before the math even happens.
2. **Binary Bypass is the Silver Bullet:** Never guess where an audio bug lives. Strip the pipeline to absolute zero and re-add components one by one (Routing -> Windowing -> Transform -> Math -> Reconstruction).
3. **Platform-Specific Quirks Matter:** Code that compiles on one machine isn't guaranteed to work on another. `std::array` and static buffer sizing are mandatory for stable C++ DSP.

Building this plugin reinforced a critical software engineering principle: you cannot blindly tweak variables to fix a complex system. You must isolate, verify, and rebuild from the ground up.
