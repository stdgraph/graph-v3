# Implementation Plan: Pass `vertex_id_t<G>` by `const&`

Implements [vertex_id_const_ref_strategy.md](vertex_id_const_ref_strategy.md).

## Safety Principles

Every step in this plan follows these safety rules:

0. **Work on a feature branch.** All changes are made on a dedicated branch
   `feature/vertex-id-const-ref` created from the current `main` (or primary development branch).
   Each phase gate is a natural commit point. The branch is merged only after final validation.

1. **One logical change per step.** Each step modifies one category of files (e.g., one header,
   or one group of tightly-coupled headers). Never mix changes across unrelated subsystems.

2. **Build gate after every step.** No step is considered complete until it compiles cleanly on
   at least `linux-gcc-debug` and `linux-clang-debug`. Steps involving reference semantics also
   require `linux-gcc-asan`.

3. **Test gate after every step.** Run the relevant targeted test suite after each step, plus a
   full test suite run at the end of each phase.

4. **No skipping ahead.** Each phase depends on the previous phase being fully green. Phase 1
   depends on Phase 0. Phase 2 depends on Phase 1. Etc.

5. **Revert on red.** If a step breaks the build or tests, revert it entirely before diagnosing.
   Do not attempt fixes on top of a broken state.

6. **Static assertions as guardrails.** Add `static_assert` checks at key points to verify type
   assumptions at compile time, especially for `vertex_id_store_t<G>` resolution.

7. **Preserve existing `using` aliases.** When changing algorithm internals, keep
   `using id_type = ...` on its own line so the diff is minimal and reviewable.

---

## Build & Test Commands

```bash
# Create and switch to feature branch (do this once, before any changes):
git checkout -b feature/vertex-id-const-ref

# Targeted build (one preset):
cmake --build build/linux-gcc-debug -j $(nproc)

# Targeted test (one preset):
ctest --test-dir build/linux-gcc-debug --output-on-failure

# ASan build + test:
cmake --build build/linux-gcc-asan -j $(nproc)
ctest --test-dir build/linux-gcc-asan --output-on-failure

# Full matrix (all presets):
./scripts/test-all.sh
```

---

## Phase 0: Type Alias Foundation

**Goal:** Add `raw_vertex_id_t<G>` and `vertex_id_store_t<G>` aliases; protect `vertex_id_t<G>`
with `remove_cvref_t`. These are purely additive — no existing code changes behavior.

### Step 0.1: Protect `vertex_id_t<G>` in `graph_cpo.hpp`

**File:** `include/graph/adj_list/detail/graph_cpo.hpp`

**Change:** Wrap the `vertex_id_t<G>` definition in `std::remove_cvref_t<>`.

```cpp
// Before:
template <typename G>
using vertex_id_t = decltype(vertex_id(std::declval<G&>(), std::declval<vertex_t<G>>()));

// After:
template <typename G>
using vertex_id_t = std::remove_cvref_t<
    decltype(vertex_id(std::declval<G&>(), std::declval<vertex_t<G>>()))>;
```

**Add** the `raw_vertex_id_t<G>` alias immediately below:
```cpp
template <typename G>
using raw_vertex_id_t = decltype(vertex_id(std::declval<G&>(), std::declval<vertex_t<G>>()));
```

**Validation:**
- Build: `linux-gcc-debug`, `linux-clang-debug`
- Test: `ctest --test-dir build/linux-gcc-debug --output-on-failure`
- Verify: `remove_cvref_t` on a value type is a no-op — all existing tests must pass unchanged.

### Step 0.2: Protect `vertex_id_t<G>` in `edge_list.hpp`

**File:** `include/graph/edge_list/edge_list.hpp`

**Change:** Apply the same `std::remove_cvref_t<>` wrapping to the edge_list `vertex_id_t`
definition, if not already wrapped.

**Validation:** Build + test `linux-gcc-debug`.

### Step 0.3: Add `vertex_id_store_t<G>` alias

**File:** `include/graph/algorithm/traversal_common.hpp` (already included by all algorithms)

