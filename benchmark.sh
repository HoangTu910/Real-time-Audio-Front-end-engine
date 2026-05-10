!bin/bash

echo "Running gem5 simulation..."
./gem5/build/ARM/gem5.opt arm_core_config.py

echo "Generating performance report..."

python3 gem5_performance_analyzer.py m5out/stats.txt gem5_arm_core_performance_report.html

echo "Benchmark completed!"