# Map-Based Container Strategy — Implementation Plan

This plan derives from [map_container_strategy.md](map_container_strategy.md) and is
structured so that an agent can execute each phase or sub-phase independently. Every
sub-phase is self-contained: it states what to read, what to create/modify, how to
verify, and what to commit.

**Branch:** `mapped`

**Invariant:** After every sub-phase, `ctest` passes all existing tests (4343+). No
sub-phase may break backward compatibility.

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

## Phase 0 — Preparation (no code changes)

### 0.1 Verify Baseline

| Item | Detail |
|------|--------|
| **Action** | Run the full test suite to confirm the starting baseline is green. |
| **Verify** | `cd build/linux-clang-debug && ctest --output-on-failure` — all tests pass |

### 0.2 Create Reference Directory

| Item | Detail |
|------|--------|
| **Action** | Create `include/graph/algorithm/index/` and copy all 14 algorithm `.hpp` files into it as byte-identical snapshots. |
| **Create** | `include/graph/algorithm/index/` containing: |

```
traversal_common.hpp
breadth_first_search.hpp
depth_first_search.hpp
topological_sort.hpp
dijkstra_shortest_paths.hpp
bellman_ford_shortest_paths.hpp
connected_components.hpp
articulation_points.hpp
biconnected_components.hpp
label_propagation.hpp
mis.hpp
jaccard.hpp
tc.hpp
mst.hpp
```

| **Verify** | `diff include/graph/algorithm/breadth_first_search.hpp include/graph/algorithm/index/breadth_first_search.hpp` — no diff (repeat for each file) |

### 0.3 Commit

| Item | Detail |
|------|--------|
| **Commit** | `chore: snapshot index-only algorithm implementations for reference` |

---

## Phase 1 — Concepts & Vertex Map Infrastructure

All changes in this phase are **additive only** — no existing files are modified in a
way that changes behavior. Existing tests must still pass.

### 1.1 Add `mapped_vertex_range` and Composed Concepts

| Item | Detail |
|------|--------|
| **Read** | `include/graph/adj_list/adjacency_list_concepts.hpp` — understand current concept hierarchy |
| **Modify** | `include/graph/adj_list/adjacency_list_concepts.hpp` |
| **Changes** | Add after `index_vertex_range` / `index_adjacency_list` / `index_bidirectional_adjacency_list`: |

```cpp
/// Satisfied by graphs whose vertex IDs are hashable keys (map/unordered_map).
/// Vertex IDs are sparse; lookup is via find_vertex(g, uid).
template <class G>
concept hashable_vertex_id = requires(const vertex_id_t<G>& uid) {
  { std::hash<vertex_id_t<G>>{}(uid) } -> std::convertible_to<size_t>;
};

template <class G>
concept mapped_vertex_range =
    !index_vertex_range<G> &&
    hashable_vertex_id<G> &&
    requires(G& g) {
      { vertices(g) } -> std::ranges::forward_range;
    } &&
    requires(G& g, const vertex_id_t<G>& uid) {
      find_vertex(g, uid);
    };

template <class G>
concept mapped_adjacency_list = adjacency_list<G> && mapped_vertex_range<G>;

template <class G>
concept mapped_bidirectional_adjacency_list =
    bidirectional_adjacency_list<G> && mapped_vertex_range<G>;
```

| **Verify** | Build succeeds; `ctest` — all existing tests pass |

### 1.2 Create `vertex_map.hpp`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/adj_list/adjacency_list_concepts.hpp` (for `index_vertex_range`, `vertex_id_t`); `include/graph/algorithm/traversal_common.hpp` (for `num_vertices`, `views::vertexlist` usage patterns) |
| **Create** | `include/graph/adj_list/vertex_map.hpp` |
| **Contents** | |

```cpp
// vertex_map<G,T> type alias
// make_vertex_map — eager (with init value) and lazy (capacity only) overloads
// vertex_map_contains(map, uid) — always true for vector, calls contains() for unordered_map
// vertex_map_get(map, uid, default_val) — no-insertion read with fallback
```