**Change:** Add the `vertex_id_store_t<G>` alias, gated on `<functional>` and `<type_traits>`:
```cpp
/// Efficient storage type for vertex IDs in algorithm-internal containers.
/// For integral IDs: same as vertex_id_t<G> (zero overhead).
/// For non-trivial IDs (e.g., string from map-based graphs): reference_wrapper
/// to the stable key in the graph's map node (8 bytes, trivially copyable).
/// Requires: the graph must not be mutated while values of this type are alive.
template <typename G>
using vertex_id_store_t = std::conditional_t<
    std::is_reference_v<adj_list::raw_vertex_id_t<G>>,
    std::reference_wrapper<std::remove_reference_t<adj_list::raw_vertex_id_t<G>>>,
    adj_list::vertex_id_t<G>>;
```

**Validation:** Build + full test suite on `linux-gcc-debug`.

### Step 0.4: Phase 0 gate — full matrix build and commit

**Commands:**
```bash
./scripts/test-all.sh
git add -A && git commit -m "Phase 0: Add raw_vertex_id_t, vertex_id_store_t; protect vertex_id_t with remove_cvref_t"
```

**Exit criterion:** All four presets build and test green. No behavioral changes — this phase
is purely additive type aliases.

---

## Phase 1: Descriptor Return Types

**Goal:** Change `vertex_descriptor::vertex_id()`, `edge_descriptor::source_id()`, and
`edge_descriptor::target_id()` from `auto` (by value) to `decltype(auto)` so that map-based
graphs return a `const&` to the stable key rather than copying it. This is the change that
makes `raw_vertex_id_t<G>` return a reference for map-based graphs.

**Risk level: MEDIUM.** Returning a reference from a descriptor method can cause dangling
references if the descriptor is a temporary. Every call site must be audited.

### Step 1.1: Audit temporary descriptor usage

**Action (read-only — no code changes):** Search for all call sites where `vertex_id()`,
`source_id()`, or `target_id()` are called on a temporary descriptor. Patterns to search:

```
vertex_id()     — called on vertex_descriptor returned by *find_vertex(g, uid)
source_id()     — called on edge_descriptor values
target_id()     — called on edge_descriptor values
```

Verify that in every case, the descriptor is either:
- Bound to a named variable (safe — reference extends lifetime), or
- The result is immediately copied into a `vertex_id_t<G>` local (safe — copy), or
- Used in an expression where the descriptor's lifetime spans the use (safe).

Document any unsafe patterns found. They must be fixed before proceeding.

**Validation:** No build needed — this is a review step.

### Step 1.2: Change `vertex_descriptor::vertex_id()` to `decltype(auto)`

**File:** `include/graph/adj_list/vertex_descriptor.hpp`

**Change:** `auto` → `decltype(auto)`. Parenthesize the keyed-storage return:
```cpp
[[nodiscard]] constexpr decltype(auto) vertex_id() const noexcept {
    if constexpr (std::random_access_iterator<VertexIter>) {
        return storage_;                       // prvalue (integral), unchanged
    } else {
        return (std::get<0>(*storage_));       // lvalue ref to map key
    }
}
```

**Validation:**
- Build: `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-asan`
- Test targeted: container tests (`tests/container/dynamic_graph/`), especially
  `test_dynamic_graph_integration`, `test_dynamic_graph_nonintegral_ids`,
  `test_dynamic_graph_cpo_map_vertices`, `test_dynamic_graph_cpo_unordered_map_vertices`
- Test: `ctest --test-dir build/linux-gcc-asan --output-on-failure` (full, to catch
  dangling references via ASan)

### Step 1.3: Change `edge_descriptor::source_id()` and `target_id()` to `decltype(auto)`

**File:** `include/graph/adj_list/edge_descriptor.hpp`

**Change:** Same pattern as Step 1.2. `source_id()` delegates to `vertex_descriptor::vertex_id()`
so it may only need the return type change. `target_id()` has multiple extraction branches —
parenthesize only the keyed-storage branches.

**Validation:** Same as Step 1.2.

### Step 1.4: Verify `raw_vertex_id_t<G>` now resolves to a reference for map-based graphs

