# Perf Testing

Run from `/home/siddharth/ApexFeed/Test`:

```bash
./run_test.sh --perf
```

Optional environment overrides:

```bash
PERF_SYMBOLS=48 PERF_CYCLES=4000 PERF_ITERATIONS=5 PERF_REPORT=750000 ./run_test.sh --perf
```

Artifacts go to:

- `PerfTesting/results/<timestamp>/bootstrap.log`
- `PerfTesting/results/<timestamp>/perf_iterations.csv`
- `PerfTesting/results/<timestamp>/perf_summary.txt`
- `PerfTesting/results/<timestamp>/iteration_*/replay.log`
- `PerfTesting/results/<timestamp>/iteration_*/apexalpha_events.csv`
