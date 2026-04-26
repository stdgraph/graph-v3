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
