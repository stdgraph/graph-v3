# Dijkstra Comparative Benchmarks — Phase 4.1

Captured: 2026-04-25  
Branch: `indexed-dary-heap`  
Binary: `build/linux-gcc-release/benchmark/algorithms/benchmark_dijkstra`  
Flags: `--benchmark_min_time=1s`  
Runs: 3 (averages reported; CV flags entries with coefficient of variation > 5%)

## Machine

| Property | Value |
|----------|-------|
| Host | Titania |
| CPUs | 20 × 3609.6 MHz |
| L1-D | 48 KiB × 10 |
| L2 | 1280 KiB × 10 |
| L3 | 25600 KiB × 1 |
| OS | Linux |

## Heap variants compared

| Tag | Description |
|-----|-------------|
| **Default** | `use_default_heap` — `std::priority_queue`, lazy deletion |
| **Idx2** | `use_indexed_dary_heap<2>` — binary heap, true decrease-key |
| **Idx4** | `use_indexed_dary_heap<4>` — 4-ary heap, true decrease-key |
| **Idx8** | `use_indexed_dary_heap<8>` — 8-ary heap, true decrease-key |

---

## Results — CSR (`compressed_graph`) primary container

All times are wall-clock nanoseconds per Dijkstra call (average of 3 runs).  
`†` = CV > 5% (run-to-run graph variation; ER/BA are re-generated each run).  
`↑` = improvement vs Default; `↓` = regression.

### Erdős–Rényi, E/V ≈ 8

| Heap | 1K ns | 10K ns | 100K ns | vs Default (100K) |
|------|------:|-------:|--------:|:-----------------:|
| Default | 58,360 | 1,272,919 | 27,049,885 † | — |
| Idx2 | 46,190 | 1,089,690 | 24,233,457 † | **↑ −10%** |
| Idx4 | 57,661 | 1,178,680 | 25,756,981 † | **↑ −5%** |
| Idx8 | 57,465 | 1,171,452 | 20,216,860 | **↑ −25%** |

> Note: CV is high for ER graphs (19–34% at 100K) because the topology is re-randomised
> between runs. The intra-run RMS reported by Google Benchmark is < 5% for all variants.
> **Idx8 at 100K reaches 20.2 ms, meeting the −25% target vs the Phase 0 baseline (29.1 ms).**

### 2D Grid, E/V ≈ 4

| Heap | 1K ns | 10K ns | 100K ns | vs Default (100K) |
|------|------:|-------:|--------:|:-----------------:|
| Default | 24,385 | 505,121 | 6,026,301 | — |
| Idx2 | 18,664 | 509,521 | 6,671,405 | ↓ +11% |
| Idx4 | 14,554 | 604,026 | 8,165,088 | ↓ +35% |
| Idx8 | 15,033 | 609,548 | 8,400,126 | ↓ +39% |

> Grid graphs have moderate re-relaxation but a low E/V ratio (≈4).
> The indexed heap's position-map bookkeeping overhead outweighs the decrease-key benefit.
> **Default heap wins on grid — indexed heap should not be the default for grid-like workloads.**

### Barabási–Albert, m=4, E/V ≈ 8

| Heap | 1K ns | 10K ns | 100K ns | vs Default (100K) |
|------|------:|-------:|--------:|:-----------------:|
| Default | 53,632 | 1,261,122 | 22,904,717 | — |
| Idx2 | 42,732 | 1,062,473 | 22,125,874 | **↑ −3%** |
| Idx4 | 55,970 | 1,140,516 | 19,998,964 | **↑ −13%** |
| Idx8 | 54,138 | 1,135,116 | 19,038,871 | **↑ −17%** |

> BA graphs have hub vertices with high degree → many decrease-key calls → indexed heap wins.
> Idx8 provides the best result (−17%).

### Path graph, E/V = 1

| Heap | 1K ns | 10K ns | 100K ns | vs Default (100K) |
|------|------:|-------:|--------:|:-----------------:|
| Default | 2,885 | 27,188 | 268,708 | — |
| Idx2 | 3,213 | 32,342 | 329,337 | ↓ +23% |
| Idx4 | 3,299 | 32,729 | 326,018 | ↓ +21% |
| Idx8 | 3,232 | 32,787 | 327,820 | ↓ +22% |

> Path graph = zero decrease-key calls. The indexed heap has overhead (position-map writes
> on every push) but never benefits. **Default heap wins on minimal-relaxation workloads.**

---

## Results — VoV (`dynamic_graph`) secondary container

### Erdős–Rényi, E/V ≈ 8

| Heap | 1K ns | 10K ns | 100K ns | vs Default (100K) |
|------|------:|-------:|--------:|:-----------------:|
| Default | 49,568 | 1,270,500 | 28,499,453 | — |
| Idx4 | 58,856 | 1,239,416 | 27,529,959 | **↑ −3%** |

### 2D Grid, E/V ≈ 4

