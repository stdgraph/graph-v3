<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Implementation Matrix

> **Purpose:** Single source of truth for what is implemented. Generated from the source tree.

</td>
</tr></table>

> All user-facing documentation must reference this file rather than maintaining independent lists.

---

## Table of Contents

- [Algorithms](#algorithms)
- [Views](#views)
- [Adaptors](#adaptors)
- [Containers](#containers)
- [dynamic\_graph Trait Combinations](#dynamic_graph-trait-combinations)
- [Generators](#generators)
- [Graph I/O](#graph-io)
- [Edge List](#edge-list)
- [Adjacency List Infrastructure](#adjacency-list-infrastructure)
- [Top-Level Headers](#top-level-headers)
- [Umbrella Headers](#umbrella-headers)
- [Namespace Organization](#namespace-organization)
- [CMake Consumer Instructions](#cmake-consumer-instructions)

---

## Algorithms

15 algorithm headers in `include/graph/algorithm/` (14 user-facing + 1 shared infrastructure):

> **Note:** `tarjan_scc.hpp` is not included by the `algorithms.hpp` umbrella header — include it directly.

| Algorithm | Header | Test File | Status |
|-----------|--------|-----------|--------|
| Dijkstra shortest paths | `dijkstra_shortest_paths.hpp` | `test_dijkstra_shortest_paths.cpp` | Implemented |
| Bellman-Ford shortest paths | `bellman_ford_shortest_paths.hpp` | `test_bellman_ford_shortest_paths.cpp` | Implemented |
| Breadth-first search | `breadth_first_search.hpp` | `test_breadth_first_search.cpp` | Implemented |
| Depth-first search | `depth_first_search.hpp` | `test_depth_first_search.cpp` | Implemented |
| Topological sort | `topological_sort.hpp` | `test_topological_sort.cpp` | Implemented |
| Connected components (Kosaraju SCC) | `connected_components.hpp` | `test_connected_components.cpp`, `test_scc_bidirectional.cpp` | Implemented |
| Tarjan SCC | `tarjan_scc.hpp` | `test_tarjan_scc.cpp` | Implemented ⚠️ not in umbrella |
| Articulation points | `articulation_points.hpp` | `test_articulation_points.cpp` | Implemented |
| Biconnected components | `biconnected_components.hpp` | `test_biconnected_components.cpp` | Implemented |
| MST (Prim / Kruskal) | `mst.hpp` | `test_mst.cpp` | Implemented |
| Triangle counting | `tc.hpp` | `test_triangle_count.cpp` | Implemented |
| Maximal independent set | `mis.hpp` | `test_mis.cpp` | Implemented |
| Label propagation | `label_propagation.hpp` | `test_label_propagation.cpp` | Implemented |
| Jaccard coefficient | `jaccard.hpp` | `test_jaccard.cpp` | Implemented |

---

## Views

8 user-facing view headers in `include/graph/views/`:

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

Infrastructure headers (not user-facing views): `view_concepts.hpp`, `basic_views.hpp`, `adaptors.hpp`, `search_base.hpp`, `edge_accessor.hpp`.

---

## Adaptors

2 graph adaptors in `include/graph/adaptors/`:

| Adaptor | Header | Description |
|---------|--------|-------------|
| `filtered_graph` | `adaptors/filtered_graph.hpp` | Non-owning wrapper filtering vertices/edges by predicate |
| BGL graph adaptor | `adaptors/bgl/graph_adaptor.hpp` | Adapts Boost.Graph types for use with graph-v3 |

Supporting headers:
- `adaptors/bgl/bgl_edge_iterator.hpp` — C++20 iterator wrapper for BGL iterators
- `adaptors/bgl/property_bridge.hpp` — Bridge BGL property maps to graph-v3 value functions



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

## Generators

4 graph generators in `include/graph/generators/`:

| Generator | Header | Description |
|-----------|--------|-------------|
| Path graph | `generators/path.hpp` | Linear chain: 0 → 1 → … → n-1 |
| Grid graph | `generators/grid.hpp` | 2D lattice with right/down edges |
| Erdős–Rényi | `generators/erdos_renyi.hpp` | Random G(n, p) model |
| Barabási–Albert | `generators/barabasi_albert.hpp` | Preferential attachment (scale-free) |

Test file: `tests/generators/test_generators.cpp`

---

## Graph I/O

3 I/O formats in `include/graph/io/`:

| Format | Header | Writer | Reader | Reader return type | Description |
|--------|--------|--------|--------|-------------------|-------------|
| DOT (GraphViz) | `io/dot.hpp` | `write_dot()` | `read_dot()` | `dot_graph` | Most common graph visualization format |
| GraphML (XML) | `io/graphml.hpp` | `write_graphml()` | `read_graphml()` | `graphml_graph` | XML-based graph interchange |
| JSON | `io/json.hpp` | `write_json()` | `read_json()` | `json_graph` | JGF-inspired JSON format |

> Readers return a format-specific intermediate structure (`dot_graph`, `graphml_graph`, `json_graph`)
> rather than populating a graph container directly. Callers convert the structure into a container of their choice.

Shared utilities: `io/detail/common.hpp` (concepts: `formattable`, `has_vertex_value`, `has_edge_value`)

Test file: `tests/io/test_io.cpp`

---

## Edge List

3 headers in `include/graph/edge_list/`:

| Header | Description |
|--------|-------------|
| `edge_list/edge_list_descriptor.hpp` | `edge_descriptor<VId, EV>` — descriptor for a single (source, target, value) edge |
| `edge_list/edge_list_traits.hpp` | `edge_list_traits<EL>` — type aliases (`vertex_id_t`, `edge_value_t`, etc.) for edge-list containers |
| `edge_list/edge_list.hpp` | Umbrella: includes both of the above |

Test files: `tests/edge_list/test_edge_list_concepts.cpp`, `test_edge_list_cpo.cpp`, `test_edge_list_descriptor.cpp`, `test_edge_list_integration.cpp`

---

## Adjacency List Infrastructure

11 headers in `include/graph/adj_list/` providing the descriptor-based CPO machinery:

| Header | Description |
|--------|-------------|
| `adj_list/descriptor.hpp` | Core descriptor concepts, `out_edge_tag`, `in_edge_tag`, base descriptor traits |
| `adj_list/descriptor_traits.hpp` | `is_vertex_descriptor_v`, `is_edge_descriptor_v` type traits |
| `adj_list/vertex_descriptor.hpp` | `vertex_descriptor<Iter>` — wraps an iterator with a `vertex_id()` method |
| `adj_list/vertex_descriptor_view.hpp` | `vertex_descriptor_view<Iter>` — range adaptor that synthesizes vertex descriptors over a container |
| `adj_list/vertex_property_map.hpp` | `vertex_property_map<G, T>` — O(1) per-vertex property storage (index-based graphs) |
| `adj_list/edge_descriptor.hpp` | `edge_descriptor<EIter, VIter, Dir>` — wraps edge and vertex iterators with source/target access |
| `adj_list/edge_descriptor_view.hpp` | `edge_descriptor_view` — range adaptor that synthesizes edge descriptors |
| `adj_list/adjacency_list_concepts.hpp` | Concepts: `adjacency_list`, `index_adjacency_list`, `bidirectional_adjacency_list`, `mapped_adjacency_list`, etc. |
| `adj_list/adjacency_list_traits.hpp` | Type aliases: `vertex_t<G>`, `edge_t<G>`, `vertex_id_t<G>`, `vertex_range_t<G>`, etc. |
| `adj_list/graph_utility.hpp` | Utility: `num_vertices`, `num_edges`, and related convenience wrappers |
| `adj_list/detail/graph_cpo.hpp` | All CPO definitions: `vertices`, `out_edges`, `in_edges`, `find_vertex`, `source_id`, `target_id`, `vertex_id`, `edge_value`, `vertex_value`, `graph_value`, `degree`, `find_vertex_edge`, `contains_edge`, and partition CPOs |

---

## Top-Level Headers

| Header | Description |
|--------|-------------|
| `graph/graph.hpp` | Core umbrella: adj_list infrastructure, edge_list types, views, `graph_data.hpp`, `graph_concepts.hpp` |
| `graph/graph_concepts.hpp` | `vertex_value_function<VVF,G,V>` and `edge_value_function<EVF,G,E>` concepts used by views and algorithms |
| `graph/graph_data.hpp` | `graph_error`, `vertex_data<VId,V,VV>`, `edge_data<VId,Sourced,E,EV>` — value aggregates returned by views |
| `graph/views.hpp` | All 8 view headers + infrastructure |
| `graph/algorithms.hpp` | 13 of 14 algorithm headers (excludes `tarjan_scc.hpp`) |
| `graph/generators.hpp` | All 4 generator headers |
| `graph/io.hpp` | All 3 I/O format headers |

---

## Umbrella Headers

| Header | Includes | Notes |
|--------|----------|-------|
| `graph/graph.hpp` | `adj_list/` infrastructure, `edge_list/`, views, `graph_data.hpp`, `graph_concepts.hpp` | Does **not** include containers, algorithms, generators, or I/O |
| `graph/views.hpp` | All 8 view headers + infrastructure | — |
| `graph/algorithms.hpp` | 13 of 14 algorithm headers | `tarjan_scc.hpp` must be included separately |
| `graph/generators.hpp` | All 4 generator headers | — |
| `graph/io.hpp` | All 3 I/O format headers | — |

---

## Namespace Organization

| Namespace | Purpose |
|-----------|---------|
| `graph::` | Root — common edge list CPOs, re-exports `adj_list` types/CPOs for convenience |
| `graph::adj_list::` | Adjacency list CPOs, descriptors, concepts, traits |
| `graph::edge_list::` | Edge list concepts, traits, descriptors |
| `graph::views::` | Graph views (vertexlist, edgelist, neighbors, BFS, DFS, etc.) |
| `graph::container::` | Concrete graph containers |
| `graph::generators::` | Synthetic graph generators |
| `graph::io::` | Graph I/O (DOT, GraphML, JSON) |
| `graph::detail::` | Internal implementation details |

---

## CMake Consumer Instructions

```cmake
find_package(graph3 REQUIRED)
target_link_libraries(your_target PRIVATE graph::graph3)
```
