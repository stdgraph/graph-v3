# Implement Articulation Points Algorithm

## Reference Material
- Spec: /mnt/d/dev_graph/P1709/D3128_Algorithms/tex/algorithms.tex §Articulation Points (line ~1028)
- Reference signatures: /mnt/d/dev_graph/P1709/D3128_Algorithms/src/connected_components.hpp (lines 4–6)
- Study for implementation pattern: include/graph/algorithm/mis.hpp
- Study for test pattern: tests/algorithms/test_mis.cpp

## Algorithm Summary
Find articulation points, or cut vertices, which when removed disconnect the graph into multiple components.

Time complexity based on Hopcroft-Tarjan algorithm.

## Signatures to Implement
template <index_adjacency_list G, class Iter>
requires output_iterator<Iter, vertex_id_t<G>>
void articulation_points(G&& g, Iter cut_vertices);

## Design Decisions
- **Namespace:** `graph::` — consistent with all other algorithms.
- **Return type:** `void` — matches D3128 spec. Results are written through the output iterator.
- **Output iterator contract:** `Iter` must satisfy `std::output_iterator<Iter, vertex_id_t<G>>`,
  exactly as `maximal_independent_set` uses. Caller supplies a `std::back_inserter` or similar.
  No ordering guarantee on the emitted vertices.
- **Algorithm choice:** Iterative Hopcroft-Tarjan DFS using discovery times and low-link values,
  consistent with the spec. Avoids recursion to prevent stack overflow on large graphs.
  - `disc[v]` = DFS discovery time of vertex `v`.
  - `low[v]` = minimum discovery time reachable from the subtree rooted at `v` via back-edges.
  - Root rule: root of DFS tree is an articulation point iff it has ≥ 2 DFS children.
  - Non-root rule: non-root `u` is an articulation point iff any child `v` has `low[v] >= disc[u]`.
- **Graph directionality:** The spec lists "Directed? Yes" but the algorithm is semantically
  meaningful only on undirected graphs (or bidirectional directed graphs where every edge has a
  reverse). Document this clearly: callers must supply both `{u,v}` and `{v,u}` edges.
- **Multi-edges:** Spec says "Multi-edge? No". The Hopcroft-Tarjan algorithm must avoid treating
  a parallel edge back to the parent as a back-edge. Track the parent vertex ID and skip exactly
  one reverse edge per tree-edge descent. Use parent-vertex tracking (not parent-edge-index) since
  parallel edges are not supported.
- **Self-loops:** Spec says "Self-loops? Yes". Self-loops do not affect articulation point
  detection and can be ignored during the DFS.
- **Storage for DFS state:** Use `std::vector<size_t>` for `disc` and `low`, with sentinel value
  `std::numeric_limits<size_t>::max()` for "not yet visited". Use `std::vector<vertex_id_t<G>>`
  for `parent`, with sentinel `N` meaning "no parent". All vectors sized to `num_vertices(g)`.
- **Iterative DFS:** Use an explicit stack of `(uid, edge_iterator, parent_id)` tuples to avoid
  system-stack overflow. The edge iterator type is `vertex_edge_iterator_t<G>` (available via
  `using adj_list::vertex_edge_iterator_t`). Emit an articulation point decision when backtracking
  from a child.
- **Duplicate emission prevention:** A vertex with multiple qualifying DFS children would be emitted
  once per qualifying child without a guard. Add a `std::vector<bool> emitted(N, false)` and only
  write a vertex to the output iterator when `!emitted[uid]`, then set `emitted[uid] = true`.
- **Header guard:** `#ifndef GRAPH_ARTICULATION_POINTS_HPP` / `#define GRAPH_ARTICULATION_POINTS_HPP`.
- **`requires` clause:** `requires std::output_iterator<Iter, vertex_id_t<G>>` on the template,
  matching the mis.hpp / reference signature pattern.

## Files to Create/Modify
| Action   | File                                                        | What to do                                            |
|----------|-------------------------------------------------------------|-------------------------------------------------------|
| Create   | `include/graph/algorithm/articulation_points.hpp`           | Full implementation + Doxygen doc                     |
| Modify   | `include/graph/algorithms.hpp`                              | Add `#include "algorithm/articulation_points.hpp"`    |
| Create   | `tests/algorithms/test_articulation_points.cpp`             | Catch2 tests (see Test Plan)                          |
| Modify   | `tests/algorithms/CMakeLists.txt`                           | Add `test_articulation_points.cpp` to the test target |

