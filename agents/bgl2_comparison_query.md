# Comparison with modernized boost graph library (BGL2)

The original boost graph library (BGL1, located at `/home/phil/dev_graph/boost/libs/graph/`) has been 
modernized to use C++20 (`BGL2`, located at `/home/phil/dev_graph/boost/libs/graph/modern/`).

**BGL2 Container Selectors (Now Fully Implemented):**

BGL2 now supports 10 container selectors defined in `container_selectors.hpp`:

| Selector | Container | Characteristics |
|----------|-----------|-----------------|
| `vecS` | `std::vector` | O(1) random access, index-based descriptors |
| `listS` | `std::list` | O(1) insertion/removal, stable iterator-based descriptors |
| `setS` | `std::set` | O(log n), ordered unique elements, stable iterators |
| `mapS` | `std::set` (for edges) | O(log n), prevents parallel edges, stable iterators |
| `multisetS` | `std::multiset` | O(log n), ordered with duplicates allowed |
| `multimapS` | `std::multiset` (for edges) | O(log n), duplicate keys allowed |
| `hash_setS` | `std::unordered_set` | O(1) average, hash-based unique, **unstable iterators** |
| `hash_mapS` | `std::unordered_set` (for edges) | O(1) average, hash-based, **unstable iterators** |
| `hash_multisetS` | `std::unordered_multiset` | O(1) average, duplicates allowed, **unstable iterators** |
| `hash_multimapS` | `std::unordered_multiset` (for edges) | O(1) average, duplicate keys, **unstable iterators** |

> **Note on "map" selectors:** Despite having "map" in their names, `mapS`/`multimapS`/`hash_mapS`/`hash_multimapS` all map to **set** containers (not `std::map`). The "map" name reflects the vertex→edge *mapping semantic*, not the container type. The uniqueness/ordering behavior of the underlying set provides the desired parallel-edge control.

The `adjacency_list.hpp` provides implementations for:
- **Vertex list selectors:** `vecS` (index-based), `listS` (iterator-based), `setS` (iterator-based)
- **Out-edge list selectors:** All 10 selectors work for edge containers

**Key BGL2 Design Features:**
- Selector concepts: `ContainerSelector`, `SequenceSelector`, `AssociativeSelector`, `UnorderedSelector`, `StableIteratorSelector`, `RandomAccessSelector`
- Selector traits (per-selector `static constexpr bool`): `is_random_access`, `has_stable_iterators`, `is_ordered`, `is_unique`
- `container_gen<Selector, ValueType>` maps selectors to container types
- `parallel_edge_category_for<OutEdgeListS>` determines if parallel edges are allowed (based on `is_unique`)

**BGL2 Additional Graph Containers:**
- `adjacency_matrix.hpp` — dense adjacency matrix representation (fixed vertex count, O(V²) space, O(1) edge lookup)
- `compressed_sparse_row_graph.hpp` — CSR (compressed sparse row) graph (static, cache-efficient)
- `grid_graph.hpp` — N-dimensional implicit grid graph (O(1) space, used in image processing / scientific computing)

**BGL2 Additional Design Features:**
- `strong_descriptor.hpp` — type-safe `descriptor<Tag>` wrapper preventing vertex/edge descriptor mix-ups at compile time
- `validation.hpp` — `validate_graph()` framework checking dangling edges, self-loops, parallel edges, bidirectional consistency, etc.
- `algorithm_params.hpp` / `algorithm_result.hpp` — named parameter structs (C++20 designated initializers) and rich result types (e.g., `dijkstra_result` with `path_to()`, `is_reachable()`)
- `visitor_callbacks.hpp` — 9-event BFS/DFS callback structs with `null_callback` defaults and single-event wrappers
- `composable_algorithms.hpp` — range adaptors: `find_components()`, `find_distances_from()`, `find_k_core()`, `find_neighbors()`, `find_common_neighbors()`, `degree_distribution()`
- `parallel_algorithms.hpp` — `parallel_bfs()` (level-synchronous), `parallel_connected_components()` (Shiloach-Vishkin), `parallel_for_each_vertex/edge()`
- `coroutine_traverse.hpp` — lazy coroutine-based BFS/DFS: `bfs_traverse()`, `dfs_traverse()`, multi-source and all-components variants
- `MutableGraph` concept — individual `add_vertex()`, `remove_vertex()`, `add_edge()`, `remove_edge()`

