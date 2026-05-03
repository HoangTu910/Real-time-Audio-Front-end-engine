# Real-time Audio Front-end engine

This repo was inspired from AudioNoise repo of Linus Torvald. I think it would be great if I can implement some ADSP modules in both fixed-point and floating-point number, any modules that I can come up with. And of course, try to optimize it as good as possible, apply some methods like loop-unrolling, software-pipelining and so on. Learning about DSP and write optimize program should be the objective of this repo, and analyze the profiling data also.

Benchmarking on hardware could be done in the future. In the early phase of this project, all the modules should work properly first on my computer, then optimize, and then benchmark it on hardware. I think about some hardwares like Arm-M4 or M7, or even some SoC platforms if possible. The benchmarking data of hardware with and without FPU could be also good, it would give a better perspective about fixed-point and floating-point implementation (in term of sound quality and performance trade-offs).

Extension for streaming song or microphone, or some advance modules or even multi-channel processing could also be considered in the future (if I have time and interest enough).

More update later, for now, nothing can be used in this repo, some testing still on-going and still thinking about which module I should do next.

And yes, no vibe-coding in this project, maybe I will need some help from AI (still learning though), but will try not to depend too much on them.

In case you want to run some test:
```bash
make test_*(* is anything you found it makefile)
```

## How it works
No ADC or DAC for now (I will do it in the future :D). It just purely reads the .wav file, starts the processing, and then gives the output .wav file. Reading all data in the .wav file into a buffer array and then processing it would not be sufficient. In the case of a 0 - 1 second WAV file, it still works fine, but imagine 3 or 5 minutes at a 48 kHz sampling rate, the buffer needed to store those data would be too large. So this module will process each frame of the .wav file, frame length is about 5-10 ms. It will perform as follows: Read the .wav file, extract the data frame, check the current state of the .wav file, process the frame, output the frame, merge the output frame to the output .wav file, and continue until the end of the .wav file. With this frame-based processing, the memory needed for each processing loop will be reduced. Of course, there will be a trade-off between memory and speed (you know, some more latencies for accessing the memory and allocating it); choosing the appropriate frame size for the module is required to give the best performance.

Run the below test to see how it processes an audio file:
```bash
make test_buffer_frame
```