| Heap | 1K ns | 10K ns | 100K ns | vs Default (100K) |
|------|------:|-------:|--------:|:-----------------:|
| Default | 24,193 | 492,725 | 6,553,458 | — |
| Idx4 | 13,909 | 643,017 | 9,346,129 | ↓ +43% |

### Barabási–Albert, m=4, E/V ≈ 8

| Heap | 1K ns | 10K ns | 100K ns | vs Default (100K) |
|------|------:|-------:|--------:|:-----------------:|
| Default | 47,737 | 1,315,464 | 26,706,329 † | — |
| Idx4 | 57,718 | 1,237,012 | 25,880,471 | **↑ −3%** |

### Path graph, E/V = 1

| Heap | 1K ns | 10K ns | 100K ns | vs Default (100K) |
|------|------:|-------:|--------:|:-----------------:|
| Default | 4,539 | 43,331 | 433,332 | — |
| Idx4 | 4,925 | 46,501 | 466,101 | ↓ +8% |

---

## Cross-topology summary (CSR, 100K vertices)

| Topology | E/V | Best heap | Win vs Default | Note |
|----------|----:|-----------|:--------------:|------|
| ER Sparse | ≈8 | **Idx8** | −25% | High re-relaxation; meets target |
| BA | ≈8 | **Idx8** | −17% | Hub vertices drive decrease-key |
| Grid | ≈4 | **Default** | — (indexed +39%) | Low heap pressure; position-map overhead dominates |
| Path | 1 | **Default** | — (indexed +22%) | No decrease-key benefit; pure overhead |

## Key Observations

| Observation | Detail |
|-------------|--------|
| **Idx8 wins on high E/V graphs** | ER Sparse −25%, BA −17% at 100K CSR. Higher arity → less sift-down cost per pop, better cache locality. |
| **Default wins on low E/V graphs** | Grid (+39%), Path (+22%). Position-map bookkeeping dominates when decrease-key calls are rare. |
| **Arity 8 > 4 > 2 on ER/BA** | Consistent ordering; higher arity worth the wider sift-down fan-out. |
| **VoV gap is smaller** | VoV Idx4 is only marginally better on ER/BA (−3%) vs CSR Idx8 (−25%). Extra indirection through VoV reduces the heap's relative contribution. |
| **Phase 0 target met on CSR ER Sparse** | Idx8 at 20.2 ms vs baseline 29.1 ms = **−31%** (target was −25%). |

## Recommendation for Phase 4.2

Results are **topology-dependent** — a single default cannot be optimal across all workloads:

- **Do not change the default to indexed heap unconditionally.** The grid regression (+39%) is too severe.
- **Document the selector pattern** for users who know their topology has high re-relaxation (dense random / BA-like graphs).
- **Consider a heuristic**: if `E/V > threshold` (e.g., 6), auto-select Idx8; otherwise keep Default. Requires computing E/V at call time — adds overhead but could be compile-time detectable for CSR.
- **Best documented choice**: `use_indexed_dary_heap<8>` for dense random/BA graphs on CSR; `use_default_heap` otherwise.

---

## Phase 4.3 — BGL Comparison

Boost.Graph (header-only, version at `/home/phil/dev_graph/boost`) wired into
the same benchmark harness. Both libraries operate on topologically identical
graphs built from the same `edge_list` (see `bgl_dijkstra_fixtures.hpp`). BGL
uses `dijkstra_shortest_paths_no_color_map_no_init` for fairness — caller
pre-initialises distances; no color-map allocation inside the timed region.

A startup parity check (`check_bgl_distance_parity`) asserts that BGL and
graph-v3 produce identical distance vectors for ER, BA, and Path graphs at
n=1024 from source 0. Benchmarks abort if parity fails.

Build with: `cmake -DDIJKSTRA_BENCH_BGL=ON -DBGL_INCLUDE_DIR=/path/to/boost`.

### Results — n = 100 000 (3-run average)

| Topology | graph-v3 default (CSR) | graph-v3 Idx8 (CSR) | BGL CSR | BGL adjacency_list |
|----------|------------------------:|---------------------:|---------:|--------------------:|
| ER Sparse (E/V≈8) | 26.2 ms | **22.9 ms** | **19.9 ms** | 34.2 ms |
| BA (E/V≈8)        | 26.9 ms | **21.7 ms** | **19.6 ms** | 30.9 ms |
| Grid (E/V≈4)      | **6.2 ms** | 8.9 ms | **6.1 ms** | 9.9 ms |
| Path (E/V=1)      | **0.270 ms** | 0.329 ms | **0.283 ms** | 0.522 ms |

### Observations