**Action:** Add a temporary `static_assert` in one test file (e.g.,
`test_dynamic_graph_integration.cpp`) to confirm:
```cpp
// For vector-based graph:
static_assert(!std::is_reference_v<adj_list::raw_vertex_id_t<VectorGraph>>);
// For map-based graph:
static_assert(std::is_reference_v<adj_list::raw_vertex_id_t<MapGraph>>);
```

**Validation:** Build + test. Remove the temporary `static_assert` after confirming (or keep
it as a permanent regression guard).

### Step 1.5: Phase 1 gate — full matrix build with ASan and commit

**Commands:**
```bash
./scripts/test-all.sh
cmake --build build/linux-gcc-asan -j $(nproc) && ctest --test-dir build/linux-gcc-asan --output-on-failure
git add -A && git commit -m "Phase 1: Descriptor return types to decltype(auto) for reference semantics"
```

**Exit criterion:** All presets green. ASan reports no errors.

---

## Phase 2: Concept and Trait Definitions

**Goal:** Change `vertex_id_t<G> uid` → `const vertex_id_t<G>& uid` in requires-expressions
of concepts and trait-detection concepts. This ensures concepts match the `const&` calling
convention that the rest of the library will adopt.

**Risk level: LOW-MEDIUM.** Concept changes can affect overload resolution. The key safety
measure is to verify concept satisfaction for all existing graph container types after each step.

### Step 2.1: Update `adjacency_list_traits.hpp`

**File:** `include/graph/adj_list/adjacency_list_traits.hpp`

**Change:** In every trait-detection concept that takes `vertex_id_t<G> uid`, change to
`const vertex_id_t<G>& uid`:
- `has_degree_uid_impl`
- `has_find_vertex_impl`
- `has_find_vertex_edge_uidvid_impl`
- `has_contains_edge`
- Any others found in the file

**Validation:**
- Build: `linux-gcc-debug`, `linux-clang-debug`
- Test: `ctest --test-dir build/linux-gcc-debug --output-on-failure` (full — concept changes
  can have surprising effects)

### Step 2.2: Update `adjacency_list_concepts.hpp`

**File:** `include/graph/adj_list/adjacency_list_concepts.hpp`

**Change:** In every concept that takes `vertex_id_t<G> uid` in a requires-expression, change
to `const vertex_id_t<G>& uid`:
- `vertex`
- `index_vertex_range`
- `adjacency_list`
- `bidirectional_adjacency_list`
- Any others found in the file

**Validation:**
- Build: `linux-gcc-debug`, `linux-clang-debug`
- Test: full test suite. Pay special attention to `tests/adj_list/` and `tests/container/` —
  these exercise concept satisfaction directly.

### Step 2.3: Phase 2 gate — full matrix build and commit

**Commands:**
```bash
./scripts/test-all.sh
git add -A && git commit -m "Phase 2: Concept and trait requires-expressions use const vertex_id_t<G>&"
```

**Exit criterion:** All presets green.

---

## Phase 3: View Factory Functions and Constructors

**Goal:** Change all view factory functions and view class constructors from
`vertex_id_t<G> uid` to `const vertex_id_t<G>& uid`. This is a high-count but low-risk
mechanical change.

**Approach:** Process one view family at a time, build+test after each. This limits blast
radius if a change introduces an issue.

### Step 3.1: `incidence.hpp` — factory functions and constructors

**File:** `include/graph/views/incidence.hpp`

**Change:** All factory function overloads (`incidence`, `basic_incidence`, `out_incidence`,
`basic_out_incidence`, `in_incidence`, `basic_in_incidence`) and view class constructors:
`vertex_id_t<G> uid` → `const vertex_id_t<G>& uid`.

**Validation:**
- Build: `linux-gcc-debug`
- Test: `tests/views/test_incidence.cpp`, `tests/views/test_basic_incidence.cpp`,
  `tests/views/test_in_incidence.cpp`

### Step 3.2: `neighbors.hpp` — factory functions and constructors

**File:** `include/graph/views/neighbors.hpp`