Key implementation details:
- `vertex_map<G,T>` = `conditional_t<index_vertex_range<G>, vector<T>, unordered_map<vertex_id_t<G>, T>>`
- Eager `make_vertex_map(g, init_value)`: vector(N, init) for index, iterate vertexlist + insert for mapped
- Lazy `make_vertex_map<G, T>(g)`: vector(N) for index, empty reserved map for mapped
- `vertex_map_contains`: dispatch on `random_access_range` — always `true` vs `m.contains(uid)`
- `vertex_map_get`: dispatch on `random_access_range` — `m[size_t(uid)]` vs `find()`+default

| **Verify** | Build succeeds |

### 1.3 Add `is_null_range_v` Trait

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/traversal_common.hpp` — locate `_null_range_type` |
| **Modify** | `include/graph/algorithm/traversal_common.hpp` |
| **Changes** | Add near `_null_range_type`: |

```cpp
template <class T>
inline constexpr bool is_null_range_v =
    std::is_same_v<std::remove_cvref_t<T>, _null_range_type>;
```

| **Verify** | Build succeeds; `ctest` — all existing tests pass |

### 1.4 Create Map-Based Graph Fixtures for Algorithms

| Item | Detail |
|------|--------|
| **Read** | `tests/common/map_graph_test_data.hpp` (existing sparse data); `tests/common/graph_fixtures.hpp` (existing index-graph fixtures — Dijkstra CLRS, BFS tree, etc.); `tests/common/algorithm_test_types.hpp` (SPARSE_VERTEX_TYPES macro) |
| **Create** | `tests/common/map_graph_fixtures.hpp` |
| **Contents** | Helper functions that build small map-based graphs matching the same topologies used in existing algorithm tests: |

- `make_clrs_dijkstra_graph<Graph>()` — the 5-vertex weighted graph used by Dijkstra tests, with sparse vertex IDs (e.g., 10, 20, 30, 40, 50) for map-based types, standard IDs for index types
- `make_bfs_tree<Graph>()` — small BFS-testable graph
- `make_dfs_graph<Graph>()` — small graph for DFS/topological sort
- `make_connected_components_graph<Graph>()` — graph with 2–3 components

Uses `fixture_selector<Graph>::use_sparse` to pick ID scheme.

| **Verify** | Build succeeds |

### 1.5 Create Vertex Map Unit Tests

| Item | Detail |
|------|--------|
| **Read** | `include/graph/adj_list/vertex_map.hpp` (from 1.2) |
| **Create** | `tests/adj_list/test_vertex_map.cpp` |
| **Modify** | `tests/CMakeLists.txt` — add the new test file to the build |
| **Tests** | |

- `vertex_map<G, int>` resolves to `vector<int>` for `vov_weighted` and `unordered_map<uint32_t, int>` for `mov_weighted`
- `make_vertex_map(g, 0)` (eager) — correct size, all values initialized
- `make_vertex_map<G, int>(g)` (lazy) — empty map for mapped, sized vector for index
- `vertex_map_contains` — true for all index UIDs; false for absent keys in mapped
- `vertex_map_get(m, uid, default_val)` — returns mapped value if present, default if absent (no insertion)

| **Verify** | `ctest` — all existing + new tests pass |

### 1.6 Create Concept Static-Assert Tests

| Item | Detail |
|------|--------|
| **Create** | `tests/adj_list/test_mapped_concepts.cpp` |
| **Modify** | `tests/CMakeLists.txt` — add the new test file |
| **Tests** | |

- `static_assert(index_adjacency_list<vov_weighted>)` — index types satisfy index concept
- `static_assert(!mapped_vertex_range<vov_weighted>)` — index types do NOT satisfy mapped concept
- `static_assert(mapped_adjacency_list<mov_weighted>)` — map types satisfy mapped concept
- `static_assert(!index_vertex_range<mov_weighted>)` — map types do NOT satisfy index concept
- `static_assert(adjacency_list<vov_weighted>)` — both satisfy the base concept
- `static_assert(adjacency_list<mov_weighted>)` — both satisfy the base concept
- `static_assert(hashable_vertex_id<mov_weighted>)` — map types have hashable IDs

| **Verify** | `ctest` — all tests pass |

### 1.7 Commit

| Item | Detail |
|------|--------|
| **Commit** | `feat: add mapped_adjacency_list concepts and vertex_map infrastructure` |

### 1.R Review Gate

Confirm before proceeding to Phase 2:
- `mapped_vertex_range` and `index_vertex_range` are mutually exclusive
- `vertex_map` produces correct types for both families
- `make_vertex_map` eager/lazy work correctly
- `vertex_map_contains` and `vertex_map_get` have no insertion side-effects for mapped
- All 4343+ existing tests still pass

---

## Phase 2 — Traversal Algorithms

Generalize BFS, DFS, and topological sort. These are the simplest algorithms — only
internal state arrays, no user-supplied property map parameters.

**Workflow per algorithm:**
1. Read the algorithm header + its index/ snapshot + its test file
2. Relax concept from `index_adjacency_list` to `adjacency_list`
3. Replace internal `vector<T>` with `make_vertex_map` (lazy where possible)
4. Replace `vertex_id_t<G>` in containers with `vertex_id_store_t<G>`
5. Replace any `for(uid=0; uid<N)` loops with `views::vertexlist`
6. Run existing tests — must all pass
7. Add new `TEMPLATE_TEST_CASE` sections using `SPARSE_VERTEX_TYPES`

### 2.1 Generalize `breadth_first_search`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/breadth_first_search.hpp`, `include/graph/algorithm/index/breadth_first_search.hpp`, `tests/algorithms/test_breadth_first_search.cpp`, `include/graph/adj_list/vertex_map.hpp` |
| **Modify** | `include/graph/algorithm/breadth_first_search.hpp` |
| **Changes** | |

