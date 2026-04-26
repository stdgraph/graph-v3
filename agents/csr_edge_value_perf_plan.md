# `compressed_graph` edge-value access path — performance investigation plan

## Background

Phase 4.3a (see `indexed_dary_heap_results.md`) measured graph-v3 +
`use_indexed_dary_heap<D>` against BGL `compressed_sparse_row_graph` +
`d_ary_heap_indirect<…, 4, …>` on identical 100 K-vertex graphs:

| Topology | Idx4 vs BGL | Idx8 vs BGL |
|---|---:|---:|
| ER Sparse | +7.7% | +5.9% |
| BA        | +9.4% | +4.6% |
| Grid      | +36.5% | +38.5% |
| Path      | +15.0% | +14.6% |

Open Question 6 ruled out arity (Idx4/Idx8 within 1–3 pp) and the heap (gap is
*largest* on Grid, the topology with the most predictable heap-access pattern).
Open Question 3 confirmed `compare_(distance_(...))` inlines to a single
`ucomisd` against a direct base+idx*8 load. The remaining gap therefore lives
in the **relax loop's edge-value access path** — i.e. between
`for (auto&& [vid, uv] : views::incidence(g, u))` and the load of
`edge_value(g, uv)` inside the user's `WeightFn` lambda.

This document plans an investigation with no implementation commitments yet.

---

## What the access path actually does today

For `compressed_graph<double, void, void, uint32_t, uint32_t>`:

**Storage** (`include/graph/container/compressed_graph.hpp`)
- `row_index_`  : `vector<row_entry>`  — one entry per vertex, `.index` is offset into `col_index_`.
- `col_index_`  : `vector<col_entry>`  — one entry per edge, `.index` is `vertex_id_t` of target.
- `edge_value_` : `vector<EV>`         — one entry per edge, parallel to `col_index_`, stored in a *separate buffer*.

**Per-edge work in the Dijkstra inner loop** (`dijkstra_shortest_paths.hpp:460`):
```cpp
for (auto&& [vid, uv] : views::incidence(g, u)) {
  const auto w = weight_fn(g, uv);   // → edge_value(g, uv)
  // → g.edge_value(uv.value() - g.col_index_.begin())
  // → edge_value_[k]
  ...
  relax_target(uv, uid);             // reads target_id(g, uv) = uv.value()->index → col_index_[k].index
}
```

So the inner loop touches two parallel arrays per edge:

| Load | Source | Cache-line cost (typical) |
|---|---|---|
| `vid` (target id, u32) | `col_index_[k].index`     | 1 line per 16 edges |
| `w`   (weight, f64)    | `edge_value_[k]`          | 1 line per 8 edges  |
| `distance[vid]` (f64)  | distance buffer (random)  | 1 line per visit (random access) |

BGL's `compressed_sparse_row_graph` with a bundled property and
`get(&prop::weight, g)` typically resolves to a raw `Weight*` of length
`num_edges()`. The data layout is therefore comparable — *but* BGL's
adjacency arrays are also stored in distinct buffers. The dense-graph win
suggests the per-edge work, not the layout, is what differs.

Other suspects to investigate, ordered by prior probability:

1. **Iterator descriptor materialisation.** `views::incidence` yields a
   structured `[vid, uv]` pair where `uv` is a full `edge_descriptor`
   carrying the source vertex (for `target_id` symmetry), an iterator into
   `col_index_`, possibly a graph back-reference. The relax loop only needs
   `target_id` (already in `vid`) and `edge_value`; `uv` is also re-passed to
   `weight_fn`, where `edge_value(g, uv)` recomputes
   `uv.value() - g.col_index_.begin()` — a pointer subtraction the iterator
   itself already has implicit (it *is* the iterator).