**Change:** Same pattern — all `neighbors`, `basic_neighbors`, `out_neighbors`,
`basic_out_neighbors`, `in_neighbors`, `basic_in_neighbors` overloads.

**Validation:**
- Build: `linux-gcc-debug`
- Test: `tests/views/test_neighbors.cpp`, `tests/views/test_basic_neighbors.cpp`,
  `tests/views/test_in_neighbors.cpp`

### Step 3.3: `bfs.hpp` — factory functions and constructors

**File:** `include/graph/views/bfs.hpp`

**Change:** All `vertices_bfs` and `edges_bfs` factory overloads, plus `vertices_bfs_view`
and `edges_bfs_view` constructors.

**Validation:**
- Build: `linux-gcc-debug`
- Test: `tests/views/test_bfs.cpp`

### Step 3.4: `dfs.hpp` — factory functions and constructors

**File:** `include/graph/views/dfs.hpp`

**Change:** All `vertices_dfs` and `edges_dfs` factory overloads, plus view constructors.

**Validation:**
- Build: `linux-gcc-debug`
- Test: `tests/views/test_dfs.cpp`

### Step 3.5: `transpose.hpp` — friend functions

**File:** `include/graph/views/transpose.hpp`

**Change:** `find_vertex(transpose_view&, vertex_id_t<G> uid)` →
`find_vertex(transpose_view&, const vertex_id_t<G>& uid)` (both const and non-const overloads).

**Validation:**
- Build: `linux-gcc-debug`
- Test: `tests/views/test_transpose.cpp`

### Step 3.6: `topological_sort.hpp` (view) — if it takes vertex_id

**File:** `include/graph/views/topological_sort.hpp`

**Change:** Check if this view takes `vertex_id_t<G>` parameters. If so, apply same change.

**Validation:** Build + test `tests/views/test_topological_sort.cpp`.

### Step 3.7: `vertexlist.hpp` and `edgelist.hpp` — check for vertex_id parameters

**Files:** `include/graph/views/vertexlist.hpp`, `include/graph/views/edgelist.hpp`

**Change:** Check if any factory functions take `vertex_id_t<G>` parameters (e.g., subrange
overloads with uid bounds). If so, apply same change.

**Validation:** Build + test relevant view tests.

### Step 3.8: Phase 3 gate — full matrix build, benchmarks, and commit

**Commands:**
```bash
./scripts/test-all.sh
cmake --build build/linux-gcc-release -j $(nproc)
./build/linux-gcc-release/benchmark/graph3_benchmarks
git add -A && git commit -m "Phase 3: View factory functions and constructors use const vertex_id_t<G>&"
```

**Exit criterion:** All presets green. Benchmarks show no regression.

---

## Phase 4a: Algorithm Parameter Signatures

**Goal:** Change algorithm function parameters from `vertex_id_t<G> source` to
`const vertex_id_t<G>& source`, and internal lambda parameters similarly. This phase does
**not** change internal storage — that is Phase 4b.

**Approach:** One algorithm at a time. Each algorithm has its own test file, so validation is
straightforward.

### Step 4a.1: `dijkstra_shortest_paths.hpp`

**File:** `include/graph/algorithm/dijkstra_shortest_paths.hpp`

**Changes:**
- Multi-source: no `vertex_id_t<G>` parameter (takes `Sources` range) — no change to signature.
- Single-source `dijkstra_shortest_paths`: `vertex_id_t<G> source` → `const vertex_id_t<G>& source`
- Single-source `dijkstra_shortest_distances`: same change
- `relax_target` lambda: `vertex_id_t<G> uid` → `const vertex_id_t<G>& uid`

**Validation:**
- Build: `linux-gcc-debug`
- Test: `tests/algorithms/test_dijkstra_shortest_paths.cpp`

### Step 4a.2: `bellman_ford_shortest_paths.hpp`

**File:** `include/graph/algorithm/bellman_ford_shortest_paths.hpp`

**Changes:** Same pattern — single-source overloads and `relax_target` lambda.

**Validation:** Build + test `tests/algorithms/test_bellman_ford_shortest_paths.cpp`.

### Step 4a.3: `breadth_first_search.hpp`

