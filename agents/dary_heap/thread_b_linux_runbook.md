# Thread B — Linux/GCC investigation runbook

This runbook closes Thread B of `agents/csr_edge_value_perf_plan.md`.

The MSVC side (Phases 1.1, 2) is complete and committed. The original
Phase 4.3a "graph-v3 is +7 % to +37 % slower than BGL" gap was measured on
**Linux/GCC** and has not been reproduced under MSVC `/O2 /Ob3`. We need
to know whether the gap still exists on Linux/GCC at the current branch
HEAD before deciding whether Phases 3-5 (interventions) should run.

## Constraint: WSL has no PMU

WSL2 does not expose the host's hardware performance counters, so:

- ❌ No `perf stat -e cache-misses,L1-dcache-load-misses,LLC-load-misses`
- ❌ No `perf record -e cycles -F 4000`
- ❌ No Linux equivalent of VTune microarchitecture exploration
- ✅ Wall-clock benchmarks work fine
- ✅ `perf stat` for software events (`task-clock`, `instructions:u`,
     `context-switches`, etc.) — covered by the helper script
- ✅ `objdump --demangle` — full disassembly comparison against MSVC

Everything that *does* need PMU has been pre-collected on the Windows
side and lives in `artifacts/perf/msvc_profile/`. The Linux side compares
against it directly.

## Pre-collected MSVC reference (`artifacts/perf/msvc_profile/`)

| File | What it gives the Linux comparison |
|---|---|
| `wallclock_baseline.json` | 96 rows (24 benchmarks × 4 aggregates) on `windows-msvc-profile`. Run the same filter on Linux and use `bench_compare.py` to diff. |
| `hotspots.csv` | VTune software-mode hotspots top-N (function-level CPU time). Linux reproduces this with `perf record --call-graph=fp -F 999` (no PMU needed). |
| `callstacks.csv` | VTune callstack tree. Linux gets the same shape from `perf script` after a software-event `perf record`. |
| `sift_down_csr_idx{2,4,8}.asm` | Per-arity inlined heap-sift body. Each is ~190 lines, 5-insn-per-comparison shape. |
| `sift_down_vov_idx4.asm` | VoV variant for control. |
| `sift_up_csr_idx4.asm` | sift_up_ counterpart (~110 lines). |
| `dijkstra_csr_idx{2,4,8}.asm` | The actual Dijkstra-with-relax-loop body, ~206 lines each. |
| `dijkstra_bgl_csr.asm` | BGL's `dijkstra_shortest_paths_no_color_map_no_init` for `compressed_sparse_row_graph`. **505 lines on MSVC.** Compare line counts and per-edge instruction count vs graph-v3's 206. |
| `bgl_dary_sift_down_csr.asm` | BGL's `preserve_heap_property_down`, ~299 lines. Compare against graph-v3's `sift_down_csr_idx4.asm` (184 lines). |
| `bgl_dary_sift_up_csr.asm` | BGL's `preserve_heap_property_up`, ~204 lines. |
| `container_value_fn.asm` | graph-v3's value-function adapter. |

### The size signal

MSVC line counts at `/O2 /Ob3 /Zi`:

```
graph-v3 dijkstra body (Idx4)           206 lines
BGL      dijkstra body (CSR)            505 lines    (~2.5x)

graph-v3 sift_down_ (Idx4)              184 lines
BGL      preserve_heap_property_down    299 lines    (~1.6x)
```

This is consistent with graph-v3 being 34-64 % faster than BGL on MSVC
(measured in Phase 1.1). The Linux question is whether GCC produces a
similar size ratio (in which case Linux/GCC should also see graph-v3
ahead) or whether GCC compresses BGL more aggressively (which would
explain the original 4.3a gap).

## Setup (WSL)

```bash
# 1. Configure & build the Linux release preset
cmake --preset linux-gcc-release \
      -DDIJKSTRA_BENCH_BGL=ON \
      -DBGL_INCLUDE_DIR=/path/to/boost
cmake --build --preset linux-gcc-release -j

# 2. Verify the benchmark binary exists
file build/linux-gcc-release/benchmark/algorithms/benchmark_dijkstra
```