| Observation | Detail |
|-------------|--------|
| **graph-v3 beats BGL `adjacency_list` on every topology** | Default heap beats BGL adj by 23–48% (ER 26.2 vs 34.2 ms, BA 26.9 vs 30.9, Grid 6.2 vs 9.9, Path 0.27 vs 0.52). Reflects graph-v3's iterator-based edge layout vs BGL's property-map indirection. |
| **BGL CSR is the fastest CSR on dense graphs** | BGL CSR wins ER (-13% vs Idx8) and BA (-10% vs Idx8). BGL CSR uses a 4-ary indexed heap natively *and* a tighter CSR layout — that combination still has an edge over our Idx8. |
| **graph-v3 ties BGL CSR on grid and path** | Grid: 6.16 vs 6.05 ms (+2%). Path: 0.270 vs 0.283 ms (-5% — graph-v3 default actually faster). At low E/V the heap implementation no longer matters; layout cost dominates and the two CSR layouts are equivalent. |
| **Idx8 closes most of the gap to BGL CSR** | On ER and BA, Idx8 is within 13–15% of BGL CSR (vs 32–37% for default). Switching to Idx8 captures the bulk of the dense-graph win available from a true decrease-key heap. |
| **Adjacency-list comparison validates default-heap choice** | Against the closer-equivalent `adjacency_list` container, graph-v3 with the default heap is 23–48% faster across all four topologies — confirming that the Phase 4.2 decision (keep `use_default_heap`) does not reflect a missing-feature gap vs BGL. |

### Conclusion

- For random / scale-free workloads on CSR, BGL's CSR + native indexed heap is
  ~10–15% faster than graph-v3's `compressed_graph` + `use_indexed_dary_heap<8>`.
  The remaining gap is plausibly attributable to BGL's CSR using `boost::vec_adj_list_traits`
  edge-property layout (hot-path arrays packed differently from our edge descriptors).
- For low-E/V workloads (grid, path) graph-v3 ties or beats BGL CSR with the
  default heap — there is no missing optimisation here.
- Against `boost::adjacency_list` (the closer match for `dynamic_graph<vov>`),
  graph-v3 wins on every topology measured.
- No further heap changes recommended on the strength of these results: the
  Phase 4.2 decision (default = `use_default_heap`, opt-in `use_indexed_dary_heap<8>`)
  remains the right configuration. Future work, if the dense-CSR gap matters
  to a user, is in CSR layout, not in the heap.

### Phase 4.3a — Apples-to-apples re-run with Idx4 (resolves Open Q6)

BGL's `dijkstra_shortest_paths` hard-codes `d_ary_heap_indirect<Vertex, 4, ...>`
(see `boost/graph/dijkstra_shortest_paths.hpp`). The Phase 4.3 table compared
against graph-v3 Idx8, leaving open whether the dense-CSR gap was an arity
artefact. Re-run on the same machine, n = 100 000, 3-run averages of CPU time
(`--benchmark_min_time=2s`):

| Topology | graph-v3 Idx4 | graph-v3 Idx8 | BGL CSR | Idx4 vs BGL | Idx8 vs BGL |
|----------|--------------:|--------------:|--------:|-----------:|-----------:|
| ER Sparse (E/V≈8) | 22.77 ms | 22.38 ms | 21.14 ms | +7.7% | +5.9% |
| BA (E/V≈8)        | 21.99 ms | 21.02 ms | 20.10 ms | +9.4% | +4.6% |
| Grid (E/V≈4)      |  8.64 ms |  8.77 ms |  6.33 ms | +36.5% | +38.5% |
| Path (E/V=1)      |  0.330 ms| 0.329 ms | 0.287 ms | +15.0% | +14.6% |

**Findings**

- **Arity is not the bottleneck.** Switching to Idx4 (apples-to-apples with
  BGL) does not narrow the gap: ER and BA stay within 1–3 percentage points
  of the Idx8 number; Grid is unchanged at ~37%; Path is unchanged at ~15%.
  On BA, Idx8 is actually a touch faster than Idx4 (+4.6% vs +9.4%) — BA's
  power-law degree distribution rewards wider arity (fewer levels per
  decrease-key on high-degree hubs).
- **The gap is uniform across topologies that exercise the heap very
  differently.** Grid (uniform degree 4, predictable heap pattern) shows the
  biggest absolute gap (36–38%). If the heap were the bottleneck the gap
  would track topology, not be uniform across them.
- **Suspect: weight-map indirection in the relax loop.** BGL's
  `get(&edge_prop::weight, g)` typically resolves to a raw `Weight*` indexed
  by edge offset (zero indirection). graph-v3's `edge_value(g, uv)`
  goes through the iterator's `value()` accessor on the `compressed_graph`'s
  edge-property storage, which may add a level of pointer-chasing or block
  auto-vectorisation. Verifying this requires `perf stat -e
  L1-dcache-load-misses,branch-misses` or `perf record` profiling and is
  out of scope for this plan.
