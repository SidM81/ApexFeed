#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ENGINE_DIR="$ROOT_DIR/ApexFeed"
TEST_DIR="$ROOT_DIR/Test"
MODE_DIR="$TEST_DIR/PerfTesting"
GENERATED_DIR="$MODE_DIR/generated"
RESULTS_DIR="$MODE_DIR/results"
BUILD_DIR="${BUILD_DIR:-$TEST_DIR/build_apexalpha}"
GENERATOR="$TEST_DIR/generate_synthetic_itch.py"
CONFIG_FILE="$MODE_DIR/strategy_perf.cfg"

PERF_SYMBOLS="${PERF_SYMBOLS:-32}"
PERF_CYCLES="${PERF_CYCLES:-2500}"
PERF_BASE_PRICE="${PERF_BASE_PRICE:-10000}"
PERF_PRICE_STEP="${PERF_PRICE_STEP:-25}"
PERF_ITERATIONS="${PERF_ITERATIONS:-3}"
PERF_CHUNK="${PERF_CHUNK:-65536}"
PERF_REPORT="${PERF_REPORT:-500000}"

mkdir -p "$GENERATED_DIR" "$RESULTS_DIR"

RUN_TS="$(date -u +%Y%m%d_%H%M%SZ)"
RUN_DIR="$RESULTS_DIR/perf_$RUN_TS"
mkdir -p "$RUN_DIR"

FEED_FILE="$GENERATED_DIR/perf_feed.bin"
MANIFEST_FILE="$GENERATED_DIR/perf_feed_manifest.txt"

echo "[PERF] Generating feed"
python3 "$GENERATOR" \
  --out "$FEED_FILE" \
  --manifest "$MANIFEST_FILE" \
  --symbols "$PERF_SYMBOLS" \
  --cycles "$PERF_CYCLES" \
  --base-price "$PERF_BASE_PRICE" \
  --price-step "$PERF_PRICE_STEP"

echo "[PERF] Configure + build (Release)"
cmake -S "$ENGINE_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release > "$RUN_DIR/bootstrap.log" 2>&1
cmake --build "$BUILD_DIR" -j >> "$RUN_DIR/bootstrap.log" 2>&1

echo "[PERF] Run ApexAlpha unit tests"
{
  "$BUILD_DIR/test_apexalpha_features"
  "$BUILD_DIR/test_apexalpha_router"
  "$BUILD_DIR/test_apexalpha_strategy_sim"
} >> "$RUN_DIR/bootstrap.log" 2>&1

MESSAGE_COUNT="$(awk -F= '/^messages=/{print $2}' "$MANIFEST_FILE")"
SUMMARY_CSV="$RUN_DIR/perf_iterations.csv"

echo "iteration,wall_sec,user_sec,sys_sec,max_rss_kb,msg_per_sec,csv_rows,trade_events,total_pnl" > "$SUMMARY_CSV"

for iteration in $(seq 1 "$PERF_ITERATIONS"); do
  ITER_DIR="$RUN_DIR/iteration_$iteration"
  mkdir -p "$ITER_DIR"

  echo "[PERF] Iteration $iteration/$PERF_ITERATIONS"
  /usr/bin/time -f "wall_sec=%e\nuser_sec=%U\nsys_sec=%S\nmax_rss_kb=%M" \
    -o "$ITER_DIR/time.txt" \
    "$BUILD_DIR/replay_strategy" "$FEED_FILE" \
      --chunk="$PERF_CHUNK" \
      --report="$PERF_REPORT" \
      --out="$ITER_DIR/apexalpha_events.csv" \
      --strategy-config="$CONFIG_FILE" > "$ITER_DIR/replay.log" 2>&1

  WALL_SEC="$(awk -F= '/^wall_sec=/{print $2}' "$ITER_DIR/time.txt")"
  USER_SEC="$(awk -F= '/^user_sec=/{print $2}' "$ITER_DIR/time.txt")"
  SYS_SEC="$(awk -F= '/^sys_sec=/{print $2}' "$ITER_DIR/time.txt")"
  MAX_RSS_KB="$(awk -F= '/^max_rss_kb=/{print $2}' "$ITER_DIR/time.txt")"
  CSV_ROWS="$(awk 'END{if (NR > 0) print NR - 1; else print 0}' "$ITER_DIR/apexalpha_events.csv")"
  TRADE_EVENTS="$(grep -o 'trade_events=[0-9]*' "$ITER_DIR/replay.log" | tail -n1 | cut -d= -f2)"
  TOTAL_PNL="$(awk '/^\[ALPHA\]\[TOTAL\]/{for (i = 1; i <= NF; ++i) if ($i ~ /^total=/) {split($i, value, "="); print value[2]}}' "$ITER_DIR/replay.log" | tail -n1)"
  MSG_PER_SEC="$(awk -v messages="$MESSAGE_COUNT" -v wall="$WALL_SEC" 'BEGIN { if (wall > 0) printf "%.2f", messages / wall; else print "0.00" }')"

  echo "$iteration,$WALL_SEC,$USER_SEC,$SYS_SEC,$MAX_RSS_KB,$MSG_PER_SEC,$CSV_ROWS,$TRADE_EVENTS,$TOTAL_PNL" >> "$SUMMARY_CSV"
done

AVG_WALL_SEC="$(awk -F, 'NR > 1 {sum += $2; count += 1} END {if (count > 0) printf "%.6f", sum / count; else print "0.000000"}' "$SUMMARY_CSV")"
AVG_MSG_PER_SEC="$(awk -F, 'NR > 1 {sum += $6; count += 1} END {if (count > 0) printf "%.2f", sum / count; else print "0.00"}' "$SUMMARY_CSV")"
BEST_MSG_PER_SEC="$(awk -F, 'NR == 2 || $6 > best {best = $6} END {if (best != "") printf "%.2f", best; else print "0.00"}' "$SUMMARY_CSV")"

cp "$MANIFEST_FILE" "$RUN_DIR/feed_manifest.txt"
{
  echo "mode=perf"
  cat "$MANIFEST_FILE"
  echo "iterations=$PERF_ITERATIONS"
  echo "avg_wall_sec=$AVG_WALL_SEC"
  echo "avg_msg_per_sec=$AVG_MSG_PER_SEC"
  echo "best_msg_per_sec=$BEST_MSG_PER_SEC"
} > "$RUN_DIR/perf_summary.txt"

echo "$RUN_DIR" > "$RESULTS_DIR/latest.txt"
echo "[PERF] Summary CSV: $SUMMARY_CSV"
echo "[PERF] Summary TXT: $RUN_DIR/perf_summary.txt"
echo "[PERF] Results: $RUN_DIR"