- Relax concept: `index_adjacency_list<G>` → `adjacency_list<G>`
- `vector<bool> visited(num_vertices(g), false)` → `auto visited = make_vertex_map<G, bool>(g, false);` (eager init — but see note: could be lazy+contains for large sparse graphs; start with eager for correctness parity)
- Queue element type: `vertex_id_t<G>` → `vertex_id_store_t<G>`
- Any `for(uid = 0; uid < N; ++uid)` → `for(auto&& [uid, u] : views::vertexlist(g))`
- Include `vertex_map.hpp`

| **Modify** | `tests/algorithms/test_breadth_first_search.cpp` |
| **Changes** | Add `TEMPLATE_TEST_CASE` sections using `SPARSE_VERTEX_TYPES` for BFS with map-based graph fixtures |
| **Verify** | `ctest` — all existing + new tests pass |

### 2.2 Review Gate — BFS

Review diff: `diff include/graph/algorithm/breadth_first_search.hpp include/graph/algorithm/index/breadth_first_search.hpp`

Confirm:
- Index-graph behavior is identical
- Map-graph BFS discovers all reachable vertices
- No `static_cast<size_t>` remains
- No `for(uid=0; uid<N)` remains

### 2.3 Generalize `depth_first_search`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/depth_first_search.hpp`, `include/graph/algorithm/index/depth_first_search.hpp`, `tests/algorithms/test_depth_first_search.cpp` |
| **Modify** | `include/graph/algorithm/depth_first_search.hpp` |
| **Changes** | |

- Relax concept: `index_adjacency_list<G>` → `adjacency_list<G>`
- `vector<Color> color(num_vertices(g), White)` → lazy `vertex_map<G, uint8_t>` with default=White via `vertex_map_get`
- Stack frame vertex_id field → `vertex_id_store_t<G>`
- Include `vertex_map.hpp`

| **Modify** | `tests/algorithms/test_depth_first_search.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test sections |
| **Verify** | `ctest` — all existing + new tests pass |

### 2.4 Review Gate — DFS

Review diff against `index/depth_first_search.hpp`. Confirm:
- Stack management works with `vertex_id_store_t<G>` (reference_wrapper for map graphs)
- Color defaults work via lazy pattern

### 2.5 Generalize `topological_sort`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/topological_sort.hpp`, `include/graph/algorithm/index/topological_sort.hpp`, `tests/algorithms/test_topological_sort.cpp` |
| **Modify** | `include/graph/algorithm/topological_sort.hpp` |
| **Changes** | |

- Relax concept: `index_adjacency_list<G>` → `adjacency_list<G>`
- Same color pattern as DFS
- `vector<vertex_id_t<G>> finish_order` → `vector<vertex_id_store_t<G>>`
- Include `vertex_map.hpp`

