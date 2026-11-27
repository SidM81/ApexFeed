#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

BUILD_DIR="${1:-build}"
RESULT_DIR="${2:-Benchmarks/results}"
SMOKE_COUNT="${3:-200000}"
INGEST_COUNT="${4:-120000}"
INGEST_REPORT="${5:-50000}"
LAT_ITERS="${6:-2000000}"

mkdir -p "${RESULT_DIR}"

TS="$(date -u +%Y%m%d_%H%M%SZ)"
REPORT_FILE="${RESULT_DIR}/perf_${TS}.txt"
LATEST_FILE="${RESULT_DIR}/latest.txt"

SMOKE_OUTPUT=""
INGEST_OUTPUT=""
LAT_OUTPUT=""

run_capture() {
  local title="$1"
  shift
  echo "## ${title}" | tee -a "${REPORT_FILE}"
  echo "\$ $*" | tee -a "${REPORT_FILE}"

  local cmd_output
  cmd_output="$($@ 2>&1)"
  echo "${cmd_output}" | tee -a "${REPORT_FILE}"
  echo | tee -a "${REPORT_FILE}"
  printf '%s' "${cmd_output}"
}

{
  echo "# ApexFeed Benchmark Report"
  echo "generated_utc: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
  echo "host: $(hostname)"
  echo "build_dir: ${BUILD_DIR}"
  echo "smoke_count: ${SMOKE_COUNT}"
  echo "ingest_count: ${INGEST_COUNT}"
  echo "ingest_report_every: ${INGEST_REPORT}"
  echo "latency_iterations: ${LAT_ITERS}"
  echo
} > "${REPORT_FILE}"

SMOKE_OUTPUT="$(run_capture "TCP Smoke" ./scripts/smoke_tcp.sh "${BUILD_DIR}" "${SMOKE_COUNT}")"
INGEST_OUTPUT="$(run_capture "Epoll Ingest Smoke" ./scripts/smoke_ingest.sh "${BUILD_DIR}" "${INGEST_COUNT}" "${INGEST_REPORT}")"
LAT_OUTPUT="$(run_capture "Latency Benchmark" "${BUILD_DIR}/latency_bench" "${LAT_ITERS}")"

{
  echo "## Summary"
  echo "smoke_last_perf: $(echo "${SMOKE_OUTPUT}" | grep '^\[PERF\]' | tail -n1 || true)"
  echo "smoke_type_stats: $(echo "${SMOKE_OUTPUT}" | grep '^\[STATS\]' | tail -n1 || true)"
  echo "ingest_last_perf: $(echo "${INGEST_OUTPUT}" | grep '^\[PERF\]' | tail -n1 || true)"
  echo "ingest_backpressure: $(echo "${INGEST_OUTPUT}" | grep '^\[INGEST\]' | tail -n1 || true)"
  echo "latency: $(echo "${LAT_OUTPUT}" | grep '^\[LAT\]' | tail -n1 || true)"
} | tee -a "${REPORT_FILE}"

cp "${REPORT_FILE}" "${LATEST_FILE}"
echo "[REPORT] ${REPORT_FILE}"
