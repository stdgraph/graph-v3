# Changelog

## [Unreleased]

### Added
- **`filtered_graph` adaptor** (`adaptors/filtered_graph.hpp`) — non-owning wrapper that filters vertices and edges by predicate during traversal. Satisfies `adjacency_list` so all views and algorithms work transparently on the filtered subgraph. Uses self-contained `filtering_iterator` to avoid `std::views::filter` iterator dangling issues.
- **BGL graph adaptor** (`adaptors/bgl/graph_adaptor.hpp`) — adapts Boost.Graph types for use with graph-v3 CPOs, views, and algorithms. Includes `bgl_edge_iterator.hpp` (C++20 iterator wrapper) and `property_bridge.hpp` (BGL property maps → graph-v3 value functions). Supports adjacency_list, CSR, and bidirectional BGL graphs.
- `graph::adaptors::keep_all` — sentinel predicate (accepts everything, zero-overhead)
- `graph::adaptors::filtering_iterator` — self-contained forward iterator with predicate stored in `std::optional` (custom assignment for lambda non-assignability)
- `graph::adaptors::filtered_vertices(fg)` — lazy filtered vertex iteration convenience function
- `graph::bgl::graph_adaptor` with CTAD, `graph_bgl_adl` namespace for ADL dispatch
- `graph::bgl::make_bgl_edge_weight_fn`, `make_vertex_id_property_fn` — property bridge factories
- 5 filtered_graph tests + 34 BGL adaptor test cases (107 assertions)
- **Indexed d-ary heap for Dijkstra** (`detail/indexed_dary_heap.hpp`) — opt-in O(V)-bounded heap with true decrease-key, parameterized by arity. Selected via the new `use_indexed_dary_heap<Arity>` heap-selector tag on `dijkstra_shortest_paths` / `dijkstra_shortest_distances`. Supports both dense graphs (via `vector_position_map`) and mapped / hashable-vertex-id graphs (via `assoc_position_map`).
- `use_default_heap` and `use_indexed_dary_heap<Arity>` heap-selector tags. **`use_default_heap` remains the default** — it wins on grid (E/V≈4) and path (E/V=1) workloads. Use `use_indexed_dary_heap<8>` for high-E/V random / scale-free graphs on `compressed_graph`, where Phase 4 benchmarks measured −25% (Erdős–Rényi) and −17% (Barabási–Albert) at 100K vertices vs. the default. See `agents/indexed_dary_heap_results.md` for full numbers.
- `vector_position_map` / `assoc_position_map` adapters (`detail/heap_position_map.hpp`) used by the indexed heap.
- **Tarjan's SCC algorithm** (`tarjan_scc.hpp`) — single-pass O(V+E) strongly connected components using iterative DFS with low-link values; no transpose graph needed
- 17 new Tarjan SCC tests (`test_tarjan_scc.cpp`)
- **Mapped (sparse) graph algorithm support** — all 14 algorithms now accept `adjacency_list<G>` (both index and map-based containers)
- `mapped_vertex_range`, `mapped_adjacency_list`, `mapped_bidirectional_adjacency_list` concepts
- `vertex_property_map<G, T>` type alias and `make_vertex_property_map` factory (vector for index graphs, unordered_map for mapped)
- `vertex_property_map_for<M, G>` concept for algorithm parameter constraints
- `vertex_property_map_value_t<Container>` trait for extracting per-vertex value types
- `is_sparse_vertex_container_v<G>` trait for compile-time graph type dispatch
- Map-based graph test fixtures (`map_graph_fixtures.hpp`) with sparse vertex IDs
- 456 new algorithm tests for sparse graph types (4343 → 4799)
- **Graph I/O** (`<graph/io/>`) — read/write support for three formats:
  - `write_dot(os, g)` / `write_dot(os, g, vertex_attr_fn, edge_attr_fn)` — DOT/GraphViz writer; auto-formats vertex/edge values when `std::formattable`, emits user-supplied attribute strings otherwise (`io/dot.hpp`)
  - `read_dot(is) -> dot_graph` — DOT parser returning a lightweight `dot_graph` structure with string-keyed vertex/edge attributes (`io/dot.hpp`)
  - `write_graphml(os, g)` / `write_graphml(os, g, vertex_properties_fn, edge_properties_fn)` — GraphML (XML) writer with optional property key declarations (`io/graphml.hpp`)
  - `read_graphml(is) -> graphml_graph` — GraphML parser returning a `graphml_graph` structure with per-vertex/per-edge attribute maps (`io/graphml.hpp`)
  - `write_json(os, g, indent = 2)` / `write_json(os, g, vertex_attr_fn, edge_attr_fn, indent)` — JSON writer; `indent=0` for compact single-line output (`io/json.hpp`)
  - `read_json(is) -> json_graph` — JSON parser returning a `json_graph` structure with string-valued attribute maps (`io/json.hpp`)
  - 19 I/O tests covering write, read, and roundtrip for all three formats
