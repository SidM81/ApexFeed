#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ENGINE_DIR="$ROOT_DIR/ApexFeed"
TEST_DIR="$ROOT_DIR/Test"
RESULTS_DIR="${RESULTS_DIR:-$TEST_DIR/results}"
BUILD_DIR="${BUILD_DIR:-$TEST_DIR/build_apexalpha}"
RUN_PREFIX="${RUN_PREFIX:-}"
SKIP_BUILD="${SKIP_BUILD:-0}"
SKIP_UNIT_TESTS="${SKIP_UNIT_TESTS:-0}"

FEED_FILE="${1:-}"
CONFIG_FILE="${2:-$TEST_DIR/strategy_default.cfg}"
CHUNK="${CHUNK:-65536}"
REPORT="${REPORT:-100000}"

if [[ -z "$FEED_FILE" ]]; then
  echo "Usage: $0 <feed_file> [strategy_config]"
  echo "Example: $0 /home/siddharth/ApexFeed/Test/data/sample_itch.bin"
  exit 1
fi

if [[ ! -f "$FEED_FILE" ]]; then
  echo "[ERROR] Feed file not found: $FEED_FILE"
  exit 1
fi

if [[ ! -f "$CONFIG_FILE" ]]; then
  echo "[ERROR] Strategy config not found: $CONFIG_FILE"
  exit 1
fi

mkdir -p "$RESULTS_DIR"
RUN_TS="$(date -u +%Y%m%d_%H%M%SZ)"
if [[ -n "$RUN_PREFIX" ]]; then
  RUN_DIR="$RESULTS_DIR/${RUN_PREFIX}_$RUN_TS"
else
  RUN_DIR="$RESULTS_DIR/$RUN_TS"
fi
mkdir -p "$RUN_DIR"

echo "[INFO] Run directory: $RUN_DIR"
echo "[INFO] Feed: $FEED_FILE"
echo "[INFO] Config: $CONFIG_FILE"
echo "[INFO] Chunk: $CHUNK"
echo "[INFO] Report: $REPORT"
echo "[INFO] Build dir: $BUILD_DIR"

if [[ "$SKIP_BUILD" != "1" ]]; then
  echo "[STEP] Configure + build (Release)"
  cmake -S "$ENGINE_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release > "$RUN_DIR/build.log" 2>&1
  cmake --build "$BUILD_DIR" -j >> "$RUN_DIR/build.log" 2>&1
else
  echo "[STEP] Configure + build (Release) [SKIPPED]"
  printf "[SKIP] Build skipped by SKIP_BUILD=1\n" > "$RUN_DIR/build.log"
fi

if [[ "$SKIP_UNIT_TESTS" != "1" ]]; then
  echo "[STEP] Run ApexAlpha unit tests"
  {
    "$BUILD_DIR/test_apexalpha_features"
    "$BUILD_DIR/test_apexalpha_router"
    "$BUILD_DIR/test_apexalpha_strategy_sim"
  } | tee "$RUN_DIR/tests.log"
else
  echo "[STEP] Run ApexAlpha unit tests [SKIPPED]"
  printf "[SKIP] Unit tests skipped by SKIP_UNIT_TESTS=1\n" > "$RUN_DIR/tests.log"
fi

echo "[STEP] Run replay_strategy"
"$BUILD_DIR/replay_strategy" "$FEED_FILE" \
  --chunk="$CHUNK" \
  --report="$REPORT" \
  --out="$RUN_DIR/apexalpha_events.csv" \
  --strategy-config="$CONFIG_FILE" | tee "$RUN_DIR/replay.log"

cp "$CONFIG_FILE" "$RUN_DIR/strategy_used.cfg"
echo "$RUN_DIR" > "$RESULTS_DIR/latest.txt"

echo "[DONE] Outputs saved in: $RUN_DIR"
echo "       CSV: $RUN_DIR/apexalpha_events.csv"
echo "       Summary log: $RUN_DIR/replay.log"
