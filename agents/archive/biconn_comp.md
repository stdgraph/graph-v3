# Implement Biconnected Components Algorithm

## Reference Material
- Spec: /mnt/d/dev_graph/P1709/D3128_Algorithms/tex/algorithms.tex §Biconnected Components (line ~1076)
- Reference signatures: /mnt/d/dev_graph/P1709/D3128_Algorithms/src/connected_components.hpp (lines 8-16)
- Study for implementation pattern: include/graph/algorithm/articulation_points.hpp
- Study for test pattern: tests/algorithms/test_articulation_points.cpp

## Algorithm Summary
Find the biconnected components, or maximal biconnected subgraphs of a graph. A subgraph is
biconnected if it is connected and has no articulation points — i.e., it remains connected after
removing any single vertex. The algorithm is a direct extension of the Hopcroft-Tarjan articulation-
point DFS: an edge stack is maintained during the DFS, and whenever an articulation-point boundary
is detected (including at the DFS root), the edge stack is popped to yield one biconnected component.
Isolated vertices (degree 0) form trivial biconnected components of their own.
Time complexity: O(|V| + |E|).

## Signatures to Implement
```cpp
template <index_adjacency_list G,
          forward_range OuterContainer>
requires forward_range<range_value_t<OuterContainer>> &&
         integral<range_value_t<range_value_t<OuterContainer>>>
void biconnected_components(G&&             g,
                            OuterContainer& components);
```

The intended concrete type for `OuterContainer` is `std::vector<std::vector<vertex_id_t<G>>>`.
`OuterContainer` is passed by non-const reference; the algorithm `push_back`s one inner container
per biconnected component found. (The `forward_range` constraint in the spec is a lower bound;
`push_back` is required in practice.)

## Design Decisions
- **Namespace:** `graph::` — consistent with all other algorithms.
- **Return type:** `void` — matches D3128 spec. Results are appended to `components`.
- **Output representation:** Each biconnected component is stored as a `std::vector<vertex_id_t<G>>`
  of vertex IDs. Because articulation-point vertices belong to more than one biconnected component,
  such vertices appear in every inner container to which they belong. Callers should expect
  duplicates across (not within) inner containers.
- **Isolated vertices:** A vertex with no edges is a trivially biconnected subgraph. The algorithm
  emits a single-element inner container `{uid}` for every unvisited isolated vertex after the main
  DFS loop, so that every vertex appears in at least one component.
- **Algorithm choice:** Iterative Hopcroft-Tarjan DFS extended with an explicit edge stack.
  - During the DFS, each tree edge and back edge is pushed onto the edge stack as it is traversed.
  - When backtracking from child `v` to parent `u`:
    - If `low[v] >= disc[u]` (i.e., `u` is an articulation point, or `u` is the DFS root), pop
      all edges from the edge stack down to and including the edge `(u, v)`. Collect the unique
      vertex IDs of those edges into a new inner container and `push_back` it onto `components`.
  - After the DFS root's stack frame is done, if the root had ≥ 2 DFS children a component was
    already flushed per child; if it had exactly 1 child, the final edge pop happens on backtrack.
- **Graph directionality:** Semantically meaningful only on undirected graphs. Callers must supply
  both directions of each undirected edge (`{u,v}` and `{v,u}`). Directed graphs with only one
  direction will produce incorrect results.
- **Multi-edges:** Parallel edges between `u` and `v` each appear independently on the edge stack.
  Only the first reverse edge to the DFS parent is skipped as the tree-edge reverse; further
  parallel edges to the parent update the low-link value (same rule as articulation_points.hpp).
- **Self-loops:** Ignored during DFS traversal. Self-loops do not affect biconnected component
  detection.
- **Storage for DFS state:** Use `std::vector<size_t>` for `disc` and `low` (sentinel =
  `std::numeric_limits<size_t>::max()` for "not yet visited"). Use `std::vector<vertex_id_t<G>>`
  for `parent` (sentinel = `N` meaning "no parent"). All vectors sized to `num_vertices(g)`.
- **Edge stack:** Use `std::stack<std::pair<vid_t, vid_t>>` storing `(source, target)` pairs for
  each directed edge as it is first traversed. When flushing a component, iterate the popped pairs
  and collect unique vertex IDs using a `std::set<vid_t>` or an `unordered_set` for deduplication.