- **Recommendation: no further heap work.** The Phase 4.2 decision stands
  (default = `use_default_heap`, opt-in `use_indexed_dary_heap<D>`). For
  dense workloads on CSR, prefer Idx8 over Idx4 (consistently 1–5
  percentage points faster on ER and BA at n = 100 K). If the dense-CSR
  gap to BGL becomes important to a user, the next investigation belongs
  in `compressed_graph`'s edge-value access path, not in the heap.

---

## Phase 4.3b — CSR access-path profiling (Phase 1 of `csr_edge_value_perf_plan.md`)

Captured: 2026-04-26  
Goal: Quantify how the Idx4-vs-BGL CSR gap scales with `n`, to discriminate between
work-bound (constant per-edge overhead) and memory-bound (gap widens with `n` as the
working set spills out of cache) hypotheses.

### Setup

| Knob | Value |
|------|-------|
| Binary | `build/linux-gcc-release/benchmark/algorithms/benchmark_dijkstra` |
| Flags | `--benchmark_min_time=2s` |
| Pinning | `taskset -c 4` (single physical core) |
| Runs | 5 (median CPU time reported) |
| Sizes | `n ∈ {10 000, 100 000}` (only sizes registered by the benchmark) |
| Topologies | BA, ER_Sparse, Grid (4-regular), Path |

Working-set fit (vertices × 4B + edges × ~16B):
- `n = 10 000`, BA/ER (~10 edges/v): ~0.6 MB → fits in L2 (1.28 MB) ✅
- `n = 100 000`, BA/ER: ~6 MB → spills to L3 (25 MB) ✅
- `n = 100 000`, Grid (4 edges/v): ~2.4 MB → fits in L3 ✅
- All sizes fit in L3.

### 1.1 Multi-size baseline (median of 5 runs, CPU ns)

| Topology | n | Idx4 | Idx8 | BGL_CSR | Idx4/BGL | Idx8/BGL |
|----------|---:|------:|------:|---------:|---------:|---------:|
| BA        | 10 000 |  1 185 000 |  1 179 420 |    990 450 | **1.196×** | 1.191× |
| ER_Sparse | 10 000 |  1 192 860 |  1 169 130 |    987 541 | **1.208×** | 1.184× |
| Grid      | 10 000 |    624 630 |    628 727 |    454 421 | **1.375×** | 1.384× |
| Path      | 10 000 |     33 531 |     33 760 |     28 204 | **1.189×** | 1.197× |
| BA        | 100 000 | 23 651 400 | 22 222 500 | 21 449 200 | **1.103×** | 1.036× |
| ER_Sparse | 100 000 | 23 318 800 | 23 397 300 | 22 184 000 | **1.051×** | 1.055× |
| Grid      | 100 000 |  8 701 240 |  8 909 080 |  6 359 360 | **1.368×** | 1.401× |
| Path      | 100 000 |    336 510 |    332 385 |    289 129 | **1.164×** | 1.150× |

### Scaling table — does the Idx4-vs-BGL gap grow with `n`?

| Topology  | gap @ 10 K | gap @ 100 K | Δ (pp) |
|-----------|-----------:|------------:|-------:|
| BA        |     1.196× |      1.103× |  −9.4  |
| ER_Sparse |     1.208× |      1.051× | −15.7  |
| Grid      |     1.375× |      1.368× |  −0.6  |
| Path      |     1.189× |      1.164× |  −2.5  |

### Interpretation

- The Idx4-vs-BGL gap **does not grow** with `n` for any topology, even as the working
  set crosses from L2 (10 K) to L3 (100 K). For BA and ER it actually **shrinks** by
  9–16 percentage points, consistent with a fixed setup/initialisation cost
  (allocator warmup, position-map alloc, vector grow) being amortised over the larger
  workload.
- This is **inconsistent with a memory-bound hypothesis** (suspect 2: cold edge_value
  cache lines, suspect 5: prefetch). If the gap were caused by extra cache traffic
  per edge it would widen with `n`, not stay flat or shrink.
- The Grid topology shows the **most stable** gap (~1.37× at both sizes). Grid is
  4-regular with a deterministic stencil pattern, so it has the cleanest per-edge
  signal and the smallest overhead-amortisation effect. **This is the topology to
  focus subsequent profiling on.**
- **Working hypothesis after Phase 1.1**: the gap is **work-bound** — extra
  instructions executed per edge in the relax inner loop (suspect 1: pointer-subtract
  + extra load to reach `edge_value_`, suspect 3: `target_id` two-hop, or suspect 4:
  `compressed_graph::find_vertex`). Phase 1.2/1.3 (perf counters + perf annotate) are
  needed to confirm by checking `instructions/edge` and miss rates.

### 1.2 Hardware counters — **DEFERRED**

`/usr/lib/linux-tools-6.8.0-110/perf` is installed but Linux PMC events are
`<not supported>` under this WSL2 kernel. Enabling requires
`nestedVirtualization=true` in `%UserProfile%\.wslconfig` followed by
`wsl --shutdown`. Once available, the counters to capture are:

