#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build}"
COUNT="${2:-300000}"

RECV_LOG="$(mktemp)"
SEND_LOG="$(mktemp)"
RECV_PID=""

cleanup() {
  if [[ -n "${RECV_PID}" ]] && kill -0 "${RECV_PID}" >/dev/null 2>&1; then
    kill -TERM "${RECV_PID}" >/dev/null 2>&1 || true
    wait "${RECV_PID}" >/dev/null 2>&1 || true
  fi
}
trap cleanup EXIT

"${BUILD_DIR}/test_feed_receiver" >"${RECV_LOG}" 2>&1 &
RECV_PID=$!

sleep 1
"${BUILD_DIR}/test_feed_sender" --count="${COUNT}" >"${SEND_LOG}" 2>&1
wait "${RECV_PID}"

cat "${RECV_LOG}"
if [[ -s "${SEND_LOG}" ]]; then
  echo "[SENDER LOG]"
  cat "${SEND_LOG}"
fi
