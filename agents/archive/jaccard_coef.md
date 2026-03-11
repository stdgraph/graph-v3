# Implement Jaccard Coefficient Algorithm

## Reference Material
- Spec: /mnt/d/dev_graph/P1709/D3128_Algorithms/tex/algorithms.tex §Jaccard Coefficient (line ~1356)
- Reference signatures: /mnt/d/dev_graph/P1709/D3128_Algorithms/src/jaccard.hpp (lines 4-6)
- Study for implementation pattern: include/graph/algorithm/tc.hpp (neighbor-set enumeration)
- Study for test pattern: tests/algorithms/test_triangle_count.cpp

## Algorithm Summary
For every directed edge `(u, v)` in the graph the Jaccard coefficient is defined as:

$$J(u,v) = \frac{|N(u) \cap N(v)|}{|N(u) \cup N(v)|}$$

where $N(x)$ is the open neighborhood of vertex $x$ (all adjacent vertices, including the
endpoint on the other side of the edge under consideration). The result measures the fraction
of shared neighbors relative to all neighbors of the two endpoint vertices and lies in [0, 1].
The spec places the algorithm in the Link-Analysis section. Time complexity is O(|N|³) in the
worst case (|N| = maximum vertex degree), driven by the neighbor-set intersection step.

## Signatures to Implement
```cpp
template <index_adjacency_list G, typename OutOp, typename T = double>
requires std::invocable<OutOp, vertex_id_t<G>, vertex_id_t<G>, edge_t<G>&, T>
void jaccard_coefficient(G&& g, OutOp out);
```

**Note on `requires` clause:** The reference file uses
`is_invocable_v<OutOp, vertex_id_t<G>&, vertex_id_t<G>&, edge_reference_t<G>, T>` (C++17 style).
For this codebase, use `std::invocable` (C++20 concept style) with `edge_t<G>&` — the codebase
has `adj_list::edge_t<G>` but no `edge_reference_t<G>` alias. The `out` call passes `uid` and
`vid` by value and the edge reference via ref-to-value (`edge_t<G>&`).

## Design Decisions
- **Namespace:** `graph::` — consistent with all other algorithms.
- **Return type:** `void` — results delivered via the `OutOp` callback.
- **Callback semantics (`out`):** Called as `out(uid, vid, uv, val)` for every directed edge
  `(uid, vid)` that exists in the graph. For an undirected graph stored bidirectionally
  (both `{u,v}` and `{v,u}`), `out` is called twice per undirected edge — once as
  `out(u,v,…)` and once as `out(v,u,…)` — both with the same coefficient value since
  $J(u,v) = J(v,u)$. Callers that want per-undirected-edge output should filter for `uid < vid`.
- **Neighborhood definition:** Open neighborhood including the other endpoint.
  `N(u)` is built from all `target_id` values in `edges(g, u)`. Self-loops are excluded (spec:
  "Self-loops? No") — skip any edge where `target == source`.
- **Precomputed neighbor sets:** Before the main loop, build
  `std::vector<std::unordered_set<vid_t>> nbrs(N)` by iterating every vertex's edge list once.
  This costs O(V + E) time and O(V + E) space, but makes per-edge intersection queries O(d_min)
  instead of O(d_u · d_v).
- **Intersection / union computation (per edge):**
  1. `intersect_size` = count elements of `nbrs[uid]` that are also in `nbrs[vid]` by iterating
     the smaller set and probing the larger. O(min(d_u, d_v)).
  2. `union_size = |nbrs[uid]| + |nbrs[vid]| - intersect_size` (inclusion-exclusion). O(1).
  3. `val = (union_size == 0) ? T{0} : static_cast<T>(intersect_size) / static_cast<T>(union_size)`.
     Zero-division guard: union can only be 0 when both neighborhoods are empty, which means
     neither vertex has any edge — but by definition we only visit edges, so `(uid, vid)` exists
     and each neighborhood contains at least the other endpoint. Guard is kept for robustness.
- **Directed graphs:** Spec says "Directed? Yes". The algorithm works on any directed graph;
  the caller is responsible for edge direction semantics.