| **Modify** | `tests/algorithms/test_topological_sort.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test sections |
| **Verify** | `ctest` — all existing + new tests pass |

### 2.6 Review Gate — Topological Sort

Review diff against `index/topological_sort.hpp`.

### 2.7 Commit

| Item | Detail |
|------|--------|
| **Commit** | `feat: generalize BFS, DFS, topological_sort for mapped graphs` |

---

## Phase 3 — Shortest Path Infrastructure

Generalize `init_shortest_paths` and `_null_range_type` detection. No algorithm logic
changes yet — only the shared infrastructure.

### 3.1 Add Graph-Parameterized `init_shortest_paths` Overloads

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/traversal_common.hpp` — locate existing `init_shortest_paths` overloads |
| **Modify** | `include/graph/algorithm/traversal_common.hpp` |
| **Changes** | |

Keep existing overloads unchanged for backward compatibility. Add new overloads that
take `const G& g` as the first parameter:

```cpp
// New overload — distances only
template <class G, class Distances>
constexpr void init_shortest_paths(const G& g, Distances& distances);

// New overload — distances + predecessors
template <class G, class Distances, class Predecessors>
constexpr void init_shortest_paths(const G& g, Distances& distances, Predecessors& predecessors);
```

For index graphs (`random_access_range<Distances>`): same fill/iota behavior as today.
For mapped graphs: if map is empty (lazy), no-op. If pre-populated, fill existing entries.
Use `is_null_range_v<Predecessors>` to skip predecessor handling.

| **Verify** | Build succeeds; `ctest` — all existing tests pass |

### 3.2 Verify `_null_range_type` Detection

| Item | Detail |
|------|--------|
| **Action** | Add a static_assert or small unit test confirming `is_null_range_v<_null_range_type>` is `true` and `is_null_range_v<vector<int>>` is `false` |
| **Modify** | Can add to `tests/adj_list/test_vertex_map.cpp` or a new test file |
| **Verify** | Test passes |

### 3.3 Commit

| Item | Detail |
|------|--------|
| **Commit** | `feat: generalize init_shortest_paths for mapped graphs` |

### 3.R Review Gate

Confirm init functions handle:
- Index graph with pre-sized vector → fill + iota (unchanged)
- Mapped graph with empty map → no-op
- Mapped graph with pre-populated map → fill existing entries
- `_null_range_type` → skipped via `is_null_range_v`

---

## Phase 4 — Shortest Path Algorithms

Generalize Dijkstra and Bellman-Ford. These have the deepest index coupling: user-provided
`Distances`/`Predecessors`, priority queue, `static_cast<size_t>`.

### 4.1 Generalize `dijkstra_shortest_paths`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/dijkstra_shortest_paths.hpp`, `include/graph/algorithm/index/dijkstra_shortest_paths.hpp`, `tests/algorithms/test_dijkstra_shortest_paths.cpp`, `include/graph/algorithm/traversal_common.hpp` |
| **Modify** | `include/graph/algorithm/dijkstra_shortest_paths.hpp` |
| **Changes** | |

Two approaches (per strategy §3 — separate overloads):

**Option A (simpler first pass):** Use `if constexpr` within existing functions:
- Relax concept to `adjacency_list<G>` (if parameter types are generic enough)
- Replace `static_cast<size_t>(uid)` → direct `[uid]`
- Queue element → `vertex_id_store_t<G>`
- Use graph-parameterized `init_shortest_paths<G>(g, distances, predecessors)`

**Option B (full separate overloads — per strategy decision):**
- Keep existing `index_adjacency_list` + `random_access_range` overloads **unchanged**
- Add new overloads: `mapped_adjacency_list` + `vertex_id_map<Distances, G>` + `vertex_id_map<Predecessors, G>`
- Both overloads can share a private `_impl` function

Start with Option A if the parameter concepts can be generalized without breaking
existing callers. Fall back to Option B if needed.

