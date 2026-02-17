# View Signature Refactoring — Implementation Plan

Implements [view_signature_goal.md](view_signature_goal.md).

## Approach

Each phase targets one view family. Within each phase, steps add `basic_` variants first (additive, 
non-breaking), then refactor the existing view to match the goal signatures, then update callers.
Tests are updated or added at each step so the build stays green.

The four view families are ordered by dependency and usage:
1. **vertexlist** — no callers in other views; lays down the pattern
2. **incidence** — most-used in algorithms (12 call sites across 6 algorithm files)
3. **neighbors** — no algorithm callers; only tests and benchmarks
4. **edgelist** — 2 algorithm call sites (bellman_ford); also has edge_list variant

A final phase updates adaptors (pipe syntax), benchmarks, examples, and documentation.

## Current Call-Site Counts

| View | Tests | Algorithms | Benchmarks | Examples | Total |
|------|------:|----------:|---------:|----------:|------:|
| vertexlist | 91 | 5 | 5 | 0 | 101 |
| incidence | 95 | 12 | 1 | 1 | 109 |
| neighbors | ~56 | 0 | 1 | 0 | ~57 |
| edgelist | 100 | 2 | 1 | 0 | 103 |

---

## Phase 1: vertexlist — add `basic_vertexlist` and subrange overloads

### Step 1.1: Add `basic_vertexlist_view` class (void and VVF specializations)
- Add to `include/graph/views/vertexlist.hpp`
- `basic_vertexlist_view<G, void>` returns `vertex_info<VId, void, void>` (id only)
- `basic_vertexlist_view<G, VVF>` returns `vertex_info<VId, void, VV>` (id + value)
- Include deduction guides
- **Build & run existing tests** — no breakage expected (additive only)

### Step 1.2: Add `basic_vertexlist()` factory functions
- `basic_vertexlist(g)` and `basic_vertexlist(g, vvf)`
- **Build & run existing tests**

### Step 1.3: Add subrange overloads to `vertexlist`
- `vertexlist(g, first_u, last_u)` and `vertexlist(g, first_u, last_u, vvf)` — descriptor range
- `vertexlist(g, vr)` and `vertexlist(g, vr, vvf)` — vertex range
- **Build & run existing tests**

### Step 1.4: Add subrange overloads to `basic_vertexlist`
- `basic_vertexlist(g, first_uid, last_uid)` and `basic_vertexlist(g, first_uid, last_uid, vvf)` — id range
- `basic_vertexlist(g, vr)` and `basic_vertexlist(g, vr, vvf)` — vertex range
- **Build & run existing tests**

### Step 1.5: Add tests for `basic_vertexlist` and subrange overloads
- Add test cases in `tests/views/test_vertexlist.cpp` (or a new file `test_basic_vertexlist.cpp`)
- Cover: void, VVF, subrange, vr, structured bindings, empty graph, single vertex
- **Build & run all view tests**

### Step 1.6: Verify `vertexlist` return types match goal
- Existing `vertexlist(g)` already returns `vertex_info<VId, V, void>` — verify
- Existing `vertexlist(g, vvf)` already returns `vertex_info<VId, V, VV>` — verify
- If adjustments needed, make them and update existing test structured bindings
- **Build & run all tests**

### Step 1.7: Update algorithm callers of `vertexlist`
- `connected_components.hpp` uses `vertexlist(g)` (5 sites) — determine if each should use
  `basic_vertexlist(g)` instead; update if so
- Update tests in `tests/algorithms/` if structured bindings change
- **Build & run all tests**

### Step 1.8: Commit Phase 1
- Commit message: "Phase 1: Add basic_vertexlist and vertexlist subrange overloads"

---

## Phase 2: incidence — add `basic_incidence`, refactor existing overloads

### Step 2.1: Add `basic_incidence_view` class (void and EVF specializations)
- Add to `include/graph/views/incidence.hpp`
- `basic_incidence_view<G, void>` takes `uid`, returns `edge_info<VId, false, void, void>` (target id only)
- `basic_incidence_view<G, EVF>` takes `uid`, returns `edge_info<VId, false, void, EV>` (target id + value)
- Include deduction guides
- **Build & run existing tests**

### Step 2.2: Add `basic_incidence()` factory functions
- `basic_incidence(g, uid)` and `basic_incidence(g, uid, evf)`
- **Build & run existing tests**

### Step 2.3: Refactor existing `incidence(g, uid)` overloads
- Current `incidence(g, uid)` returns `edge_info<VId, false, E, void>` — it resolves uid to a descriptor
  internally and delegates to the descriptor-based view
- **Remove** `incidence(g, uid)` and `incidence(g, uid, evf)` factory functions
- Callers must switch to either `incidence(g, u)` (descriptor) or `basic_incidence(g, uid)` (id)
- **Do NOT build yet** — callers will break; fix in next steps

