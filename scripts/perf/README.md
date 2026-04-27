# scripts/perf — performance investigation tooling

Helpers for running the benchmark suites, comparing runs, parsing VTune
output, and pulling targeted disassembly. Built to support the work tracked
in `agents/csr_edge_value_perf_plan.md` and `agents/indexed_dary_heap_results.md`.

All scripts are stdlib-only (Python 3.10+). Windows-specific bits (process
affinity / priority, `dumpbin`) degrade gracefully on other OSes.

## Files

| Script | Purpose |
|---|---|
| `bench_run.py` | Run a benchmark filter with core-pinning + High priority; emit median rows as JSON. |
| `bench_compare.py` | Diff two `bench_run.py` JSONs as a markdown Δ% table. |
| `vtune_top.py` | Parse a VTune CSV hotspots report; emit a normalized top-N table. |
| `disasm_func.py` | Find a function by demangled-name substring; dump only that function's bytes via `dumpbin`. |

## Typical workflow (Phase 1 of csr_edge_value_perf_plan.md)

```pwsh
# 1. Capture baseline (graph-v3 vs BGL on Grid)
python scripts/perf/bench_run.py `
  --exe build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe `
  --filter "BM_Dijkstra_(CSR|BGL_CSR)_Grid(_Idx4)?/(10000|100000)$" `
  --reps 5 --min-time 2s `
  --label "msvc-profile" `
  --out artifacts/grid_msvc_profile.json

# 2. Apply a code change, rebuild, capture candidate
python scripts/perf/bench_run.py ... --label "with-change" --out artifacts/grid_after.json

# 3. Diff
python scripts/perf/bench_compare.py `
  --baseline artifacts/grid_msvc_profile.json `
  --candidate artifacts/grid_after.json
```

## VTune hotspots (Phase 4.3-style)

```pwsh
& "C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune.exe" `
  -collect hotspots -knob sampling-mode=sw `
  -result-dir vtune/hot_001 -- `
  build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe `
  --benchmark_filter="BM_Dijkstra_CSR_Grid_Idx4/100000" --benchmark_min_time=15s

& "C:\Program Files (x86)\Intel\oneAPI\vtune\latest\bin64\vtune.exe" `
  -report hotspots -r vtune/hot_001 -format csv > vtune/hot_001.csv

python scripts/perf/vtune_top.py --csv vtune/hot_001.csv --top 15 --markdown
```

## Targeted disassembly (Phase 2)

```pwsh
# Run from a vcvars64 shell (so dumpbin is on PATH).
python scripts/perf/disasm_func.py `
  --exe build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe `
  --pattern dijkstra_shortest_paths --pattern Idx4 --pattern Grid `
  --list-only
```

## Output convention

All scripts write either to stdout or to `--out <path>`. There is no global
artifacts directory imposed — the convention used in the agents/ docs is
`artifacts/perf/` (gitignored), but pass any path you like.