| **Modify** | `tests/algorithms/test_dijkstra_shortest_paths.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test sections using map-based CLRS Dijkstra fixture |
| **Verify** | `ctest` — all existing + new tests pass |

### 4.2 Review Gate — Dijkstra

Review diff against `index/dijkstra_shortest_paths.hpp`. Confirm:
- Priority queue comparator works with `vertex_id_store_t<G>`
- Distance reads use `vertex_map_get` with infinity default (no spurious insertion)
- Predecessor writes only occur during relaxation
- Existing index-graph performance is unchanged (no extra branching in hot path)

### 4.3 Generalize `bellman_ford_shortest_paths`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/bellman_ford_shortest_paths.hpp`, `include/graph/algorithm/index/bellman_ford_shortest_paths.hpp`, `tests/algorithms/test_bellman_ford_shortest_paths.cpp` |
| **Modify** | `include/graph/algorithm/bellman_ford_shortest_paths.hpp` |
| **Changes** | |

- Same Distances/Predecessors treatment as Dijkstra
- Convert `for(k = 0; k < N; ++k)` relaxation loop to `views::vertexlist` iteration
- Replace `static_cast<size_t>()` → direct `[uid]`
- Use graph-parameterized `init_shortest_paths`

| **Modify** | `tests/algorithms/test_bellman_ford_shortest_paths.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test sections |
| **Verify** | `ctest` — all existing + new tests pass |

### 4.4 Review Gate — Bellman-Ford

Review diff against `index/bellman_ford_shortest_paths.hpp`.

### 4.5 Commit

| Item | Detail |
|------|--------|
| **Commit** | `feat: generalize dijkstra, bellman_ford for mapped graphs` |

---

## Phase 5 — Component Algorithms

Generalize `connected_components` (3 functions: `connected_components()`, `kosaraju()`,
`afforest()`). Each function is a separate sub-task.

### 5.1 Generalize `connected_components()`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/connected_components.hpp`, `include/graph/algorithm/index/connected_components.hpp`, `tests/algorithms/test_connected_components.cpp` |
| **Modify** | `include/graph/algorithm/connected_components.hpp` — the `connected_components()` function only |
| **Changes** | |

- Relax `Component` from `random_access_range` to accept vertex maps
- `for(uid = 0; uid < N; ++uid)` → `views::vertexlist`
- `iota` for component init → vertex iteration
- Internal `visited` → `vertex_map<G, bool>`

| **Modify** | `tests/algorithms/test_connected_components.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section for `connected_components()` |
| **Verify** | `ctest` — all existing + new tests pass |

### 5.2 Review Gate — `connected_components()`

### 5.3 Generalize `kosaraju()`

| Item | Detail |
|------|--------|
| **Modify** | `include/graph/algorithm/connected_components.hpp` — `kosaraju()` overloads only |
| **Changes** | |

- `vector<bool> visited` → `vertex_map<G, bool>`
- `vector<vertex_id_t<G>> order` → `vector<vertex_id_store_t<G>>`
- Relax concept as needed

| **Modify** | `tests/algorithms/test_scc_bidirectional.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section |
| **Verify** | `ctest` — all existing + new tests pass |

### 5.4 Review Gate — `kosaraju()`

### 5.5 Generalize `afforest()`

| Item | Detail |
|------|--------|
| **Modify** | `include/graph/algorithm/connected_components.hpp` — `afforest()` overloads only |
| **Changes** | |

- `iota(component)` → vertex iteration
- `static_cast<vertex_id_t<G>>` adjustments
- Relax concept as needed

| **Modify** | `tests/algorithms/test_connected_components.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section for `afforest()` |
| **Verify** | `ctest` — all existing + new tests pass |

### 5.6 Review Gate — `afforest()`

### 5.7 Commit

| Item | Detail |
|------|--------|
| **Commit** | `feat: generalize connected_components for mapped graphs` |

---

## Phase 6 — Simple Algorithms (batch, low risk)

Four algorithms with minimal index coupling — at most one internal array, no user
property map parameters.

### 6.1 Generalize `jaccard_coefficient`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/jaccard.hpp`, `include/graph/algorithm/index/jaccard.hpp`, `tests/algorithms/test_jaccard.cpp` |
| **Modify** | `include/graph/algorithm/jaccard.hpp` |
| **Changes** | |

- `vector<unordered_set<vid_t>> nbrs` → `vertex_map<G, unordered_set<vertex_id_store_t<G>>>`
- Relax concept to `adjacency_list<G>`

