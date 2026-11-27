#!/usr/bin/env bash
set -euo pipefail

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

usage() {
  cat <<'USAGE'
Usage:
  ./run_test.sh --stress
  ./run_test.sh --perf
  ./run_test.sh --help

Commands:
  --stress   Generate a high-volume synthetic feed and run one end-to-end stress pass.
  --perf     Generate a benchmark feed and run repeated replay benchmarks.

Useful environment overrides:
  STRESS_SYMBOLS, STRESS_CYCLES, STRESS_REPORT
  PERF_SYMBOLS, PERF_CYCLES, PERF_ITERATIONS, PERF_REPORT, PERF_CHUNK
USAGE
}

case "${1:-}" in
  --stress)
    shift
    exec "$TEST_DIR/StressTesting/run_stress_test.sh" "$@"
    ;;
  --perf)
    shift
    exec "$TEST_DIR/PerfTesting/run_perf_test.sh" "$@"
    ;;
  --help|-h)
    usage
    ;;
  *)
    usage
    exit 1
    ;;
esac