- **Iterative DFS:** Use an explicit `std::stack<dfs_frame>` with frames
  `(uid, edge_index, parent_edge_skipped)`, mirroring `articulation_points.hpp` exactly.
- **Header guard:** `#ifndef GRAPH_BICONNECTED_COMPONENTS_HPP` /
  `#define GRAPH_BICONNECTED_COMPONENTS_HPP`.
- **`requires` clause:** `requires forward_range<range_value_t<OuterContainer>> &&
  integral<range_value_t<range_value_t<OuterContainer>>>` matching the spec signature.
- **`using` declarations:** same set as `articulation_points.hpp`: `index_adjacency_list`,
  `vertex_id_t`, `vertices`, `edges`, `target_id`, `vertex_id`, `num_vertices`, `find_vertex`.

## Files to Create/Modify
| Action   | File                                                            | What to do                                                    |
|----------|-----------------------------------------------------------------|---------------------------------------------------------------|
| Create   | `include/graph/algorithm/biconnected_components.hpp`            | Full implementation + Doxygen doc                             |
| Modify   | `include/graph/algorithms.hpp`                                  | Add `#include "algorithm/biconnected_components.hpp"`         |
| Create   | `tests/algorithms/test_biconnected_components.cpp`              | Catch2 tests (see Test Plan)                                  |
| Modify   | `tests/algorithms/CMakeLists.txt`                               | Add `test_biconnected_components.cpp` to `add_executable()`   |

## Implementation Steps
1. Read `tex/algorithms.tex` §Biconnected Components (~line 1076) and lines 8–16 of
   `D3128_Algorithms/src/connected_components.hpp` to confirm the exact signature and any
   additional specification details (edge vs. vertex output, isolated-vertex handling, etc.).
2. Study `include/graph/algorithm/articulation_points.hpp` for:
   - File layout: header guard, includes, namespace, using-declarations.
   - Iterative DFS frame structure `(uid, edge_idx, parent_edge_skipped)`.
   - Low-link update and backtrack logic.
   - Doxygen style: complexity table, supported graph properties, container requirements,
     @tparam / @param / @pre / @post / exception safety / example usage blocks.
3. Study `tests/algorithms/test_articulation_points.cpp` for:
   - Test file structure, `same_set` helper, brute-force helper pattern.
   - `TEMPLATE_TEST_CASE` parameterisation over `vov_void` and `dov_void`.
4. Implement `include/graph/algorithm/biconnected_components.hpp`:
   - File-level Doxygen comment (description, authors, SPDX).
   - Header guard and includes: `graph/graph.hpp`, `<limits>`, `<set>`, `<stack>`, `<vector>`.
   - `using` declarations (same as `articulation_points.hpp`).
   - Template + `requires` clause matching the spec signature.
   - Document using the requirements specified in `docs/algorithm_template.md`.
   - Full Doxygen doc block.
   - Algorithm body:
     a. Return immediately if `N == 0`.
     b. Allocate `disc`, `low` (sentinel = `max()`), `parent` (sentinel = `N`).
     c. Declare `std::stack<std::pair<vid_t, vid_t>> edge_stk` for the edge stack.
     d. Outer loop over all vertices to handle disconnected graphs.
     e. Inner iterative DFS using the same `dfs_frame` struct as `articulation_points.hpp`.
        - On a tree edge `(uid → vid)`: push `{uid, vid}` onto `edge_stk`, then push a new
          frame for `vid`.
        - On a back edge `(uid → vid, disc[vid] < disc[uid])`: push `{uid, vid}` onto
          `edge_stk` and update `low[uid]`.
        - On backtrack from child `v` to parent `u`: update `low[u]`; if `low[v] >= disc[u]`,
          pop edges from `edge_stk` until `{u, v}` is popped (inclusive), collect unique
          vertex IDs, and `push_back` a new inner container onto `components`.
     f. After the outer loop, handle isolated vertices (vertices with `disc[v] == UNVISITED`
        that were never reached — though the outer loop should cover all; however isolated
        vertices with no edges will be enqueued and immediately back-tracked with 0 edges;
        check whether an empty component was pushed and, if so, replace it with `{start}`).
        Alternatively, detect in the outer loop: if `num_edges(g, sv) == 0`, emit `{start}`
        directly and skip the DFS for that vertex.
5. Add `#include "algorithm/biconnected_components.hpp"` to `include/graph/algorithms.hpp`
   next to (or just after) the `#include "algorithm/articulation_points.hpp"` line.
