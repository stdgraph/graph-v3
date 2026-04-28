# Dijkstra Baseline Benchmarks (Phase 0.4)

Captured: 2026-04-25  
Branch: `indexed-dary-heap` (heap implementation: `std::priority_queue`, lazy-deletion)  
Binary: `build/linux-gcc-release/benchmark/algorithms/benchmark_dijkstra`  
Flags: `--benchmark_min_time=1s`

## Machine

| Property | Value |
|----------|-------|
| Host | Titania |
| CPUs | 20 × 3609.6 MHz |
| L1-D | 48 KiB × 10 |
| L2 | 1280 KiB × 10 |
| L3 | 25600 KiB × 1 |
| OS | Linux |

## Results

All times are wall-clock nanoseconds per Dijkstra call.  
Construction and distance-vector reset are excluded from the timed region.

### CSR (`compressed_graph`) — primary container

| Benchmark | 1K ns | 10K ns | 100K ns | Complexity |
|-----------|------:|-------:|--------:|------------|
| ER Sparse (E/V≈8) | 61 016 | 1 362 706 | 29 086 330 | O(N log N) |
| Grid 2D (E/V≈4) | 24 991 | 525 412 | 6 706 910 | O(N log N) |
| Barabási–Albert m=4 (E/V≈8) | 58 157 | 1 338 566 | 25 402 054 | O(N log N) |
| Path (E/V=1) | 2 991 | 28 017 | 275 799 | O(N) |

### VoV (`dynamic_graph`) — secondary container

| Benchmark | 1K ns | 10K ns | 100K ns | Complexity |
|-----------|------:|-------:|--------:|------------|
| ER Sparse (E/V≈8) | 56 416 | 1 396 716 | 32 867 125 | O(N²) † |
| Grid 2D (E/V≈4) | 25 512 | 511 499 | 8 587 763 | O(N log N) |
| Barabási–Albert m=4 (E/V≈8) | 52 471 | 1 402 162 | 32 211 598 | O(N²) † |
| Path (E/V=1) | 4 599 | 43 635 | 440 975 | O(N) |

† Google Benchmark fit `O(N²)` for VoV ER/BA at the measured scale; the
  true complexity is O((V + E) log V) — this is likely a fitting artefact
  from only three data points at high constant factors.  Treat as O(N log N)
  for interpretation purposes.

## Key Observations

| Observation | Detail |
|-------------|--------|
| CSR ≈ VoV at small scale (1K–10K) | Traversal cost is not yet dominant |
| CSR outperforms VoV at 100K | 29 ms vs 33 ms (ER), 6.7 ms vs 8.6 ms (Grid) |
| BA ≈ ER on CSR | Both E/V≈8, similar times as expected |
| Path is dramatically cheaper | 276 µs vs 29 ms at 100K — confirms lazy-deletion overhead with large heaps |
| CSR Path ≈ 2.76N | Sub-logarithmic: path graph → at most n pushes, heap stays tiny |
| VoV Path ≈ 4.41N | Consistent ~60% overhead vs CSR across all scales |

## What to Beat in Phase 4

After the indexed d-ary heap is integrated, every CSR row should improve.
The path graph is the hardest to beat (heap barely fills) and the ER/BA
graphs are the easiest to win on (O(E) heap pops with lazy-deletion vs
O(V) with decrease-key).

Target: CSR ER Sparse 100K ≤ **22 ms** (−25% vs 29 ms baseline).