**File:** `include/graph/algorithm/breadth_first_search.hpp`

**Changes:** Single-source overload: `vertex_id_t<G> source` → `const vertex_id_t<G>& source`.

**Validation:** Build + test `tests/algorithms/test_breadth_first_search.cpp`.

### Step 4a.4: `depth_first_search.hpp` — verify, no change expected

**File:** `include/graph/algorithm/depth_first_search.hpp`

**Action:** Verify it already uses `const vertex_id_t<G>& source`. No change expected.

**Validation:** Build + test `tests/algorithms/test_depth_first_search.cpp`.

### Step 4a.5: `topological_sort.hpp`

**File:** `include/graph/algorithm/topological_sort.hpp`

**Changes:** Single-source overload: `vertex_id_t<G> source` → `const vertex_id_t<G>& source`.

**Validation:** Build + test `tests/algorithms/test_topological_sort.cpp`.

### Step 4a.6: `mis.hpp`

**File:** `include/graph/algorithm/mis.hpp`

**Changes:** `vertex_id_t<G> seed = 0` — evaluate whether to change to
`const vertex_id_t<G>& seed`. Since this algorithm requires `index_adjacency_list` (integral
IDs), the default `= 0` is valid. If changing to `const&`, the default becomes
`= vertex_id_t<G>{}`. Choose the option that compiles cleanly.

**Validation:** Build + test `tests/algorithms/test_mis.cpp`.

### Step 4a.7: `connected_components.hpp`

**File:** `include/graph/algorithm/connected_components.hpp`

**Changes:** Check all function signatures for `vertex_id_t<G>` parameters. Internal lambdas
taking vertex_id by value should change to `const vertex_id_t<G>&`.

**Validation:** Build + test `tests/algorithms/test_connected_components.cpp`.

### Step 4a.8: `label_propagation.hpp`

**File:** `include/graph/algorithm/label_propagation.hpp`

**Changes:** Check for `vertex_id_t<G>` parameters. Apply same pattern.

**Validation:** Build + test `tests/algorithms/test_label_propagation.cpp`.

### Step 4a.9: Remaining algorithms — `mst.hpp`, `jaccard.hpp`, `tc.hpp`, `articulation_points.hpp`, `biconnected_components.hpp`

**Files:** All remaining algorithm headers.

**Changes:** Check each for `vertex_id_t<G>` function/lambda parameters. Change to `const&`
where applicable. Loop variables (`for (vertex_id_t<G> uid = 0; ...)`) remain by value.

**Validation:** Build + test each algorithm's test file.

### Step 4a.10: Phase 4a gate — full matrix build and commit

**Commands:**
```bash
./scripts/test-all.sh
git add -A && git commit -m "Phase 4a: Algorithm parameter signatures use const vertex_id_t<G>&"
```

**Exit criterion:** All presets green.

---

## Phase 4b: Algorithm Internal Storage (`vertex_id_store_t<G>`)

**Goal:** Change algorithm-internal `using id_type = vertex_id_t<G>` to
`using id_type = vertex_id_store_t<G>`. Add `static_assert` guards.

**Risk note:** For all current algorithms constrained to `index_adjacency_list`, this is a
no-op — `vertex_id_store_t<G>` resolves to `vertex_id_t<G>` for integral IDs. The
`static_assert` guards verify this. This phase establishes the pattern for future algorithms
that may relax the integral constraint.

### Step 4b.1: `dijkstra_shortest_paths.hpp`

**File:** `include/graph/algorithm/dijkstra_shortest_paths.hpp`

**Change:**
```cpp
// Before:
using id_type = vertex_id_t<G>;

// After:
using id_type = vertex_id_store_t<G>;
// Guard: for index_adjacency_list, id_type must be the value type
static_assert(std::is_same_v<id_type, vertex_id_t<G>>,
              "vertex_id_store_t<G> should equal vertex_id_t<G> for index_adjacency_list");
```

**Validation:**
- Build: `linux-gcc-debug`, `linux-clang-debug`
- Test: `tests/algorithms/test_dijkstra_shortest_paths.cpp`

### Step 4b.2: `bellman_ford_shortest_paths.hpp`