6. Write `tests/algorithms/test_biconnected_components.cpp` (see Test Plan).
7. Add `test_biconnected_components.cpp` to `tests/algorithms/CMakeLists.txt` inside the
   `add_executable(test_algorithms …)` block.
8. Build: `cmake --build build/linux-gcc-debug`
9. Run targeted tests: `ctest --test-dir build/linux-gcc-debug -R biconnected_components`
10. Run full suite: `ctest --test-dir build/linux-gcc-debug` — confirm 0 failures.

## Test Plan

### Helper functions to define (model on test_articulation_points.cpp)
```cpp
/// Sort a vector<vector<T>>: sort each inner vector, then sort the outer vector.
/// Used for order-independent comparison of component lists.
template <typename T>
std::vector<std::vector<T>> normalize_components(std::vector<std::vector<T>> comps) {
  for (auto& c : comps) std::sort(c.begin(), c.end());
  std::sort(comps.begin(), comps.end());
  return comps;
}

/// Brute-force check: verify that a vertex set forms a biconnected subgraph.
/// Returns true if the induced subgraph on `vids` is connected and removing any
/// single vertex from it keeps the remainder connected.
template <typename G>
bool is_biconnected_subgraph(const G& g, const std::vector<typename G::vertex_id_type>& vids);

/// Brute-force: compute all biconnected components by enumerating and checking.
/// Useful as a cross-check oracle on small graphs.
// (implement using recursive or edge-stack DFS as a separate reference)

/// Count occurrences of a vertex ID across all inner containers.
template <typename T>
size_t count_occurrences(const std::vector<std::vector<T>>& comps, T vid);
```

### Core test cases
| Test case | Graph | Expected components (as vertex sets) |
|-----------|-------|--------------------------------------|
| Empty graph (0 vertices) | — | `{}` — empty outer container; no crash |
| Single vertex, no edges | 1 vertex | `{{0}}` — trivial isolated component |
| Single edge 0↔1 | 2 vertices | `{{0, 1}}` |
| Path graph 0-1-2-3 (bidirectional) | 4 vertices | `{{0,1}, {1,2}, {2,3}}` — three trivial components (each bridge is its own component); vertex 1 and 2 each appear in two inner containers |
| Cycle graph 0-1-2-3-4-0 (bidirectional) | 5 vertices | `{{0,1,2,3,4}}` — one biconnected component |
| Star graph centre=0, leaves 1-4 (bidirectional) | 5 vertices | `{{0,1}, {0,2}, {0,3}, {0,4}}` — four trivial components all sharing vertex 0 |
| Two triangles joined by bridge 2-3 | 6 vertices | `{{0,1,2}, {2,3}, {3,4,5}}` — bridge 2-3 is its own component |
| Complete graph K4 (bidirectional) | 4 vertices | `{{0,1,2,3}}` — one biconnected component |
| Disconnected graph: path 0-1-2 + edge 3-4 | 5 vertices | `{{0,1}, {1,2}, {3,4}}` |
| Barbell: K3(0-1-2) — bridge 2-3 — bridge 3-4 — K3(4-5-6) | 7 vertices | `{{0,1,2}, {2,3}, {3,4}, {4,5,6}}` |
| Graph with self-loop on vertex 1, path 0-1-2 | 3 vertices | Same as path 0-1-2 without self-loop: `{{0,1}, {1,2}}` |
| Parallel edge between 0 and 1 (two edges 0↔1), no other edges | 2 vertices | `{{0,1}}` — parallel edges keep the same component |

### Parameterisation
Run the core correctness cases (path, cycle, star, bridge, K4) as `TEMPLATE_TEST_CASE`
over `vov_void` and `dov_void` from `algorithm_test_types.hpp`, to confirm that the
algorithm is container-independent.

### Verification strategy
For each test:
1. Collect `std::vector<std::vector<vertex_id_t<G>>>` via `push_back` / `components` output.
2. Normalise via `normalize_components()` and compare against the expected (also normalised)
   component list — this gives an order-independent full equality check.
3. For small graphs also verify structural correctness:
   - Every edge of the graph appears in exactly one component.
   - Every vertex appears in at least one component.
   - Articulation-point vertices appear in more than one component; all others in exactly one.
   - Each component's induced subgraph, when checked with `is_biconnected_subgraph()`, returns `true`.
