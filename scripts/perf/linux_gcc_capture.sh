#!/usr/bin/env bash
# linux_gcc_capture.sh - capture the Linux/GCC counterpart of the MSVC reference
# in artifacts/perf/msvc_profile/.
#
# WSL has no hardware performance counters (PMU is not exposed), so this
# script intentionally avoids `perf stat -e cache-misses,...` and uses only:
#   - wall-clock (Google Benchmark median across 5 reps)
#   - software perf events that work in WSL
#   - objdump for per-symbol disassembly comparison
#
# Run from the workspace root after cloning to a Linux/WSL machine:
#
#   cmake --preset linux-gcc-release
#   cmake --build --preset linux-gcc-release
#   bash scripts/perf/linux_gcc_capture.sh
#
# Output lands in artifacts/perf/linux_gcc/, mirroring artifacts/perf/msvc_profile/.

set -euo pipefail

EXE="${1:-build/linux-gcc-release/benchmark/algorithms/benchmark_dijkstra}"
OUT_DIR="${2:-artifacts/perf/linux_gcc}"

if [[ ! -x "$EXE" ]]; then
  echo "ERROR: benchmark exe not found or not executable: $EXE" >&2
  echo "Usage: $0 [path/to/benchmark_dijkstra] [out_dir]" >&2
  exit 2
fi

mkdir -p "$OUT_DIR"
echo "==> capturing Linux/GCC reference into $OUT_DIR"

# ---------- 1. wall-clock baseline ----------
echo "--- 1. wall-clock baseline (5 reps, median, taskset core 4) ---"
taskset -c 4 python3 scripts/perf/bench_run.py \
  --exe "$EXE" \
  --filter 'BM_Dijkstra_(CSR|BGL_CSR)_(ER_Sparse|Grid|BA|Path)(_Idx4)?/(10000|100000)$' \
  --reps 5 --min-time 2s \
  --label "linux-gcc-release" \
  --out "$OUT_DIR/wallclock_baseline.json"

# ---------- 2. software perf-stat counters (WSL-friendly) ----------
# These software events do NOT need the PMU; they work in WSL.
SW_EVENTS="task-clock,context-switches,page-faults,cpu-migrations,instructions:u,cycles:u"
echo "--- 2. perf stat (software events, no PMU required) ---"
for bench in BM_Dijkstra_CSR_Grid_Idx4/100000 BM_Dijkstra_BGL_CSR_Grid/100000 \
             BM_Dijkstra_CSR_Path_Idx4/100000 BM_Dijkstra_BGL_CSR_Path/100000; do
  safe="${bench//\//_}"
  echo "  perf stat $bench"
  taskset -c 4 perf stat -e "$SW_EVENTS" -r 3 -- \
    "$EXE" --benchmark_filter="^${bench}$" --benchmark_min_time=3s \
    > "$OUT_DIR/perfstat_${safe}.stdout" \
    2> "$OUT_DIR/perfstat_${safe}.stderr" || \
    echo "    note: perf stat returned non-zero for $bench (may indicate no PMU)" >&2
done

# ---------- 3. objdump per-symbol captures ----------
# GCC's objdump does the demangling MSVC's dumpbin does, but with --demangle.
echo "--- 3. objdump captures (mirrors MSVC manifest) ---"
python3 scripts/perf/objdump_capture.py \
  --exe "$EXE" \
  --manifest agents/perf_capture_manifest_linux.txt \
  --out-dir "$OUT_DIR"

echo
echo "==> Linux capture complete. Diff against MSVC with:"
echo "    python scripts/perf/bench_compare.py \\"
echo "      --baseline artifacts/perf/msvc_profile/wallclock_baseline.json \\"
echo "      --candidate $OUT_DIR/wallclock_baseline.json \\"
echo "      --label-baseline msvc --label-candidate gcc"
