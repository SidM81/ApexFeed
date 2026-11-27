#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
COUNT="${2:-200000}"
REPORT="${3:-100000}"

INGEST_LOG="$(mktemp)"
SEND_LOG="$(mktemp)"
INGEST_PID=""

cleanup() {
  if [[ -n "${INGEST_PID}" ]] && kill -0 "${INGEST_PID}" >/dev/null 2>&1; then
    kill -TERM "${INGEST_PID}" >/dev/null 2>&1 || true
    wait "${INGEST_PID}" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT

"${BUILD_DIR}/feed_ingest" --mode=tcp --port=9001 --report="${REPORT}" >"${INGEST_LOG}" 2>&1 &
INGEST_PID=$!

sleep 1
"${BUILD_DIR}/test_feed_sender" --count="${COUNT}" >"${SEND_LOG}" 2>&1
sleep 1

kill -INT "${INGEST_PID}" >/dev/null 2>&1 || true
wait "${INGEST_PID}" >/dev/null 2>&1 || true
INGEST_PID=""

cat "${INGEST_LOG}"
if [[ -s "${SEND_LOG}" ]]; then
  echo "[SENDER LOG]"
  cat "${SEND_LOG}"
fi