- **Graph generators** (`<graph/generators/>`) — produce `edge_list<VId, EV>` sorted by source, loadable into any container via `load_edges()`:
  - `erdos_renyi(n, p, seed, dist)` — G(n,p) random directed graph using the Batagelj–Brandes geometric-skip algorithm; O(E) time (`generators/erdos_renyi.hpp`)
  - `grid_2d(rows, cols, seed, dist)` — bidirectional 4-connected grid graph; E/V ≈ 4 (`generators/grid.hpp`)
  - `barabasi_albert(n, m, seed, dist)` — preferential-attachment scale-free graph; E/V ≈ 2m (`generators/barabasi_albert.hpp`)
  - `path_graph(n, seed, dist)` — directed path 0 → 1 → … → n−1; E = n−1 (`generators/path.hpp`)
  - `weight_dist` enum (`uniform` U[1,100], `exponential` Exp(0.1)+1, `constant_one`) — passed to all generators (`generators/common.hpp`)
  - `edge_list<VId, EV>` / `edge_entry<VId, EV>` type aliases; all generators accept a `VId` template parameter (default `uint32_t`) for large graphs (`generators/common.hpp`)
  - 6 generator tests covering basic properties, weight distributions, and `uint64_t` vertex IDs

### Changed
- **`edge_descriptor` simplified to iterator-only storage** — removed the `conditional_t<random_access_iterator, size_t, EdgeIter>` dual-storage path; edges always store the iterator directly since edges always have physical containers. Eliminates 38 `if constexpr` branches across 6 files (~500 lines removed).
- **`compressed_graph::vertices(g)` returns `iota_view`** — simplified to `std::ranges::iota_view<size_t, size_t>(0, num_vertices())`, which the `vertices` CPO wraps automatically via `_wrap_if_needed`.
- **`vertex_descriptor_view` CTAD deduction guides** — updated from `Container::iterator`/`const_iterator` to `std::ranges::iterator_t<>` for compatibility with views like `iota_view`.
- **`edge_descriptor_view` forward_list compatibility** — fixed constructor to use `if constexpr` for `sized_range` check so `std::ranges::size()` is not compiled for non-sized ranges like `forward_list`.
- All algorithms relaxed from `index_adjacency_list<G>` to `adjacency_list<G>`
- Algorithm internal arrays use `make_vertex_property_map` (vector or unordered_map depending on graph type)
- User-facing `Distances`, `Predecessors`, `Weight`, `Component`, `Label` parameters accept vertex property maps
- Index-based for-loops replaced with `views::basic_vertexlist(g)` iteration
- Validation uses `if constexpr (index_vertex_range<G>)` for size checks, `find_vertex` for mapped graphs

---

## [0.5.0] - 2026-03-01 *(Initial Beta)*

### Added
- **Bidirectional edge access** — `in_edges`, `in_degree`, `find_in_edge`, `contains_in_edge` CPOs
- `bidirectional_adjacency_list` concept
- `in_edge_range_t<G>`, `in_edge_iterator_t<G>`, `in_edge_t<G>` type aliases
- `out_edge_accessor` / `in_edge_accessor` stateless edge-access policies (`edge_accessor.hpp`)
- `in_incidence` and `in_neighbors` views with pipe-syntax adaptors
- Accessor template parameter on BFS, DFS, and topological sort views for reverse traversal
- `dynamic_graph` `Bidirectional` template parameter for incoming-edge lists
- `undirected_adjacency_list` bidirectional support (incoming = outgoing)
- Kosaraju SCC algorithm and `transpose_graph` view
- 144 new tests (4261 → 4405)
- New documentation: bidirectional access tutorial, updated views/concepts/CPO/container docs

### Documentation
- Complete documentation reorganization: user guide, reference, contributor docs
- New README with badges, compiler table, feature highlights
- Separated adjacency list and edge list documentation
- Full container documentation including 26 dynamic_graph trait combinations
- Consolidated CPO implementation guide (from 3140 lines → ~970)
- Created FAQ, migration guide, getting started, and examples pages
- Fixed algorithms.hpp umbrella header (6 broken includes)
- Archived stale design documents
- Added canonical metrics tracking (docs/status/metrics.md)