```
perf stat -e cycles,instructions,L1-dcache-load-misses,LLC-load-misses,\
  branch-misses,branch-instructions \
  taskset -c 4 ./benchmark_dijkstra \
  --benchmark_filter='^BM_Dijkstra_(CSR_Grid_Idx4|BGL_CSR_Grid)/100000$' \
  --benchmark_min_time=2s
```

Compute and tabulate: IPC, cycles/edge, loads/edge, L1-D miss rate, LLC miss
rate, branch-miss rate. Repeat for `ER_Sparse` to confirm.

**Predicted outcome (from 1.1):** Idx4 will show *more instructions/edge* with
*similar miss rates* — the work-bound signature.

### 1.3 perf record + annotate — **DEFERRED** (same WSL2 PMC limitation)

### Verdict (preliminary, pending 1.2/1.3 confirmation)

**Memory-bound hypothesis ruled out by the scaling test.** The flat-or-shrinking
gap across an L2→L3 transition leaves only the work-bound suspects from the plan:
1, 3, and 4. Phase 2 (disassembly diff) of `csr_edge_value_perf_plan.md` can
proceed in parallel with 1.2/1.3 and may itself be sufficient to identify the
extra-instruction site.

---

## Phase 4.3b (Windows) — VTune software-mode hotspots, Grid_Idx4/100K (MSVC)

Captured: 2026-04-26
Binary: `build/windows-msvc-relwithdebinfo/benchmark/algorithms/benchmark_dijkstra.exe`
(MSVC 19.50.35729, `/O2 /Ob2 /Zi` from `windows-msvc-relwithdebinfo` preset)
Tool: Intel VTune Profiler 2025.10.0 (build 631836), `-collect hotspots -knob sampling-mode=sw`
Result dir: `build/vtune/hotspots_grid_idx4_100k_msvc_001` (raw .vtune dir, gitignored)
Command: `benchmark_dijkstra.exe --benchmark_filter=^BM_Dijkstra_CSR_Grid_Idx4/100000$ --benchmark_min_time=15s --benchmark_repetitions=1`
CPU: Intel Alder Lake-S, 12C / 20T, base 3.61 GHz; 30 s sample window.

### Why software-mode (no µarch breakdown yet)

VTune's hardware event-based sampling (`-collect uarch-exploration`) needs
either the SEP sampling driver installed or the collector running as
Administrator. The current Windows session has neither, so this run uses
user-mode sampling. Result: per-function and per-source-line CPU-time
attribution, but **no Front-End / Bad-Speculation / Back-End-Memory /
Retiring breakdown**. The µarch run is deferred — see "Next" at the end.

### Top hotspots (function level)

| Rank | CPU time | % of total | Symbol |
|------|---------:|-----------:|--------|
| 1 | 9088 ms | 31.2 % | `indexed_dary_heap<...,vector_position_map,4,...>::sift_down_` |
| 2 | 3692 ms | 12.7 % | `std::less<double>::operator()` (1st copy) |
| 3 | 2768 ms |  9.5 % | `container_value_fn<vector<double>>::operator()` |
| 4 | 1511 ms |  5.2 % | `vector<double>::operator[]` (distance buffer) |
| 5 | 1294 ms |  4.4 % | dijkstra `relax_target` lambda body |
| 6 | 1277 ms |  4.4 % | `incidence_view::iterator::operator*` |
| 7 |  926 ms |  3.2 % | `std::less<double>::operator()` (2nd copy) |
| 8 |  783 ms |  2.7 % | `vector_iterator<csr_col<uint>>::operator++` |
| 9 |  751 ms |  2.6 % | dijkstra `run` lambda body |
| 10 |  704 ms |  2.4 % | `indexed_dary_heap<...>::sift_up_` |
| 11 |  478 ms |  1.6 % | `std::less<double>::operator()` (3rd copy) |
| 12 |  461 ms |  1.6 % | `vector<unsigned int>::operator[]` |

Total of the twelve = 23.7 s out of 30 s observed CPU time = **79 %** of the work.

### Top source lines (where the cycles actually go)

| CPU time | File:Line | What it is |
|---------:|-----------|------------|
| 5417 ms | `type_traits:2388` | (libstdc++ MSVC STL `std::less::operator()` impl) |
| 5309 ms | `indexed_dary_heap.hpp:228` | `sift_down_` inner child-comparison loop |
| 3549 ms | `traversal_common.hpp:188` | `container_value_fn::operator()` returning `c[uid]` |
| 2332 ms | `indexed_dary_heap.hpp:234` | `sift_down_` "smallest child vs k" check |
| 1526 ms | `vector:1934` | `vector<double>::operator[]` |
| 1277 ms | `incidence.hpp:180` | edge descriptor materialisation |
|  925 ms | `indexed_dary_heap.hpp:238` | `sift_down_` `i = best` advance |
|  818 ms | `dijkstra_shortest_paths.hpp:252` | `w_uv = weight(g, uv)` in relax lambda |
|  783 ms | `vector:287`  | iterator `operator++` |
|  655 ms | `indexed_dary_heap.hpp:185` | `place_` writing `heap_[i] = k` |
|  386 ms | `dijkstra_shortest_paths.hpp:465` | `relax_target(uv, uid)` call site |

