# BGL → graph-v3 Migration Strategy

A comprehensive analysis of the Boost Graph Library (BGL) and graph-v3, identifying migration paths, gaps, and recommended extensions to enable a smooth upgrade transition.

> **Last reviewed:** 2026-06-01 against `include/graph/` source tree.

---

## How to use this document

This guide serves two audiences:

- **Migrating a BGL codebase?** Start with [§3 Graph Container Mapping](#3-graph-container-mapping), [§5 Property System Migration](#5-property-system-migration), [§10 API Pattern Migration Guide](#10-api-pattern-migration-guide), and [§11 Adapting an Existing BGL Graph](#11-adapting-an-existing-bgl-graph-for-graph-v3). These are task-oriented and show side-by-side before/after code.
- **Planning graph-v3 work or tracking parity?** See [Appendix A. Algorithm Gap Analysis](#appendix-a-algorithm-gap-analysis) and [Appendix B. Recommended Extensions & Roadmap](#appendix-b-recommended-extensions--roadmap), which are reference/planning material.

[§1 Executive Summary](#1-executive-summary) gives the high-level picture for both.

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Architectural Comparison](#2-architectural-comparison)
3. [Graph Container Mapping](#3-graph-container-mapping)
4. [Concept & Type System Comparison](#4-concept--type-system-comparison)
5. [Property System Migration](#5-property-system-migration)
6. [Visitor Pattern Migration](#6-visitor-pattern-migration)
7. [Graph Adaptors & Views](#7-graph-adaptors--views)
8. [Graph I/O & Serialization](#8-graph-io--serialization)
9. [Graph Generators](#9-graph-generators)
10. [API Pattern Migration Guide](#10-api-pattern-migration-guide)
11. [Adapting an Existing BGL Graph for graph-v3](#11-adapting-an-existing-bgl-graph-for-graph-v3)

**Appendices** (reference & planning material)

- [Appendix A. Algorithm Gap Analysis](#appendix-a-algorithm-gap-analysis)
- [Appendix B. Recommended Extensions & Roadmap](#appendix-b-recommended-extensions--roadmap)
- [Appendix C. Migration Readiness Scorecard](#appendix-c-migration-readiness-scorecard)

---

## 1. Executive Summary

graph-v3 is a ground-up C++20 redesign targeting ISO standardization (P3126–P3131, P3337). It replaces BGL's iterator-pair/tag-dispatch/property-list architecture with CPOs, ranges, concepts, and structured bindings. The redesign is fundamentally cleaner but still covers only a modest fraction of classic BGL's total algorithm surface area and lacks several adaptors and algorithms that BGL users rely on.

**Key strengths of graph-v3 over BGL:**
- Zero-config adaptation of standard containers (`vector<vector<int>>` is a valid graph)
- C++20 concepts replace Boost Concept Checks — better error messages, compile-time safety
- CPO-based access eliminates the need for `graph_traits` specialization
- Structured bindings for ergonomic iteration (`for (auto [uid, u] : vertexlist(g))`)
- Internal color/visited tracking — users never manage color maps
- `vertex_property_map` auto-selects `vector` vs `unordered_map` based on graph type
- Lazy search views (`vertices_bfs`, `edges_dfs`) composable with `std::views`
- `transpose_view` is a zero-cost adaptor (no wrapper descriptor types)
- `filtered_graph` adaptor (vertex/edge predicates) modelling `adjacency_list`
- Ready-to-use BGL adaptor (`graph::bgl::graph_adaptor`) for incremental migration
- Full BGL I/O format parity plus JSON (DOT, GraphML, JSON, DIMACS, METIS, adjacency-list text)
- Full BGL graph-generator parity (Erdős-Rényi G(n,p)/G(n,m), Barabási-Albert, Watts-Strogatz, R-MAT, PLOD, SSCA#2, 2D grid, path, complete graph)

**Key gaps requiring attention for BGL migration:**
- Dozens of missing algorithms across flow, matching, coloring, planarity, isomorphism, centrality, layout, and related areas
- No `subgraph` hierarchy with descriptor mapping
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
| `adjacency_list<vecS, vecS, directedS, VP, EP>` | `dynamic_graph<EV, VV, GV, VId, false, vov_graph_traits>` | Closest built-in vector/vector analogue |
| `adjacency_list<vecS, vecS, bidirectionalS, VP, EP>` | `dynamic_graph<EV, VV, GV, VId, true, vov_graph_traits>` | Closest built-in vector/vector bidirectional analogue |
| `adjacency_list<vecS, vecS, undirectedS, VP, EP>` | `undirected_adjacency_list<EV, VV, GV, VId>` | Closest built-in undirected container; dual-list design, not a storage/layout match for BGL `vecS` / `vecS` |
| `compressed_sparse_row_graph<directedS, VP, EP>` | `compressed_graph<EV, VV, GV, VId, EIndex>` | Both CSR; graph-v3 uses projection-based loading |
| `edge_list<Iter>` | Any `input_range` satisfying `basic_sourced_edgelist` | graph-v3 is more flexible — any range of edge-like tuples works |

> **`adjacency_matrix` API note (2026-06):** use `exists(u, v)` (or `has_edge(u, v)` alias) for edge presence checks. For weighted matrices, direct value read access is `const operator()(u, v)`. The older `weight(u, v)` helper and mutable `operator()(u, v)` are removed.

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
| **`adjacency_matrix`** | ✅ `adjacency_matrix` | Direct replacement — dense `n x n` container (C++20; C++23 `md_adjacency_matrix` adds an `mdspan` view). Build from an order or an edge range + projection |
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
| `EdgeListGraph` | `basic_sourced_edgelist<EL>` / `basic_sourced_index_edgelist<EL>` | Concepts for an edgelist (range of edges); `edgelist(g)` is a *view* that produces such a range from an adjacency list |
| `VertexAndEdgeListGraph` | `adjacency_list<G>` | Combined by default |
| `AdjacencyMatrix` | ❌ Not available | No O(1) edge lookup concept |
| `MutableGraph` | ❌ No unified concept | No mutating CPOs; mutation is provided as member functions on `dynamic_graph` and `undirected_adjacency_list` |
| `PropertyGraph` | ❌ No formal concept | CPOs `vertex_value`/`edge_value` and their matching `vertex_value_t<G>/edge_value_t<G>` serve the role |
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
| `graph_traits<G>::out_edge_iterator` | `out_edge_iterator_t<G>` |
| `graph_traits<G>::in_edge_iterator` | `in_edge_iterator_t<G>` |
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

## 6. Visitor Pattern Migration

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
| **Event composition** | `make_bfs_visitor(pair<recorder1, pair<recorder2, ...>>)` | `make_visitor(on_tree_edge(predecessor_recorder(pred)), ...)` — see `visitor_factory.hpp` |

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

### Composable Visitor Adaptors — Implemented

BGL provides reusable event visitor adaptors (`predecessor_recorder`, `distance_recorder`, `time_stamper`, `property_writer`) that can be composed via `std::pair` chaining. graph-v3 now provides an equivalent toolkit in `include/graph/algorithm/visitor_factory.hpp` (included by the `graph/algorithms.hpp` umbrella).

Three layers:

**Layer 1 — single-event adaptors.** Wrap a callable so it fires for exactly one traversal event, analogous to BGL event tags:
```cpp
on_discover_vertex(f)   // vertex events
on_tree_edge(f)         // edge events — on_tree_edge, on_back_edge,
                        // on_edge_relaxed, on_finish_edge, etc.
```

**Layer 2 — `make_visitor(...)`.** Fan one traversal out to any number of sub-visitors. Each child may be a single-event adaptor, a prebuilt recorder, or any struct with `on_*` methods. Supports mixing descriptor-form and id-form children in the same call:
```cpp
auto vis = make_visitor(
    on_discover_vertex([&](auto& g, auto uid) { order.push_back(uid); }),
    on_tree_edge(predecessor_recorder(pred))
);
breadth_first_search(g, {s}, vis);
```

**Layer 3 — prebuilt recorders.** Ready-made callables for common bookkeeping; the caller binds each to the desired event:

| Recorder | Description | Typical event binding |
|----------|-------------|----------------------|
| `predecessor_recorder(pred)` | `pred[target_id] = source_id` | `on_tree_edge` (BFS/DFS), `on_edge_relaxed` (Dijkstra) |
| `distance_recorder(dist, weight_fn)` | weighted distance accumulation | `on_edge_relaxed` |
| `distance_recorder(dist)` | hop-count distance | `on_tree_edge` |
| `time_stamper(time_map, clock)` | `time[vertex_id] = clock++` | `on_discover_vertex`, `on_finish_vertex` |

**BGL vs. graph-v3 comparison:**
```cpp
// BGL:
breadth_first_search(g, s, visitor(make_bfs_visitor(std::make_pair(
    record_predecessors(pred.data(), on_tree_edge()),
    record_distances(dist.data(), on_tree_edge())))));

// graph-v3:
breadth_first_search(g, {s},
    make_visitor(
        on_tree_edge(predecessor_recorder(pred)),
        on_tree_edge(distance_recorder(dist)),
        on_discover_vertex([&](auto&, auto uid){ order.push_back(uid); })));
```

Key differences from BGL:
- Recorders are plain `(g, x)->void` callables bound to events by the caller — no event-tag type system.
- Visitors are held by reference, not copied; no `boost::ref` wrapper needed for stateful visitors.
- `composite_visitor` exposes `on_X` only when at least one child handles event `X`, keeping `has_on_*` / `valid_visitor` detection accurate.
- `property_writer` (BGL) has no direct equivalent; use a lambda with `on_examine_vertex` / `on_examine_edge`.

---

## 7. Graph Adaptors & Views

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

## 8. Graph I/O & Serialization

### BGL I/O Support vs. graph-v3

| Format | BGL | graph-v3 | Priority |
|--------|-----|----------|----------|
| **DOT / GraphViz** | `read_graphviz()`, `write_graphviz()` | ✅ `write_dot()`, `read_dot()` | 🔴 High — most common format |
| **GraphML (XML)** | `read_graphml()`, `write_graphml()` | ✅ `write_graphml()`, `read_graphml()` | 🟡 Medium |
| **DIMACS** | `read_dimacs_max_flow()`, `write_dimacs_max_flow()` | ✅ `write_dimacs()`, `write_dimacs_max_flow()`, `read_dimacs()` | 🟡 Medium (needed for flow algorithms) |
| **METIS** | `metis_reader` class | ✅ `write_metis()`, `read_metis()` | 🟢 Low |
| **Adjacency List Text** | `operator<<` / `operator>>` | ✅ `write_adjacency_list_text()`, `read_adjacency_list_text()` | 🟢 Low |
| **JSON** | None | ✅ `write_json()`, `read_json()` | 🟡 Medium (modern format) |

**Status:** All six interchange formats are implemented and shipped. Headers live under `include/graph/io/` (`dot.hpp`, `graphml.hpp`, `json.hpp`, `dimacs.hpp`, `metis.hpp`, `adjacency_list_text.hpp`). Writers are generic over any graph satisfying the adjacency-list concepts; readers return lightweight type-tagged parsed structures (`dot_graph`, `graphml_graph`, `json_graph`, `dimacs_graph`, `metis_graph`, `adjacency_list_text_graph`) suitable for post-processing into any graph-v3 container. DIMACS supports the max-flow, shortest-path, and edge (clique/coloring) variants; METIS handles the optional `fmt`/`ncon` weight flags; both normalize the file's 1-indexed ids to 0-indexed.

**Recommendation:** None outstanding — graph-v3 now matches BGL's I/O format coverage and adds JSON on top. Future work is limited to broadening the parsed subsets (e.g. DOT subgraphs, GraphML nested graphs) as needs arise.

### DOT API — `std::format`-Based (implemented)

The DOT writer leverages `std::format` (C++20) for type-safe value serialization. This avoids inventing a new extension point — users who specialize `std::formatter<T>` get DOT output for free.

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

| Aspect | BGL | graph-v3 |
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

### GraphML API (implemented)

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

## 9. Graph Generators

### BGL Generators vs. graph-v3

| Generator | BGL Header | graph-v3 | Priority |
|-----------|-----------|----------|----------|
| **Erdős-Rényi G(n,p)** | `erdos_renyi_generator.hpp` | ✅ `<graph/generators/erdos_renyi.hpp>` | ✅ Done |
| **Erdos-Renyi G(n,m)** | (same) | ✅ `<graph/generators/gnm.hpp>` | ✅ Done |
| **Barabási–Albert (preferential attachment)** | — | ✅ `<graph/generators/barabasi_albert.hpp>` | ✅ Done |
| **2D Grid (4-connected)** | `mesh_graph_generator.hpp` | ✅ `<graph/generators/grid.hpp>` | ✅ Done |
| **Path graph** | — | ✅ `<graph/generators/path.hpp>` | ✅ Done |
| **Small World (Watts-Strogatz)** | `small_world_generator.hpp` | ✅ `<graph/generators/watts_strogatz.hpp>` | ✅ Done |
| **PLOD (Power-Law Out-Degree)** | `plod_generator.hpp` | ✅ `<graph/generators/plod.hpp>` | ✅ Done |
| **R-MAT** | `rmat_graph_generator.hpp` | ✅ `<graph/generators/rmat.hpp>` | ✅ Done |
| **SSCA#2** | `ssca_graph_generator.hpp` | ✅ `<graph/generators/ssca.hpp>` | ✅ Done |
| **Complete Graph K(n)** | — (manual) | ✅ `<graph/generators/complete.hpp>` | ✅ Done |

### graph-v3 Generator API

Generators live in `<graph/generators.hpp>` (umbrella) or individual headers under `include/graph/generators/`. All return a `std::vector<copyable_edge_t<VId, double>>` sorted by source_id, suitable for loading into any graph container via `load_edges()`.

```cpp
#include <graph/generators.hpp>
using namespace graph::generators;

// Erdős–Rényi G(n,p) — O(E) geometric-skip algorithm (Batagelj & Brandes 2005)
auto er = erdos_renyi(10'000u, 8.0 / 10'000);    // ~80K directed edges

// Erdős–Rényi G(n,m) — fixed edge count, distinct edges sampled uniformly
auto erm = erdos_renyi_gnm(10'000u, 80'000u);    // exactly 80K directed edges

// 2D grid — bidirectional 4-connected, E/V ≈ 4
auto grid = grid_2d(100u, 100u);                  // 10K vertices, ~40K edges

// Barabási–Albert — scale-free / power-law degree distribution
auto ba = barabasi_albert(10'000u, 4u);           // E/V ≈ 8

// Path — 0 → 1 → 2 → … → (n−1), minimum-traffic baseline
auto path = path_graph(1'000u);                   // 999 edges

// Complete K(n) — all ordered pairs (u,v), u ≠ v; dense stress test
auto kn = complete_graph(100u);                   // 100*99 = 9'900 edges

// Watts–Strogatz — small-world ring lattice with random rewiring
auto ws = watts_strogatz(1'000u, 6u, 0.1);        // degree 6, 10% rewired

// R-MAT — recursive-matrix, Graph500-style power-law / community structure
auto rm = rmat(16u, 1u << 18);                    // 65'536 vertices, ~256K edges

// PLOD — power-law out-degree (BGL parity; prefer Barabási–Albert)
auto pl = plod(1'000u);                            // power-law out-degree

// SSCA#2 — clique-based HPCS benchmark (dense cliques + sparse inter-clique)
auto ss = ssca(1'000u, 8u, 0.2);                   // clustered graph

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

None — all BGL graph generators now have graph-v3 equivalents. graph-v3 also adds
the Barabási–Albert, complete-graph K(n), and path generators that BGL lacks as
named functions.

---

## 10. API Pattern Migration Guide

### Common BGL Patterns → graph-v3 Equivalents

#### Pattern 1: Basic Graph Type Definition

**BGL:**
```cpp
typedef boost::adjacency_list<vecS, vecS, directedS,
    no_property, property<edge_weight_t, double>> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex; // integral (size_t) for vecS
typedef graph_traits<Graph>::edge_descriptor   Edge;
typedef graph_traits<Graph>::vertices_size_type VId;  // same as vertex_descriptor for vecS
```

**graph-v3:**

Define a graph based on `dynamic_graph`. This is similar to the graph used by BGL, but it's
not the only graph type supported.
```cpp
#include <graph/container/traits/vov_graph_traits.hpp>
using Graph  = graph::container::vov_graph<double>; // vecS vertices, vecS edges, EV=double
using Vertex = vertex_t<Graph>;
using Edge   = edge_t<Graph>;
using VId    = graph::vertex_id_t<Graph>; // integral in this example, but may be non-integral
```

Graphs may also be defined using standard containers. The following has the same characteristics
as the `vov_graph` above. Construction and mutation functions (e.g. add/remove for vertices and
edges) are unique to a graph type.
```cpp
using Graph  = vector<<vector<pair<size_t,double>>>; // vecS vertices, vecS edges, EV=double
using Vertex = vertex_t<Graph>;
using Edge   = edge_t<Graph>;
using VId    = graph::vertex_id_t<Graph>; // integral in this example, but may be non-integral
```

User-defined graphs can easily be used by by overriding 3-7 CPO functions for the graph.

#### Pattern 2: Graph Construction with add_edge

**BGL:**
```cpp
Graph g(5); // 5 vertices, no edges
add_edge(0, 1, 10.0, g);
add_edge(1, 2, 20.0, g);
add_edge(2, 3, 30.0, g);
```

**graph-v3:**
```cpp
Graph g(5); // 5 vertices, no edges
g.add_edge(0, 1, 10.0);
g.add_edge(1, 2, 20.0);
g.add_edge(2, 3, 30.0);
```
* Note that add_edge is a member function of `dynamic_graph`. There are no mutating CPOs at this time.

Or with an _initializer list_
```cpp
Graph g({{0,1,10.0}, {1,2,20.0}, {2,3,30.0}});
```

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

Using views:
```cpp
for (auto&& [uid, u] : vertexlist(g)) {
    for (auto&& [vid, uv] : incidence(g, u)) {
        double w = edge_value(g, uv);
    }
}
```

Using lower-level CPO functions directly:
```cpp
for (auto u : vertices(g)) {
    for (auto uv : out_edges(g, u)) {
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
vector<double> dist(num_vertices(g));
init_shortest_paths(g, dist, pred);  // dist -> infinite_distance, pred -> self-id
dijkstra_shortest_paths(g, {s}, container_value_fn(dist), container_value_fn(pred),
    [](const auto& g, const auto& uv) { return edge_value(g, uv); });
```

> **Note:** BGL's full `dijkstra_shortest_paths` initializes the distance and predecessor
> maps internally. graph-v3 does not auto-initialize, so the caller must call
> `init_shortest_paths(g, dist, pred)` first (or pre-fill `dist` with `infinite_distance<double>()`).

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
graph::views::transpose_view tg(g);  // requires bidirectional graph
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

## 11. Adapting an Existing BGL Graph for graph-v3

This section describes how to make an existing `boost::adjacency_list` (or any BGL-modelling type) usable as input to graph-v3 algorithms and views, **without rewriting the storage**. The goal is incremental migration: keep the BGL container, expose graph-v3 CPOs on top of it.

> **✅ Library-provided adaptor available.** graph-v3 now ships a ready-to-use adaptor in `include/graph/adaptors/bgl/`:
> - `graph_adaptor.hpp` — one-line wrapper (`graph::bgl::graph_adaptor(bgl_g)`)
> - `bgl_edge_iterator.hpp` — C++20 iterator wrapper for BGL iterators
> - `property_bridge.hpp` — factory functions bridging BGL property maps to graph-v3 function objects
>
> See the [BGL Adaptor User Guide](../../docs/user-guide/bgl-adaptor.md) and [examples/bgl_adaptor_example.cpp](../../examples/bgl_adaptor_example.cpp) for usage.
> The manual approach described below remains useful for understanding the CPO dispatch mechanism or for adapting non-standard BGL types.

### 11.1 What graph-v3 Requires

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

> **Note:** This refers to *adapting* an external graph type — there is no trait to
> specialise to teach graph-v3 about your type; you satisfy the CPOs instead. This is
> distinct from `dynamic_graph`, the container used in many examples, which *does* take a
> traits template parameter that selects the container types for vertices and edges.
> Pre-defined combinations are provided as aliases in their own headers (e.g. `vov_graph`
> = vector-of-vector). Because those traits constrain containers via *concepts* (not
> concrete standard-library types), containers from other libraries can be used as long
> as they model the required concepts.

For BGL types you will use **option 2**: add free functions in `namespace boost`. ADL will find them because `boost::adjacency_list` lives in that namespace.

### 11.2 Recommended Layout

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

### 11.3 Minimal Adapter for `boost::adjacency_list`

> **Note:** graph-v3 already ships a complete, production-ready BGL adaptor — prefer it
> over hand-rolling your own. Include `<graph/adaptors/bgl/graph_adaptor.hpp>` (and
> `<graph/adaptors/bgl/property_bridge.hpp>` for property maps). It supports `vecS`/`listS`/`setS`
> storage, `directedS`/`undirectedS`/`bidirectionalS`, `in_edges`, and BGL property-map bridging.
> See [docs/user-guide/bgl-adaptor.md](../docs/user-guide/bgl-adaptor.md) and
> [examples/bgl_adaptor_example.cpp](../examples/bgl_adaptor_example.cpp).
>
> The minimal example below is kept for reference: it shows *how* CPO-based adaptation works
> and serves as a template for adapting other third-party graph types.

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

### 11.4 Bidirectional Graphs

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

### 11.5 Property Access

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

**No bundled properties.** When the BGL `adjacency_list` carries no bundled properties, you don't
need `vertex_value` / `edge_value` at all. Instead, define a separate function object per property
whose call interface takes the graph plus a descriptor — `prop(g, u)` for vertex properties or
`prop(g, uv)` for edge properties — and hand each one to the algorithms that consume it:

```cpp
// Pull values from an interior or exterior BGL property map.
auto weight = [wmap = ::boost::get(::boost::edge_weight, g)](const auto& g, auto uv) -> double {
    return ::boost::get(wmap, uv);     // prop(g, uv)
};
auto vcolor = [cmap = ::boost::get(::boost::vertex_color, g)](const auto& g, auto u) {
    return ::boost::get(cmap, u);      // prop(g, u)
};

graph::dijkstra_shortest_paths(g, {source}, distances, predecessors, weight);
```

Each property is an independent callable, so you only define the ones a given algorithm requires.

### 11.6 Other BGL Container Configurations

| BGL `OutEdgeS` / `VertexS` | Vertex descriptor | Adapter notes |
|----------------------------|-------------------|---------------|
| `vecS` / `vecS` | integer | Adapter above works as-is. |
| `listS` / `vecS` | integer (vertex), pointer-like (edge) | Adapter above works; descriptors still index into vertex table. |
| `vecS` / `listS` | opaque (pointer) | `vertex_id` cannot be the descriptor itself; build an external `unordered_map<vertex_descriptor, std::size_t>` and have `vertex_id` look up there. Algorithms then see a non-integer id; only `mapped_adjacency_list` algorithms apply. |
| `setS` / `vecS` | integer | Works; out-edge range is sorted. |
| `hash_setS` / `vecS` | integer | Works; out-edge range is unordered. |

For non-`vecS` `VertexS`, prefer copying the graph into a graph-v3 container at the migration boundary rather than maintaining an id-mapping adapter.

### 11.7 Verifying the Adapter

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

### 11.8 Migration Workflow

1. Drop in the adapter header — no source changes to existing BGL code.
2. Pick one new code path (e.g. a new analytics pass) and write it against graph-v3 CPOs, taking the existing BGL graph as input.
3. As tests cover more code paths, port BGL call sites one at a time using the API mappings in [Section 11](#11-api-pattern-migration-guide).
4. Once all algorithm call sites are ported, replace the BGL container with `graph::dynamic_graph` (or another graph-v3 container) and remove the adapter.

### 11.9 Limitations

- The adapter exposes the BGL graph as **read-only** to graph-v3. Mutation must continue to go through BGL APIs (`add_vertex`, `add_edge`).
- `partition_id`, `num_partitions`, and other multi-partition CPOs are not provided; graph-v3 defaults to a single partition, which is correct for plain BGL graphs.
- Algorithms that require `ranges::sized_range` on `edges(g, u)` (rare) need the BGL graph to use a sized out-edge container (`vecS`, `setS`).
- If `vertex_descriptor` is a pointer or opaque type, the integer-id assumption in `vertex_id` / `target_id` does not hold; see §11.6.

---

# Appendices

## Appendix A. Algorithm Gap Analysis

### Algorithms Present in Both Libraries

#### Traversal
| Algorithm | BGL | graph-v3 | Interface Differences |
|-----------|-----|----------|-----------------------|
| **BFS** | `breadth_first_search(g, s, visitor(v).color_map(c))` | `breadth_first_search(g, sources, visitor)` | graph-v3: multi-source native, no color map, concept-checked visitor |
| **DFS** | `depth_first_search(g, visitor(v).color_map(c))` | `depth_first_search(g, sources, visitor)` | graph-v3: edge classification (tree/back/forward_or_cross), multi-source |
| **Topological Sort** | `topological_sort(g, back_inserter(order))` | `topological_sort(g, output_iter)` | Both use output iterators; graph-v3 returns bool (cycle detection); also `vertices_topological_sort(g)` lazy view |

#### Shortest Paths
| Algorithm | BGL | graph-v3 | Interface Differences |
|-----------|-----|----------|-----------------------|
| **Dijkstra** | `dijkstra_shortest_paths(g, s, weight_map(w).distance_map(d).predecessor_map(p))` | `dijkstra_shortest_paths(g, sources, dist_fn, pred_fn, weight_fn, visitor)` | graph-v3: function-object properties, multi-source, custom compare/combine |
| **Bellman-Ford** | `bellman_ford_shortest_paths(g, N, weight_map(w).distance_map(d))` | `bellman_ford_shortest_paths(g, sources, dist_fn, pred_fn, weight_fn, visitor)` | Similar; graph-v3 detects negative cycles via visitor |

#### Minimum Spanning Tree
| Algorithm | BGL | graph-v3 | Interface Differences |
|-----------|-----|----------|-----------------------|
| **Kruskal MST** | `kruskal_minimum_spanning_tree(g, back_inserter(mst))` | `kruskal(edges, mst_output)` | graph-v3 works on edge lists directly; also `inplace_kruskal` |
| **Prim MST** | `prim_minimum_spanning_tree(g, pmap)` | `prim(g, source, mst_output, weight_fn)` | graph-v3 takes a source; thin wrapper over `dijkstra_shortest_paths` |

#### Connectivity
| Algorithm | BGL | graph-v3 | Interface Differences |
|-----------|-----|----------|-----------------------|
| **Connected Components** | `connected_components(g, comp_map)` | `connected_components(g, comp_fn)` | graph-v3 uses function objects |
| **Strong Components** | `strong_components(g, comp_map)` | `kosaraju(g, comp_fn)` + `tarjan_scc(g, comp_fn)` | graph-v3 has both Kosaraju and Tarjan; `tarjan_scc.hpp` is **not** in `algorithms.hpp` umbrella (include directly) |
| **Biconnected Components** | `biconnected_components(g, comp_map)` | `biconnected_components(g, output)` | Similar |
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

| Algorithm | Header | Notes |
|-----------|--------|-------|
| **Jaccard Coefficient** | `jaccard.hpp` | Similarity metric for adjacent vertex neighborhoods |
| **Label Propagation** | `label_propagation.hpp` | Community detection algorithm |
| **Maximal Independent Set** | `mis.hpp` | Greedy MIS |
| **Triangle Count** | `tc.hpp` | `triangle_count()` and `directed_triangle_count()` |
| **Afforest** | `connected_components.hpp` | Parallel-friendly connected components (Sutton et al.) |

---

## Appendix B. Recommended Extensions & Roadmap

This roadmap is for graph-v3 maintainers: a phased plan for closing the parity gaps in [Appendix A](#appendix-a-algorithm-gap-analysis). It is not a migration task list for BGL users.

### Phase 1: Critical Migration Enablers (High Priority)

These items block migration for the largest number of BGL users:

| Item | Type | Effort | Rationale |
|------|------|--------|-----------|
| **A\* Search** | Algorithm | Medium | Heavily used in pathfinding, robotics, game AI |
| **`copy_graph` utility** | Utility | Low | Cross-type graph copy with property mapping |
| **Betweenness Centrality** | Algorithm | Medium | Core network analysis metric |
| **PageRank** | Algorithm | Low | Widely used iterative algorithm |

> **Done since the previous revision of this plan:** `adjacency_matrix` dense container (C++20 + C++23 `mdspan` variant), `filtered_graph` adaptor, DOT/GraphML/JSON I/O plus full BGL I/O parity (DIMACS via `dimacs.hpp`, METIS via `metis.hpp`, adjacency-list text via `adjacency_list_text.hpp`), Erdős-Rényi G(n,p)/G(n,m) / Barabási-Albert / 2D grid / path / complete-graph / Watts-Strogatz / R-MAT / PLOD / SSCA#2 generators, `kosaraju` + `tarjan_scc`, `afforest`, library-shipped BGL adaptor (`include/graph/adaptors/bgl/`), composable visitor toolkit (`visitor_factory.hpp`: `make_visitor`, single-event adaptors, `predecessor_recorder`, `distance_recorder`, `time_stamper`), `valid_visitor` strict concept with `static_assert` diagnostics in BFS/DFS/Dijkstra/Bellman-Ford.

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
| ~~**DIMACS I/O**~~ | ~~I/O~~ | ~~Low~~ | ✅ Done — `dimacs.hpp` (`write_dimacs`, `write_dimacs_max_flow`, `read_dimacs`) |

### Phase 3: Advanced Features

| Item | Type | Effort | Rationale |
|------|------|--------|-----------|
| **`subgraph_view`** | Adaptor | High | Hierarchical subgraph with descriptor mapping |
| **`labeled_graph` adaptor** | Adaptor | Medium | String label → vertex mapping |
| **Boyer-Myrvold Planarity** | Algorithm | High | Planarity testing |
| **VF2 Subgraph Isomorphism** | Algorithm | High | Subgraph pattern matching |
| **Push-Relabel Max Flow** | Algorithm | High | High-performance max flow |
| **Max Cardinality Matching** | Algorithm | Medium | Bipartite matching |
| **Layout algorithms** | Algorithm | Medium | Graph visualization |
| ~~**Small World / PLOD generators**~~ | ~~Generator~~ | ~~Low~~ | ✅ Done — `watts_strogatz.hpp`, `plod.hpp`, `rmat.hpp`, `ssca.hpp` (full BGL generator parity) |
| ~~**Lambda visitor composition**~~ | ~~API~~ | ~~Low~~ | ✅ Done — `visitor_factory.hpp`: `make_visitor`, single-event adaptors, `predecessor_recorder`, `distance_recorder`, `time_stamper` |
| **BGL compatibility header** | Migration | Medium | `graph_traits` shim + name aliases for gradual migration |

### Phase 4: Ecosystem & Tooling

| Item | Type | Effort | Rationale |
|------|------|--------|-----------|
| ~~**METIS I/O**~~ | ~~I/O~~ | ~~Low~~ | ✅ Done — `metis.hpp` (`write_metis`, `read_metis`) |
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

### Standardization Strategy: Boost Incubation

A recurring objection to standardizing graph-v3 is the absence of field usage — the design has not yet been validated by a real user base. This is an evidence gap, not a design flaw, and WG21/LEWG explicitly weigh "existing practice" when evaluating library proposals.

**Strategy: ship graph-v3 to Boost as a modern successor to BGL ("BGL2") to gather users, then feed the resulting field experience back into the standards proposals (P3126–P3131, P3337).**

Why this strengthens — rather than competes with — the `std::graph` goal:

- **It directly answers the objection.** Boost adoption produces real users, bug reports, performance data versus BGL, and API-friction reports that paper review cannot surface.
- **Strong precedent.** `filesystem`, `optional`, `variant`, `any`, and networking (`asio`) all incubated in Boost before moving toward `std`. A Boost library *is* existing practice.
- **Right audience.** Boost users are standards-aware early adopters who will exercise the concept constraints, CPO ergonomics, error-message quality, and adaptor composition that most need validation.
- **Built-in migration population.** Positioning as "modern BGL" inherits BGL's mindshare and gives the library an immediate user base (the audience this document serves).

**Packaging guidance (avoid a fork):**

- Keep the **canonical source in `namespace graph`** (the working name that migrates cleanly to `std::graph`).
- Present the Boost-facing name (e.g. `boost::graph2` or `boost::graph::v2`) as an **alias / inline-namespace layer** over the canonical namespace, **not** a rename or fork. One source of truth, two presented names. This keeps the eventual `std::graph` migration a near-mechanical re-alias.
- Use Boost's pre-acceptance "no stability guarantee" window to keep refining the design while usage data accrues, so a Boost release does not prematurely calcify the API/ABI that the proposal depends on.

**Scope discipline:** the standardization core is the container / concept / CPO / traversal foundation (the core categories average ~78% in [Appendix C](#appendix-c-migration-readiness-scorecard)). Ship that foundation to Boost to validate it; the specialist algorithm domains still at 0% in Appendix A (flow, matching, coloring, planarity, isomorphism, ordering, layout) can land incrementally and block neither the Boost review nor the proposal.

**Sequencing:**

1. Boost release as "modern BGL," canonical `namespace graph` with a Boost-facing alias layer.
2. Collect usage: bug reports, benchmarks versus BGL, ergonomics/API-friction reports, and real-world coexistence testing of the CPO-suppresses-ADL behavior (see §11).
3. Feed refinements back into P3126–P3131 / P3337 as field-experience evidence.
4. Re-alias the validated subset into `std::graph`.

---

## Appendix C. Migration Readiness Scorecard

The scores below are directional editorial estimates, not audited counts.

| Category | BGL Features | graph-v3 Coverage | Score |
|----------|-------------|-------------------|-------|
| **Core graph types** | 8 containers | 4 containers + zero-config | 70% |
| **Concepts & traits** | 18+ concepts | 9 concepts (broader) | 80% (by design) |
| **Property system** | Interior + exterior + bundled (tag-dispatched) | Single `VV`/`EV` value (struct for multiple fields) + `container_value_fn` / `vertex_property_map` for external maps; tag dispatch eliminated by design | 100% (by design) |
| **Traversal algorithms** | BFS, DFS, undirected DFS, topological sort | BFS, DFS, topological sort + lazy views | 100% |
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
| **Graph adaptors** | 5 adaptors | 3 (transpose, filtered, BGL adaptor) | 60% |
| **Graph I/O** | 5 formats | 6 (DOT, GraphML, JSON, DIMACS, METIS, adjacency-list text) | 100% |
| **Graph generators** | 6 generators | 10 (path, grid, complete, Erdős–Rényi G(n,p)/G(n,m), Barabási–Albert, Watts–Strogatz, R-MAT, PLOD, SSCA#2) | 100% |
| **Visitors** | 5 types + composable adaptors | Concept-checked visitors + composable adaptors (`make_visitor`, `on_*` event wrappers, `predecessor_recorder`, `distance_recorder`, `time_stamper`). The remaining unimplemented visitor events are related to colored tranversal not supported in graph-v3. | 90% |
| **Graph mutation** | Full `MutableGraph` concept (CPOs) | Member-function mutation on both `dynamic_graph` and `undirected_adjacency_list`; no mutating CPOs | 70% |

**Overall estimated BGL API coverage: ~50%**

The unweighted average across all 20 scorecard rows is now ~50%, but the picture splits sharply:

- **Core/everyday categories** (graph types, architecture, properties, traversal, MST, connectivity, I/O, adaptors, generators, visitors, mutation — 12 rows): average ~80%. For a BGL user doing graph construction, traversal, shortest paths, MST, or connectivity work, graph-v3 covers the vast majority of the API surface.
- **Specialist algorithm domains** (network flow, matching, coloring, planarity, isomorphism, ordering, layout — 7 rows): all at 0%, and these pull the overall figure down significantly.

The coverage that exists is architecturally superior (C++20, ranges, concepts, CPOs, zero-config), and the library includes novel features (lazy traversal views, triangle counting, label propagation, Jaccard similarity) not found in BGL. The primary migration barrier is breadth of specialist algorithm coverage.
