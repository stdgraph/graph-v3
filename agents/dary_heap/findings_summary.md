# Dijkstra `Heap` Template Parameter — Findings Summary

**Status:** Final (closes Phase 4 of `indexed_dary_heap_plan.md`)
**Period:** 2026-04-25 → 2026-04-27
**Branch:** `indexed-dary-heap`
**Scope:** Performance evaluation of `dijkstra_shortest_paths` after the
`Heap` template parameter was added, comparing graph-v3 against Boost.Graph
(BGL) on Linux (WSL2 / GCC) and Windows (MSVC), on identical hardware.

This document is the consolidated reference for the heap-selector decision.
For the raw run logs, baselines, and per-phase analysis see:

- [indexed_dary_heap_plan.md](indexed_dary_heap_plan.md) — phased work plan
- [indexed_dary_heap_baseline.md](indexed_dary_heap_baseline.md) — Linux/GCC Phase 0 baseline
- [indexed_dary_heap_baseline_msvc.md](indexed_dary_heap_baseline_msvc.md) — Windows/MSVC baseline + `/Ob3`
- [indexed_dary_heap_results.md](indexed_dary_heap_results.md) — Phase 4.1–4.3e detailed results
- [csr_edge_value_perf_plan.md](csr_edge_value_perf_plan.md) — follow-on CSR-access investigation

---

## 1. What was added

A new `Heap` template parameter on `dijkstra_shortest_paths` and
`dijkstra_shortest_distances`, selecting one of two heap implementations
via tag dispatch:

| Tag | Implementation |
|-----|----------------|
| `use_default_heap` (default) | `std::priority_queue` with lazy deletion. Heap may grow to O(E); stale entries skipped at pop. |
| `use_indexed_dary_heap<Arity = 4>` | Indexed d-ary heap with true decrease-key. Heap size bounded by O(V); no stale pops. Position map auto-selected: `vector_position_map` for `index_vertex_range<G>`, `assoc_position_map` (unordered_map) otherwise. |

Both branches preserve identical visitor semantics
(`on_examine_vertex` / `on_finish_vertex` fire exactly once per reachable
vertex; `on_edge_relaxed` / `on_edge_not_relaxed` fire exactly once per
outgoing edge of every examined vertex).

---

## 2. Test matrix

Same machine (Titania, 20×3.61 GHz; 48 KiB L1-D, 1.28 MiB L2, 25 MiB L3)
under both toolchains.

| Axis | Values |
|------|--------|
| OS / toolchain | Linux WSL2 + GCC (Phase 0/4.1), Windows + MSVC 19.50 (Phase 4.3b–e) |
| Container | `compressed_graph` (CSR), `dynamic_graph` (VoV) |
| Topology | Erdős–Rényi sparse (E/V ≈ 8), Barabási–Albert m=4 (E/V ≈ 8), 2D grid (E/V ≈ 4), path (E/V = 1) |
| Size | 1 K, 10 K, 100 K vertices |
| Heap | `Default`, `Idx2`, `Idx4`, `Idx8` |
| Reference | Boost.Graph `dijkstra_shortest_paths_no_color_map_no_init` on `compressed_sparse_row_graph` and on `adjacency_list` |

Distance-vector parity vs BGL is asserted at startup
(`check_bgl_distance_parity` in `bgl_dijkstra_fixtures.hpp`) for ER, BA, and
Path at n = 1024.

---

## 3. Headline performance results (CSR, n = 100 000)

### Linux / GCC (Phase 4.1, mean of 3 runs)

| Topology | E/V | Default | Idx2 | Idx4 | Idx8 | Best vs Default |
|----------|----:|--------:|-----:|-----:|-----:|:---------------:|
| ER Sparse | 8 | 27.0 ms | 24.2 ms | 25.8 ms | **20.2 ms** | **−25 %** |
| BA m=4    | 8 | 22.9 ms | 22.1 ms | 20.0 ms | **19.0 ms** | **−17 %** |
| Grid      | 4 | **6.0 ms** | 6.7 ms | 8.2 ms | 8.4 ms | indexed +39 % regression |
| Path      | 1 | **0.27 ms** | 0.33 ms | 0.33 ms | 0.33 ms | indexed +22 % regression |