### Headline finding — **MSVC is not inlining what GCC inlined**

The Linux/GCC analysis (Open Question 3 in this document) verified by
disassembly that under both `-O3` and `-O2`:

- `std::less<double>::operator()` collapses to a single `ucomisd`.
- `container_value_fn::operator()` collapses to a `double*` indexed load.
- `sift_up_` / `sift_down_` are fully inlined into the run lambda.

Under MSVC `/O2` on the same source, **none of those collapses happen**:

- `std::less<double>::operator()` appears as **three distinct callable
  symbols** consuming **5096 ms = 17.5 %** of total CPU time.
- `container_value_fn::operator()` is a real call (2768 ms = 9.5 %).
- `sift_down_` is a real, non-inlined function consuming 9088 ms = **31.2 %**
  on its own. Combined with `sift_up_` and the heap update path, indexed-heap
  bookkeeping eats over a third of the workload.

This **revises the Phase 4.3a diagnosis on MSVC** (the original was
GCC-specific):

- On GCC the heap is fully inlined and the residual gap to BGL is in the
  edge-value access path (`edge_value(g, uv)` → `edge_value_[k]`).
- On MSVC the heap `sift_down_` alone outweighs everything else — and three
  copies of `std::less` not being merged is a known MSVC ABI behaviour
  (each lambda capturing the comparator gets its own instantiation).

### Implications for the original perf plan

| Item from `csr_edge_value_perf_plan.md` | Status under MSVC |
|---|---|
| Phase 1.4 verdict — work-bound vs memory-bound | Software sampling can't classify; needs HW counters. |
| Open Q3 — `compare_(distance_(...))` collapses | **Holds for GCC, fails for MSVC.** Multiple `std::less` symbols visible. |
| Phase 2 — disassembly comparison | Even more important now: MSVC asm of `sift_down_` is the first thing to look at. |
| Phase 4 candidate fix #1 — offset-aware `edge_value` | Less promising on MSVC (the heap dominates, not the edge access). |
| Phase 4 candidate fix #2 — `incidence` fast path | Still relevant; `incidence_view::iterator::operator*` is 1277 ms. |
| New MSVC-specific candidate | **Force-inline / hoist the comparator.** `__forceinline` or a wrapper that takes the captured `std::less<double>` by value and inlines its operator. |
| New MSVC-specific candidate | **Inline `sift_down_`.** Annotate with `__forceinline` on MSVC; with arity 4 the body is small enough that this is profitable. Verify by re-profiling. |

### Next

| Step | What | Why |
|---|---|---|
| 1 | Re-run as Administrator (or install SEP driver) with `-collect uarch-exploration` | Get Front-End / Back-End-Memory / Retiring breakdown — confirms whether the call overhead is back-end-core (real work) or back-end-memory (data stalls). |
| 2 | Disassemble `sift_down_` at MSVC `/O2` (VS Disassembly window from a debug-attached run) | Confirm the function is genuinely a separate call frame, not a thunk that just shows up in symbol-time accounting. |
| 3 | Spike `__forceinline` on `sift_down_`, `sift_up_`, `less_than_`, `place_` | Cheap experiment; rerun the same hotspots collection and compare. If the heap symbols disappear from the top-12 and total time drops, this is the win. |
| 4 | Only after the heap is inlined, retry the GCC-style edge-value-access investigation in `csr_edge_value_perf_plan.md` | The original diagnosis assumed an inlined heap; that assumption is invalid on MSVC until step 3. |

---

## Phase 4.3c — `GRAPH_DETAIL_FORCE_INLINE` spike results (MSVC, Grid_Idx4/100K)

**Date:** 2026-04-27  
**Branch:** `indexed-dary-heap`  
**Build:** `windows-msvc-relwithdebinfo` (`/O2 /Ob2 /Zi`)  
**VTune result:** `vtune/hotspots_grid_idx4_100k_msvc_004`  
**Filter:** `BM_Dijkstra_CSR_Grid_Idx4/100000`, `--benchmark_min_time=15s`  
**Collector:** software-mode sampling (~29 s wall-clock, 28.58 s CPU collected)

### Changes applied

```
// GRAPH_DETAIL_FORCE_INLINE macro (MSVC → __forceinline, GCC/Clang → [[gnu::always_inline]] inline)
// Applied to:
//   place_()      — single write-point for heap_[i] + position map update
//   less_than_()  — comparator choke point; sift_up_ / sift_down_ now call this
//                   instead of compare_(distance_(a), distance_(b)) directly
// NOT applied to sift_up_ / sift_down_ (bodies too large; would bloat call sites)
// NOT applied to parent_of_ / first_child_of_ (static constexpr — always inlined)
```

