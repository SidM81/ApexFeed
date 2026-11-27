#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TEST_DIR="$ROOT_DIR/Test"
MODE_DIR="$TEST_DIR/StressTesting"
GENERATED_DIR="$MODE_DIR/generated"
RESULTS_DIR="$MODE_DIR/results"
GENERATOR="$TEST_DIR/generate_synthetic_itch.py"
RUNNER="$TEST_DIR/run_apexalpha_test.sh"
CONFIG_FILE="$MODE_DIR/strategy_stress.cfg"

STRESS_SYMBOLS="${STRESS_SYMBOLS:-24}"
STRESS_CYCLES="${STRESS_CYCLES:-2000}"
STRESS_BASE_PRICE="${STRESS_BASE_PRICE:-10000}"
STRESS_PRICE_STEP="${STRESS_PRICE_STEP:-25}"
STRESS_CHUNK="${STRESS_CHUNK:-65536}"
STRESS_REPORT="${STRESS_REPORT:-250000}"

mkdir -p "$GENERATED_DIR" "$RESULTS_DIR"

FEED_FILE="$GENERATED_DIR/stress_feed.bin"
MANIFEST_FILE="$GENERATED_DIR/stress_feed_manifest.txt"

echo "[STRESS] Generating feed"
python3 "$GENERATOR" \
  --out "$FEED_FILE" \
  --manifest "$MANIFEST_FILE" \
  --symbols "$STRESS_SYMBOLS" \
  --cycles "$STRESS_CYCLES" \
  --base-price "$STRESS_BASE_PRICE" \
  --price-step "$STRESS_PRICE_STEP"

echo "[STRESS] Running end-to-end ApexAlpha stress test"
RESULTS_DIR="$RESULTS_DIR" \
RUN_PREFIX="stress" \
CHUNK="$STRESS_CHUNK" \
REPORT="$STRESS_REPORT" \
"$RUNNER" "$FEED_FILE" "$CONFIG_FILE"

RUN_DIR="$(cat "$RESULTS_DIR/latest.txt")"
CSV_FILE="$RUN_DIR/apexalpha_events.csv"
SUMMARY_FILE="$RUN_DIR/stress_summary.txt"
cp "$MANIFEST_FILE" "$RUN_DIR/feed_manifest.txt"

CSV_ROWS=0
if [[ -f "$CSV_FILE" ]]; then
  CSV_ROWS="$(awk 'END{if (NR > 0) print NR - 1; else print 0}' "$CSV_FILE")"
fi

{
  echo "mode=stress"
  cat "$MANIFEST_FILE"
  echo "csv_rows=$CSV_ROWS"
  grep '^\[ALPHA' "$RUN_DIR/replay.log"
} > "$SUMMARY_FILE"

echo "[STRESS] Summary: $SUMMARY_FILE"
echo "[STRESS] Results: $RUN_DIR"
