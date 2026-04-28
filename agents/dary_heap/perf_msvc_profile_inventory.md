# `artifacts/perf/msvc_profile/` — pre-collected MSVC reference

Captured: 2026-04-27, branch `indexed-dary-heap`, host Titania
Build: `windows-msvc-profile` preset (`/O2 /Ob3 /Zi /DNDEBUG`, `/DEBUG`
linker, `DIJKSTRA_BENCH_BGL=ON`, BGL at `D:/dev_graph/boost`)

This directory is **gitignored** (it lives under `artifacts/`), but the
file inventory below is committed so a Linux/GCC session knows what
artefacts to compare against. Re-generate on Windows with:

```pwsh
# Wall-clock baseline (2-3 min)
python scripts/perf/bench_run.py `
  --exe build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe `
  --filter "BM_Dijkstra_(CSR|BGL_CSR)_(ER_Sparse|Grid|BA|Path)(_Idx4)?/(10000|100000)$" `
  --reps 5 --min-time 2s --label "windows-msvc-profile" `
  --out artifacts/perf/msvc_profile/wallclock_baseline.json

# Symbol disasm captures (~1 min after symbol-index cold call)
python scripts/perf/capture_asm.py `
  --exe build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe `
  --manifest agents/perf_capture_manifest.txt `
  --out-dir artifacts/perf/msvc_profile

# VTune hotspots + callstacks (~30s collect + 5s report)
& $vtune -collect hotspots -knob sampling-mode=sw `
  -result-dir vtune/hot_grid_idx4_profile_001 -- `
  build/windows-msvc-profile/benchmark/algorithms/benchmark_dijkstra.exe `
  --benchmark_filter="BM_Dijkstra_CSR_Grid_Idx4/100000" --benchmark_min_time=15s
& $vtune -report hotspots   -r vtune/hot_grid_idx4_profile_001 -format csv `
  > artifacts/perf/msvc_profile/hotspots.csv
& $vtune -report callstacks -r vtune/hot_grid_idx4_profile_001 -format csv `
  > artifacts/perf/msvc_profile/callstacks.csv
```

## Inventory

| File | Lines / size | Description |
|------|-------------:|-------------|
| `wallclock_baseline.json`         | 96 rows | bench_run.py JSON for 24 benchmarks × 4 aggregates |
| `hotspots.csv`                    | 38 KB   | VTune software-mode hotspots, function-level CPU time |
| `callstacks.csv`                  | 597 KB  | VTune callstack tree |
| **graph-v3 heap**                 |         |             |
| `sift_down_csr_idx2.asm`          | 186     | Idx2 heap sift-down body |
| `sift_down_csr_idx4.asm`          | 184     | Idx4 heap sift-down body |
| `sift_down_csr_idx8.asm`          | 186     | Idx8 heap sift-down body |
| `sift_down_vov_idx4.asm`          | 191     | VoV Idx4 sift-down (control) |
| `sift_up_csr_idx4.asm`            | 109     | Idx4 sift-up body |
| **graph-v3 algorithm**            |         |             |
| `dijkstra_csr_idx2.asm`           | 206     | Outer Dijkstra body, Idx2 |
| `dijkstra_csr_idx4.asm`           | 206     | Outer Dijkstra body, Idx4 |
| `dijkstra_csr_idx8.asm`           | 206     | Outer Dijkstra body, Idx8 |
| `container_value_fn.asm`          | 85      | edge_value adapter |
| **BGL counterparts**              |         |             |
| `bgl_dary_sift_down_csr.asm`      | 299     | preserve_heap_property_down (CSR) |
| `bgl_dary_sift_up_csr.asm`        | 204     | preserve_heap_property_up (CSR) |
| `dijkstra_bgl_csr.asm`            | 505     | dijkstra_shortest_paths_no_color_map_no_init (CSR) |

## Headline observation

Line counts are a proxy for instruction count when comparing functions
compiled at the same `/O2` level on the same toolchain. On MSVC:

|                       | graph-v3 | BGL | ratio |
|-----------------------|---------:|----:|------:|
| Dijkstra body         |      206 | 505 |  2.5× |
| sift_down_            |      184 | 299 |  1.6× |

This is consistent with graph-v3 being **34-64 % faster than BGL** on
MSVC profile (Phase 1.1 wall-clock data). The Linux side needs to confirm
or refute the same ratio under GCC.
