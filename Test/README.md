# ApexAlpha Test Workspace

This folder is a dedicated workspace for running ApexAlpha and storing all run artifacts.

## Folder layout

- `strategy_default.cfg` ? default strategy thresholds/config.
- `run_apexalpha_test.sh` ? one-command runner (build + unit tests + replay).
- `run_test.sh` ? front door for the new mode-based test flows.
- `StressTesting/` ? synthetic high-volume end-to-end stress tests.
- `PerfTesting/` ? repeated replay benchmarks with throughput summaries.
- `results/<timestamp>/` ? output for direct generic runs.
- `results/latest.txt` ? path of the latest direct generic run.

## Quick start

```bash
cd /home/siddharth/ApexFeed/Test
./run_test.sh --stress
./run_test.sh --perf
```

## Direct runner

```bash
cd /home/siddharth/ApexFeed/Test
./run_apexalpha_test.sh /absolute/path/to/feed.bin
```

## What each mode does

1. `--stress`
   Generates a large synthetic multi-symbol feed, runs build + unit tests + one full replay pass, and writes artifacts to `StressTesting/results/<timestamp>/`.
2. `--perf`
   Generates a benchmark feed, runs build + unit tests once, then runs repeated replay iterations and writes timing summaries to `PerfTesting/results/<timestamp>/`.

## Useful overrides

```bash
STRESS_SYMBOLS=32 STRESS_CYCLES=3000 ./run_test.sh --stress
PERF_SYMBOLS=48 PERF_CYCLES=4000 PERF_ITERATIONS=5 ./run_test.sh --perf
```
