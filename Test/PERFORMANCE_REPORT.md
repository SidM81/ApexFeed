# ApexFeed and ApexAlpha Performance Report

## Executive Summary

The recorded tests verify two different workflows:

- **ApexFeed TCP smoke:** 200,000 synthetic ITCH Add Order messages were sent over loopback TCP, parsed, and applied to the order book. The receiver reported **193,517 messages/sec** and **140.918 ns average parse time**.
- **ApexAlpha stress replay:** 576,000 messages across 24 synthetic symbols were replayed through parsing, symbol routing, feature calculation, strategy evaluation, simulation, and CSV output. All messages were routed, at **244,194 messages/sec** for this one run.
- **ApexAlpha repeated replay benchmark:** 960,000 messages across 32 synthetic symbols, run three times. Average throughput was **117,595 messages/sec**, with a best run of **123,393 messages/sec**.

These are synthetic local benchmarks, not production-feed or trading-profitability guarantees. The rates are not directly comparable because the tested workloads and included work differ.

## ApexFeed

### TCP sender/receiver smoke test

The test starts `test_feed_receiver`, then `test_feed_sender` transmits the requested number of messages over loopback TCP. The receiver parses the byte stream and applies decoded Add Order messages to the book. This exercises the socket-to-parser-to-order-book path, including TCP message delivery.

Verified command (run from the nested `ApexFeed/` project directory):

```bash
make BUILD_DIR=build-e2e-check smoke COUNT=200000
```

The separate build directory is intentional: the existing default `build/` CMake cache points at the parent directory and cannot be reused for this nested project.

| Metric | Result |
| --- | ---: |
| Messages sent and received | 200,000 |
| Message type | Add Order (`A`) only |
| Elapsed receiver time | 1.0335 sec |
| End-to-end receive rate | 193,517 messages/sec |
| Average parser time | 140.918 ns/message |
| Decoded order count | 200,000 |
| Unhandled messages | 0 reported |

The throughput figure covers the local sender/receiver run; the parser timing is the receiver's reported average. This test does not cover a mixed ITCH message distribution or UDP/multicast.

### Existing parser and epoll benchmark report

The repository also contains a stored report dated **2026-03-07**. These measurements were not rerun as part of the November ApexAlpha tests and should be treated as historical reference data:

| Test | Recorded result | Notes |
| --- | ---: | --- |
| Parser latency benchmark | 29.955 ns/message; 80.516 cycles/message | 2,000,000 parser iterations; parser-focused, not end-to-end |
| Epoll ingest smoke | 57,616 messages/sec; 176.371 ns average parse time | Report says 115,648 received from a 120,000-message send; shutdown occurred before all messages were accounted for |
| Epoll backpressure | 0 events | As reported for that run |

Source artifact: [stored ApexFeed benchmark report](../ApexFeed/Benchmarks/results/latest.txt).

## ApexAlpha

### Unit tests and synthetic stress replay

The stress runner creates a binary ITCH feed with 24 symbols and 2,000 cycles per symbol. It builds Release binaries, runs the three ApexAlpha unit tests, then replays the feed through the parser, symbol router, feature engine, strategy, execution simulator, and CSV writer.

Verified command (run from `Test/`):

```bash
./run_test.sh --stress
```

All three tests passed: feature engine, symbol router, and strategy/simulation.

| Metric | Result |
| --- | ---: |
| Input messages | 576,000 |
| Input bytes | 16,992,000 |
| Symbols | 24 |
| Routed / unrouted | 576,000 / 0 |
| Feature updates | 576,000 |
| Signals / simulated executions | 287,976 / 287,976 |
| CSV event rows | 576,000 |
| Replay rate | 244,194 messages/sec |
| Total simulated PnL | -143,988,000 PnL units |

Artifacts: [stress summary](StressTesting/results/stress_20261127_171717Z/stress_summary.txt) and [full replay log](StressTesting/results/stress_20261127_171717Z/replay.log).

### Repeated synthetic replay benchmark

The performance runner generates a feed with 32 symbols and 2,500 cycles per symbol, builds Release binaries, runs all three ApexAlpha tests, and replays the feed three times. The recorded throughput uses total replay message count divided by wall time; the replay path includes file input and CSV output as well as ApexAlpha processing.

Verified command (run from `Test/`):

```bash
./run_test.sh --perf
```

| Iteration | Wall time | Throughput | Peak RSS | CSV rows | Simulated executions | Total simulated PnL |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 7.78 sec | 123,393 msg/sec | 39,216 KB | 960,000 | 479,968 | -239,984,000 |
| 2 | 8.09 sec | 118,665 msg/sec | 39,352 KB | 960,000 | 479,968 | -239,984,000 |
| 3 | 8.67 sec | 110,727 msg/sec | 39,468 KB | 960,000 | 479,968 | -239,984,000 |
| **Average / best** | **8.18 sec average** | **117,595 average / 123,393 best msg/sec** | **about 39 MB** | | | |

Artifacts: [performance summary](PerfTesting/results/perf_20261127_171753Z/perf_summary.txt) and [per-iteration CSV](PerfTesting/results/perf_20261127_171753Z/perf_iterations.csv).

## How to Interpret the ApexAlpha Counts and PnL

The generated feed manifest's `expected_trade_events` counts trade-related messages in the input feed. The replay's `trade_events` or “simulated executions” count orders generated by the strategy simulator. They measure different things and should not be expected to match.

The negative PnL is the outcome of the synthetic feed and test strategy configuration, not a measure of system correctness or a live strategy's expected performance. The stress configuration is deliberately permissive to exercise signal and execution paths. These runs establish that the components process the test workload and emit internally reported results; they do not establish that the trading logic is profitable or validated against exchange reference data.

## Scope and Limitations

- Verified: Release build, three ApexAlpha unit tests, local TCP sender/receiver smoke, book updates, and file-based multi-symbol ApexAlpha replay with CSV output.
- Not established by these runs: live exchange feed behavior, packet loss/recovery, UDP or multicast behavior, long-duration stability, correctness against an independent ITCH reference decoder, or production hardware performance.
- Results depend on CPU, operating system load, compiler/build settings, and filesystem behavior. Treat them as local measurements, not guaranteed rates.