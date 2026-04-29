# BGL → graph-v3 Upgrade Strategy

A comprehensive analysis of the Boost Graph Library (BGL) and graph-v3, identifying gaps, migration paths, and recommended extensions to enable a smooth upgrade transition.

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Architectural Comparison](#2-architectural-comparison)
3. [Graph Container Mapping](#3-graph-container-mapping)
4. [Concept & Type System Comparison](#4-concept--type-system-comparison)
5. [Property System Migration](#5-property-system-migration)
6. [Algorithm Gap Analysis](#6-algorithm-gap-analysis)
7. [Visitor Pattern Migration](#7-visitor-pattern-migration)
8. [Graph Adaptors & Views](#8-graph-adaptors--views)
9. [Graph I/O & Serialization](#9-graph-io--serialization)
10. [Graph Generators](#10-graph-generators)
11. [API Pattern Migration Guide](#11-api-pattern-migration-guide)
12. [Adapting an Existing BGL Graph for graph-v3](#12-adapting-an-existing-bgl-graph-for-graph-v3)
13. [Recommended Extensions & Roadmap](#13-recommended-extensions--roadmap)

---

## 1. Executive Summary

graph-v3 is a ground-up C++20 redesign targeting ISO standardization (P3126–P3131, P3337). It replaces BGL's iterator-pair/tag-dispatch/property-list architecture with CPOs, ranges, concepts, and structured bindings. The redesign is fundamentally cleaner but still covers only a modest fraction of classic BGL's total algorithm surface area and lacks several adaptors, I/O formats, and generators that BGL users rely on.

**Key strengths of graph-v3 over BGL:**
- Zero-config adaptation of standard containers (`vector<vector<int>>` is a valid graph)
- C++20 concepts replace Boost Concept Checks — better error messages, compile-time safety
- CPO-based access eliminates the need for `graph_traits` specialization
- Structured bindings for ergonomic iteration (`for (auto [uid, u] : vertexlist(g))`)
- Internal color/visited tracking — users never manage color maps
- `vertex_property_map` auto-selects `vector` vs `unordered_map` based on graph type
- Lazy search views (`vertices_bfs`, `edges_dfs`) composable with `std::views`
- `transpose_view` is a zero-cost adaptor (no wrapper descriptor types)

**Key gaps requiring attention for BGL migration:**
- Dozens of missing algorithms across flow, matching, coloring, planarity, isomorphism, centrality, layout, and related areas
- No `subgraph` hierarchy with descriptor mapping
- No graph I/O (DIMACS, METIS)
- Graph generators partially implemented (Erdős-Rényi, Barabási-Albert, grid, path available; Watts-Strogatz, R-MAT still missing)
- `dynamic_graph` lacks individual mutation (`add_vertex`, `add_edge`, `remove_vertex`, `remove_edge`)
- No `adjacency_matrix` container
- No `copy_graph` utility with cross-type and property mapping support
- No `labeled_graph` adaptor (string labels → vertex mapping)
- No named parameter interface (BGL users must learn new positional API)

---

## 2. Architectural Comparison

| Aspect | BGL | graph-v3 |
|--------|-----|----------|
| **C++ standard** | C++03/11 (Boost dependencies) | C++20 (no Boost) |
| **Type introspection** | `graph_traits<G>` specialization | CPO return-type deduction (`vertex_id_t<G>`, `edge_t<G>`) |
| **Concept checking** | `BOOST_CONCEPT_ASSERT` macros | C++20 `concept` constraints |
| **Graph access** | Free functions returning iterator pairs | CPOs returning ranges |
| **Iteration** | `tie(vi, vi_end) = vertices(g); for (; vi != vi_end; ++vi)` | `for (auto [uid, u] : vertexlist(g))` |
| **Vertex identity** | `vertex_descriptor` (container-dependent type) | `vertex_t<G>` is the descriptor-like handle; `vertex_id_t<G>` is the key/index |
| **Edge identity** | `edge_descriptor` (opaque struct) | `edge_t<G>` plus CPO accessors such as `source_id()` / `target_id()` |
| **Property access** | `property_map<G, tag>`, `get(tag, g)`, `g[v].field` | `vertex_value(g, u)`, `edge_value(g, uv)` CPOs |
| **Algorithm params** | Named parameter chains (`weight_map(w).visitor(v)`) | Direct function parameters with defaults |
| **Capability dispatch** | `traversal_category` tag hierarchy | C++20 concept constraints |
| **External adaptation** | Specialize `graph_traits<G>` + free functions | Implement CPOs (member, ADL, or default fallback) |
| **Directedness** | `directedS`/`undirectedS`/`bidirectionalS` selector | Separate types: `dynamic_graph` (directed/bidir), `undirected_adjacency_list` |

### Namespace Mapping

| BGL | graph-v3 |
|-----|----------|
| `boost::` | `graph::` |
| `boost::graph_traits<G>::` | `graph::vertex_id_t<G>`, `graph::edge_t<G>`, etc. |
| `boost::property_map<G, Tag>::type` | No single replacement; common cases use `graph::vertex_property_map<G, T>` or custom callable accessors |
| `boost::adjacency_list<...>` | `graph::dynamic_graph<...>` or `graph::undirected_adjacency_list<...>` |

---

## 3. Graph Container Mapping

### Direct Equivalents

| BGL Container | graph-v3 Equivalent | Notes |
|---------------|---------------------|-------|
| `adjacency_list<vecS, vecS, directedS, VP, EP>` | `dynamic_graph<EV, VV, GV, VId, false, vov_graph_traits>` | Closest built-in vector/vector analogue; `dynamic_graph` still lacks individual `add_edge` / `remove_edge` |
| `adjacency_list<vecS, vecS, bidirectionalS, VP, EP>` | `dynamic_graph<EV, VV, GV, VId, true, vov_graph_traits>` | Closest built-in vector/vector bidirectional analogue |
| `adjacency_list<vecS, vecS, undirectedS, VP, EP>` | `undirected_adjacency_list<EV, VV, GV, VId>` | Closest built-in undirected container; dual-list design, not a storage/layout match for BGL `vecS` / `vecS` |
| `compressed_sparse_row_graph<directedS, VP, EP>` | `compressed_graph<EV, VV, GV, VId, EIndex>` | Both CSR; graph-v3 uses projection-based loading |
| `edge_list<Iter>` | Any `input_range` satisfying `basic_sourced_edgelist` | graph-v3 is more flexible — any range of edge-like tuples works |

### Closest Selector Analogues

These are behavioral analogues, not strict one-to-one translations. In particular, classic BGL `mapS` / `hash_mapS` selectors map to set-like containers, whereas graph-v3 `om` / `oum` traits use true map / unordered_map edge containers keyed by target vertex ID.

| BGL `OutEdgeListS` | graph-v3 Edge Container Suffix |
|--------------------|-------------------------------|
| `vecS` | `ov` (e.g., `vov_graph_traits`) |
| `listS` | `ol` |
| `setS` | `os` |
| `multisetS` | — (not available) |
| `hash_setS` | `ous` (unordered_set) |
| `mapS` | Closest uniqueness/ordering analogue: `os`; graph-v3 also offers `om`, but that is a true `map`-backed edge container |
| `hash_mapS` | Closest uniqueness/hash analogue: `ous`; graph-v3 also offers `oum`, but that is a true `unordered_map`-backed edge container |

| BGL `VertexListS` | graph-v3 Vertex Container Prefix |
|--------------------|---------------------------------|
| `vecS` | `v` (e.g., `vov_graph_traits`) |
| `listS` | — (not available; choose `d` for contiguous IDs or `m` / `u` for keyed storage) |
| `setS` | — (not available; closest ordered-key analogue is `m`) |

### Missing Containers

| BGL Container | Status in graph-v3 | Migration Path |
|---------------|--------------------|--------------------|
| **`adjacency_matrix`** | ❌ Not available | Use `dynamic_graph` with `os`/`ous` edge container for O(log n)/O(1) edge lookup; or implement `adjacency_matrix` |
| **`directed_graph`** | ❌ Not available | Use `dynamic_graph<EV, VV, GV, VId, false>` for a directed graph, or `true` when you also need in-edge traversal |
| **`undirected_graph`** | ✅ `undirected_adjacency_list` | Direct replacement |
| **`labeled_graph`** | ❌ Not available | Use map-based `dynamic_graph` with `mo*` traits and string keys; or implement as an adaptor |
| **`grid_graph`** | ❌ Not available | Implement as zero-storage implicit graph satisfying `adjacency_list` concept |
| **`subgraph`** | ❌ Not available | No hierarchy support; `extract_subgraph()` test helper creates independent copies |

---

## 4. Concept & Type System Comparison

### Concept Mapping

| BGL Concept | graph-v3 Concept | Notes |
|-------------|------------------|-------|
| `Graph` | (implicit — any type with CPOs) | No base concept needed |
| `IncidenceGraph` | Part of `adjacency_list<G>` | `out_edges(g, u)` returns edge range |
| `BidirectionalGraph` | `bidirectional_adjacency_list<G>` | `in_edges(g, u)` available |
| `AdjacencyGraph` | Part of `adjacency_list<G>` | `neighbors(g, u)` view |
| `VertexListGraph` | `vertex_range<G>` | `vertices(g)` returns range |
| `EdgeListGraph` | `edgelist(g)` view | Not a concept — a view function |
| `VertexAndEdgeListGraph` | `adjacency_list<G>` | Combined by default |
| `AdjacencyMatrix` | ❌ Not available | No O(1) edge lookup concept |
| `MutableGraph` | ❌ No unified concept | `dynamic_graph` is batch-load oriented; `undirected_adjacency_list` supports create-vertex/create-edge and edge erasure |
| `PropertyGraph` | ❌ No formal concept | CPOs `vertex_value`/`edge_value` serve the role |
| `ColorValue` | ❌ Internal only | Hidden inside algorithms |

### Key Concept Differences

**BGL's fine-grained concept hierarchy** (IncidenceGraph, BidirectionalGraph, AdjacencyGraph, VertexListGraph, EdgeListGraph, AdjacencyMatrix, MutableGraph, PropertyGraph...) is **collapsed** in graph-v3 into fewer, broader concepts:

- `adjacency_list<G>` ≈ IncidenceGraph + VertexListGraph + AdjacencyGraph
- `index_adjacency_list<G>` ≈ above + VertexIndexGraph (integral IDs, random access)
- `bidirectional_adjacency_list<G>` ≈ above + BidirectionalGraph

**Impact:** BGL algorithms can specify minimal requirements (e.g., "only needs IncidenceGraph"). graph-v3 algorithms typically require `adjacency_list<G>` as a minimum, which is a broader requirement. This makes graph-v3 less granular but simpler.

### Type Alias Mapping

| BGL | graph-v3 |
|-----|----------|
| `graph_traits<G>::vertex_descriptor` | `vertex_t<G>` (descriptor-like handle); use `vertex_id_t<G>` when you specifically need the key/index |
| `graph_traits<G>::edge_descriptor` | `edge_t<G>` |
| `graph_traits<G>::vertex_iterator` | `vertex_iterator_t<G>` |
| `graph_traits<G>::out_edge_iterator` | `iterator_t<vertex_edge_range_t<G>>` |
| `graph_traits<G>::in_edge_iterator` | `iterator_t<in_edge_range_t<G>>` |
| `graph_traits<G>::adjacency_iterator` | (use `neighbors(g, u)` view) |
| `graph_traits<G>::directed_category` | (implicit in type: `dynamic_graph` vs `undirected_adjacency_list`) |
| `graph_traits<G>::traversal_category` | (replaced by concept constraints) |

---

## 5. Property System Migration

### BGL Property Mechanisms → graph-v3 Equivalents

| BGL Mechanism | graph-v3 Equivalent | Migration Notes |
|---------------|---------------------|-----------------|
| **Interior `property<tag, T>`** chains | Single `VV`/`EV`/`GV` template params | Use struct for multiple properties: `struct VP { string name; double weight; };` |
| **Bundled properties** (`g[v].name`) | `vertex_value(g, u).name` | No `operator[]` subscript on graph; use CPO |
| **`get(vertex_index, g)`** | `vertex_id(g, u)` CPO | Automatic — no separate index map |
| **`get(edge_weight, g)`** | `edge_value(g, uv)` CPO | Single value type; access fields of struct |
| **`property_map<G, Tag>::type`** | No single replacement | `vertex_property_map<G, T>` covers the common per-vertex case; edge-keyed and tag-dispatched maps usually become custom containers or callable accessors |
| **`make_iterator_property_map(v.begin(), index_map)`** | `container_value_fn(vec)` | Wraps `vec[uid]` into `(g, uid) -> T&` |
| **`exterior_property<G, T>`** | Often `vertex_property_map<G, T>` | `make_vertex_property_map(g, init)` is the common per-vertex case |
| **Color maps (`two_bit_color_map`)** | Internal to algorithms | Users never create or pass color maps |
| **Multiple property tags per edge** | Single `EV` struct type | Bundle all properties into one struct |

### Migration Example: BGL Interior Properties → graph-v3

**BGL:**
```cpp
typedef property<edge_weight_t, double, 
        property<edge_color_t, int>> EdgeProp;
typedef adjacency_list<vecS, vecS, directedS, no_property, EdgeProp> G;
G g(num_vertices);
auto wmap = get(edge_weight, g);
auto cmap = get(edge_color, g);
put(wmap, e, 3.14);
```

**graph-v3:**
```cpp
struct EdgeProp { double weight; int color; };
dynamic_graph<EdgeProp> g({{0,1,{3.14, 0}}, {1,2,{2.71, 1}}});
// Access:
for (auto&& [tid, uv] : incidence(g, u)) {
    edge_value(g, uv).weight = 3.14;
    edge_value(g, uv).color = 1;
}
```

### Migration Example: BGL Exterior Property Map → graph-v3

**BGL:**
```cpp
vector<double> dist(num_vertices(g), numeric_limits<double>::max());
auto dmap = make_iterator_property_map(dist.begin(), get(vertex_index, g));
dijkstra_shortest_paths(g, s, distance_map(dmap));
```

**graph-v3:**
```cpp
vector<double> dist(num_vertices(g), numeric_limits<double>::max());
dijkstra_shortest_paths(g, {s}, container_value_fn(dist), ...);
```

---

## 6. Algorithm Gap Analysis

### Algorithms Present in Both Libraries

| Algorithm | BGL | graph-v3 | Interface Differences |
|-----------|-----|----------|-----------------------|
| **BFS** | `breadth_first_search(g, s, visitor(v).color_map(c))` | `breadth_first_search(g, sources, visitor)` | graph-v3: multi-source native, no color map, concept-checked visitor |
| **DFS** | `depth_first_search(g, visitor(v).color_map(c))` | `depth_first_search(g, sources, visitor)` | graph-v3: edge classification (tree/back/forward_or_cross), multi-source |
| **Dijkstra** | `dijkstra_shortest_paths(g, s, weight_map(w).distance_map(d).predecessor_map(p))` | `dijkstra_shortest_paths(g, sources, dist_fn, pred_fn, weight_fn, visitor)` | graph-v3: function-object properties, multi-source, custom compare/combine |
| **Bellman-Ford** | `bellman_ford_shortest_paths(g, N, weight_map(w).distance_map(d))` | `bellman_ford_shortest_paths(g, sources, dist_fn, pred_fn, weight_fn, visitor)` | Similar; graph-v3 detects negative cycles via visitor |
| **Kruskal MST** | `kruskal_minimum_spanning_tree(g, back_inserter(mst))` | `kruskal(edges, mst_output)` | graph-v3 works on edge lists directly; also `inplace_kruskal` |
| **Prim MST** | `prim_minimum_spanning_tree(g, pmap)` | `prim(g, mst_output, weight_fn)` | Similar interface |
| **Connected Components** | `connected_components(g, comp_map)` | `connected_components(g, comp_fn)` | graph-v3 uses function objects |
| **Strong Components** | `strong_components(g, comp_map)` | `kosaraju(g, comp_fn)` + `tarjan_scc(g, comp_fn)` | graph-v3 has both Kosaraju and Tarjan |
| **Biconnected Components** | `biconnected_components(g, comp_map)` | `biconnected_components(g, output)` | Similar |
| **Topological Sort** | `topological_sort(g, back_inserter(order))` | `topological_sort(g, output_iter)` | Both use output iterators; graph-v3 returns bool (cycle detection) |
| **Articulation Points** | `articulation_points(g, back_inserter(art))` | `articulation_points(g, output_iter)` | Similar |

### Algorithms in BGL but MISSING from graph-v3

#### Shortest Paths (3 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **A\* Search** | 🔴 High | Medium | Heavily used; needs heuristic function concept |
| **Johnson All-Pairs** | 🟡 Medium | Medium | Builds on Dijkstra + Bellman-Ford (both exist) |
| **Floyd-Warshall** | 🟡 Medium | Low | O(V³) all-pairs; straightforward implementation |
| **DAG Shortest Paths** | 🟢 Low | Low | Linear-time on DAGs; topological sort + relaxation |

#### Network Flow (7 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Edmonds-Karp Max Flow** | 🔴 High | Medium | Core max-flow; needs `edge_reverse` and `residual_capacity` properties |
| **Push-Relabel Max Flow** | 🟡 Medium | High | Higher performance than Edmonds-Karp |
| **Boykov-Kolmogorov Max Flow** | 🟡 Medium | High | Used in computer vision |
| **Stoer-Wagner Min Cut** | 🟡 Medium | Medium | Undirected min-cut |
| **Cycle Canceling (min-cost flow)** | 🟢 Low | High | Specialized |
| **Successive Shortest Path** | 🟢 Low | High | Min-cost flow variant |
| **Find Flow Cost** | 🟢 Low | Low | Utility for min-cost flow |

#### Matching (2 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Max Cardinality Matching** | 🟡 Medium | Medium | Useful for bipartite problems |
| **Maximum Weighted Matching** | 🟢 Low | High | Complex implementation |

#### Coloring (2 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Sequential Vertex Coloring** | 🟡 Medium | Low | Simple greedy coloring |
| **Edge Coloring** | 🟢 Low | Medium | Vizing's theorem |

#### Planarity (7 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Boyer-Myrvold Planarity Test** | 🟡 Medium | High | Core planarity test |
| **Kuratowski Subgraph** | 🟢 Low | High | Planarity certificate |
| **Chrobak-Payne Drawing** | 🟢 Low | High | Planar graph drawing |
| **Planar Canonical Ordering** | 🟢 Low | High | For planar drawings |
| **Planar Face Traversal** | 🟢 Low | Medium | Dual graph operations |
| **Make Biconnected/Connected/Maximal Planar** | 🟢 Low | Medium | Graph augmentation |

#### Isomorphism (3 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Graph Isomorphism** | 🟡 Medium | High | General isomorphism test |
| **VF2 Subgraph Isomorphism** | 🟡 Medium | High | Pattern matching in graphs |
| **McGregor Common Subgraphs** | 🟢 Low | High | Maximum common subgraph |

#### Centrality & Metrics (7 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Betweenness Centrality** | 🔴 High | Medium | Widely used in network analysis |
| **PageRank** | 🔴 High | Low | Iterative; straightforward |
| **Degree Centrality** | 🟡 Medium | Low | Trivial with `degree()` CPO |
| **Closeness Centrality** | 🟡 Medium | Medium | Requires all-pairs shortest paths |
| **Clustering Coefficient** | 🟡 Medium | Low | Related to existing triangle count |
| **Eccentricity** | 🟢 Low | Medium | Max distance from vertex |
| **Geodesic Distance** | 🟢 Low | Medium | Average shortest path |

#### Ordering & Bandwidth (6 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Cuthill-McKee Ordering** | 🟡 Medium | Medium | Bandwidth reduction; used in sparse matrix solvers |
| **King Ordering** | 🟢 Low | Medium | Profile reduction |
| **Sloan Ordering** | 🟢 Low | Medium | Wavefront reduction |
| **Minimum Degree Ordering** | 🟢 Low | High | Fill-reduction ordering |
| **Smallest Last Ordering** | 🟢 Low | Low | Coloring heuristic |
| **Bandwidth/Profile/Wavefront** | 🟢 Low | Low | Metrics only |

#### Cycle Detection (3 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Tiernan All Cycles** | 🟢 Low | Medium | Enumerate all cycles |
| **Hawick Circuits** | 🟢 Low | Medium | All elementary circuits |
| **Howard Cycle Ratio** | 🟢 Low | High | Min/max cycle ratio |

#### DAG & Closure (3 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Transitive Closure** | 🟡 Medium | Medium | Used in dependency analysis |
| **Transitive Reduction** | 🟡 Medium | Medium | Minimal equivalent DAG |
| **Dominator Tree** | 🟢 Low | High | Compiler/control flow analysis |

#### Layout (5 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Fruchterman-Reingold** | 🟡 Medium | Medium | Force-directed layout |
| **Kamada-Kawai** | 🟡 Medium | Medium | Spring layout |
| **Gursoy-Atun** | 🟢 Low | Medium | Topology-based layout |
| **Circle Layout** | 🟢 Low | Low | Trivial |
| **Random Layout** | 🟢 Low | Low | Trivial |

#### Other (8 missing)
| Algorithm | Priority | Difficulty | Notes |
|-----------|----------|------------|-------|
| **Core Numbers** | 🟡 Medium | Medium | k-core decomposition |
| **Maximum Adjacency Search** | 🟡 Medium | Medium | Used by Stoer-Wagner |
| **Bipartite Test** | 🟡 Medium | Low | BFS-based 2-coloring |
| **TSP Approximation** | 🟢 Low | Medium | Metric TSP heuristic |
| **Random Spanning Tree** | 🟢 Low | Medium | Wilson's algorithm |
| **Copy Graph** | 🟡 Medium | Low | Cross-type copy with property mapping |
| **Bron-Kerbosch All Cliques** | 🟢 Low | Medium | Maximal clique enumeration |
| **Create Condensation Graph** | 🟢 Low | Low | DAG from SCC |

### Algorithms in graph-v3 but NOT in BGL

| Algorithm | Notes |
|-----------|-------|
| **Jaccard Coefficient** | Similarity metric for adjacent vertex neighborhoods |
| **Label Propagation** | Community detection algorithm |
| **Maximal Independent Set** | Greedy MIS |
| **Triangle Count** | `triangle_count()` and `directed_triangle_count()` |
| **Afforest** | Parallel-friendly connected components (Sutton et al.) |

---

## 7. Visitor Pattern Migration

### BGL Visitor → graph-v3 Visitor

**BGL pattern:**
```cpp
struct my_visitor : boost::default_bfs_visitor {
    void discover_vertex(vertex_t v, const Graph& g) { ... }
    void examine_edge(edge_t e, const Graph& g) { ... }
};
breadth_first_search(g, s, visitor(my_visitor()));
```

**graph-v3 pattern:**
```cpp
struct my_visitor {
    void on_discover_vertex(const Graph& g, vertex_id_t<Graph> uid) { ... }
    void on_examine_edge(const Graph& g, edge_t<Graph> uv) { ... }
};
breadth_first_search(g, {s}, my_visitor{});
```

### Key Differences

| Aspect | BGL | graph-v3 |
|--------|-----|----------|
| **Base class** | Must inherit `default_*_visitor` | No base class needed |
| **Detection** | Virtual dispatch or CRTP | SFINAE concept detection (`has_on_*` concepts) |
| **Missing events** | Base class provides no-ops | Simply omit the method — zero overhead |
| **Parameter order** | `(vertex, graph)` or `(edge, graph)` | `(graph, vertex_id)` or `(graph, edge)` — graph first |
| **Vertex parameter** | `vertex_descriptor` | Both `vertex_id_t<G>` and `vertex_t<G>` overloads supported |
| **Event composition** | `make_bfs_visitor(pair<recorder1, pair<recorder2, ...>>)` | Not available — write a combined visitor struct |

### BGL Event → graph-v3 Event Mapping

| BGL Event | graph-v3 Event | Notes |
|-----------|----------------|-------|
| `initialize_vertex(v, g)` | `on_initialize_vertex(g, v)` | Param order reversed |
| `discover_vertex(v, g)` | `on_discover_vertex(g, v)` | |
| `examine_vertex(v, g)` | `on_examine_vertex(g, v)` | |
| `finish_vertex(v, g)` | `on_finish_vertex(g, v)` | |
| `examine_edge(e, g)` | `on_examine_edge(g, e)` | |
| `tree_edge(e, g)` | `on_tree_edge(g, e)` | DFS only |
| `back_edge(e, g)` | `on_back_edge(g, e)` | DFS only |
| `forward_or_cross_edge(e, g)` | `on_forward_or_cross_edge(g, e)` | DFS only |
| `edge_relaxed(e, g)` | `on_edge_relaxed(g, e)` | Dijkstra/Bellman-Ford |
| `edge_not_relaxed(e, g)` | `on_edge_not_relaxed(g, e)` | |
| `edge_minimized(e, g)` | `on_edge_minimized(g, e)` | Bellman-Ford |
| `non_tree_edge(e, g)` | ❌ Not available | BFS-specific; use edge classification |
| `gray_target(e, g)` | ❌ Not available | BFS-specific; color-dependent |
| `black_target(e, g)` | ❌ Not available | BFS-specific; color-dependent |
| `start_vertex(v, g)` | `on_start_vertex(g, v)` | DFS root |
| `finish_edge(e, g)` | `on_finish_edge(g, e)` | |

**Missing events:** `non_tree_edge`, `gray_target`, `black_target` are BFS-specific events that distinguish edge targets by color state. These could be added to graph-v3's BFS visitor concept if needed for migration.

### Composable Visitor Adaptors — Gap

BGL provides reusable event visitor adaptors (`predecessor_recorder`, `distance_recorder`, `time_stamper`, `property_writer`) that can be composed via `std::pair` chaining. graph-v3 has no equivalent — users write monolithic visitor structs. This is less composable but also less complex.

**Recommendation:** Consider providing lambda-based visitor construction:
```cpp
auto vis = make_visitor(
    on_discover_vertex([&](auto& g, auto uid) { ... }),
    on_examine_edge([&](auto& g, auto uv) { ... })
);
```

---

## 8. Graph Adaptors & Views

### BGL Adaptors → graph-v3 Equivalents

| BGL Adaptor | graph-v3 Equivalent | Gap Analysis |
|-------------|---------------------|--------------|
| **`reverse_graph<G>`** | `transpose_view<G>` | ✅ Functional equivalent; cleaner CPO-based design |
| **`filtered_graph<G, EP, VP>`** | ✅ `filtered_graph` adaptor (`<graph/adaptors/filtered_graph.hpp>`) | Vertex/edge predicate filtering; satisfies `adjacency_list` |
| **`subgraph<G>`** | ❌ None | 🟡 Test helper `extract_subgraph()` creates independent copies only |
| **`labeled_graph<G, Label>`** | ❌ None | 🟡 Use map-based `dynamic_graph` with string keys |
| **`graph_as_tree<G>`** | ❌ None | 🟢 Low priority |
| **`vector_as_graph`** | ✅ Native — `vector<vector<int>>` works via CPOs | graph-v3 is superior here |
| **`matrix_as_graph`** | ❌ None | 🟢 Low priority |

### Filtered Graph — Implemented

`filtered_graph` is available at `<graph/adaptors/filtered_graph.hpp>`. It creates a non-owning view over an existing graph that hides vertices and/or edges based on predicates, while satisfying `adjacency_list<G>` so it can be passed directly to all algorithms.

```cpp
#include <graph/adaptors/filtered_graph.hpp>
using namespace graph::adaptors;

// Filter edges by weight, keep all vertices:
auto fg = filtered_graph(g, keep_all{}, [&](auto&& uv) { return edge_value(g, uv) < 10.0; });
dijkstra_shortest_paths(fg, {source}, distances, predecessors, weight_fn);

// Filter vertices only:
auto fg2 = filtered_graph(g, [](auto uid) { return uid != 5; }, keep_all{});
```

Key design details:
- Uses `filtering_iterator` (not `std::views::filter`) to avoid dangling — predicate stored in `std::optional` with custom assignment for lambda non-assignability.
- `keep_all{}` sentinel predicate is zero-overhead.
- Delegates `begin()`/`end()`/`size()`/`operator[]` to underlying graph for vertex access.

### graph-v3 Views with No BGL Equivalent

graph-v3's lazy view system is a significant advancement over BGL:

| graph-v3 View | Description | BGL Equivalent |
|---------------|-------------|----------------|
| `vertices_bfs(g, seed)` | Lazy BFS vertex traversal, composable with `std::views` | None — BFS is imperative only |
| `edges_bfs(g, seed)` | Lazy BFS edge traversal | None |
| `vertices_dfs(g, seed)` | Lazy DFS vertex traversal with `depth()`, `cancel()` | None |
| `edges_dfs(g, seed)` | Lazy DFS edge traversal | None |
| `vertices_topological_sort(g)` | Lazy topological ordering | None |
| `vertexlist(g)` | `[uid, u]` structured binding view | `vertices(g)` returns iterator pair only |
| `basic_incidence(g, uid)` | `[tid]` target-ID-only view | No equivalent lightweight view |
| `basic_neighbors(g, uid)` | `[tid]` neighbor-ID-only view | No equivalent |

---

## 9. Graph I/O & Serialization

### BGL I/O Support vs. graph-v3

| Format | BGL | graph-v3 | Priority |
|--------|-----|----------|----------|
| **DOT / GraphViz** | `read_graphviz()`, `write_graphviz()` | ✅ `write_dot()`, `read_dot()` | 🔴 High — most common format |
| **GraphML (XML)** | `read_graphml()`, `write_graphml()` | ✅ `write_graphml()`, `read_graphml()` | 🟡 Medium |
| **DIMACS** | `read_dimacs_max_flow()`, `write_dimacs_max_flow()` | ❌ None | 🟡 Medium (needed for flow algorithms) |
| **METIS** | `metis_reader` class | ❌ None | 🟢 Low |
| **Adjacency List Text** | `operator<<` / `operator>>` | ❌ None | 🟢 Low |
| **JSON** | None | ✅ `write_json()`, `read_json()` | 🟡 Medium (modern format) |

**Recommendation:** Implement DOT and GraphML as the first I/O formats. These cover the vast majority of BGL user needs. Design the I/O layer as generic free functions taking any `adjacency_list<G>`.

### Proposed DOT API — `std::format`-Based

The DOT writer should leverage `std::format` (C++20) for type-safe value serialization. This avoids inventing a new extension point — users who specialize `std::formatter<T>` get DOT output for free.

**Design: auto-format with opt-in override.**

```cpp
#include <graph/io/dot.hpp>

// (A) Zero-config default: auto-detects formattable VV/EV types.
//     Emits [label="<formatted value>"] when std::formattable<VV> / std::formattable<EV>.
//     Omits attributes when VV=void or type is not formattable.
void write_dot(ostream& os, const adjacency_list auto& g);

// (B) User-supplied attribute functions override the default.
//     vertex_attr_fn: (const G&, vertex_id_t<G>) -> string   e.g. R"([label="A", color="red"])"
//     edge_attr_fn:   (const G&, edge_t<G>)      -> string   e.g. R"([weight=3.14])"
void write_dot(ostream& os, const adjacency_list auto& g,
               auto vertex_attr_fn,
               auto edge_attr_fn);

// Read into a dynamic_graph (vertex/edge values parsed from DOT attributes).
auto read_dot(istream& is) -> dynamic_graph<...>;
```

**Implementation strategy for the default (A):**

```cpp
template <adjacency_list G>
void write_dot(ostream& os, const G& g) {
  os << "digraph {\n";
  for (auto&& [uid, u] : vertexlist(g)) {
    os << "  " << uid;
    if constexpr (has_vertex_value<G> && std::formattable<vertex_value_t<G>, char>) {
      os << std::format(" [label=\"{}\"]", vertex_value(g, u));
    }
    os << ";\n";
    for (auto&& [tid, uv] : incidence(g, u)) {
      os << "  " << uid << " -> " << tid;
      if constexpr (has_edge_value<G> && std::formattable<edge_value_t<G>, char>) {
        os << std::format(" [label=\"{}\"]", edge_value(g, uv));
      }
      os << ";\n";
    }
  }
  os << "}\n";
}
```

**Comparison with BGL's approach:**

| Aspect | BGL | Proposed graph-v3 |
|--------|-----|-------------------|
| Value → string | `dynamic_properties` + per-property converters | `std::format` via `std::formatter<T>` specialization |
| Customization | Verbose `dp.property("name", get(&VP::name, g))` for each field | Single `vertex_attr_fn` / `edge_attr_fn` callable |
| Zero-config | No — must register properties explicitly | Yes — auto-formats if `std::formattable<VV>` |
| Multiple attributes | Each registered separately | User returns full attribute string, or formats struct members |
| Type safety | Runtime string conversion | Compile-time `std::formattable` concept check |

**Struct with multiple fields** — user provides a formatter or attribute function:

```cpp
struct CityVertex { std::string name; double population; };

// Option 1: specialize std::formatter<CityVertex> — auto-picked up by write_dot
template<> struct std::formatter<CityVertex> : std::formatter<std::string> {
  auto format(const CityVertex& v, auto& ctx) const {
    return std::format_to(ctx.out(), "{} (pop: {:.0f})", v.name, v.population);
  }
};
write_dot(cout, g);  // uses formatter automatically

// Option 2: explicit attribute function for full DOT control
write_dot(cout, g,
  [&](const auto& g, auto uid) {
    auto& v = vertex_value(g, vertices(g)[uid]);
    return std::format(R"([label="{}", population="{:.0f}"])", v.name, v.population);
  },
  [&](const auto& g, auto uv) {
    return std::format(R"([weight="{:.2f}"])", edge_value(g, uv));
  });
```

### Proposed GraphML API

```cpp
#include <graph/io/graphml.hpp>

// Write — property names inferred from struct member names (requires reflection
// or explicit property registration). Simpler initial version: single label only.
void write_graphml(ostream& os, const adjacency_list auto& g);
void write_graphml(ostream& os, const adjacency_list auto& g,
                   auto vertex_properties_fn, auto edge_properties_fn);

// Read — returns dynamic_graph with string-valued vertex/edge properties.
auto read_graphml(istream& is) -> dynamic_graph<std::string, std::string>;
```

---

## 10. Graph Generators

### BGL Generators vs. graph-v3

| Generator | BGL Header | graph-v3 | Priority |
|-----------|-----------|----------|----------|
| **Erdős-Rényi G(n,p)** | `erdos_renyi_generator.hpp` | ✅ `<graph/generators/erdos_renyi.hpp>` | ✅ Done |
| **Erdos-Renyi G(n,m)** | (same) | ❌ Not available | 🟡 Medium |
| **Barabási–Albert (preferential attachment)** | — | ✅ `<graph/generators/barabasi_albert.hpp>` | ✅ Done |
| **2D Grid (4-connected)** | `mesh_graph_generator.hpp` | ✅ `<graph/generators/grid.hpp>` | ✅ Done |
| **Path graph** | — | ✅ `<graph/generators/path.hpp>` | ✅ Done |
| **Small World (Watts-Strogatz)** | `small_world_generator.hpp` | ❌ Not available | 🟡 Medium |
| **PLOD (Power-Law Out-Degree)** | `plod_generator.hpp` | ❌ Not available (use Barabási–Albert) | 🟡 Medium |
| **R-MAT** | `rmat_graph_generator.hpp` | ❌ Not available | 🟡 Medium |
| **SSCA#2** | `ssca_graph_generator.hpp` | ❌ Not available | 🟢 Low |
| **Complete Graph K(n)** | — (manual) | ❌ Not available | 🟢 Low |

### graph-v3 Generator API

Generators live in `<graph/generators.hpp>` (umbrella) or individual headers under `include/graph/generators/`. All return a `std::vector<copyable_edge_t<VId, double>>` sorted by source_id, suitable for loading into any graph container via `load_edges()`.

```cpp
#include <graph/generators.hpp>
using namespace graph::generators;

// Erdős–Rényi G(n,p) — O(E) geometric-skip algorithm (Batagelj & Brandes 2005)
auto er = erdos_renyi(10'000u, 8.0 / 10'000);    // ~80K directed edges

// 2D grid — bidirectional 4-connected, E/V ≈ 4
auto grid = grid_2d(100u, 100u);                  // 10K vertices, ~40K edges

// Barabási–Albert — scale-free / power-law degree distribution
auto ba = barabasi_albert(10'000u, 4u);           // E/V ≈ 8

// Path — 0 → 1 → 2 → … → (n−1), minimum-traffic baseline
auto path = path_graph(1'000u);                   // 999 edges

// Load into any container:
compressed_graph<double> g;
g.load_edges(er, std::identity{}, 10'000u);
```

**Template parameter:** All generators accept `VId` as a template parameter (defaults to `uint32_t`):
```cpp
auto edges = erdos_renyi<uint64_t>(1'000'000ULL, 0.00001);
```

**Weight distributions:** Each generator accepts a `weight_dist` enum:
- `weight_dist::uniform` — U[1, 100] (default)
- `weight_dist::exponential` — Exp(0.1) + 1
- `weight_dist::constant_one` — always 1.0

### Remaining Gaps

To achieve full BGL parity, the following generators are still needed:

| Generator | Notes |
|-----------|-------|
| Erdős-Rényi G(n,m) | Fixed edge count variant; wrap existing G(n,p) with rejection or Fisher-Yates |
| Watts-Strogatz small world | Ring lattice + random rewiring |
| R-MAT | Recursive matrix; important for Graph500 benchmarks |
| Complete graph K(n) | Trivial to implement |

---

## 11. API Pattern Migration Guide

### Common BGL Patterns → graph-v3 Equivalents

#### Pattern 1: Basic Graph Type Definition

**BGL:**
```cpp
typedef boost::adjacency_list<vecS, vecS, directedS,
    no_property, property<edge_weight_t, double>> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex;
typedef graph_traits<Graph>::edge_descriptor Edge;
```

**graph-v3:**
```cpp
using Graph = graph::dynamic_graph<double>; // EV=double, VV=void
using VId = graph::vertex_id_t<Graph>;
```

#### Pattern 2: Graph Construction with add_edge

**BGL:**
```cpp
Graph g(5);
add_edge(0, 1, 10.0, g);
add_edge(1, 2, 20.0, g);
add_edge(2, 3, 30.0, g);
```

**graph-v3:**
```cpp
Graph g({{0,1,10.0}, {0,2,20.0}, {2,3,30.0}});
// Or with undirected_adjacency_list for individual mutation:
undirected_adjacency_list<double> g;
g.create_vertex(); g.create_vertex(); // ...
g.create_edge(0, 1, 10.0);
```

> ⚠️ **Migration friction:** `dynamic_graph` does not support incremental `add_edge`. Code that builds graphs edge-by-edge must either:
> 1. Collect edges into a vector first, then construct
> 2. Use `undirected_adjacency_list` (undirected only)
> 3. Wait for mutation API to be added to `dynamic_graph`

#### Pattern 3: Vertex/Edge Iteration

**BGL:**
```cpp
graph_traits<Graph>::vertex_iterator vi, vi_end;
for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    graph_traits<Graph>::out_edge_iterator ei, ei_end;
    for (tie(ei, ei_end) = out_edges(*vi, g); ei != ei_end; ++ei) {
        double w = get(edge_weight, g, *ei);
    }
}
```

**graph-v3:**
```cpp
for (auto&& [uid, u] : vertexlist(g)) {
    for (auto&& [tid, uv] : incidence(g, u)) {
        double w = edge_value(g, uv);
    }
}
```

#### Pattern 4: Dijkstra Shortest Paths

**BGL:**
```cpp
vector<Vertex> pred(num_vertices(g));
vector<double> dist(num_vertices(g));
dijkstra_shortest_paths(g, s,
    predecessor_map(make_iterator_property_map(pred.begin(), get(vertex_index, g)))
    .distance_map(make_iterator_property_map(dist.begin(), get(vertex_index, g))));
```

**graph-v3:**
```cpp
vector<VId> pred(num_vertices(g));
vector<double> dist(num_vertices(g), numeric_limits<double>::max());
dijkstra_shortest_paths(g, {s}, container_value_fn(dist), container_value_fn(pred),
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });
```

#### Pattern 5: Custom BFS Visitor

**BGL:**
```cpp
struct my_vis : default_bfs_visitor {
    void discover_vertex(Vertex v, const Graph& g) { cout << v; }
};
breadth_first_search(g, s, visitor(my_vis()));
```

**graph-v3:**
```cpp
struct my_vis {
    void on_discover_vertex(const Graph& g, VId uid) { cout << uid; }
};
breadth_first_search(g, {s}, my_vis{});
```

#### Pattern 6: Filtered Graph

**BGL:**
```cpp
auto ep = [&](Edge e) { return get(edge_weight, g, e) > 5.0; };
filtered_graph<Graph, decltype(ep)> fg(g, ep);
dijkstra_shortest_paths(fg, s, ...);
```

**graph-v3:**
```cpp
#include <graph/adaptors/filtered_graph.hpp>
auto ep = [&](auto&& uv) { return edge_value(g, uv) > 5.0; };
auto fg = graph::adaptors::filtered_graph(g, graph::adaptors::keep_all{}, ep);
dijkstra_shortest_paths(fg, {s}, dist_fn, pred_fn, weight_fn);
```

#### Pattern 7: Reverse/Transpose Graph

**BGL:**
```cpp
reverse_graph<Graph> rg(g);
depth_first_search(rg, visitor(vis));
```

**graph-v3:**
```cpp
auto tg = graph::views::transpose(g);  // requires bidirectional graph
depth_first_search(tg, {s}, vis);
```

#### Pattern 8: Lazy BFS/DFS (graph-v3 only — no BGL equivalent)

**graph-v3:**
```cpp
for (auto&& [uid] : vertices_bfs(g, source)) {
    if (uid == target) break;  // early termination
}

// Compose with standard views
auto first_10 = vertices_dfs(g, source) | std::views::take(10);
```

---

## 12. Adapting an Existing BGL Graph for graph-v3

This section describes how to make an existing `boost::adjacency_list` (or any BGL-modelling type) usable as input to graph-v3 algorithms and views, **without rewriting the storage**. The goal is incremental migration: keep the BGL container, expose graph-v3 CPOs on top of it.

> **✅ Library-provided adaptor available.** graph-v3 now ships a ready-to-use adaptor in `include/graph/adaptors/bgl/`:
> - `graph_adaptor.hpp` — one-line wrapper (`graph::bgl::graph_adaptor(bgl_g)`)
> - `bgl_edge_iterator.hpp` — C++20 iterator wrapper for BGL iterators
> - `property_bridge.hpp` — factory functions bridging BGL property maps to graph-v3 function objects
>
> See the [BGL Adaptor User Guide](../../docs/user-guide/bgl-adaptor.md) and [examples/bgl_adaptor_example.cpp](../../examples/bgl_adaptor_example.cpp) for usage.
> The manual approach described below remains useful for understanding the CPO dispatch mechanism or for adapting non-standard BGL types.

### 12.1 What graph-v3 Requires

graph-v3 dispatches everything through CPOs and a small set of concepts. The minimum surface to satisfy `graph::adjacency_list<G>` is:

| CPO | Purpose | Required for |
|-----|---------|--------------|
| `vertices(g)` | Range of vertex descriptors | All algorithms |
| `edges(g, u)` | Out-edge range for vertex `u` | All algorithms |
| `target_id(g, uv)` | Target vertex id of an edge | All algorithms |
| `vertex_id(g, u)` | Id of a vertex descriptor | Most algorithms (auto-derivable for indexed graphs) |
| `num_vertices(g)` | Vertex count | Algorithms that pre-size storage (Dijkstra, BFS) |
| `source_id(g, uv)` | Source vertex id | Sourced-edge algorithms (Kruskal, edge-list views) |
| `in_edges(g, u)` | In-edge range | `bidirectional_adjacency_list` only |
| `vertex_value(g, u)` / `edge_value(g, uv)` | Bundled property access | Algorithms that read interior properties |

There is **no `graph_traits` to specialise**. A CPO is satisfied in one of three ways, in priority order:

1. A member function on `G` (e.g. `g.vertices()`).
2. A free function found by ADL in the namespace of `G` (e.g. `vertices(g)` in namespace `boost`).
3. A built-in default that works when `G` matches a recognised pattern (e.g. random-access range of inner ranges of integers).

For BGL types you will use **option 2**: add free functions in `namespace boost`. ADL will find them because `boost::adjacency_list` lives in that namespace.

### 12.2 Recommended Layout

Put the adapter in its own header, included **before** any graph-v3 algorithm header that will be invoked on a BGL graph:

```cpp
// my_project/bgl_adapter.hpp
#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <graph/graph.hpp>
#include <ranges>
```

All adapter functions go inside `namespace boost { ... }` so ADL picks them up.

### 12.3 Minimal Adapter for `boost::adjacency_list`

The example below adapts `boost::adjacency_list<vecS, vecS, directedS, VBundle, EBundle>` — the most common BGL configuration. It is the analogue of graph-v3's `vov_graph_traits`-backed `dynamic_graph`.

```cpp
namespace boost {

// --- vertices(g) -----------------------------------------------------------
// Return a range of vertex descriptors. For vecS storage these are integers
// 0..num_vertices(g)-1. graph-v3 wraps the result in vertex_descriptor_view
// automatically when needed.
template <class OutEdgeS, class VertexS, class DirS,
          class VBundle, class EBundle, class GBundle, class EdgeListS>
auto vertices(adjacency_list<OutEdgeS, VertexS, DirS,
                             VBundle, EBundle, GBundle, EdgeListS>& g) {
    auto [first, last] = ::boost::vertices(g);          // BGL free function
    return std::ranges::subrange(first, last);
}

template <class... Ts>
auto vertices(const adjacency_list<Ts...>& g) {
    auto [first, last] = ::boost::vertices(g);
    return std::ranges::subrange(first, last);
}

// --- num_vertices(g) -------------------------------------------------------
// BGL already provides this; nothing to do — ADL finds boost::num_vertices.

// --- edges(g, u) -----------------------------------------------------------
// graph-v3's edges(g, u) is BGL's out_edges(u, g).
template <class... Ts>
auto edges(adjacency_list<Ts...>& g,
           typename graph_traits<adjacency_list<Ts...>>::vertex_descriptor u) {
    auto [first, last] = ::boost::out_edges(u, g);
    return std::ranges::subrange(first, last);
}

template <class... Ts>
auto edges(const adjacency_list<Ts...>& g,
           typename graph_traits<adjacency_list<Ts...>>::vertex_descriptor u) {
    auto [first, last] = ::boost::out_edges(u, g);
    return std::ranges::subrange(first, last);
}

// --- target_id(g, uv) / source_id(g, uv) -----------------------------------
// For vecS storage, vertex_descriptor IS the vertex id.
template <class... Ts>
auto target_id(const adjacency_list<Ts...>& g,
               typename graph_traits<adjacency_list<Ts...>>::edge_descriptor uv) {
    return ::boost::target(uv, g);
}

template <class... Ts>
auto source_id(const adjacency_list<Ts...>& g,
               typename graph_traits<adjacency_list<Ts...>>::edge_descriptor uv) {
    return ::boost::source(uv, g);
}

// --- vertex_id(g, u) -------------------------------------------------------
// For vecS storage the descriptor is its own id.
template <class... Ts>
auto vertex_id(const adjacency_list<Ts...>& /*g*/,
               typename graph_traits<adjacency_list<Ts...>>::vertex_descriptor u) {
    return u;   // identity for integer descriptors
}

} // namespace boost
```

With just these five functions, a `boost::adjacency_list<vecS, vecS, directedS>` satisfies `graph::index_adjacency_list` and can be passed to BFS, DFS, and topological sort.

### 12.4 Bidirectional Graphs

For algorithms that need in-edges (e.g. transposed traversals), add:

```cpp
namespace boost {

template <class... Ts>
auto in_edges(adjacency_list<Ts...>& g,
              typename graph_traits<adjacency_list<Ts...>>::vertex_descriptor u) {
    auto [first, last] = ::boost::in_edges(u, g);     // requires bidirectionalS
    return std::ranges::subrange(first, last);
}

} // namespace boost
```

This only compiles when the underlying graph is `bidirectionalS`.

### 12.5 Property Access

BGL graphs expose properties three different ways. graph-v3 reads them through `vertex_value(g, u)` / `edge_value(g, uv)` CPOs, plus user-supplied callables for algorithm-specific maps (e.g. weight).

**Bundled properties (recommended).** When the BGL graph is declared with bundled types, expose them directly:

```cpp
namespace boost {

template <class... Ts>
decltype(auto) vertex_value(adjacency_list<Ts...>& g,
                            typename graph_traits<adjacency_list<Ts...>>::vertex_descriptor u) {
    return g[u];     // returns reference to bundled VBundle
}

template <class... Ts>
decltype(auto) edge_value(adjacency_list<Ts...>& g,
                          typename graph_traits<adjacency_list<Ts...>>::edge_descriptor uv) {
    return g[uv];    // returns reference to bundled EBundle
}

} // namespace boost
```

**Interior properties (`property<weight_t, double>`).** Do **not** wrap with `vertex_value` / `edge_value`. Instead, supply a per-algorithm projection:

```cpp
auto wmap   = ::boost::get(::boost::edge_weight, g);
auto weight = [&](auto e) -> double { return ::boost::get(wmap, e); };

graph::dijkstra_shortest_paths(g, {source}, distances, predecessors, weight);
```

**Exterior property maps.** Pass them as ordinary lambdas — graph-v3 algorithms accept any invocable taking an edge or vertex descriptor.

### 12.6 Other BGL Container Configurations

| BGL `OutEdgeS` / `VertexS` | Vertex descriptor | Adapter notes |
|----------------------------|-------------------|---------------|
| `vecS` / `vecS` | integer | Adapter above works as-is. |
| `listS` / `vecS` | integer (vertex), pointer-like (edge) | Adapter above works; descriptors still index into vertex table. |
| `vecS` / `listS` | opaque (pointer) | `vertex_id` cannot be the descriptor itself; build an external `unordered_map<vertex_descriptor, std::size_t>` and have `vertex_id` look up there. Algorithms then see a non-integer id; only `mapped_adjacency_list` algorithms apply. |
| `setS` / `vecS` | integer | Works; out-edge range is sorted. |
| `hash_setS` / `vecS` | integer | Works; out-edge range is unordered. |

For non-`vecS` `VertexS`, prefer copying the graph into a graph-v3 container at the migration boundary rather than maintaining an id-mapping adapter.

### 12.7 Verifying the Adapter

After writing the adapter, confirm the concepts compile:

```cpp
#include "my_project/bgl_adapter.hpp"

using BGLGraph = boost::adjacency_list<
    boost::vecS, boost::vecS, boost::directedS,
    /*VBundle*/ MyVertex, /*EBundle*/ MyEdge>;

static_assert(graph::adjacency_list<BGLGraph>);
static_assert(graph::index_adjacency_list<BGLGraph>);
// For bidirectionalS:
// static_assert(graph::bidirectional_adjacency_list<BGLGraph>);
```

If the assertions pass, all graph-v3 algorithms constrained on `index_adjacency_list` will accept the BGL graph.

### 12.8 Migration Workflow

1. Drop in the adapter header — no source changes to existing BGL code.
2. Pick one new code path (e.g. a new analytics pass) and write it against graph-v3 CPOs, taking the existing BGL graph as input.
3. As tests cover more code paths, port BGL call sites one at a time using the API mappings in [Section 11](#11-api-pattern-migration-guide).
4. Once all algorithm call sites are ported, replace the BGL container with `graph::dynamic_graph` (or another graph-v3 container) and remove the adapter.

### 12.9 Limitations

- The adapter exposes the BGL graph as **read-only** to graph-v3. Mutation must continue to go through BGL APIs (`add_vertex`, `add_edge`).
- `partition_id`, `num_partitions`, and other multi-partition CPOs are not provided; graph-v3 defaults to a single partition, which is correct for plain BGL graphs.
- Algorithms that require `ranges::sized_range` on `edges(g, u)` (rare) need the BGL graph to use a sized out-edge container (`vecS`, `setS`).
- If `vertex_descriptor` is a pointer or opaque type, the integer-id assumption in `vertex_id` / `target_id` does not hold; see §12.6.

---

## 13. Recommended Extensions & Roadmap

### Phase 1: Critical Migration Enablers (High Priority)

These items block migration for the largest number of BGL users:

| Item | Type | Effort | Rationale |
|------|------|--------|-----------|
| **Individual mutation on directed graph** | Container | High | `add_vertex()`, `add_edge()`, `remove_vertex()`, `remove_edge()` on `dynamic_graph`. Most BGL code builds graphs incrementally. |
| **`filtered_graph_view`** | Adaptor | Medium | Preserves `adjacency_list` concept; enables algorithm composition on subsets |
| **A\* Search** | Algorithm | Medium | Heavily used in pathfinding, robotics, game AI |
| **DOT format read/write** | I/O | Medium | Primary graph interchange format |
| **Erdos-Renyi generator** | Generator | Low | Essential for testing and benchmarking |
| **`copy_graph` utility** | Utility | Low | Cross-type graph copy with property mapping |
| **Betweenness Centrality** | Algorithm | Medium | Core network analysis metric |
| **PageRank** | Algorithm | Low | Widely used iterative algorithm |

### Phase 2: Common Algorithm Coverage

| Item | Type | Effort | Rationale |
|------|------|--------|-----------|
| **Edmonds-Karp Max Flow** | Algorithm | Medium | Core network flow |
| **Floyd-Warshall** | Algorithm | Low | Simple all-pairs shortest paths |
| **Johnson All-Pairs** | Algorithm | Medium | Efficient all-pairs with negative weights |
| **Bipartite Test** | Algorithm | Low | BFS-based 2-coloring |
| **Sequential Vertex Coloring** | Algorithm | Low | Greedy graph coloring |
| **Graph Isomorphism** | Algorithm | High | Pattern matching |
| **Transitive Closure/Reduction** | Algorithm | Medium | DAG analysis |
| **Core Numbers (k-core)** | Algorithm | Medium | Network analysis |
| **Cuthill-McKee Ordering** | Algorithm | Medium | Sparse matrix bandwidth reduction |
| **GraphML read/write** | I/O | Medium | XML-based interchange |

### Phase 3: Advanced Features

| Item | Type | Effort | Rationale |
|------|------|--------|-----------|
| **`adjacency_matrix`** | Container | Medium | Dense graph storage |
| **`subgraph_view`** | Adaptor | High | Hierarchical subgraph with descriptor mapping |
| **`labeled_graph` adaptor** | Adaptor | Medium | String label → vertex mapping |
| **Boyer-Myrvold Planarity** | Algorithm | High | Planarity testing |
| **VF2 Subgraph Isomorphism** | Algorithm | High | Subgraph pattern matching |
| **Push-Relabel Max Flow** | Algorithm | High | High-performance max flow |
| **Max Cardinality Matching** | Algorithm | Medium | Bipartite matching |
| **Layout algorithms** | Algorithm | Medium | Graph visualization |
| **Small World / PLOD generators** | Generator | Low | Synthetic graph generation |
| **Lambda visitor composition** | API | Low | `make_visitor(on_discover_vertex([&](...){...}), ...)` |
| **BGL compatibility header** | Migration | Medium | `graph_traits` shim + name aliases for gradual migration |

### Phase 4: Ecosystem & Tooling

| Item | Type | Effort | Rationale |
|------|------|--------|-----------|
| **DIMACS I/O** | I/O | Low | Needed for flow algorithm benchmarks |
| **JSON I/O** | I/O | Medium | Modern interchange format |
| **Parallel algorithms** | Algorithm | High | Parallel BFS, CC, PageRank |
| **`grid_graph`** | Container | Medium | Implicit N-dimensional grid |
| **Condensation graph** | Algorithm | Low | DAG from SCC |
| **Cycle enumeration** | Algorithm | Medium | All cycles / circuits |

### BGL Compatibility Shim (Optional)

For users migrating large codebases, a thin compatibility header could ease the transition:

```cpp
// <graph/compat/bgl.hpp> — proposed
namespace graph::compat {
    template <class G>
    struct graph_traits {
        using vertex_descriptor = vertex_t<G>;
        using edge_descriptor = edge_t<G>;
        // ...
    };
    
    // BGL-style free function wrappers
    template <adjacency_list G>
    auto out_edges(vertex_t<G> u, G& g) { return edges(g, u); }
    
    template <adjacency_list G>
    auto adjacent_vertices(vertex_t<G> u, G& g) { return neighbors(g, u); }
    
    template <adjacency_list G>
    auto out_degree(vertex_t<G> u, G& g) { return degree(g, u); }
    
    // Named parameter emulation via designated initializers (C++20)
    struct dijkstra_params {
        auto weight_map = {};
        auto distance_map = {};
        auto predecessor_map = {};
        auto visitor = empty_visitor{};
    };
}
```

> **Recommendation:** A compatibility shim should be provided as a **separate, non-standardized** header — not part of the core library. It serves as a migration aid, not a long-term API.

---

## Summary: Migration Readiness Scorecard

The scores below are directional editorial estimates, not audited counts.

| Category | BGL Features | graph-v3 Coverage | Score |
|----------|-------------|-------------------|-------|
| **Core graph types** | 8 containers | 3 containers + zero-config | 60% |
| **Concepts & traits** | 18+ concepts | 9 concepts (broader) | 80% (by design) |
| **Property system** | Interior + exterior + bundled | Single value + external maps | 70% |
| **Traversal algorithms** | BFS, DFS, undirected DFS | BFS, DFS + lazy views | 90% |
| **Shortest paths** | 8 algorithms | 2 algorithms | 25% |
| **MST** | 2 algorithms | 2 algorithms (+ in-place) | 100% |
| **Connectivity** | 5 algorithms | 5 algorithms | 100% |
| **Network flow** | 7 algorithms | 0 | 0% |
| **Matching** | 2 algorithms | 0 | 0% |
| **Centrality/metrics** | 8 algorithms | 1 (Jaccard) | 12% |
| **Coloring** | 2 algorithms | 0 | 0% |
| **Planarity** | 7 algorithms | 0 | 0% |
| **Isomorphism** | 3 algorithms | 0 | 0% |
| **Ordering/bandwidth** | 8 algorithms | 0 | 0% |
| **Layout** | 5 algorithms | 0 | 0% |
| **Graph adaptors** | 5 adaptors | 1 (transpose) | 20% |
| **Graph I/O** | 5 formats | 3 (DOT, GraphML, JSON) | 60% |
| **Graph generators** | 6 generators | 4 (path, grid, Erdős–Rényi, Barabási–Albert) | 67% |
| **Visitors** | 5 types + composable adaptors | Concept-checked visitors | 75% |
| **Graph mutation** | Full `MutableGraph` concept | Partial (undirected only) | 40% |

**Overall estimated BGL API coverage: ~33%**

The coverage that exists is architecturally superior (C++20, ranges, concepts, CPOs, zero-config), and the library includes novel features (lazy traversal views, triangle counting, label propagation, Jaccard similarity) not found in BGL. The primary migration barrier is breadth of algorithm and utility coverage.