| **Modify** | `tests/algorithms/test_jaccard.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section |
| **Verify** | `ctest` — all existing + new tests pass |

### 6.2 Generalize `mis`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/mis.hpp`, `include/graph/algorithm/index/mis.hpp`, `tests/algorithms/test_mis.cpp` |
| **Modify** | `include/graph/algorithm/mis.hpp` |
| **Changes** | |

- `vector<uint8_t> removed(N)` → lazy `vertex_map<G, uint8_t>` + `vertex_map_contains`
- Relax concept to `adjacency_list<G>`

| **Modify** | `tests/algorithms/test_mis.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section |
| **Verify** | `ctest` — all existing + new tests pass |

### 6.3 Generalize `triangle_count`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/tc.hpp`, `include/graph/algorithm/index/tc.hpp`, `tests/algorithms/test_triangle_count.cpp` |
| **Modify** | `include/graph/algorithm/tc.hpp` |
| **Changes** | |

- `for(uid = 0; uid < N; ++uid)` → `views::vertexlist`
- Relax concept to `adjacency_list<G>`

| **Modify** | `tests/algorithms/test_triangle_count.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section |
| **Verify** | `ctest` — all existing + new tests pass |

### 6.4 Generalize `label_propagation`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/label_propagation.hpp`, `include/graph/algorithm/index/label_propagation.hpp`, `tests/algorithms/test_label_propagation.cpp` |
| **Modify** | `include/graph/algorithm/label_propagation.hpp` |
| **Changes** | |

- `vector<vertex_id_t<G>> order` → `vector<vertex_id_store_t<G>>`
- `iota` → vertex iteration
- Relax `Label` parameter from `random_access_range` to accept vertex maps
- Relax concept to `adjacency_list<G>`

| **Modify** | `tests/algorithms/test_label_propagation.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section |
| **Verify** | `ctest` — all existing + new tests pass |

### 6.5 Review Gate — All Four

Review diffs for jaccard, mis, triangle_count, label_propagation against their
`index/` snapshots.

### 6.6 Commit

| Item | Detail |
|------|--------|
| **Commit** | `feat: generalize jaccard, mis, triangle_count, label_propagation for mapped graphs` |

---

## Phase 7 — Structural Algorithms (highest effort)

Articulation points (5 internal arrays) and biconnected components (3 internal arrays).

### 7.1 Generalize `articulation_points`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/articulation_points.hpp`, `include/graph/algorithm/index/articulation_points.hpp`, `tests/algorithms/test_articulation_points.cpp` |
| **Modify** | `include/graph/algorithm/articulation_points.hpp` |
| **Changes** | |

- 5 internal arrays → vertex_maps:
  - `disc` → `make_vertex_map<G, size_t>(g)` (or appropriate type)
  - `low` → `make_vertex_map<G, size_t>(g)`
  - `parent` → `make_vertex_map<G, vertex_id_store_t<G>>(g)`
  - `child_count` → `make_vertex_map<G, size_t>(g)`
  - `emitted` → `make_vertex_map<G, bool>(g, false)`
- `static_cast<vid_t>(N)` sentinel → `vertex_map_contains` or a dedicated sentinel
- Relax concept to `adjacency_list<G>`

| **Modify** | `tests/algorithms/test_articulation_points.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section |
| **Verify** | `ctest` — all existing + new tests pass |

### 7.2 Review Gate — Articulation Points

Most complex transformation. Carefully review:
- Sentinel values work correctly for mapped graphs
- All 5 vertex_maps are correctly initialized
- No `for(uid=0; uid<N)` remains

### 7.3 Generalize `biconnected_components`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/biconnected_components.hpp`, `include/graph/algorithm/index/biconnected_components.hpp`, `tests/algorithms/test_biconnected_components.cpp` |
| **Modify** | `include/graph/algorithm/biconnected_components.hpp` |
| **Changes** | |

- 3 internal arrays → vertex_maps: `disc`, `low`, `parent`
- Same sentinel change as articulation_points
- Relax concept to `adjacency_list<G>`

