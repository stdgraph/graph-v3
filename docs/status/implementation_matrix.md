<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Implementation Matrix

> **Purpose:** Single source of truth for what is implemented. Generated from the source tree.

</td>
</tr></table>

> All user-facing documentation must reference this file rather than maintaining independent lists.

---

## Algorithms

13 implemented algorithms in `include/graph/algorithm/` (excluding `traversal_common.hpp`):

| Algorithm | Header | Test File | Status |
|-----------|--------|-----------|--------|
| Dijkstra shortest paths | `dijkstra_shortest_paths.hpp` | `test_dijkstra_shortest_paths.cpp` | Implemented |
| Bellman-Ford shortest paths | `bellman_ford_shortest_paths.hpp` | `test_bellman_ford_shortest_paths.cpp` | Implemented |
| Breadth-first search | `breadth_first_search.hpp` | `test_breadth_first_search.cpp` | Implemented |
| Depth-first search | `depth_first_search.hpp` | `test_depth_first_search.cpp` | Implemented |
| Topological sort | `topological_sort.hpp` | `test_topological_sort.cpp` | Implemented |
| Connected components (Kosaraju SCC) | `connected_components.hpp` | `test_connected_components.cpp`, `test_scc_bidirectional.cpp` | Implemented |
| Articulation points | `articulation_points.hpp` | `test_articulation_points.cpp` | Implemented |
| Biconnected components | `biconnected_components.hpp` | `test_biconnected_components.cpp` | Implemented |
| MST (Prim / Kruskal) | `mst.hpp` | `test_mst.cpp` | Implemented |
| Triangle counting | `tc.hpp` | `test_triangle_count.cpp` | Implemented |
| Maximal independent set | `mis.hpp` | `test_mis.cpp` | Implemented |
| Label propagation | `label_propagation.hpp` | `test_label_propagation.cpp` | Implemented |
| Jaccard coefficient | `jaccard.hpp` | `test_jaccard.cpp` | Implemented |

---

## Views

10 user-facing views in `include/graph/views/` (excluding infrastructure headers):

| View | Header | Category |
|------|--------|----------|
| vertexlist | `vertexlist.hpp` | Basic |
| edgelist | `edgelist.hpp` | Basic |
| incidence / out_incidence / in_incidence | `incidence.hpp` | Basic |
| neighbors / in_neighbors | `neighbors.hpp` | Basic |
| BFS (vertices + edges) | `bfs.hpp` | Search |
| DFS (vertices + edges) | `dfs.hpp` | Search |
| Topological sort (vertices + edges) | `topological_sort.hpp` | Search |
| Transpose adaptor | `transpose.hpp` | Adaptor |

Infrastructure headers (not user views): `view_concepts.hpp`, `adaptors.hpp`, `basic_views.hpp`, `search_base.hpp`, `edge_accessor.hpp`.

---

## Containers

3 graph containers in `include/graph/container/`:

| Container | Header | Mutability | Description |
|-----------|--------|------------|-------------|
| `dynamic_graph` | `dynamic_graph.hpp` | Mutable | General-purpose, traits-configured vertex and edge containers |
| `compressed_graph` | `compressed_graph.hpp` | Immutable after construction | CSR (compressed sparse row) for high-performance read-only access |
| `undirected_adjacency_list` | `undirected_adjacency_list.hpp` | Mutable | Dual doubly-linked lists; O(1) edge removal |

Utility header: `container_utility.hpp`.

---

## `dynamic_graph` Trait Combinations

27 combinations in `include/graph/container/traits/`:

### Vertex containers

| Container | Iterator | Vertex ID | Abbreviation |
|-----------|----------|-----------|--------------|
| `std::vector` | Random access | Integral index | `v` |
| `std::deque` | Random access | Integral index | `d` |
| `std::map` | Bidirectional | Ordered key | `m` |
| `std::unordered_map` | Forward | Hashable key | `u` |

