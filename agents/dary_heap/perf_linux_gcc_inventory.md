# `artifacts/perf/linux_gcc/` — pre-collected Linux GCC reference

Captured: 2026-04-28, branch `indexed-dary-heap`, host Titania (WSL2)
Build: `linux-gcc-release` preset (g++ 13.x, `-O3 -DNDEBUG`,
`DIJKSTRA_BENCH_BGL=ON`, BGL at `/home/phil/dev_graph/boost`)

This directory is **gitignored** (it lives under `artifacts/`), but the
file inventory below is committed so a future session — or a Windows
session diff'ing against `msvc_profile/` — knows what artefacts to
expect. Re-generate on Linux/WSL with:

```bash
cmake --preset linux-gcc-release \
      -DDIJKSTRA_BENCH_BGL=ON \
      -DBGL_INCLUDE_DIR=/home/phil/dev_graph/boost
cmake --build --preset linux-gcc-release -j --target benchmark_dijkstra

bash scripts/perf/linux_gcc_capture.sh
```

The script drives `scripts/perf/bench_run.py` (wall-clock), `perf stat`
(software events only — WSL has no PMU), and
`scripts/perf/objdump_capture.py` (per-symbol asm via the manifest at
`agents/perf_capture_manifest_linux.txt`).

## WSL-specific constraints

- Hardware PMU events (`cache-misses`, `LLC-load-misses`,
  `L1-dcache-load-misses`, `cycles` for `perf record`) silently fail
  or return zero on WSL2. The capture script attempts a
  software-only set (`task-clock,context-switches,page-faults,
  cpu-migrations,instructions:u,cycles:u`); on this host even those
  return non-zero from `perf stat`, so the `perfstat_*` files exist
  but should not be relied on. Wall-clock + objdump are the primary
  signals on this side.
- Hardware-counter analysis (cache miss rates, frontend stalls,
  branch mispredict ratios) was done on Windows under VTune; results
  in `artifacts/perf/msvc_profile/{hotspots,callstacks}.csv`.

## Inventory

| File | Lines / size | Description |
|------|-------------:|-------------|
| `wallclock_baseline.json`        | 96 rows | bench_run.py JSON, 24 benches × 4 aggregates |
| `diff_msvc_vs_gcc.md`            | 26      | Cross-toolchain markdown table from `bench_compare.py` |
| `perfstat_*.{stdout,stderr}`     | 8 files | `perf stat` software events (PMU N/A; informational only) |
| **graph-v3 algorithm bodies**    |         | (sift_down_/sift_up_/comparator are *fully inlined* — no separate symbols) |
| `dijkstra_csr_idx2.asm`          | 361     | Outer dijkstra closure, Idx2 (heap inlined) |
| `dijkstra_csr_idx4.asm`          | 387     | Outer dijkstra closure, Idx4 (heap inlined) |
| `dijkstra_csr_idx8.asm`          | 382     | Outer dijkstra closure, Idx8 (heap inlined) |
| `dijkstra_vov_idx4.asm`          | 465     | Outer dijkstra closure, VoV Idx4 (heap inlined) |
| **BGL counterparts**             |         |             |
| `dijkstra_bgl_csr.asm`           | 412     | run_bgl_dijkstra<csr_graph> (full BGL body inlined) |
| `dijkstra_bgl_adj.asm`           | 424     | run_bgl_dijkstra<adjacency_list> (full BGL body inlined) |

### Symbols that have no standalone body under GCC

Under `-O3`, GCC inlines all of these into the dijkstra closure they're
called from. The MSVC build does emit standalone bodies; the inlined
GCC instructions are folded into the dijkstra body counts above.

| Symbol (MSVC name)                              | MSVC body | GCC |
|-------------------------------------------------|----------:|-----|
| graph-v3 `sift_down_` (per arity)               | 184–186   | inlined |
| graph-v3 `sift_up_`                             |       109 | inlined |
| graph-v3 `container_value_fn::operator()`       |        85 | inlined |
| BGL `preserve_heap_property_down`               |       299 | inlined |
| BGL `preserve_heap_property_up`                 |       204 | inlined |

The absence is itself a real codegen data point — see
`agents/csr_edge_value_perf_plan.md` Phase 2 Linux GCC.

## Headline observation

Treating "fully inlined dijkstra body" as the unit of comparison
(MSVC sum = dijkstra body + sift_down_ + sift_up_; GCC = the single
emitted closure body):

|                            | graph-v3 Idx4 | BGL CSR | ratio |
|----------------------------|--------------:|--------:|------:|
| MSVC `/O2 /Ob3` (sum)      |           499 |  1,008  | 2.0×  |
| GCC `-O3` (closure body)   |           387 |    412  | 1.06× |

graph-v3's closure compresses ~22 % under GCC; BGL's compresses ~59 %.
That delta is consistent with the wall-clock observation that on Linux
GCC graph-v3 CSR Idx4 is **+15 % to +40 % slower than BGL CSR**, while
on MSVC it is 34–64 % *faster*.

## Gap-status verdict (decision tree from `thread_b_linux_runbook.md`)

> graph-v3 still +30 %+ slower on Grid (the original 4.3a worst case)

The Phase 4.3a Linux/GCC gap is **intact at HEAD**:

| Topology  | 2025 4.3a Idx4 vs BGL | 2026-04-28 Idx4 vs BGL |
|-----------|-----------------------|------------------------|
| ER_Sparse | +7.7 %                | +14.7 % – +21.9 %      |
| Grid      | +36.5 %               | **+36.2 % – +39.9 %**  |
| BA        | +9.4 %                | +6.0 % – +18.8 %       |
| Path      | +15.0 %               | +15.2 % – +15.6 %      |

Phases 3–5 of `csr_edge_value_perf_plan.md` are un-deferred.

## Manifest reference

Capture targets are listed in `agents/perf_capture_manifest_linux.txt`
(format: `basename[:N]  length_hex  regex  [substr ...]`). Differences
from the MSVC manifest:

- GCC mangling uses `Nul` for `unsigned long` non-type template args
  (e.g. `use_indexed_dary_heap<4ul>`), not MSVC's `4u`.
- The graph-v3 dijkstra body lives inside an inner closure
  `{lambda(auto:1&)#1}::operator()`; the manifest matches it via the
  combination of `use_indexed_dary_heap<Nul>` + `operator()` +
  graph-type substring (`compressed_graph` / `dynamic_graph`).
- BGL's body is captured via `run_bgl_dijkstra` +
  `compressed_sparse_row_graph` / `adjacency_list`. The
  `dijkstra_shortest_paths_no_color_map_no_init` regex from the MSVC
  manifest matches no symbol on GCC (fully inlined).
- `.cold` partitions exist for several of these. `nm --print-size`
  reports only the hot partition; the cold partition is at a lower
  address (see `BM_Dijkstra_*_Idx4(...) [clone .cold]` symbols) and is
  not currently captured.