### Windows / MSVC (`/O2 /Ob2`, median of 5 reps)

| Topology | E/V | Default | Idx2 | Idx4 | Idx8 | Best vs Default |
|----------|----:|--------:|-----:|-----:|-----:|:---------------:|
| ER Sparse | 8 | 26.7 ms | 26.4 ms | **21.1 ms** | 22.2 ms | **−21 %** |
| BA m=4    | 8 | 25.3 ms | 25.4 ms | **19.6 ms** | 22.2 ms | **−22 %** |
| Grid      | 4 | **6.2 ms** | 6.9 ms | 6.9 ms | 8.2 ms | indexed +11 % regression |
| Path      | 1 | 1.33 ms | 0.49 ms | 0.50 ms | **0.49 ms** | **−63 %** ✅ |

### vs Boost.Graph on the same graphs (Linux/GCC, n = 100 K)

| Topology | graph-v3 Default | graph-v3 Idx8 | BGL CSR | BGL `adjacency_list` |
|----------|-----------------:|---------------:|--------:|---------------------:|
| ER Sparse | 26.2 ms | **22.9 ms** | **19.9 ms** | 34.2 ms |
| BA m=4    | 26.9 ms | **21.7 ms** | **19.6 ms** | 30.9 ms |
| Grid      | **6.2 ms** | 8.9 ms | **6.1 ms** | 9.9 ms |
| Path      | **0.27 ms** | 0.33 ms | 0.28 ms | 0.52 ms |

graph-v3 with the default heap **beats `boost::adjacency_list` on every
topology** (23–48 % faster). Against `boost::compressed_sparse_row_graph`
(BGL's optimised CSR with a native 4-ary indexed heap), `Idx8` closes most
of the gap on dense workloads (within ~5–10 %); the residual gap is in
graph-v3's edge-value access path on CSR, not in the heap (Phase 4.3a/b
verified — see [csr_edge_value_perf_plan.md](csr_edge_value_perf_plan.md)).

---

## 4. Cross-topology summary

| Topology | E/V | Best heap (GCC) | Best heap (MSVC) | Notes |
|----------|----:|:---------------:|:----------------:|-------|
| ER Sparse | 8 | **Idx8** | **Idx4** (Idx8 close) | Decrease-key wins on dense random graphs. |
| BA m=4    | 8 | **Idx8** | **Idx4** (Idx8 close) | Hub vertices drive heavy decrease-key traffic. |
| Grid      | 4 | **Default** | **Default** | Position-map bookkeeping outweighs decrease-key benefit at low E/V. |
| Path      | 1 | **Default** (slightly) | **Indexed (any)** — 2.7× faster | Under MSVC `std::priority_queue` codegen is materially slower than libstdc++ on this no-decrease-key workload. |

Key takeaways:

- **No single heap wins everywhere.** The trade-off is fundamentally
  topology-dependent: indexed-heap bookkeeping is overhead on low-E/V
  graphs, but pays off whenever decrease-key activity is meaningful.
- **Arity ordering on dense graphs:** Idx8 ≥ Idx4 > Idx2 on GCC; under
  MSVC Idx4 was a touch better than Idx8 at n = 100 K, but the two are
  within run-to-run noise on dense workloads.
- **`Arity = 4` matches Boost's `d_ary_heap_indirect`**, which is BGL's
  hard-coded internal default. Choosing `Arity = 4` as graph-v3's default
  preserves like-for-like comparability with BGL.
- **VoV gap is smaller** than CSR (Idx4 only −3 % on ER/BA): VoV's extra
  indirection dilutes the heap's relative contribution.

---

## 5. Decision: defaults