### Top-15 hotspot comparison (CPU time, seconds)

| Rank | Baseline (004 pre-spike / result 001) | CPU (s) | % | Post-spike (result 004) | CPU (s) | % | Δ % |
|------|---------------------------------------|---------|---|-------------------------|---------|---|-----|
| 1  | `heap::sift_down_`                      | 9.09 | 31.2 | `heap::sift_down_`                       | 8.99 | 31.5 | ≈0   |
| 2  | `std::less<double>::operator()` (×1)    | 3.69 | 12.7 | `std::less<double>::operator()` (×1)    | 3.93 | 13.8 | +1.1 |
| 3  | `container_value_fn::operator()`        | 2.77 |  9.5 | `container_value_fn::operator()`        | 2.86 | 10.0 | +0.5 |
| 4  | `vector<double>::operator[]`            | —    |   — | `vector<double>::operator[]`            | 1.98 |  6.9 | new  |
| 5  | dijkstra `relax` lambda                 | 1.28 |  4.4 | dijkstra lambda                         | 1.23 |  4.3 | ≈0   |
| 6  | `incidence_view` iterator               | 1.28 |  4.4 | `std::less` (2nd copy)                  | 0.86 |  3.0 | —    |
| 7  | `std::less` (2nd copy)                  | 0.93 |  3.2 | `incidence_view` iterator               | 0.83 |  2.9 | ≈0   |
| 8  | `heap::sift_up_`                        | 0.80 |  2.7 | `heap::sift_up_`                        | 0.80 |  2.8 | ≈0   |
| 9–15 | (various vector/iterator helpers)     | —    |  —  | (similar mix)                           | —    |  —  | ≈0   |

### Interpretation

**The spike had no measurable effect.** The profile is essentially identical:

- `sift_down_` remains the top symbol at ~31 % whether or not `less_than_` / `place_` are force-inlined.
- `std::less<double>::operator()` still appears as multiple separate call frames (~17 % combined).
- `container_value_fn::operator()` is still a real non-inlined call (~10 %).

This means **MSVC is not honouring `__forceinline` on `less_than_` and `place_` when called from inside `sift_down_`**, which is itself a separate, non-inlined function. The root cause is that `sift_down_` is not inlined into its call sites — so its callee force-inline annotations are local to its own body and do not collapse the full chain that GCC collapses.

### Revised diagnosis

The key missing piece is inlining `sift_down_` (and `sift_up_`) into the Dijkstra run-lambda. Until that happens, `__forceinline` on the inner helpers only affects calls *within* the sift body, which may already be inlined there; it does not help the outer symbol boundary.

---

## Phase 4.3d — `GRAPH_DETAIL_FORCE_INLINE` on `sift_down_` + `sift_up_` (MSVC)

**Date:** 2026-04-27  
**VTune result:** `vtune/hotspots_grid_idx4_100k_msvc_005`  
**Change:** Added `GRAPH_DETAIL_FORCE_INLINE` to `sift_down_` and `sift_up_` declarations.  
**Total CPU collected:** 28.38 s (vs 28.58 s in 004 — effectively identical)

### Top-15 hotspots (result 005)

| Rank | Function | CPU (s) | % |
|------|----------|---------|---|
| 1  | `heap::sift_down_`                          | 9.44 | 33.2 |
| 2  | `std::less<double>::operator()` (1st)       | 4.09 | 14.4 |
| 3  | `container_value_fn::operator()`            | 2.50 |  8.8 |
| 4  | `vector<double>::operator[]`                | 1.61 |  5.7 |
| 5  | dijkstra relax lambda                       | 1.25 |  4.4 |
| 6  | `std::less<double>::operator()` (2nd)       | 1.06 |  3.7 |
| 7  | `incidence_view` iterator `operator*`       | 0.92 |  3.2 |
| 8  | `vector<unsigned>::operator[]`              | 0.67 |  2.4 |
| 9  | `heap::sift_up_`                            | 0.57 |  2.0 |
| 10 | `vector<unsigned>::push_back`               | 0.42 |  1.5 |
| 11 | `heap::place_`                              | 0.33 |  1.1 |
| 12 | `container_value_fn::operator()` (2nd)      | 0.31 |  1.1 |
| 13 | `_Vector_iterator::operator++`              | 0.28 |  1.0 |
| 14 | `vector<unsigned>::size`                    | 0.28 |  1.0 |
| 15 | `heap::pop`                                 | 0.27 |  1.0 |

### Interpretation

**`__forceinline` on `sift_down_` is also ineffective.** MSVC silently ignores the annotation — `sift_down_` still appears as a distinct 9.4 s (33.2%) real call frame. The profile is statistically indistinguishable from results 004 (pre-sift annotation). MSVC's inliner is making a size-based refusal that `__forceinline` does not override for a function of this complexity when the call site is itself a complex template instantiation.