- **Multi-edges:** Spec says "Multi-edge? No". Parallel edges would cause duplicate insertions
  into the neighbor sets; since `unordered_set` deduplicates, correctness is maintained.
  Document "behaviour is well-defined but callers should prefer simple graphs".
- **Self-loops:** Spec says "Self-loops? No". Skip edges where `target_id == source_id` when
  building neighbor sets and when iterating edges for the outer loop.
- **Header guard:** `#ifndef GRAPH_JACCARD_HPP` / `#define GRAPH_JACCARD_HPP`.
- **`using` declarations:** same set as other algorithms: `index_adjacency_list`, `vertex_id_t`,
  `edge_t`, `vertices`, `edges`, `target_id`, `vertex_id`, `num_vertices`, `find_vertex`.
- **Includes:** `graph/graph.hpp`, `<functional>`, `<unordered_set>`, `<vector>`.
- **T constraints:** `T` must be a floating-point type for meaningful results (0.0–1.0 range).
  Document a `@note` that `T = double` is the recommended default.

## Files to Create/Modify
| Action   | File                                                            | What to do                                                    |
|----------|-----------------------------------------------------------------|---------------------------------------------------------------|
| Create   | `include/graph/algorithm/jaccard.hpp`                           | Full implementation + Doxygen doc                             |
| Modify   | `include/graph/algorithms.hpp`                                  | Add `#include "algorithm/jaccard.hpp"` under Link Analysis    |
| Create   | `tests/algorithms/test_jaccard.cpp`                             | Catch2 tests (see Test Plan)                                  |
| Modify   | `tests/algorithms/CMakeLists.txt`                               | Add `test_jaccard.cpp` to `add_executable(test_algorithms …)` |

## Implementation Steps
1. Read `tex/algorithms.tex` §Jaccard Coefficient (~line 1356) and lines 4–6 of
   `D3128_Algorithms/src/jaccard.hpp` to confirm signature.
2. Study `include/graph/algorithm/tc.hpp` for:
   - File layout: header guard, includes, namespace, using-declarations.
   - Neighbor-enumeration pattern with `edges(g, u)` / `target_id`.
   - Doxygen style: complexity table, supported graph properties, @pre/@post,
     exception safety, example usage.
3. Study `tests/algorithms/test_triangle_count.cpp` for test file structure.
4. Implement `include/graph/algorithm/jaccard.hpp`:
   - File-level Doxygen comment (description, authors, SPDX).
   - Header guard and includes: `graph/graph.hpp`, `<functional>`, `<unordered_set>`, `<vector>`.
   - `using` declarations (same set as other algorithms in the library).
   - Template + `requires` clause matching updated spec signature.
   - Full Doxygen doc block (complexity, supported properties, @tparam, @param,
     @pre, @post, exception safety, example usage).
   - Documentation follows requirements in docs/algorithm_template.md.
   - Algorithm body:
     a. Return immediately if `N == 0`.
     b. Build `std::vector<std::unordered_set<vid_t>> nbrs(N)`:
        for each vertex u, for each edge (u, t): if `t != u`, insert `t` into `nbrs[u]`.
     c. Outer loop over all vertices `u`:
        for each edge `(u, vid)` via `edges(g, u)`:
        - Skip self-loops (`vid == uid`).
        - Compute `intersect_size`: iterate the smaller of `nbrs[uid]`, `nbrs[vid]`
          and count membership in the larger.
        - Compute `union_size = nbrs[uid].size() + nbrs[vid].size() - intersect_size`.
        - Compute `val = (union_size == 0) ? T{0} : static_cast<T>(intersect_size) / static_cast<T>(union_size)`.
        - Call `out(uid, vid, uv, val)`.
