# Indexed d-ary Heap for Dijkstra & Prim — Plan

This plan introduces a true decrease-key priority queue to replace the
`std::priority_queue` lazy-deletion pattern currently used by Dijkstra
(and likely useful for Prim's MST). The goal is to remove stale-pop
overhead, reduce heap memory from O(E) to O(V), and bring visitor
semantics in line with BGL.

**Branch:** `indexed-dary-heap`

**Invariant:** After every phase, `ctest` passes all existing tests. No
phase may break the public API of `dijkstra_shortest_paths`,
`dijkstra_shortest_distances`, or any algorithm that already uses
`std::priority_queue` internally.

---

## Conventions

| Symbol | Meaning |
|--------|---------|
| **File** | Absolute path relative to repo root |
| **Read** | Files the agent must read for context before editing |
| **Create** | New files to create |
| **Modify** | Existing files to edit |
| **Verify** | Commands to run and expected outcomes |
| **Commit** | Git commit message (conventional-commit style) |

---

## Background

### Current state

`dijkstra_shortest_paths` uses `std::priority_queue<weighted_vertex>`
with re-insertion when a vertex's distance improves. The recently
added stale-pop skip:

```cpp
if (compare(distance(g, uid), w)) continue;
```

makes this correct and gives single-shot visitor semantics, but the
heap can hold up to O(E) entries and every relaxed edge causes a
push.

### Target state

A min-heap that:

- Stores at most one entry per vertex (size ≤ V).
- Supports `push`, `top`, `pop`, `decrease(vid)`, `contains(vid)`.
- Looks up a vertex's current distance via the user-supplied
  `DistanceFn` (so heap order tracks live distance).
- Is parameterized on arity `d` (default `d = 4`, matching Boost's
  `d_ary_heap_indirect`).
- Uses an external position map (`vertex_id -> heap_index`) so that
  `decrease` is O(log_d V).

### Performance hypothesis

| Workload | Expected change vs. current |
|----------|-----------------------------|
| Sparse graph, few re-relaxations | Small win (push count drops, log V vs log E) |
| Dense graph, many re-relaxations | Large win (heap size O(V) vs O(E)) |
| Mapped (associative) vertex containers | Win depends on position-map cost |

Hypothesis must be confirmed by benchmarks (Phase 4) before declaring
the new heap the default.

---

## Phase 0 — Preparation (no code changes)

### 0.1 Verify Baseline

| Item | Detail |
|------|--------|
| **Action** | Confirm the full test suite is green on the branch base. |
| **Verify** | `cd build/linux-gcc-debug && ctest --output-on-failure` — all tests pass |

### 0.2 Capture Baseline Benchmarks

| Item | Detail |
|------|--------|
| **Action** | Record current Dijkstra benchmark numbers. |
| **Read** | `benchmark/algorithms/` for existing Dijkstra benchmarks |
| **Verify** | Save numbers to `agents/indexed_dary_heap_baseline.md` (gitignored or committed as reference). If no Dijkstra benchmark exists, create one in 0.3. |

### 0.3 Add Dijkstra Benchmark (if missing)

| Item | Detail |
|------|--------|
| **Action** | Ensure a Google Benchmark target exercises Dijkstra over (a) sparse random graph, (b) dense random graph, (c) grid graph, each at multiple V sizes. |
| **Create** | `benchmark/algorithms/benchmark_dijkstra.cpp` if not present |
| **Verify** | Benchmark builds and produces stable numbers across runs (CV < 5%). |

---

## Phase 1 — Indexed d-ary Heap Container

### 1.1 Design Header

| Item | Detail |
|------|--------|
| **Read** | `boost/libs/graph/include/boost/graph/detail/d_ary_heap.hpp` for reference |
| **Create** | `include/graph/detail/indexed_dary_heap.hpp` |

Sketch of the public interface:

```cpp
namespace graph::detail {

// External-key, indirect-comparison d-ary heap.
//
// Key      : the user's vertex id type (must be usable as an index/lookup key)
// DistanceFn: callable (key) -> Distance&  (or const Distance&)
// Compare  : strict weak order over Distance values (min-heap if less<>)
// PositionMap: random-access mapping key -> size_t (heap position) or NPOS
// Arity    : children per node (default 4)
template <
    class Key,
    class DistanceFn,
    class Compare,
    class PositionMap,
    std::size_t Arity = 4,
    class Allocator = std::allocator<Key>>
class indexed_dary_heap {
public:
    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    indexed_dary_heap(DistanceFn d, Compare c, PositionMap p, const Allocator& = {});

    bool   empty() const noexcept;
    size_t size()  const noexcept;

    void   push(Key k);              // O(log_d N)
    Key    top()   const;            // O(1)
    void   pop();                    // O(d log_d N)
    void   decrease(Key k);          // O(log_d N) — distance must already be lower
    bool   contains(Key k) const;    // O(1)
    void   clear();

private:
    std::vector<Key, Allocator> heap_;
    DistanceFn   distance_;
    Compare      compare_;
    PositionMap  position_;          // heap stores positions back into here on every move

    void sift_up_(size_t i);
    void sift_down_(size_t i);
    void place_(size_t i, Key k);    // writes heap_[i] = k AND position_[k] = i
};

} // namespace graph::detail
```

Notes:
- `PositionMap` is a *concept-style* requirement: `size_t& operator()(Key)` or
  similar. For index-based graphs, it can wrap a `std::vector<size_t>`. For
  mapped graphs, it can wrap an `std::unordered_map`. Decision deferred to
  1.3.
- `DistanceFn` is the *same* function the user passes to
  `dijkstra_shortest_paths`. The heap reads, never writes.
- Comparator is `Compare`, applied to *distances* (not keys). Internally:
  `compare_(distance_(a), distance_(b))`.

### 1.2 Implement Core Operations

| Item | Detail |
|------|--------|
| **Action** | Implement `push`, `pop`, `sift_up_`, `sift_down_`, `decrease`, `contains`, `clear`. Keep `place_` as the single point where positions are written, to avoid bookkeeping bugs. |
| **Verify** | Unit-tests in 1.4 pass. |

Key correctness rules:
- Every assignment to `heap_[i]` must go through `place_` so `position_` stays in sync.
- `decrease(k)` reads `position_(k)` then sifts up only — caller guarantees the new distance is no worse.
- `pop()` swaps last → root, marks the popped key's position as `npos`, then sifts down.

### 1.3 Position Map Adapter

| Item | Detail |
|------|--------|
| **Create** | `include/graph/detail/heap_position_map.hpp` |
| **Action** | Provide two adapters: <br> 1. `vector_position_map` — wraps a `std::vector<size_t>` indexed by integral key. <br> 2. `assoc_position_map` — wraps `std::unordered_map<Key, size_t>` for non-integral keys. <br> Both default-construct to `npos` semantics. |
| **Verify** | Adapters compile with the heap. Covered by tests in 1.4. |

### 1.4 Unit Tests

| Item | Detail |
|------|--------|
| **Create** | `tests/common/test_indexed_dary_heap.cpp` |
| **Action** | Cover: empty heap, single element, ascending/descending pushes, mixed push+pop, repeated `decrease`, `contains` before/after push/pop, both arity 2 and 4, custom comparator (max-heap), both position-map adapters. |
| **Verify** | `ctest -R indexed_dary_heap` — all pass. |
| **Commit** | `feat(detail): indexed d-ary heap with external position map` |

---

## Phase 2 — Integrate into Dijkstra (opt-in)

### 2.1 Add Heap-Selector Tag (or Template Parameter)

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/dijkstra_shortest_paths.hpp` |
| **Modify** | Add an optional template parameter `Heap = use_default_heap` (a tag). When `use_default_heap`, behavior is unchanged. When `use_indexed_dary_heap<Arity>`, the new heap is used. |
| **Verify** | Existing tests still pass (default path unchanged). |
| **Commit** | `feat(dijkstra): add heap-selector template parameter` |

Rationale: keeps the change additive and reversible. We can flip the
default in a later phase once benchmarks confirm parity or improvement.

### 2.2 Implementation Branch

| Item | Detail |
|------|--------|
| **Modify** | Inside `dijkstra_shortest_paths`, dispatch to one of two inner implementations based on the `Heap` tag. Share the visitor / relax / source-seeding code via a small helper. |
| **Action** | The indexed-heap implementation: <br> - Removes the stale-pop skip (no stale entries possible). <br> - Replaces re-push with `decrease` on the relax path. <br> - Removes `weighted_vertex` (heap stores ids only; distance is read live via `DistanceFn`). |
| **Verify** | All existing Dijkstra tests pass under both code paths. Add a test variant that exercises each test with the indexed heap. |
| **Commit** | `feat(dijkstra): indexed d-ary heap implementation path` |

### 2.3 Visitor Semantics Audit

| Item | Detail |
|------|--------|
| **Action** | Confirm `on_examine_vertex` and `on_finish_vertex` fire exactly once per reachable vertex on the indexed-heap path. Confirm `on_edge_relaxed` and `on_edge_not_relaxed` counts match Boost's behavior. |
| **Verify** | Add a counting-visitor test that asserts call counts on a reference graph with both heap paths. |
| **Commit** | `test(dijkstra): visitor call-count parity across heap paths` |

---

## Phase 3 — Mapped-Container Support

### 3.1 Position Map for Mapped Graphs

| Item | Detail |
|------|--------|
| **Read** | `agents/map_container_strategy.md`, `agents/map_container_plan.md` |
| **Action** | Wire the `assoc_position_map` adapter into the indexed-heap dispatch when `vertex_id_t<G>` is non-integral or the graph is a mapped container. Decision criterion to be documented. |
| **Verify** | Run the Dijkstra test suite against mapped graph types with the indexed heap. |
| **Commit** | `feat(dijkstra): indexed-heap support for mapped containers` |

### 3.2 Vertex-Property-Map Position Storage (optional)

| Item | Detail |
|------|--------|
| **Action** | Investigate whether the position map can live inside the graph as a vertex property map (matching Boost's `vertex_property_map_generator`). Spike only — implement only if it removes a meaningful allocation on hot paths. |
| **Verify** | Benchmark before/after on mapped graphs. |
| **Commit** | `feat(dijkstra): in-graph position map for mapped containers` (only if accepted) |

---

## Phase 4 — Benchmarks & Default Selection

### 4.1 Comparative Benchmarks

| Item | Detail |
|------|--------|
| **Action** | Run the Phase 0.3 benchmarks against (a) `priority_queue` path, (b) `indexed_dary_heap<2>`, (c) `indexed_dary_heap<4>`, (d) `indexed_dary_heap<8>`. Record results in `agents/indexed_dary_heap_results.md`. |
| **Verify** | Numbers stable across at least 3 runs. |

### 4.2 Decide Default

| Item | Detail |
|------|--------|
| **Action** | Based on results: <br> - If indexed `d=4` wins or ties on every workload, make it the default. <br> - If it loses on sparse small graphs, keep `priority_queue` default and document the selector. <br> - If results are mixed, consider a heuristic dispatch (e.g., based on E/V ratio) — but only with strong evidence. |
| **Modify** | Default heap parameter, plus a CHANGELOG entry. |
| **Verify** | Full test suite still green. Benchmarks regenerated. |
| **Commit** | `perf(dijkstra): switch default heap to indexed d-ary` (or document why not) |

---

## Phase 5 — Reuse for Prim's MST (optional follow-up)

### 5.1 Audit Prim's Implementation

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/mst.hpp` (or wherever Prim lives) |
| **Action** | Identify whether Prim has the same lazy-deletion pattern. If yes, plan a parallel migration. |
| **Verify** | N/A (planning only). |

### 5.2 Apply Indexed Heap to Prim

| Item | Detail |
|------|--------|
| **Action** | Mirror Phase 2 for Prim: opt-in selector → integrate → benchmark → switch default. |
| **Verify** | MST test suite green. |
| **Commit** | `perf(mst): indexed d-ary heap path for Prim` |

---

## Open Questions

1. **PositionMap ownership.** Owned by the heap (simplest, allocates per call), or
   passed in (zero-allocation for repeated calls, more API surface)? Default to
   owned-by-heap for the first cut.
2. **Arity as runtime vs compile-time.** Compile-time only — runtime would lose
   the constexpr unrolling that justifies d-ary heaps in the first place.
3. **`Compare` indirection cost.** The heap calls `compare_(distance_(a), distance_(b))`
   twice per sift-down step (one comparator call per child + one against the parent).
   For trivial `DistanceFn` (vector lookup) this should inline; verify in benchmarks.
4. **Visitor `on_examine_vertex` semantics on multi-source seeding.** The current
   multi-source code seeds N vertices into the queue. With the indexed heap, the
   first pop of each source is the settled pop (no re-pushes possible since
   distance is already 0). Confirm visitor semantics are unchanged.
5. **Should the new heap live in `graph/detail/` or be promoted to `graph/container/`?**
   Defer the decision — start in `detail/` and promote only if external code finds it
   useful.

---

## Out of Scope

- Fibonacci heap, pairing heap, or radix heap implementations.
- Replacing other algorithms' priority queues (BFS variants, A*, etc.).
- Changing public algorithm signatures beyond adding the optional `Heap` template
  parameter.
- Parallel / concurrent heap variants.
