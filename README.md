# Real-time Audio Front-end engine
## Some first words
*NOTE: (just ignore this part if you not interested in)*

This repo was inspired from AudioNoise repo of Linus Torvald. I think it would be great if I can implement some ADSP modules in both fixed-point and floating-point number, any modules that I can come up with. And of course, try to optimize it as good as possible, apply some methods like loop-unrolling, software-pipelining and so on. Learning about DSP and write optimize program should be the objective of this repo, and analyze the profiling data also.

Benchmarking on hardware could be done in the future. In the early phase of this project, all the modules should work properly first on my computer, then optimize, and then benchmark it on hardware. I think about some hardwares like Arm-M4 or M7, or even some SoC platforms if possible. The benchmarking data of hardware with and without FPU could be also good, it would give a better perspective about fixed-point and floating-point implementation (in term of sound quality and performance trade-offs).

Extension for streaming song or microphone, or some advance modules or even multi-channel processing could also be considered in the future (if I have time and interest enough).

And yes, no vibe-coding in this project, maybe I will need some help from AI (still learning though), but will try not to depend too much on them.

No ADC or DAC for now (maybe I will do it in the future). It just purely reads the .wav file, starts the processing, and then gives the output .wav file. Reading all data in the .wav file into a buffer array and then processing it would not be sufficient. In the case of a 0 - 1 second WAV file, it still works fine, but imagine 3 or 5 minutes at a 48 kHz sampling rate, the buffer needed to store those data would be too large. So this module will process each frame of the .wav file, frame length is about 5-10 ms with 50% overlapping (the overlap size can be adjusted). It will perform as follows: Read the .wav file, extract the data frame, check the current state of the .wav file, process the frame, output the frame, merge the output frame to the output .wav file, and continue until the end of the .wav file. With this frame-based processing, the memory needed for each processing loop will be reduced. Of course, there will be a trade-off between memory and speed (you know, some more latencies for accessing the memory and allocating it); choosing the appropriate frame size for the module is required to give the best performance.

## Build & Run
The current skeleton implementation for this project is *test_benchmark.c*. You can go to that file and enable any module flag you want to run, remember to config the module first before running. The build script will only build that source file for testing.

For more infomation about build script, please run:
```bash
./build.sh --help
```

gem5 is used as simulation environment. For now, only *x86* and *arm* architecture is enabled for this project. To benchmark with *arm* architecture, you first need to build the source code with the arm flag. You can also change the CPU type and adjust some parameters about memory in *arm_core_config.py*.

Then you run the benchmark script, it will generate you an html file containing all profiling data of the module:
```bash
./benchmark.sh
```
## Important modules
### Filter
TBD
### Noise suppresion
For noise suppresion module, there are multiple methods to implement a proper one. I have researched some methods that normally used for embedded devices and I stumbed upon the **Spectral Subtraction** approach. The mathematics model is quite simple, I think this is one of the simplest model for noise suppresion model, the most important thing to make this model works well is the **Noise Estimation** algorithm.

Given noisy model as:
```
y(n) = x(n) + d(n)
```

With *y(n)* is noisy signal, *x(n)* is clean signal and *d(n)* is noise, if we look into the frequency domain, the model can be state as:
```
Y(k) = X(k) + D(k)
```

So the core idea is if we can extract the noisy frequency in the signal, we can get a clean signal:
```
X(k) = Y(k) - D(k)
```

The problem is how to find *D(k)*, there are also several ways to estimate the noise signal, in this project, I used **Minimal Statistic** method to estimate *D(k)* (I will update about it if I have time).

The current result:
![Noisy Signal](image-2.png)

![Processed Signal](image-3.png)