**File + Change:** Same pattern.

**Validation:** Build + test.

### Step 4b.3: `breadth_first_search.hpp`

**File + Change:** Same pattern (if it has `using id_type = vertex_id_t<G>`).

**Validation:** Build + test.

### Step 4b.4: `depth_first_search.hpp`

**File + Change:** Same pattern.

**Validation:** Build + test.

### Step 4b.5: Remaining algorithms

**Files:** All remaining algorithm headers with `using id_type = vertex_id_t<G>` or equivalent.

**Change:** Same pattern with `static_assert` guard.

**Validation:** Build + test each.

### Step 4b.6: Phase 4b gate — full matrix build with ASan and commit

**Commands:**
```bash
./scripts/test-all.sh
cmake --build build/linux-gcc-asan -j $(nproc) && ctest --test-dir build/linux-gcc-asan --output-on-failure
git add -A && git commit -m "Phase 4b: Algorithm internals use vertex_id_store_t<G> with static_assert guards"
```

**Exit criterion:** All presets green. ASan clean.

---

## Phase 5: Adaptor / Pipe Syntax

**Goal:** Review and update adaptor functions in `adaptors.hpp` to ensure forwarding works
correctly with `const vertex_id_t<G>&` arguments.

### Step 5.1: Review and update `adaptors.hpp`

**File:** `include/graph/views/adaptors.hpp`

**Action:** Review each adaptor that forwards a vertex ID. These typically use:
```cpp
adj_list::vertex_id_t<std::remove_cvref_t<G>>(std::forward<UID>(uid))
```
Verify that `UID` correctly deduces and forwards when the caller passes `const vertex_id_t<G>&`.
If any adaptor explicitly constructs a `vertex_id_t<G>` value, that is intentional (copying
for storage) and should be left as-is.

**Validation:**
- Build: `linux-gcc-debug`
- Test: `tests/views/test_adaptors.cpp`

### Step 5.2: Phase 5 gate — full matrix build and commit

**Commands:**
```bash
./scripts/test-all.sh
git add -A && git commit -m "Phase 5: Adaptor/pipe syntax functions updated for const& vertex IDs"
```

**Exit criterion:** All presets green.

---

## Phase 6: Documentation and Examples

**Goal:** Update documentation and examples to reflect the new `const&` convention.

### Step 6.1: Update examples

**Files:** `examples/dijkstra_clrs_example.cpp`, `examples/basic_usage.cpp`,
`examples/mst_usage_example.cpp`

**Change:** If any example shows a function signature with `vertex_id_t<G>` by value, update
to `const vertex_id_t<G>&`. Update any prose comments.

**Validation:** Build examples — `cmake --build build/linux-gcc-debug --target examples`.

### Step 6.2: Update user-guide documentation

**Files:** `docs/user-guide/*.md`, `docs/reference/*.md`

**Change:** Search for `vertex_id_t` in documentation and update any API signature descriptions.
Add a note about the `const&` convention and `vertex_id_store_t<G>` for algorithm implementers.

**Validation:** Manual review (documentation only).

### Step 6.3: Phase 6 gate — full matrix build and commit

**Commands:**
```bash
./scripts/test-all.sh
git add -A && git commit -m "Phase 6: Documentation and examples updated for const& vertex ID convention"
```

**Exit criterion:** All presets green. All documentation accurate.

---

## Final Validation

After all phases are complete:

1. **Full matrix build:** `./scripts/test-all.sh`
2. **ASan:** `cmake --build build/linux-gcc-asan -j $(nproc) && ctest --test-dir build/linux-gcc-asan --output-on-failure`
3. **Benchmarks:** Compare before/after on `linux-gcc-release` to verify no performance regression
4. **Map-based graph tests:** Specifically run `test_dynamic_graph_integration`,
   `test_dynamic_graph_nonintegral_ids`, and `test_dynamic_graph_cpo_map_vertices` under ASan
5. **Merge:** `git checkout main && git merge --no-ff feature/vertex-id-const-ref`

---

## Status Summary