## Implementation Steps
1. Read `tex/algorithms.tex` §Articulation Points (~line 1028) and lines 4–6 of
   `D3128_Algorithms/src/connected_components.hpp` to confirm signature.
2. Study `include/graph/algorithm/mis.hpp` for file layout: header guard, namespace,
   using-declarations, Doxygen style (complexity table, supported graph properties,
   container requirements, @pre/@post, exception safety, example usage).
3. Study `tests/algorithms/test_mis.cpp` for the test file structure and helper pattern
   (`is_independent_set`, `is_maximal`) as a model for writing the helper `is_articulation_point`.
4. Implement `include/graph/algorithm/articulation_points.hpp`:
   - File-level Doxygen comment (description, authors, SPDX)
   - Header guard and includes: `graph/graph.hpp`, `<limits>`, `<numeric>`, `<stack>`, `<vector>`
   - `using` declarations: `index_adjacency_list`, `vertex_id_t`, `vertex_edge_iterator_t`,
     `vertices`, `edges`, `target_id`, `vertex_id`, `num_vertices`, `find_vertex`
   - Template + `requires` clause matching the reference signature
   - Full Doxygen doc block (complexity, supported properties, container requirements,
     @pre/@post, exception safety, example usage)
   - Iterative Hopcroft-Tarjan DFS body:
     - Return immediately if `N == 0`
     - Allocate `disc`, `low` (sentinel = `max()`), `parent` (sentinel = `N`),
       `child_count`, `emitted` (bool, for deduplication)
     - Outer loop over all vertices to handle disconnected graphs
     - Inner iterative DFS: push `(uid, vertex_edge_iterator_t<G>, parent_id)` onto stack;
       on advance, check disc/low; on backtrack, apply low-link update and articulation-point
       test; check `!emitted[uid]` before writing to output iterator;
       track root child count separately for the root rule
5. Add `#include "algorithm/articulation_points.hpp"` to `include/graph/algorithms.hpp`
   under a `// Connectivity` section.
6. Write `tests/algorithms/test_articulation_points.cpp` (see Test Plan).
7. Add `test_articulation_points.cpp` to `tests/algorithms/CMakeLists.txt`.
8. Build: `cmake --build build/linux-gcc-debug`
9. Run targeted tests: `ctest --test-dir build/linux-gcc-debug -R articulation_points`
10. Run full suite: `ctest --test-dir build/linux-gcc-debug` — confirm 0 failures.

## Test Plan

### Helper functions to define (model on test_mis.cpp)
```cpp
// Brute-force check: returns true if removing uid increases the number of weakly
// connected components. Uses BFS over remaining vertices.
template <typename G>
bool is_articulation_point(const G& g, typename G::vertex_id_type uid);

// Sort both vectors and compare (order-independent equality check).
template <typename T>
bool same_set(std::vector<T> a, std::vector<T> b);
```

### Core test cases
| Test case | Graph | Expected articulation points |
|-----------|-------|------------------------------|
| Empty graph (0 vertices) | — | none; no crash |
| Single vertex, no edges | 1 vertex | none |
| Single edge (0→1, 1→0) | 2 vertices | none |
| Path graph 0-1-2-3 (bidirectional) | 4 vertices | {1, 2} |
| Cycle graph 0-1-2-3-4-0 (bidirectional) | 5 vertices | none |
| Star graph centre=0, leaves 1-4 (bidirectional) | 5 vertices | {0} |
| Bridge graph: two triangles joined by vertices 2→3 | 6 vertices | {2, 3} |
| Complete graph K4 (bidirectional) | 4 vertices | none |
| Disconnected graph (two separate components) | 5 vertices | articulation points per component |
| "Barbell": two K3 triangles (0-1-2 and 4-5-6) joined by edge 2↔3↔4, vertex 3 is the bridge | 7 vertices | {2, 3, 4} (the two triangle attachment vertices and the bridge vertex) |
| Graph with self-loop (self-loop must not affect result) | varies | same result as without self-loop |

### Parameterisation
Run the core correctness cases (path, cycle, star, bridge) as `TEMPLATE_TEST_CASE`
over `vov_void` and `dov_void` from `algorithm_test_types.hpp`.

### Verification strategy
For each test: collect output into `std::vector<vertex_id_t<G>>` via `std::back_inserter`.
- Use `same_set()` to compare against the expected vertex ID list (order-independent).
- Cross-check with the `is_articulation_point()` brute-force helper for small graphs.

