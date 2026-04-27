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

### Side observation — MSVC vs GCC, same machine (2026-04-26)

`agents/indexed_dary_heap_baseline_msvc.md` captures the same `benchmark_dijkstra`
suite under MSVC 19.50 (Visual Studio 18.5.1, x64 Release) on the same Titania
host. Most CSR rows agree with the GCC numbers within ±10 %, with one striking
exception:

| Topology @ 100K | Heap | GCC ns | MSVC ns | MSVC ÷ GCC |
|---|---|---:|---:|---:|
| Path | Default | 268,708 | 1,331,743 | **4.96×** |
| Path | Idx4 | 326,018 | 498,438 | 1.53× |
| Path | Idx8 | 327,820 | 491,302 | 1.50× |

MSVC's `std::priority_queue` codegen is **~5×** slower than libstdc++'s on the
Path workload (no decrease-key, single-source linear chain). Switching to the
indexed heap collapses the toolchain gap to ~1.5×, so the slowdown is in
MSVC's heap implementation, not in graph-v3's CPO/visitor scaffolding.

This is **not** the gap this plan is trying to close (we're chasing the
graph-v3 vs BGL CSR gap on a single toolchain), but it is worth flagging:
any VTune profile run under this plan must be compared against MSVC numbers
for the same toolchain — never cross-compared against the Linux/GCC baseline
in `indexed_dary_heap_baseline.md`. The MSVC baseline is the anchor for
Phase 1 (Windows) below.

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

---

## Phase 1.1 — Reproduce on Windows MSVC (`windows-msvc-profile`, 2026-04-27)

**Build:** `windows-msvc-profile` preset (`/O2 /Ob3 /Zi /DNDEBUG`, `/DEBUG`
linker, `DIJKSTRA_BENCH_BGL=ON`, BGL at `D:/dev_graph/boost`).
**Methodology:** core 0 pinning, priority `High`, 5 reps, median, 2 s min
benchmark time. Same machine (Titania) as the Linux baseline.

### Results — graph-v3 Idx4 vs BGL CSR @ n = 100K

| Topology   | graph-v3 Idx4 (ns) | BGL CSR (ns) | graph-v3 vs BGL |
|------------|-------------------:|-------------:|----------------:|
| ER Sparse  |        20,147,385  |  32,849,012  | **−38.7 %** ✅   |
| Grid       |         7,203,305  |  10,927,450  | **−34.1 %** ✅   |
| BA         |        20,446,214  |  32,326,378  | **−36.7 %** ✅   |
| Path       |           394,793  |   1,101,341  | **−64.1 %** ✅   |

CV ≤ 5 % on every row except Path/Idx4 (10.3 % — single noisy run; absolute
delta is well outside any plausible CV band).

### Comparison with the original Phase 4.3a baseline (Linux GCC, 2025)

| Topology   | Phase 4.3a Idx4 vs BGL (Linux GCC) | Phase 1.1 Idx4 vs BGL (Windows MSVC, today) |
|------------|-----------------------------------:|--------------------------------------------:|
| ER Sparse  | **+7.7 %** (graph-v3 slower)       | **−38.7 %** (graph-v3 faster)               |
| Grid       | **+36.5 %** (graph-v3 slower)      | **−34.1 %** (graph-v3 faster)               |
| BA         | **+9.4 %** (graph-v3 slower)       | **−36.7 %** (graph-v3 faster)               |
| Path       | **+15.0 %** (graph-v3 slower)      | **−64.1 %** (graph-v3 faster)               |

### Interpretation

The motivating gap (graph-v3 7–37 % slower than BGL on Linux GCC) does **not
reproduce on Windows MSVC**: under MSVC `/O2 /Ob3` graph-v3 is 34–64 %
*faster* than BGL on every topology. Two non-exclusive explanations:

1. **Toolchain-dependent codegen.** GCC may inline BGL's `get(weight, g)`
   property-map machinery (heavy template specialization on tag dispatch)
   more aggressively than MSVC, while MSVC at `/Ob3` collapses graph-v3's
   `views::incidence` + `edge_value(g, uv)` chain — the exact path Phase
   4.3e proved is now fully inlined under MSVC profile flags.