### Step 2.4: Update algorithm callers of `incidence(g, uid)`
- `connected_components.hpp` (2 sites) → `basic_incidence(g, uid)`
- `depth_first_search.hpp` (3 sites) → `basic_incidence(g, uid)`
- `breadth_first_search.hpp` (1 site) → `basic_incidence(g, uid)`
- `dijkstra_shortest_paths.hpp` (1 site) → `basic_incidence(g, uid)`
- `topological_sort.hpp` (3 sites) → `basic_incidence(g, uid)`
- `mst.hpp` (1 site) → `basic_incidence(g, uid)`
- Update structured bindings to match the `basic_` return type (no edge descriptor)
- **Build algorithms — verify compilation**

### Step 2.5: Update test callers of `incidence(g, uid)`
- `tests/views/test_incidence.cpp` (~63 sites) — update uid-based calls to `basic_incidence`
- `tests/views/test_adaptors.cpp` (~14 sites) — update as needed
- `tests/views/test_edge_cases.cpp` (~7 sites) — update uid-based calls
- `tests/views/test_view_chaining.cpp` (~4 sites) — update uid-based calls
- Other test files with 1-2 sites — update
- **Build & run all tests**

### Step 2.6: Add tests for `basic_incidence`
- Add test cases for `basic_incidence(g, uid)` and `basic_incidence(g, uid, evf)`
- Cover: structured bindings, empty vertex, value function, graph types
- **Build & run all view tests**

### Step 2.7: Update example and benchmark callers
- `examples/dijkstra_clrs.hpp` (1 site) — update if uid-based
- `benchmark/benchmark_views.cpp` (1 site) — update if uid-based
- **Build & run benchmarks**

### Step 2.8: Verify `incidence(g, u)` return types match goal
- `incidence(g, u)` should return `edge_info<VId, false, E, void>` — verify
- `incidence(g, u, evf)` should return `edge_info<VId, false, E, EV>` — verify
- **Build & run all tests**

### Step 2.9: Commit Phase 2
- Commit message: "Phase 2: Add basic_incidence; remove incidence uid overloads"

---

## Phase 3: neighbors — add `basic_neighbors`, refactor existing overloads

### Step 3.1: Add `basic_neighbors_view` class (void and VVF specializations)
- Add to `include/graph/views/neighbors.hpp`
- `basic_neighbors_view<G, void>` takes `uid`, returns `neighbor_info<VId, false, void, void>` (target id only)
- `basic_neighbors_view<G, VVF>` takes `uid`, returns `neighbor_info<VId, false, void, VV>` (target id + value)
- Include deduction guides
- **Build & run existing tests**

### Step 3.2: Add `basic_neighbors()` factory functions
- `basic_neighbors(g, uid)` and `basic_neighbors(g, uid, vvf)`
- **Build & run existing tests**

### Step 3.3: Refactor existing `neighbors(g, uid)` overloads
- Keep `neighbors(g, uid)` and `neighbors(g, uid, vvf)` as convenience wrappers resolving uid→descriptor
- Update to use `index_adjacency_list` constraint and `*find_vertex` pattern (matching Phase 2)
- Update `neighbors_adaptor_fn` direct-call operators to use `index_adjacency_list<remove_cvref_t<G>>`
- **Build & run existing tests**

### Step 3.4: Update test callers of `neighbors(g, uid)`
- No changes needed — uid-based overloads kept as convenience wrappers
- **Build & run all tests**

### Step 3.5: Add tests for `basic_neighbors`
- Add test cases for `basic_neighbors(g, uid)` and `basic_neighbors(g, uid, vvf)`
- Cover: structured bindings, no-neighbors vertex, value function, graph types
- **Build & run all view tests**

### Step 3.6: Update benchmark callers
- No changes needed — benchmark uses pipe adaptor which routes through uid-based overloads
- **Build & run benchmarks**

### Step 3.7: Verify `neighbors(g, u)` return types match goal
- `neighbors(g, u)` should return `neighbor_info<VId, false, V, void>` — verify
- `neighbors(g, u, vvf)` should return `neighbor_info<VId, false, V, VV>` — verify
- **Build & run all tests**

### Step 3.8: Commit Phase 3
- Commit message: "Phase 3: Add basic_neighbors; refactor neighbors uid overloads"

---

## Phase 4: edgelist — add `basic_edgelist`

### Step 4.1: Add `basic_edgelist_view` class (void and EVF specializations)
- Add to `include/graph/views/edgelist.hpp`
- `basic_edgelist_view<G, void>` returns `edge_info<VId, true, void, void>` (source_id + target_id only)
- `basic_edgelist_view<G, EVF>` returns `edge_info<VId, true, void, EV>` (ids + value)
- Include deduction guides
- **Build & run existing tests**

### Step 4.2: Add `basic_edgelist()` factory functions
- `basic_edgelist(g)` and `basic_edgelist(g, evf)`
- **Build & run existing tests**

### Step 4.3: Add tests for `basic_edgelist`
- Add test cases for `basic_edgelist(g)` and `basic_edgelist(g, evf)`
- Cover: structured bindings, empty graph, value function, graph types
- **Build & run all view tests**

### Step 4.4: Verify `edgelist(g)` return types match goal
- `edgelist(g)` should return `edge_info<VId, true, E, void>` — already does; verify
- `edgelist(g, evf)` should return `edge_info<VId, true, E, EV>` — already does; verify
- **Build & run all tests**

