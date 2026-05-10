echo "Running gem5 simulation..."
./gem5/build/ARM/gem5.opt benchmark.py

echo "Generating performance report..."
python3 gem5_performance_analyzer.py m5out/stats.txt gem5_performance_report.html

echo "Benchmark completed!"