**Graph-v3 Key Design Features (for comparison context):**
- Concept-based CPO design — any type satisfying graph concepts works with algorithms (no adaptor class needed). A `vector<vector<int>>` or user-defined container works directly.
- Trait-based container configuration (27 trait combinations: `vov`, `vofl`, `mol`, `dov`, etc.) vs BGL2's selector-based approach
- 3 graph containers: `dynamic_graph` (mutable adjacency list), `compressed_graph` (CSR), `undirected_adjacency_list` (dual-list with O(1) edge removal from both endpoints)
- `edge_list` module — standalone edge list support for `pair<T,T>`, `tuple<T,T,EV,...>`, and `edge_data` structs
- Edge descriptors always store the edge iterator directly (no conditional `size_t`/iterator storage)
- Vertex descriptors use `size_t` for index-based graphs, iterator otherwise
- 13 view headers — lazy range adaptors including traversal views (`vertices_dfs()`, `edges_bfs()`, `vertices_topological_sort()`) and pipe syntax (`g | vertexlist() | ...`)
- `dynamic_graph` mutations are batch-oriented (`load_edges()`, `load_vertices()`) — no individual `add_vertex()` / `remove_vertex()` / `add_edge()` / `remove_edge()`
- `vertex_property_map<G, T>` — `vector<T>` for index graphs, `unordered_map<VId, T>` for mapped graphs, with `container_value_fn()` wrapping for algorithm compatibility
- Visitor-based algorithms: BFS/DFS support `on_discover_vertex`, `on_examine_edge`, `on_tree_edge`, `on_back_edge`, etc.

**Algorithm Inventory:**

| Algorithm | BGL2 | Graph-v3 |
|-----------|------|----------|
| BFS | ✅ | ✅ |
| DFS | ✅ (edge classification) | ✅ (edge classification) |
| Dijkstra shortest paths | ✅ | ✅ (multi-source) |
| Bellman-Ford shortest paths | ✅ (negative cycle detection) | ✅ (negative cycle detection) |
| Topological sort | ✅ | ✅ (full-graph, single-source, multi-source) |
| Connected components | ✅ | ✅ (DFS-based + afforest) |
| MST (Kruskal + Prim) | ✅ (separate headers) | ✅ (combined mst.hpp, includes in-place Kruskal) |
| Strongly connected components | ✅ (Tarjan) | ✅ (Kosaraju, 2 overloads: transpose or bidirectional) |
| Triangle counting | — | ✅ (undirected + directed) |
| Articulation points | — | ✅ (iterative Hopcroft-Tarjan) |
| Biconnected components | — | ✅ (Hopcroft-Tarjan with edge stack) |
| Jaccard similarity | — | ✅ (per-edge coefficient) |
| Label propagation | — | ✅ (community detection) |
| Maximum independent set | — | ✅ (greedy) |
| Composable algorithms | ✅ (k-core, degree dist, etc.) | — |
| Coroutine traversal | ✅ (lazy BFS/DFS generators) | — |
| Parallel algorithms | ✅ (parallel BFS, parallel CC) | — |

Compare and contrast BGL2 with the current library (graph-v3), making sure that the following topics are covered:
- The differences between the vertex and edge descriptors.
- The ability to adapt to pre-existing graph data structures.
- Strengths and weaknesses in the capabilities of each library.
- Container flexibility: BGL2's selector-based approach vs graph-v3's trait-based approach.
- Other areas of interest in the design and flexibility for the libraries.

Compare and contrast the algorithms implemented in each library.

Replace the contents of `agents/bgl2_comparison_result.md` with the result.
