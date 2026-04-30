<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# BGL Graph Adaptor

> Use graph-v3 algorithms and views on existing Boost.Graph data structures — zero copying, one-line setup.

</td>
</tr></table>

> [← Back to Documentation Index](../index.md)

---

## Table of Contents

- [When to Use](#when-to-use)
- [Quick Start](#quick-start)
- [Property Bridging](#property-bridging)
- [Supported BGL Graph Types](#supported-bgl-graph-types)
- [Bidirectional and Undirected Graphs](#bidirectional-and-undirected-graphs)
- [API Reference](#api-reference)
- [Known Limitations](#known-limitations)

---

## When to Use

Use the BGL adaptor when you:

- Have an existing codebase using Boost.Graph and want to incrementally adopt graph-v3 algorithms
- Want to compare BGL and graph-v3 algorithm results on the same graph data
- Need graph-v3's lazy views (BFS, DFS, topological sort) on a BGL graph
- Are migrating from BGL and want to reuse existing graph construction code

The adaptor is **non-owning** — it stores a pointer to your BGL graph, with no data copying.

---

## Quick Start

```cpp
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <boost/graph/adjacency_list.hpp>

// Your existing BGL graph
using BGL_Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                         boost::no_property, EdgeProp>;
BGL_Graph bgl_g(5);
boost::add_edge(0, 1, EdgeProp{1.5}, bgl_g);
boost::add_edge(1, 2, EdgeProp{2.5}, bgl_g);

// Wrap it — one line
auto g = graph::bgl::graph_adaptor(bgl_g);

// Now use any graph-v3 CPO or view
std::cout << graph::num_vertices(g) << " vertices\n";

for (auto u : graph::vertices(g)) {
  auto uid = graph::vertex_id(g, u);
  for (auto uv : graph::edges(g, u)) {
    std::cout << uid << " -> " << graph::target_id(g, uv) << "\n";
  }
}
```

---

## Property Bridging

BGL uses property maps for vertex/edge data. graph-v3 uses function objects. The `property_bridge.hpp` header provides factory functions to convert between them.

### Edge Weight Function

```cpp
#include <graph/adaptors/bgl/property_bridge.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>

struct EdgeProp { double weight; };

// Get BGL property map, wrap it for graph-v3
auto bgl_pm    = boost::get(&EdgeProp::weight, bgl_g);
auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(bgl_pm);
```

### Vertex Storage (Distance / Predecessor)

```cpp
std::vector<double>      dist(n, std::numeric_limits<double>::max());
std::vector<std::size_t> pred(n);

auto dist_fn = graph::bgl::make_vertex_id_property_fn(dist);
auto pred_fn = graph::bgl::make_vertex_id_property_fn(pred);
```

### Running Dijkstra

```cpp
dist_fn(g, std::size_t{0}) = 0.0;
std::vector<std::size_t> sources = {0};
graph::dijkstra_shortest_paths(g, sources, dist_fn, pred_fn, weight_fn);
```

### Custom Property Maps

For arbitrary BGL property maps, use the generic wrappers:

```cpp
// Readable property map (get only)
auto fn = graph::bgl::make_bgl_readable_property_map_fn(
    my_property_map,
    graph::bgl::edge_key_extractor{});

// Lvalue property map (read/write via reference)
auto fn = graph::bgl::make_bgl_lvalue_property_map_fn(
    my_property_map,
    graph::bgl::edge_key_extractor{});
```

---

## Supported BGL Graph Types

| BGL Type | Adaptor Support | Notes |
|----------|----------------|-------|
| `adjacency_list<vecS, vecS, directedS, ...>` | ✅ Full | Primary target; all CPOs and algorithms work |
| `adjacency_list<vecS, vecS, bidirectionalS, ...>` | ✅ Full | Includes `in_edges` support |
| `adjacency_list<vecS, vecS, undirectedS, ...>` | ✅ Full | `in_edges` delegates to `out_edges` |
| `compressed_sparse_row_graph<directedS, ...>` | ✅ Full | High-performance CSR; verified with Dijkstra |
| `adjacency_list<*, listS, *, ...>` | ✅ Supported | Non-integral vertex IDs; use `unordered_map` storage |
| `adjacency_list<*, setS, *, ...>` | ✅ Supported | Non-integral vertex IDs; use `unordered_map` storage |

**Note on non-vecS graphs:** When the vertex container is `listS` or `setS`, BGL vertex descriptors are `void*` (opaque pointers). The adaptor handles this transparently — vertices iteration, edges, source/target all work. However, algorithms requiring per-vertex storage (Dijkstra, BFS, etc.) cannot use `std::vector` indexed by vertex ID. Use `make_vertex_map_property_fn` with `std::unordered_map` instead.

---

## Non-vecS Graphs (listS / setS)

For BGL graphs using `listS` or `setS` as the vertex container, vertex descriptors are opaque `void*` pointers rather than integers. The adaptor supports these graphs with a few differences:

### Construction and Iteration

```cpp
#include <graph/adaptors/bgl/graph_adaptor.hpp>
#include <graph/adaptors/bgl/property_bridge.hpp>
#include <boost/graph/adjacency_list.hpp>

struct EdgeProp { double weight; };

// listS for vertex container — vertex descriptors are void*
using BGL_Graph = boost::adjacency_list<boost::vecS, boost::listS, boost::directedS,
                                         boost::no_property, EdgeProp>;
BGL_Graph bgl_g;
auto v0 = boost::add_vertex(bgl_g);
auto v1 = boost::add_vertex(bgl_g);
boost::add_edge(v0, v1, EdgeProp{3.14}, bgl_g);

auto g = graph::bgl::graph_adaptor(bgl_g);

// Works exactly like vecS graphs:
for (auto u : graph::vertices(g)) {
  for (auto uv : graph::edges(g, u)) {
    // vertex_id returns void* for non-vecS
    std::cout << graph::vertex_id(g, u) << " -> " << graph::target_id(g, uv) << "\n";
  }
}
```

### Per-Vertex Storage with `unordered_map`

Since vertex IDs are `void*` (not integers), you cannot index a `std::vector` by vertex ID. Instead, use `std::unordered_map<void*, T>` with `make_vertex_map_property_fn`:

```cpp
using vid_t = boost::graph_traits<BGL_Graph>::vertex_descriptor;  // void*

std::unordered_map<vid_t, double> dist;
std::unordered_map<vid_t, vid_t>  pred;

// Initialize all vertices
for (auto u : graph::vertices(g)) {
  auto vid = graph::vertex_id(g, u);
  dist[vid] = std::numeric_limits<double>::max();
  pred[vid] = vid;
}

auto dist_fn = graph::bgl::make_vertex_map_property_fn(dist);
auto pred_fn = graph::bgl::make_vertex_map_property_fn(pred);

// Use with algorithms
dist_fn(g, source_vertex) = 0.0;
```

---

## Bidirectional and Undirected Graphs

For BGL graphs with `bidirectionalS` or `undirectedS`, the adaptor automatically provides `in_edges`:

```cpp
using BGL_Bidir = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                          boost::no_property, EdgeProp>;
BGL_Bidir bgl_g(4);
boost::add_edge(0, 1, EdgeProp{1.0}, bgl_g);
boost::add_edge(2, 1, EdgeProp{2.0}, bgl_g);

auto g = graph::bgl::graph_adaptor(bgl_g);

// in_edges available — iterates edges incoming to vertex 1
for (auto u : graph::vertices(g)) {
  if (graph::vertex_id(g, u) == 1) {
    for (auto uv : graph::in_edges(g, u)) {
      std::cout << "incoming from " << graph::source_id(g, uv) << "\n";
    }
  }
}
```

---

## API Reference

### Headers

| Header | Contents |
|--------|----------|
| `<graph/adaptors/bgl/graph_adaptor.hpp>` | `graph_adaptor` class + ADL CPO functions |
| `<graph/adaptors/bgl/bgl_edge_iterator.hpp>` | `bgl_edge_iterator` C++20 iterator wrapper |
| `<graph/adaptors/bgl/property_bridge.hpp>` | Property map bridging utilities |

### `graph_adaptor<BGL_Graph>`

| Member | Description |
|--------|-------------|
| `graph_adaptor(BGL_Graph& g)` | Construct from mutable BGL graph reference |
| `bgl_graph()` | Access underlying BGL graph (const and non-const) |

### ADL Functions (found by graph-v3 CPOs)

| Function | CPO it satisfies |
|----------|-----------------|
| `vertices(ga)` | `graph::vertices` |
| `num_vertices(ga)` | `graph::num_vertices` |
| `edges(ga, u)` | `graph::edges` (out-edges) |
| `in_edges(ga, u)` | `graph::in_edges` (bidirectional/undirected only) |
| `target_id(ga, uv)` | `graph::target_id` |
| `source_id(ga, uv)` | `graph::source_id` |

### Property Bridge Factories

| Factory | Returns | Use |
|---------|---------|-----|
| `make_bgl_edge_weight_fn(pm)` | Edge weight function | Dijkstra, Bellman-Ford |
| `make_bgl_readable_property_map_fn(pm, key_extractor)` | Generic readable fn | Any edge/vertex property |
| `make_bgl_lvalue_property_map_fn(pm, key_extractor)` | Writable fn (lvalue ref) | Mutable properties |
| `make_vertex_id_property_fn(vec)` | Vertex-indexed fn | Distance, predecessor vectors (vecS only) |
| `make_vertex_map_property_fn(map)` | Map-based vertex fn | Distance, predecessor for non-vecS graphs |

---

## Known Limitations

- **Non-vecS graphs require `unordered_map` storage** — Algorithms that need per-vertex data (distances, predecessors) cannot use `std::vector` indexing for `listS`/`setS` graphs. Use `make_vertex_map_property_fn` with `std::unordered_map<void*, T>`.
- **Non-vecS graphs do not satisfy `index_adjacency_list`** — Only the `adjacency_list` concept is satisfied when vertex IDs are non-integral.
- **No `vertex_value` or `edge_value` CPO** — The adaptor does not define these CPOs. Access BGL properties through the property bridge functions or directly via BGL's `get()`.
- **No graph mutation** — `add_vertex`, `add_edge`, `remove_vertex`, `remove_edge` are not adapted. Mutate through the underlying BGL graph directly.
- **Boost headers required** — The adaptor headers include `<boost/graph/graph_traits.hpp>`. Callers must have Boost headers available.

---

> [← Back to Documentation Index](../index.md)