If `linux-gcc-release` doesn't exist as a preset on this branch, either
add one mirroring `windows-msvc-release` or build manually:

```bash
mkdir -p build/linux-gcc-release && cd build/linux-gcc-release
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_BENCHMARKS=ON \
      -DDIJKSTRA_BENCH_BGL=ON -DBGL_INCLUDE_DIR=/path/to/boost \
      ../..
ninja benchmark_dijkstra
```

## Capture

```bash
# Drives steps 1, 2, 3 below in order.
bash scripts/perf/linux_gcc_capture.sh
```

The script does:

1. **Wall-clock baseline** (`bench_run.py`, 5 reps median, taskset core 4).
   Output: `artifacts/perf/linux_gcc/wallclock_baseline.json`.
2. **`perf stat` software events** for the 4 canonical 100K benchmarks.
   Output: `artifacts/perf/linux_gcc/perfstat_*.{stdout,stderr}`.
3. **`objdump` per-symbol captures** mirroring the MSVC manifest.
   Output: `artifacts/perf/linux_gcc/*.asm`.

## Compare

```bash
# Toolchain wall-clock comparison
python3 scripts/perf/bench_compare.py \
  --baseline artifacts/perf/msvc_profile/wallclock_baseline.json \
  --candidate artifacts/perf/linux_gcc/wallclock_baseline.json \
  --label-baseline msvc --label-candidate gcc \
  --threshold 5

# Per-symbol size diff (one-liner)
for f in artifacts/perf/msvc_profile/*.asm; do
  base=$(basename "$f" .asm)
  ml=$(wc -l < "$f")
  gl=$(wc -l < "artifacts/perf/linux_gcc/$base.asm" 2>/dev/null || echo NA)
  printf "%-30s  msvc=%4s  gcc=%4s\n" "$base" "$ml" "$gl"
done
```

## Decision tree

| Outcome | Verdict |
|---------|---------|
| graph-v3 wins on Linux too (≤ 0 % delta vs BGL) | Plan **closed**. Update `csr_edge_value_perf_plan.md` Phase 5. The Phase 4.3a gap was closed by the post-4.3a commits (5085c60, 7645a19, 1c871a8, aa95fe0). |
| graph-v3 still slower than BGL on Linux (+5 % or more) | Original investigation **resumes**: run Phase 1.2 (perf-stat counters — software-only on WSL), Phase 1.3 (perf record), Phase 2 (objdump diff between graph-v3 and BGL). The pre-collected MSVC asm gives the codegen reference for what "tight" looks like on the comparable workload. |
| Linux numbers are noisy (CV > 10 %) | Re-run with `--reps 9` and a quieter system. WSL on a busy Windows host is the most likely cause; pin to a single core (`taskset -c 0`) and disable Windows background services. |

## Files added by this work

| Path | Purpose |
|------|---------|
| `scripts/perf/sym_index.py`            | Cached dumpbin symbol index (Windows) |
| `scripts/perf/disasm_func.py`          | Single-function disasm (Windows) |
| `scripts/perf/find_func.py`            | Symbol search (Windows) |
| `scripts/perf/capture_asm.py`          | Bulk dumpbin capture (Windows) |
| `scripts/perf/objdump_capture.py`      | Bulk objdump capture (Linux) |
| `scripts/perf/linux_gcc_capture.sh`    | Linux runbook driver |
| `scripts/perf/bench_run.py`            | Cross-platform Google Benchmark wrapper |
| `scripts/perf/bench_compare.py`        | JSON-diff into markdown |
| `scripts/perf/vtune_top.py`            | VTune CSV parser |
| `agents/perf_capture_manifest.txt`     | MSVC capture targets |
| `agents/perf_capture_manifest_linux.txt` | GCC capture targets |
| `artifacts/perf/msvc_profile/`         | Pre-collected MSVC reference (gitignored) |