2. **Code drift since 4.3a.** The `indexed-dary-heap` branch contains
   significant work to the access path since 4.3a was captured:
   - `5085c60` Edge desc (#23)
   - `7645a19` Simplify traversal_common.hpp by unifying property function concepts (#22)
   - `1c871a8` Phase 2: Add basic_incidence; refactor incidence uid overloads
   - `aa95fe0` feat: add target_id to incidence_view return type
   These specifically reduce the cost of the `views::incidence` +
   `edge_value` chain that the perf plan identified as the suspect.

### Decision

The plan's premise (graph-v3 slower than BGL on CSR) is **not currently
reproducible on this toolchain**. Three branches of follow-up work, in
priority order:

| Priority | Action | Rationale |
|----------|--------|-----------|
| **High** | Re-run Phase 4.3a / Phase 1.1 under Linux GCC on the same host | The original gap was a Linux-GCC-only phenomenon. Confirm whether the recent code drift (5085c60, 7645a19, 1c871a8, aa95fe0) closed it under GCC too. If yes → plan complete. If no → original investigation (Phases 1.2–4) still has work. |
| Medium | Cross-compare BGL itself: GCC vs MSVC on the same machine | If BGL gets dramatically faster under GCC than MSVC (and graph-v3 is roughly toolchain-neutral), the "gap" was always a BGL property-map advantage on GCC, not a graph-v3 deficit. |
| **Now** | Phase 2 disassembly on MSVC (next section) | Even though the gap is reversed, the plan's original Phase 2 instrumentation still tells us *why* graph-v3 wins on MSVC. Cheap with the profile preset's PDB. |

---

## Phase 2 — MSVC disassembly of `sift_down_` and the relax loop (2026-04-27)

**Tooling:** `scripts/perf/disasm_func.py` (new this session) targets a single
function by demangled-name substring instead of dumping the full 14k+ entries
of the exe.

### VTune anchor on the profile preset

```
heap::sift_down_                          34.9 %
less::operator()                           8.7 %     (1st copy)
cfn::operator()                            6.7 %
incidence_view::iterator::operator*        5.9 %
vector<double>::operator[]                 4.8 %
less::operator()                           4.1 %     (2nd copy)
dijkstra ... <lambda_1>::operator()        4.0 %
cfn::operator()                            2.3 %     (2nd copy)
heap::sift_up_                             1.7 %
```

Symbol attribution differs from Phase 4.3e (where 98.8 % collapsed into one
anonymous frame): `/Zi` keeps function boundaries visible to the linker even
when the bodies are inlined, so VTune can attribute samples to source-line
owners. The 98.8 % number was a **symbol-stripping artefact**, not an actual
codegen difference. Codegen at `/O2 /Ob3` and `/O2 /Ob3 /Zi` are the same;
only attribution differs.

### `sift_down_` (Idx4) inner child-scan loop

```
artifacts/perf/sift_down_idx4.asm  (Idx4, RVA 0x14006bbb0)

LOOP_BODY (one comparison per child, 4 unrolled per outer step):
  mov   eax, [r11 + r8*4]        ; load best-so-far child key
  mov   ecx, [r11 + r9*4]        ; load other child key
  movsd xmm0, [r10 + rax*8]      ; load best distance
  comisd xmm0, [r10 + rcx*8]     ; compare against other distance
  cmova  r8, r9                  ; if a < best → r8 := r9
```

**Per-comparison cost: 5 instructions, 2 indexed loads, 1 `comisd`, 1
conditional move.** No call instructions, no template scaffolding, no
pointer subtractions, no `std::less`/`container_value_fn` thunks visible in
the body — they have all been collapsed by `/Ob3`. This is the textbook
shape Open Question 3 hypothesised would happen; on MSVC it required
`/Ob3` to materialise (Phase 4.3d/e showed `/Ob2` was insufficient).

The outer loop unrolls 4 children per iteration (Arity = 4) using the same
5-instruction template, then falls into a 1-child remainder loop
(`0x14006bcba`–`0x14006bcd8`, identical shape). Loop-carried dependencies
are limited to `r8` (best-index) and the loop counter `r9`.

### What this tells us about the BGL "gap"

The Phase 1.1 numbers showed graph-v3 −34 % to −64 % vs BGL on every
topology under MSVC. The disassembly confirms this is **real codegen**, not
a measurement artefact:

- `sift_down_` is genuinely tight (5 insn / comparison, fully inlined
  comparator).
- The relax-loop attribution (`incidence_view::iterator::operator*`,
  `vector<double>::operator[]`, the dijkstra lambda) totals ~15 % — a
  reasonable fraction for the per-edge work.

The remaining MSVC investigation work would be confirming that BGL's
`get(weight, g)` compiles to a comparable shape on MSVC (it likely *doesn't*,
which would explain the 35–65 % graph-v3 win). That is parked pending the
Linux GCC rerun (the only place the original gap lived).

### Acceptance for Thread B (MSVC scope)

- ✅ Phase 1.1 reruns the BGL comparison; the gap inverted to a graph-v3
      win of 34–64 %.
- ✅ Phase 2 disassembly proves the win is real codegen and identifies the
      exact instruction shape.
- ⏸ Phase 3 (raw-loop microbenchmark) — **deferred**: there is no gap to
      explain on MSVC, so a "what fraction of the gap is descriptor cost"
      experiment has nothing to measure.
- ⏸ Phases 4–5 (interventions, default-heap revisit) — **deferred** until
      Linux GCC reproduces or refutes the original gap.

### Files captured this phase

```
artifacts/perf/hot_001.csv               VTune CSV export (profile build)
artifacts/perf/sift_down_first.asm       Idx2 sift_down_ (RVA 0x14006b9b0)
artifacts/perf/sift_down_idx4.asm        Idx4 sift_down_ (RVA 0x14006bbb0)
```