### Step 4.5: Update algorithm callers to use `basic_edgelist` where appropriate
- `bellman_ford_shortest_paths.hpp` (2 sites) — kept as-is; uses edge descriptor `uv` in visitor callbacks
- **No changes needed**

### Step 4.6: Commit Phase 4
- Commit message: "Phase 4: Add basic_edgelist"

---

## Phase 5: Adaptors, documentation, and cleanup

### Step 5.1: Add pipe adaptors for `basic_` views
- Add to `include/graph/views/adaptors.hpp`:
  - `basic_vertexlist` adaptor (closure + fn)
  - `basic_incidence` adaptor (closure + fn)
  - `basic_neighbors` adaptor (closure + fn)
  - `basic_edgelist` adaptor (closure + fn)
- **Build & run existing tests**

### Step 5.2: Add adaptor tests
- Update `tests/views/test_adaptors.cpp` with pipe syntax tests for all `basic_` variants
- **Build & run all tests**

### Step 5.3: Update `basic_views.hpp`
- Include the `basic_` view headers if not already accessible from the existing headers
- Verify `graph.hpp` includes are correct
- **Build & run all tests**

### Step 5.4: Update benchmark code
- Update `benchmark/benchmark_views.cpp` to include `basic_` view benchmarks
- **Build & run benchmarks**

### Step 5.5: Update documentation
- Update `docs/views.md` and any other docs referencing view signatures
- Update `README.md` if view examples need changing
- **Build & run all tests**

### Step 5.6: Apply formatting
- Run `clang-format` on all modified files
- **Build & run all tests**

### Step 5.7: Commit Phase 5
- Commit message: "Phase 5: Add basic_ pipe adaptors; update docs and benchmarks"

---

## Progress Summary

| Phase | Step | Description | Status |
|:-----:|:----:|-------------|:------:|
| 1 | 1.1 | Add `basic_vertexlist_view` class | DONE |
| 1 | 1.2 | Add `basic_vertexlist()` factory functions | DONE |
| 1 | 1.3 | Add `vertexlist` subrange overloads (descriptor) | DONE |
| 1 | 1.4 | Add `basic_vertexlist` subrange overloads (id) | DONE |
| 1 | 1.5 | Add tests for `basic_vertexlist` and subranges | DONE |
| 1 | 1.6 | Verify `vertexlist` return types match goal | DONE |
| 1 | 1.7 | Update algorithm callers of `vertexlist` | DONE (no changes needed — all use descriptor) |
| 1 | 1.8 | Commit Phase 1 | DONE |
| 2 | 2.1 | Add `basic_incidence_view` class | DONE |
| 2 | 2.2 | Add `basic_incidence()` factory functions | DONE |
| 2 | 2.3 | Refactor `incidence(g, uid)` overloads | DONE (kept as convenience wrappers resolving uid→descriptor) |
| 2 | 2.4 | Update algorithm callers of `incidence(g, uid)` | DONE |
| 2 | 2.5 | Update test callers of `incidence(g, uid)` | DONE |
| 2 | 2.6 | Add tests for `basic_incidence` | DONE |
| 2 | 2.7 | Update example and benchmark callers | DONE |
| 2 | 2.8 | Verify `incidence(g, u)` return types match goal | DONE |
| 2 | 2.9 | Commit Phase 2 | DONE |
| 3 | 3.1 | Add `basic_neighbors_view` class | DONE |
| 3 | 3.2 | Add `basic_neighbors()` factory functions | DONE |
| 3 | 3.3 | Refactor `neighbors(g, uid)` overloads | DONE (kept as convenience wrappers resolving uid→descriptor) |
| 3 | 3.4 | Update test callers of `neighbors(g, uid)` | DONE (no changes needed — uid overloads kept) |
| 3 | 3.5 | Add tests for `basic_neighbors` | DONE |
| 3 | 3.6 | Update benchmark callers | DONE (no changes needed) |
| 3 | 3.7 | Verify `neighbors(g, u)` return types match goal | DONE |
| 3 | 3.8 | Commit Phase 3 | DONE |
| 4 | 4.1 | Add `basic_edgelist_view` class | DONE |
| 4 | 4.2 | Add `basic_edgelist()` factory functions | DONE |
| 4 | 4.3 | Add tests for `basic_edgelist` | DONE |
| 4 | 4.4 | Verify `edgelist(g)` return types match goal | DONE |
| 4 | 4.5 | Update algorithm callers to use `basic_edgelist` | DONE (no changes — bellman_ford needs edge descriptor) |
| 4 | 4.6 | Commit Phase 4 | DONE |
| 5 | 5.1 | Add pipe adaptors for `basic_` views | DONE |
| 5 | 5.2 | Add adaptor tests | DONE |
| 5 | 5.3 | Update `basic_views.hpp` | DONE |
| 5 | 5.4 | Update benchmark code | DONE |
| 5 | 5.5 | Update documentation | DONE |
| 5 | 5.6 | Apply formatting | DONE (skipped — not using clang-format) |
| 5 | 5.7 | Commit Phase 5 | DONE |

**Overall: 35 / 35 steps complete — ALL PHASES DONE**
