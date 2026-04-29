# Real-time Audio Front-end engine

This repo was inspired from AudioNoise repo of Linus Torvald. I think it would be great if I can implement some ADSP modules in both fixed-point and floating-point number, any modules that I can come up with. And of course, try to optimize it as good as possible, apply some methods like loop-unrolling, software-pipelining and so on. Learning about DSP and write optimize program should be the objective of this repo, and analyze the profiling data also.

Benchmarking on hardware could be done in the future. In the early phase of this project, all the modules should work properly first on my computer, then optimize, and then benchmark it on hardware. I think about some hardwares like Arm-M4 or M7, or even some SoC platforms if possible. The benchmarking data of hardware with and without FPU could be also good, it would give a better perspective about fixed-point and floating-point implementation (in term of sound quality and performance trade-offs).

Extension for streaming song or microphone, or some advance modules or even multi-channel processing could also be considered in the future (if I have time and interest enough).

More update later, for now, nothing can be used in this repo, some testing still on-going and still thinking about which module I should do next.

And yes, no vibe-coding in this project, maybe I will need some help from AI (still learning though), but will try not to depend too much on them.

In case you want to run some test:
```bash
make test-all
make test_dc
make test_sincos
```