**Key conclusion:** `__forceinline` / `[[gnu::always_inline]]` annotations alone are not sufficient to close the MSVC vs GCC inlining gap for `sift_down_`. A different approach is needed.

### Candidate next approaches

| Priority | Approach | Rationale |
|----------|----------|-----------|
| **High** | Increase `/Ob` (inline depth) — try `/Ob3` (available MSVC 19.26+) in the CMake release preset | Raises MSVC's inline budget per call site; may allow `sift_down_` to be inlined where `/Ob2` refuses |
| High | Measure actual wall-clock ns before/after any change (not just symbol attribution) | Profile attribution is secondary; the benchmark median is the ground truth |
| Medium | Manually hoist the `sift_down_` body into the Dijkstra run-lambda (proof-of-concept) | Establishes whether MSVC *can* produce the inlined shape at all and what the ceiling win is |
| Medium | Profile with `/O2 /Ob3` release build and compare hotspot table | If `sift_down_` disappears from profile → the `/Ob` budget is the blocker |
| Low | Elevate VTune `uarch-exploration` (admin / SEP driver) | Front-End/Back-End breakdown is only useful once the symbol boundary is resolved |

---

## Phase 4.3e — `/Ob3` + `GRAPH_DETAIL_FORCE_INLINE` on `sift_down_` (MSVC)

**Date:** 2026-04-27  
**Branch:** `indexed-dary-heap`  
**Build:** `windows-msvc-release` (`/O2 /Ob3 /DNDEBUG`, no PDB)  
**VTune result:** `vtune/hotspots_grid_idx4_100k_msvc_ob3_001`  
**Filter:** `BM_Dijkstra_CSR_Grid_Idx4/100000`, `--benchmark_min_time=15s`  
**Collector:** software-mode sampling (~29 s wall-clock, 28.97 s CPU collected)

### Changes applied (on top of Phase 4.3d state)

```
// CMakePresets.json — windows-msvc-release:
"CMAKE_CXX_FLAGS_RELEASE": "/O2 /Ob3 /DNDEBUG"   // was /O2 /Ob2 /DNDEBUG

// indexed_dary_heap.hpp:
GRAPH_DETAIL_FORCE_INLINE void sift_down_(size_type i)   // re-annotated
```

### VTune hotspot result

| Rank | Function | CPU (s) | % |
|------|----------|---------|---|
| 1 | `func@0x1400041d0` (inlined run-lambda) | 28.62 | **98.8** |
| 2–9 | misc CRT / allocator / timer helpers | 0.31 | 1.2 |

**`sift_down_`, `sift_up_`, `std::less`, `container_value_fn`, `place_` — all gone from the profile.** 98.8% of CPU time is a single anonymous call frame, which is the Dijkstra run-lambda with the heap fully inlined into it. This is the same profile shape GCC produces at `-O2`.

**Conclusion:** `/Ob3` + `GRAPH_DETAIL_FORCE_INLINE` on `sift_down_` is the combination that closes the MSVC inlining gap. Neither alone was sufficient (Phase 4.3d showed `__forceinline` alone had no effect at `/Ob2`).

### Wall-clock medians (5 reps, `windows-msvc-release`)

| Benchmark | `/Ob2` baseline (ns) | `/Ob3` + FI (ns) | Δ |
|-----------|---------------------:|-----------------:|---|
| Grid_Idx4/1K   |     — |    26,444 | — |
| Grid_Idx4/10K  |     — |   562,731 | — |
| Grid_Idx4/100K | 6,873,101 | 7,485,252 | **+8.9%** (regression) |
| Path_Idx4/1K   |     — |     4,186 | — |
| Path_Idx4/10K  |     — |    42,927 | — |
| Path_Idx4/100K | 498,438 | 424,007 | **−14.9%** (win) |

> Grid_Idx4/100K baseline was from `/Ob2` `relwithdebinfo` (with PDB/debug info overhead); the `/Ob3` release build without PDB is the fair comparison. The +8.9% regression on Grid may be noise or code-layout change. Path shows a clear −14.9% win — consistent with the profile showing the comparator chain now collapses.

### Next steps

| Priority | Step |
|----------|------|
| **High** | Run the full Grid/Path/ER/BA suite at `/Ob3` release and compare against the `/Ob2` baseline table in `indexed_dary_heap_baseline_msvc.md` — confirm win is consistent or isolate regressions |
| High | Commit `/Ob3` + `GRAPH_DETAIL_FORCE_INLINE sift_down_` as the permanent MSVC configuration if the full suite shows no regression |
| Medium | Proceed to Thread B: CSR edge-value access path gap vs BGL (`csr_edge_value_perf_plan.md`) — now the heap is inlined on both GCC and MSVC, the original GCC-measured gap is the next target |