| **Modify** | `tests/algorithms/test_biconnected_components.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section |
| **Verify** | `ctest` — all existing + new tests pass |

### 7.4 Review Gate — Biconnected Components

### 7.5 Commit

| Item | Detail |
|------|--------|
| **Commit** | `feat: generalize articulation_points, biconnected_components for mapped graphs` |

---

## Phase 8 — MST Algorithms

### 8.1 Generalize `prim`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/mst.hpp`, `include/graph/algorithm/index/mst.hpp`, `tests/algorithms/test_mst.cpp` |
| **Modify** | `include/graph/algorithm/mst.hpp` — `prim()` function only |
| **Changes** | |

- Internal `vector<EV> distance` → `vertex_map<G, EV>`
- Relax `Predecessor`/`Weight` from `random_access_range` to accept vertex maps
- `for(v = 0; v < N; ++v)` → vertex iteration
- Drop `static_cast<size_t>()`
- Relax concept to `adjacency_list<G>`

| **Modify** | `tests/algorithms/test_mst.cpp` |
| **Changes** | Add `SPARSE_VERTEX_TYPES` test section for `prim()` |
| **Verify** | `ctest` — all existing + new tests pass |

### 8.2 Review Gate — Prim

### 8.3 Evaluate `kruskal` / `inplace_kruskal`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/algorithm/mst.hpp` — `kruskal()` and `inplace_kruskal()` functions |
| **Action** | Evaluate whether map support makes sense for edge-list-centric algorithms. These use `x_index_edgelist_range` and `disjoint_vector`. |
| **Decision** | If feasible with reasonable effort, generalize. If the `disjoint_vector` coupling is too deep, document as deferred and skip. |

### 8.4 Review Gate — Kruskal Decision

### 8.5 Commit (if applicable)

| Item | Detail |
|------|--------|
| **Commit** | `feat: generalize prim MST for mapped graphs` (and kruskal if included) |

---

## Phase 9 — Cleanup & Documentation

### 9.1 Final Full Test Run

| Item | Detail |
|------|--------|
| **Action** | Run tests across multiple presets for confidence: |

```bash
# Debug build
cd build/linux-clang-debug && cmake --build . && ctest --output-on-failure

# Release build
cd build/linux-clang-release && cmake --build . && ctest --output-on-failure

# Address sanitizer
cd build/linux-gcc-asan && cmake --build . && ctest --output-on-failure
```

| **Verify** | All pass in all presets |

### 9.2 Remove Reference Directory

| Item | Detail |
|------|--------|
| **Action** | `rm -rf include/graph/algorithm/index/` |
| **Verify** | Directory removed; build still succeeds (no code references it) |

### 9.3 Update Documentation

| Item | Detail |
|------|--------|
| **Modify** | `docs/user-guide/` — add section on using algorithms with map-based graphs |
| **Modify** | `docs/reference/` — update algorithm reference to document map-graph support |
| **Modify** | `CHANGELOG.md` — add entries for map-based algorithm support |
| **Verify** | Docs build/render correctly |

### 9.4 Commit

| Item | Detail |
|------|--------|
| **Commit** | `chore: remove index-only reference copies, update docs for mapped graph support` |

---

## Reference: Key Files

| File | Role |
|------|------|
| `include/graph/adj_list/adjacency_list_concepts.hpp` | Concept definitions — modified in Phase 1 |
| `include/graph/adj_list/vertex_map.hpp` | New — vertex_map type alias + helpers — created in Phase 1 |
| `include/graph/algorithm/traversal_common.hpp` | Shared infrastructure — modified in Phases 1 & 3 |
| `tests/common/algorithm_test_types.hpp` | `SPARSE_VERTEX_TYPES` macro — already exists |
| `tests/common/map_graph_test_data.hpp` | Sparse test data — already exists |
| `tests/common/map_graph_fixtures.hpp` | New — map-based algorithm fixtures — created in Phase 1 |
| `tests/common/graph_fixtures.hpp` | Existing index-graph fixtures — read for reference |
| `include/graph/algorithm/index/` | Reference snapshots — created in Phase 0, removed in Phase 9 |

---

## Progress Tracking