5. Add `#include "algorithm/jaccard.hpp"` to `include/graph/algorithms.hpp` under a
   `// Link Analysis` comment (create the section if it doesn't exist).
6. Write `tests/algorithms/test_jaccard.cpp` (see Test Plan).
7. Add `test_jaccard.cpp` to `tests/algorithms/CMakeLists.txt` inside `add_executable()`.
8. Build: `cmake --build build/linux-gcc-debug --target test_algorithms`
9. Run targeted tests: `ctest --test-dir build/linux-gcc-debug -R jaccard`
10. Run full suite: `ctest --test-dir build/linux-gcc-debug` — confirm 0 failures.

## Test Plan

### Helper functions to define
```cpp
/// Collect all callbacks into a map keyed by (uid, vid) pair for lookup.
template <typename G>
using JaccardMap = std::map<std::pair<typename G::vertex_id_type,
                                     typename G::vertex_id_type>, double>;

template <typename G>
JaccardMap<G> collect_jaccard(const G& g) {
  JaccardMap<G> result;
  jaccard_coefficient(g, [&](auto uid, auto vid, auto& /*uv*/, double val) {
    result[{uid, vid}] = val;
  });
  return result;
}

/// Brute-force Jaccard coefficient for a pair of vertices.
template <typename G>
double brute_jaccard(const G& g,
                     typename G::vertex_id_type u,
                     typename G::vertex_id_type v) {
  std::unordered_set<typename G::vertex_id_type> nu, nv;
  auto uu = *find_vertex(g, u);
  for (auto&& e : edges(g, uu)) nu.insert(target_id(g, e));
  auto vv = *find_vertex(g, v);
  for (auto&& e : edges(g, vv)) nv.insert(target_id(g, e));
  size_t isect = 0;
  for (auto x : nu) if (nv.count(x)) ++isect;
  size_t uni = nu.size() + nv.size() - isect;
  return (uni == 0) ? 0.0 : static_cast<double>(isect) / static_cast<double>(uni);
}

/// Near-equal for floating-point.
bool approx_equal(double a, double b, double eps = 1e-9) {
  return std::abs(a - b) < eps;
}
```

### Core test cases
| Test case | Graph | Representative expected values |
|-----------|-------|---------------------------------|
| Empty graph (0 vertices) | — | `out` never called; no crash |
| Single vertex, no edges | 1 vertex | `out` never called |
| Single edge 0↔1 | 2 vertices | J(0,1) = J(1,0) = 0 (N(0)={1}, N(1)={0}, ∩=∅, ∪={0,1}) |
| Path 0-1-2 (bidirectional) | 3 vertices | J(0,1) = J(1,0) = 0; J(1,2) = J(2,1) = 0 (no shared neighbors on a path) |
| Triangle 0-1-2 (bidirectional) | 3 vertices | J(0,1) = J(0,2) = J(1,2) = 1/3 ≈ 0.333… (each pair shares exactly 1 of 3 neighbors) |
| Complete graph K4 (bidirectional) | 4 vertices | J(u,v) = 2/4 = 0.5 for every edge (each pair has 2 shared out of 4 total neighbors) |
| Star graph centre=0, leaves 1-4 (bidirectional) | 5 vertices | J(0,i) = 0 for all leaf i; J(i,0) = 0 (leaves share no common neighbors: N(leaf)={0}, N(0)={1,2,3,4}) |
| "Diamond" graph: K4 minus edge 0-3 | 4 vertices | J(1,2) = 2/4 = 0.5 (share vertices 0 and 3); all others = 1/4 = 0.25 |
| Self-loop ignored: path 0-1-2 with self-loop on 1 | 3 vertices | Same as path 0-1-2 without self-loop |
| Disconnected: triangle 0-1-2 + isolated vertex 3 | 4 vertices | Same Jaccard values as triangle 0-1-2; vertex 3 never appears in any callback |
| `out` invoked once per directed edge | triangle 0-1-2 | exactly 6 callbacks total (once per directed edge) |

### Parameterisation
Run the triangle and K4 cases as `TEMPLATE_TEST_CASE` over `vov_void` and `dov_void` from
`algorithm_test_types.hpp` to confirm container independence.

### Verification strategy
For each test:
1. Collect results via `collect_jaccard()` into a `JaccardMap`.
2. Check the map size equals the number of directed edges in the graph (one callback per
   directed edge, including both (u,v) and (v,u) for bidirectional edges).
3. For each result `(uid, vid, val)`, compare against `brute_jaccard(g, uid, vid)` using
   `approx_equal()` to handle floating-point rounding.
4. Confirm $J(u,v) = J(v,u)$ for all edges stored bidirectionally.
5. Confirm all values lie in [0.0, 1.0].