2. **Redundant pointer subtraction per edge.** `edge_value` resolves the
   edge index via `uv.value() - g.col_index_.begin()`. In a tight loop this
   is one extra subtraction per edge that BGL's `weight_map[edge_descriptor]`
   may avoid (BGL's CSR `edge_descriptor` carries the index directly).
3. **`basic_incidence` not used.** `views::incidence` builds a full edge
   descriptor; `views::basic_incidence` (also documented in
   `views/incidence.hpp`) yields just `[tid]` and is documented as "lighter
   still: never materialises an edge descriptor". The Dijkstra relax loop
   could use a CSR-aware fast path that yields `(tid, edge_index)` and reads
   `edge_value_[edge_index]` directly — but that breaks the visitor
   contract (`on_examine_edge`, `on_edge_relaxed`, `on_edge_not_relaxed`
   take `const edge_t<G>&`).
4. **No prefetching.** BGL doesn't prefetch either, so this is not the gap
   on its own — but if (1)/(2) are the cause, prefetching `edge_value_[k+P]`
   at iterator increment is a free additional win.
5. **`auto&&` destructuring vs raw indexed loop.** `for (auto&& [vid, uv] :
   views::incidence(g, u))` involves a range adapter, an iterator type, and
   structured binding. GCC usually collapses this; verifying it does
   (vs BGL's raw `csr_edge_iterator` over a `pair<id, id>`) takes 5 minutes
   with `objdump`.
6. **Cache-line alignment.** `col_index_` and `edge_value_` are
   `std::vector`s; both start at 64-byte-aligned addresses by default
   (`std::allocator<T>`). Unlikely to be the issue but cheap to confirm.

---

## Investigation phases

### Phase 1 — Reproduce and quantify (no code changes)

Goal: confirm the gap is reproducible at smaller `n`, isolate the loop.

| Item | Detail |
|------|--------|
| **1.1 Re-run baseline** | `benchmark_dijkstra` Idx4 vs BGL CSR at n = 10K, 30K, 100K, 300K for ER, BA, Grid, Path (smaller sizes fit in L2; larger expose memory subsystem). 5 runs each, drop high/low, report median. |
| **1.2 Hardware counters** | `perf stat -e cycles,instructions,L1-dcache-load-misses,LLC-load-misses,branch-misses,branch-instructions ./benchmark_dijkstra --benchmark_filter=...` for `BM_Dijkstra_CSR_Grid_Idx4/100000` and `BM_Dijkstra_BGL_CSR_Grid/100000`. Tabulate IPC, cycles/edge, loads/edge, miss rates. |
| **1.3 perf record + annotate** | Single-run profile with `perf record -F 4000 --call-graph=lbr` of each, `perf annotate` on the inlined relax loop. Compare instruction mix and identify the cycle-eating instructions. |
| **1.4 Verdict** | If counters show graph-v3 has materially more **instructions/edge** with similar miss rates → suspect 1, 2, 5 (work, not memory). If similar instructions/edge but more **L1/LLC misses/edge** → suspect layout / prefetch. |

Output: a small results table appended to `indexed_dary_heap_results.md` §
"Phase 4.3b — CSR access-path profiling".

### Phase 2 — Disassembly comparison (no code changes)

Goal: verify the inlining hypothesis from Open Q3 still holds for the
*entire* relax body, not just the heap, and quantify per-edge instruction
count.

| Item | Detail |
|------|--------|
| **2.1 Locate relax loop** | `nm --demangle benchmark_dijkstra` → find the `dijkstra_shortest_paths<...CSR..., Idx4>::run::operator()<...>` constprop symbol. `objdump -d --no-show-raw-insn` over its byte range. Identify the `for (auto&&[vid, uv] : views::incidence(g, u))` body by structure (back-edge into the inner loop, distance load, ucomisd, conditional jump). |
| **2.2 Per-edge instruction count** | Count `mov`/`add`/`sub`/`cmp`/`j*` between the inner-loop top and back-edge. Repeat for BGL's compiled `csr_*` Dijkstra. Diff. |
| **2.3 Check for redundant work** | Specifically look for: (a) two separate `(%base,%idx,8)` loads from distinct base registers (= `col_index_[k].index` and `edge_value_[k]`); (b) `lea`/`sub` sequences computing `uv.value() - col_index_.begin()` per edge; (c) any `call` instructions (should be zero per Q3). |
| **2.4 Verdict** | Concrete count of "extra instructions per edge in graph-v3 vs BGL". |

### Phase 3 — Microbenchmark the descriptor cost

Goal: isolate whether the `edge_descriptor` materialisation in
`views::incidence` is the cost, vs the `edge_value_` load itself.

| Item | Detail |
|------|--------|
| **3.1 Hand-rolled raw loop benchmark** | New benchmark `BM_Dijkstra_CSR_*_Raw` that bypasses `views::incidence` and reads `g.col_index_[k].index` and `g.edge_value_[k]` directly via the CSR row range, but otherwise uses the same `dijkstra_shortest_paths` heap+visitor scaffolding. (Probably needs a small Dijkstra variant in the benchmark file, not in the public algorithm.) |
| **3.2 Compare** | Raw vs Idx4 vs BGL. If Raw closes most of the gap → confirms (1)/(2)/(5). If Raw still trails BGL → the gap is in the heap+distance-buffer access, not the edge access. |
| **3.3 Verdict** | Quantifies how much the descriptor abstraction costs in % terms. |

### Phase 4 — Decide on a fix

Driven entirely by Phase 1–3 findings. Candidate interventions, *in order
of preference*:

1. **`edge_value` overload that takes the edge offset directly.** If `uv`
   already carries the offset (or could be made to), eliminate the
   `uv.value() - col_index_.begin()` subtraction. Possibly a `compressed_graph`-
   specific friend overload of `edge_value(g, uv)` that uses an internal
   stored offset. Zero ABI impact on other graph types.
2. **`incidence` fast-path on `compressed_graph`.** A specialisation that
   precomputes `(tid, edge_index)` per step and caches the offset, so
   downstream `edge_value(g, uv)` is a single indexed load. Has to preserve
   the `edge_t<G>` exposed to visitors, so the descriptor is still
   constructible on demand.
3. **Algorithm-internal raw path on `compressed_graph`.** A Dijkstra
   `if constexpr` branch for `is_compressed_graph<G>` that walks `row_index_`
   / `col_index_` / `edge_value_` directly. Largest perf win, biggest
   maintenance cost (a second algorithm body), and skips the visitor edge
   events. Only justified if (1) and (2) are insufficient.
4. **Software prefetch.** `__builtin_prefetch(&col_index_[k+P])` and
   `&edge_value_[k+P]` inside the per-edge loop. Free perf if the bottleneck
   is memory not work; harmful if the bottleneck is work.
5. **Layout change** (interleave target id and weight in one struct of
   size `sizeof(VId) + sizeof(EV)`). Big change for uncertain win — defer.

Each candidate gets its own commit and benchmark delta, judged on the same
ER/BA/Grid/Path/100K table.

### Phase 5 — Document and decide default

| Item | Detail |
|------|--------|
| **5.1 Update results doc** | Final numbers go in `indexed_dary_heap_results.md` § "Phase 4.3b". |
| **5.2 Update plan doc** | Resolve Open Q6's "out of scope" caveat. |
| **5.3 Decide default heap** | If the fix shifts CSR + default-heap below CSR + Idx8 on dense graphs, revisit the Phase 4.2 default-heap recommendation. |

---

## Acceptance criteria

- A single-page §4.3b in `indexed_dary_heap_results.md` with the n=100K table
  rerun after each intervention.
- A clear `perf stat` / `objdump` artifact identifying the *instruction-level*
  cause of the gap (not just "the loop is slower").
- A go/no-go decision on each of the 5 candidate interventions.
- All 4848 ctest tests still pass after any landed change.

## Out of scope

- Changing public algorithm signatures.
- Changing `compressed_graph`'s public storage layout (`row_index_`,
  `col_index_`, `edge_value_` member access stays as-is for users with
  external code that touches them).
- Heap changes (settled in Phase 4.3a).
- Prim — already inherits any Dijkstra perf win via the Phase 5
  Option 1 wrapper.

## Risk

- The investigation may show the gap is in `std::vector<bool>`-style
  layout work that we *can't* close without a new container, in which case
  the correct outcome is documenting the residual gap and stopping.
- Prefetch tuning is fragile and machine-specific; if it lands it must be
  benchmarked on at least two different µarch generations before being
  enabled by default.