| Phase | Sub-Phase | Description | Status |
|-------|-----------|-------------|--------|
| **0** | **0.1** | Verify baseline (all tests pass) | Complete |
| **0** | **0.2** | Create `index/` directory, copy 14 algorithm files | Complete |
| **0** | **0.3** | Commit snapshot | Complete |
| **1** | **1.1** | Add `mapped_vertex_range` + composed concepts | Complete |
| **1** | **1.2** | Create `vertex_map.hpp` (type alias + helpers) | Complete |
| **1** | **1.3** | Add `is_null_range_v` trait | Complete |
| **1** | **1.4** | Create `map_graph_fixtures.hpp` | Complete |
| **1** | **1.5** | Create vertex_map unit tests | Complete |
| **1** | **1.6** | Create concept static_assert tests | Complete |
| **1** | **1.7** | Commit concepts + vertex_map infrastructure | Complete |
| **1** | **1.R** | Review gate: concept mutual exclusivity, vertex_map correctness | Complete |
| **2** | **2.1** | Generalize `breadth_first_search` | Complete |
| **2** | **2.2** | Review gate: BFS diff | Complete |
| **2** | **2.3** | Generalize `depth_first_search` | Complete |
| **2** | **2.4** | Review gate: DFS diff | Complete |
| **2** | **2.5** | Generalize `topological_sort` | Complete |
| **2** | **2.6** | Review gate: topological sort diff | Complete |
| **2** | **2.7** | Commit BFS, DFS, topological_sort | Complete |
| **3** | **3.1** | Add graph-parameterized `init_shortest_paths` overloads | Complete |
| **3** | **3.2** | Verify `_null_range_type` detection with `is_null_range_v` | Complete |
| **3** | **3.3** | Commit init_shortest_paths generalization | Complete |
| **3** | **3.R** | Review gate: init functions handle index/mapped/null correctly | Complete |
| **4** | **4.1** | Generalize `dijkstra_shortest_paths` | Complete |
| **4** | **4.2** | Review gate: Dijkstra diff | Complete |
| **4** | **4.3** | Generalize `bellman_ford_shortest_paths` | Complete |
| **4** | **4.4** | Review gate: Bellman-Ford diff | Complete |
| **4** | **4.5** | Commit Dijkstra + Bellman-Ford | Complete |
| **5** | **5.1** | Generalize `connected_components()` | Complete |
| **5** | **5.2** | Review gate: connected_components() diff | Complete |
| **5** | **5.3** | Generalize `kosaraju()` | Complete |
| **5** | **5.4** | Review gate: kosaraju() diff | Complete |
| **5** | **5.5** | Generalize `afforest()` | Complete |
| **5** | **5.6** | Review gate: afforest() diff | Complete |
| **5** | **5.7** | Commit connected_components | Complete |
| **5+** | — | Vertex descriptor optimization (all component + Dijkstra algorithms) | Complete |
| **6** | **6.1** | Generalize `jaccard_coefficient` | Complete |
| **6** | **6.2** | Generalize `mis` | Complete |
| **6** | **6.3** | Generalize `triangle_count` | Complete |
| **6** | **6.4** | Generalize `label_propagation` | Complete |
| **6** | **6.5** | Review gate: all four diffs | Complete |
| **6** | **6.6** | Commit jaccard, mis, triangle_count, label_propagation | Complete |
| **7** | **7.1** | Generalize `articulation_points` (5 internal arrays) | Complete |
| **7** | **7.2** | Review gate: articulation_points diff (most complex) | Complete |
| **7** | **7.3** | Generalize `biconnected_components` (3 internal arrays) | Complete |
| **7** | **7.4** | Review gate: biconnected_components diff | Not Started |
| **7** | **7.5** | Commit articulation_points + biconnected_components | Not Started |
| **8** | **8.1** | Generalize `prim` MST | Not Started |
| **8** | **8.2** | Review gate: prim diff | Not Started |
| **8** | **8.3** | Evaluate `kruskal` / `inplace_kruskal` feasibility | Not Started |
| **8** | **8.4** | Review gate: kruskal decision | Not Started |
| **8** | **8.5** | Commit MST (prim, and kruskal if applicable) | Not Started |
| **9** | **9.1** | Final full test run (clang-debug, clang-release, gcc-asan) | Not Started |
| **9** | **9.2** | Remove `include/graph/algorithm/index/` directory | Not Started |
| **9** | **9.3** | Update documentation for mapped graph support | Not Started |
| **9** | **9.4** | Commit cleanup + docs | Not Started |