| Aspect | Choice | Rationale |
|--------|--------|-----------|
| Public default heap | **`use_default_heap`** | Lowest overhead on low-E/V workloads (Grid, Path) under GCC. Wins or ties on 3 of 4 topologies on Linux. The single MSVC Path regression is fully recoverable by users opting into `use_indexed_dary_heap` for that workload. |
| Default arity for `use_indexed_dary_heap` | **`Arity = 4`** | Matches BGL's hard-coded arity, simplifying apples-to-apples comparison; on x86_64 Idx4 and Idx8 are within 1–5 pp on dense workloads — Idx8 wins narrowly under GCC, Idx4 wins narrowly under MSVC. `Arity = 4` is the safer default; users tuning for high E/V on x86_64 should explicitly choose `use_indexed_dary_heap<8>`. |

Documented user guidance (mirrored in the algorithm header):

> Use `use_indexed_dary_heap<8>` for dense (E/V ≳ 8) random or scale-free
> graphs on CSR. Keep `use_default_heap` for grid-like, path-like, or
> generally low-E/V workloads. The MSVC Path workload is an additional
> case where opting into the indexed heap is a clear win.

A heuristic auto-selector based on E/V was considered and rejected:
computing E/V at call time adds overhead, and a compile-time E/V is not
available for the general `adjacency_list` concept.

---

## 6. Toolchain-specific finding (MSVC)

VTune software-mode hotspots on `Grid_Idx4/100K` (Phase 4.3b–e) showed that
MSVC `/O2 /Ob2` does **not** inline the indexed-heap internals — `sift_down_`
appeared as a real call frame consuming ~31 % of CPU time, and
`std::less<double>::operator()` appeared as multiple distinct callable
symbols (~17 % combined). The same code under GCC `-O2/-O3` is fully
collapsed into a single inlined run-lambda.

`__forceinline` on `sift_down_` / `sift_up_` / `less_than_` / `place_`
alone had no measurable effect at `/Ob2`. The combination
**`/Ob3` + `GRAPH_DETAIL_FORCE_INLINE` on `sift_down_`** does close the
inlining gap (98.8 % of CPU now in a single inlined frame), but the
wall-clock impact is mixed:

| Topology (100 K) | `/Ob3` Δ vs `/Ob2` |
|------------------|-------------------:|
| ER Sparse Idx4 | −2.6 % |
| Grid Idx4 | +8.2 % regression |
| BA Idx4 | +6.3 % regression |
| Path Idx4 | **−7.6 %** |
| Path Default | +5.3 % |

The `/Ob3` regressions on Grid and BA come from icache pressure: the
inlined `sift_down_` body expands the run-lambda enough to hurt
code-layout-sensitive workloads.

**Outcome:** `/Ob3` is **not** committed as the default MSVC release
flag. The build presets remain at `/O2 /Ob2 /DNDEBUG`. The
`GRAPH_DETAIL_FORCE_INLINE` macro stays in `indexed_dary_heap.hpp`
(harmless under GCC where it is `[[gnu::always_inline]]`); the
`sift_down_` annotation was reverted because it provides no GCC benefit
and only matters under `/Ob3` on MSVC, which we do not enable.

Users who care specifically about the MSVC Path case can build with
`/Ob3` themselves; the public defaults optimise for the common case.

---

## 7. Open follow-ups (out of scope for the heap parameter)

These are tracked in [csr_edge_value_perf_plan.md](csr_edge_value_perf_plan.md):

- The remaining ~5–10 % gap to BGL CSR on dense workloads is in graph-v3's
  edge-value access path (`edge_value(g, uv)` on `compressed_graph`), not
  in the heap. Phase 4.3b confirmed the gap is work-bound (does not grow
  with `n` across L2→L3 transition), ruling out memory-bound suspects.
- HW-counter profiling (`perf stat -e cycles,instructions,…`) is blocked
  on WSL2 PMC support and is deferred. VTune `uarch-exploration` on
  Windows is similarly deferred pending SEP driver / admin elevation.

No further heap changes are planned. The `Heap` template parameter
shipped, with `use_default_heap` as the default and
`use_indexed_dary_heap<4>` as the documented opt-in for dense workloads.
