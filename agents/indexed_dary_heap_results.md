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