| Phase | Step | Description | Status |
|-------|------|-------------|--------|
| **0** | 0.1 | Protect `vertex_id_t<G>` in `graph_cpo.hpp`, add `raw_vertex_id_t<G>` | not-started |
| **0** | 0.2 | Protect `vertex_id_t<G>` in `edge_list.hpp` | not-started |
| **0** | 0.3 | Add `vertex_id_store_t<G>` in `traversal_common.hpp` | not-started |
| **0** | 0.4 | Phase 0 gate — full matrix build | not-started |
| **1** | 1.1 | Audit temporary descriptor usage (read-only) | not-started |
| **1** | 1.2 | `vertex_descriptor::vertex_id()` → `decltype(auto)` | not-started |
| **1** | 1.3 | `edge_descriptor::source_id()` and `target_id()` → `decltype(auto)` | not-started |
| **1** | 1.4 | Verify `raw_vertex_id_t<G>` resolves correctly (static_assert) | not-started |
| **1** | 1.5 | Phase 1 gate — full matrix + ASan | not-started |
| **2** | 2.1 | Update `adjacency_list_traits.hpp` concepts to `const&` | not-started |
| **2** | 2.2 | Update `adjacency_list_concepts.hpp` concepts to `const&` | not-started |
| **2** | 2.3 | Phase 2 gate — full matrix build | not-started |
| **3** | 3.1 | `incidence.hpp` — factory functions and constructors | not-started |
| **3** | 3.2 | `neighbors.hpp` — factory functions and constructors | not-started |
| **3** | 3.3 | `bfs.hpp` — factory functions and constructors | not-started |
| **3** | 3.4 | `dfs.hpp` — factory functions and constructors | not-started |
| **3** | 3.5 | `transpose.hpp` — friend functions | not-started |
| **3** | 3.6 | `topological_sort.hpp` (view) — check and update | not-started |
| **3** | 3.7 | `vertexlist.hpp` and `edgelist.hpp` — check and update | not-started |
| **3** | 3.8 | Phase 3 gate — full matrix + benchmarks | not-started |
| **4a** | 4a.1 | `dijkstra_shortest_paths.hpp` — signatures + lambdas | not-started |
| **4a** | 4a.2 | `bellman_ford_shortest_paths.hpp` — signatures + lambdas | not-started |
| **4a** | 4a.3 | `breadth_first_search.hpp` — signature | not-started |
| **4a** | 4a.4 | `depth_first_search.hpp` — verify (no change expected) | not-started |
| **4a** | 4a.5 | `topological_sort.hpp` — signature | not-started |
| **4a** | 4a.6 | `mis.hpp` — signature (evaluate default arg) | not-started |
| **4a** | 4a.7 | `connected_components.hpp` — signatures + lambdas | not-started |
| **4a** | 4a.8 | `label_propagation.hpp` — signatures | not-started |
| **4a** | 4a.9 | Remaining algorithms (`mst`, `jaccard`, `tc`, `articulation_points`, `biconnected_components`) | not-started |
| **4a** | 4a.10 | Phase 4a gate — full matrix build | not-started |
| **4b** | 4b.1 | `dijkstra_shortest_paths.hpp` — `id_type` + static_assert | not-started |
| **4b** | 4b.2 | `bellman_ford_shortest_paths.hpp` — `id_type` + static_assert | not-started |
| **4b** | 4b.3 | `breadth_first_search.hpp` — `id_type` + static_assert | not-started |
| **4b** | 4b.4 | `depth_first_search.hpp` — `id_type` + static_assert | not-started |
| **4b** | 4b.5 | Remaining algorithms — `id_type` + static_assert | not-started |
| **4b** | 4b.6 | Phase 4b gate — full matrix + ASan | not-started |
| **5** | 5.1 | Review and update `adaptors.hpp` | not-started |
| **5** | 5.2 | Phase 5 gate — full matrix build | not-started |
| **6** | 6.1 | Update examples | not-started |
| **6** | 6.2 | Update user-guide documentation | not-started |
| **6** | 6.3 | Phase 6 gate — full matrix build | not-started |
| — | — | **Final validation** — full matrix + ASan + benchmarks | not-started |
