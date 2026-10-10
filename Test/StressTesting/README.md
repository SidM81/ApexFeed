# Stress Testing

Run from `/home/siddharth/ApexFeed/Test`:

```bash
./run_test.sh --stress
```

Optional environment overrides:

```bash
STRESS_SYMBOLS=32 STRESS_CYCLES=3000 STRESS_REPORT=500000 ./run_test.sh --stress
```

Artifacts go to:

- `StressTesting/results/<timestamp>/build.log`
- `StressTesting/results/<timestamp>/tests.log`
- `StressTesting/results/<timestamp>/replay.log`
- `StressTesting/results/<timestamp>/apexalpha_events.csv`
- `StressTesting/results/<timestamp>/stress_summary.txt`