### Edge containers

| Container | Iterator | Properties | Abbreviation |
|-----------|----------|------------|--------------|
| `std::vector` | Random access | Cache-friendly, allows duplicates | `v` |
| `std::deque` | Random access | Efficient front/back insertion | `d` |
| `std::forward_list` | Forward | Minimal memory overhead | `fl` |
| `std::list` | Bidirectional | O(1) insertion/removal anywhere | `l` |
| `std::set` | Bidirectional | Sorted, deduplicated | `s` |
| `std::unordered_set` | Forward | Hash-based, O(1) avg lookup | `us` |
| `std::map` | Bidirectional | Sorted by target_id key | `m` |
| `std::unordered_map` | Forward | Hash-based, O(1) avg lookup by target_id | `um` |

### All 27 trait files

Naming convention: `{vertex}o{edge}_graph_traits.hpp`

| # | Traits file | Vertex | Edge |
|---|-------------|--------|------|
| 1 | `vov_graph_traits.hpp` | vector | vector |
| 2 | `vod_graph_traits.hpp` | vector | deque |
| 3 | `vofl_graph_traits.hpp` | vector | forward_list |
| 4 | `vol_graph_traits.hpp` | vector | list |
| 5 | `vos_graph_traits.hpp` | vector | set |
| 6 | `vous_graph_traits.hpp` | vector | unordered_set |
| 7 | `vom_graph_traits.hpp` | vector | map |
| 8 | `voum_graph_traits.hpp` | vector | unordered_map |
| 9 | `dov_graph_traits.hpp` | deque | vector |
| 10 | `dod_graph_traits.hpp` | deque | deque |
| 11 | `dofl_graph_traits.hpp` | deque | forward_list |
| 12 | `dol_graph_traits.hpp` | deque | list |
| 13 | `dos_graph_traits.hpp` | deque | set |
| 14 | `dous_graph_traits.hpp` | deque | unordered_set |
| 15 | `mov_graph_traits.hpp` | map | vector |
| 16 | `mod_graph_traits.hpp` | map | deque |
| 17 | `mofl_graph_traits.hpp` | map | forward_list |
| 18 | `mol_graph_traits.hpp` | map | list |
| 19 | `mos_graph_traits.hpp` | map | set |
| 20 | `mous_graph_traits.hpp` | map | unordered_set |
| 21 | `mom_graph_traits.hpp` | map | map |
| 22 | `uov_graph_traits.hpp` | unordered_map | vector |
| 23 | `uod_graph_traits.hpp` | unordered_map | deque |
| 24 | `uofl_graph_traits.hpp` | unordered_map | forward_list |
| 25 | `uol_graph_traits.hpp` | unordered_map | list |
| 26 | `uos_graph_traits.hpp` | unordered_map | set |
| 27 | `uous_graph_traits.hpp` | unordered_map | unordered_set |

---

## Umbrella Headers

| Header | Includes | Status |
|--------|----------|--------|
| `graph/graph.hpp` | Core types, concepts, traits, views, containers | Verified |
| `graph/views.hpp` | All views + CPOs | Verified |
| `graph/algorithms.hpp` | All 13 algorithm headers | Verified (fixed Phase 0) |

---

## Namespace Organization

| Namespace | Purpose |
|-----------|---------|
| `graph::` | Root — common edge list CPOs, re-exports `adj_list` types/CPOs for convenience |
| `graph::adj_list::` | Adjacency list CPOs, descriptors, concepts, traits |
| `graph::edge_list::` | Edge list concepts, traits, descriptors |
| `graph::views::` | Graph views (vertexlist, edgelist, neighbors, BFS, DFS, etc.) |
| `graph::container::` | Concrete graph containers |
| `graph::detail::` | Internal implementation details |

---

## CMake Consumer Instructions

```cmake
find_package(graph3 REQUIRED)
target_link_libraries(your_target PRIVATE graph::graph3)
```
