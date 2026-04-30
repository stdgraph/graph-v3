<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Graph Adaptors

> Non-owning wrappers that present a modified view of an existing graph.
> Adaptors satisfy the same concepts as the underlying graph, so all CPOs,
> views, and algorithms work transparently.

</td>
</tr></table>

> [← Back to Documentation Index](../index.md)

| Adaptor | Header | Purpose |
|---------|--------|---------|
| [`filtered_graph`](#1-filtered_graph) | `graph/adaptors/filtered_graph.hpp` | Filter vertices and/or edges by predicate |
| [BGL Adaptor](#2-bgl-adaptor) | `graph/adaptors/bgl/graph_adaptor.hpp` | Use Boost.Graph graphs with graph-v3 algorithms |

---

## 1. `filtered_graph`

```cpp
#include <graph/adaptors/filtered_graph.hpp>

namespace graph::adaptors {
template <typename G,
          typename VertexPredicate = keep_all,
          typename EdgePredicate   = keep_all>
class filtered_graph;
}
```

A non-owning adaptor that wraps any graph-v3 graph and filters vertices and edges
during traversal. The filtered graph satisfies the same concepts as the underlying
graph (`adjacency_list`, `index_adjacency_list`), so all views and algorithms work
on it directly.

### Predicates

| Predicate | Signature | Semantics |
|-----------|-----------|-----------|
| `VertexPredicate` | `(vertex_id_t<G>) → bool` | Controls which vertices are "visible" — edges to/from excluded vertices are skipped |
| `EdgePredicate` | `(vertex_id_t<G> source, vertex_id_t<G> target) → bool` | Controls which specific edges are included |

The default predicate `keep_all` accepts everything (zero overhead via `[[no_unique_address]]`).

### Construction

```cpp
using G = std::vector<std::vector<std::pair<int, double>>>;
G g = make_my_graph();

// Filter with both predicates
auto fg = graph::adaptors::filtered_graph(g,
    [](auto uid) { return uid != 2; },                    // exclude vertex 2
    [](auto src, auto tgt) { return !(src == 0 && tgt == 1); }); // exclude edge 0→1

// Filter vertices only (edges use keep_all)
auto fg_v = graph::adaptors::filtered_graph(g,
    [](auto uid) { return uid % 2 == 0; });  // keep even vertices

// No filtering (pass-through, useful for testing)
auto fg_all = graph::adaptors::filtered_graph(g);
```

### Usage with Views and Algorithms

Since `filtered_graph` satisfies `adjacency_list`, standard views and algorithms work:

```cpp
#include <graph/views/vertexlist.hpp>
#include <graph/views/incidence.hpp>
#include <graph/algorithm/dijkstra_shortest_paths.hpp>

// Iterate filtered edges
for (auto&& [uid, u] : graph::views::vertexlist(fg)) {
  for (auto&& [tid, uv] : graph::views::incidence(fg, u)) {
    // Only edges passing both predicates appear here
  }
}

// Run Dijkstra on the filtered subgraph
std::vector<double> dist(graph::num_vertices(fg));
std::vector<size_t> pred(graph::num_vertices(fg));
graph::init_shortest_paths(dist, pred);
graph::dijkstra_shortest_paths(fg, 0u, dist, pred,
    [](const auto& g, const auto& uv) { return graph::edge_value(g, uv); });
```

### Filtered Vertex Iteration

`vertices(fg)` returns the **full** underlying vertex range (unfiltered) to preserve
`sized_range` for concept satisfaction. For lazy filtered vertex iteration:

```cpp
for (auto&& u : graph::adaptors::filtered_vertices(fg)) {
  auto uid = graph::vertex_id(fg, u);
  // Only vertices passing vertex_pred appear here
}
```

### Design Notes

- **Non-owning** — `filtered_graph` stores a pointer to the underlying graph. The graph must outlive the adaptor.
- **Lazy filtering** — edges are filtered on-the-fly during iteration, not materialized.
- **Self-contained iterators** — uses `filtering_iterator` internally (stores predicate + end sentinel by value) to avoid the dangling-reference problem inherent to `std::views::filter`.
- **Vertex range is unfiltered** — `num_vertices(fg)` returns the underlying count. Algorithms that allocate per-vertex storage (distance arrays, etc.) use the full vertex count, which is correct since filtered-out vertices simply won't be visited.

---

## 2. BGL Adaptor

```cpp
#include <graph/adaptors/bgl/graph_adaptor.hpp>

namespace graph::bgl {
template <typename BGL_Graph>
class graph_adaptor;
}
```

A non-owning wrapper that adapts any Boost.Graph (BGL) graph for use with graph-v3
CPOs, views, and algorithms. The adapted graph satisfies `adjacency_list` and
`index_adjacency_list`.

### Supported BGL Graph Types

Any BGL graph satisfying `VertexListGraph` and `IncidenceGraph`:
- `boost::adjacency_list<...>` (all storage variants)
- `boost::compressed_sparse_row_graph<...>` (CSR)
- Bidirectional graphs (exposes `in_edges` when `bidirectional_graph_tag` is present)

### Construction

```cpp
#include <boost/graph/adjacency_list.hpp>
#include <graph/adaptors/bgl/graph_adaptor.hpp>

using BGL = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                   boost::no_property,
                                   boost::property<boost::edge_weight_t, double>>;
BGL bgl_g;
// ... populate bgl_g ...

auto g = graph::bgl::graph_adaptor(bgl_g);  // CTAD
```

### Property Bridge

The property bridge maps BGL property maps to graph-v3 value functions:

```cpp
#include <graph/adaptors/bgl/property_bridge.hpp>

// Edge weight extraction
auto weight_fn = graph::bgl::make_bgl_edge_weight_fn(bgl_g);

// Use with Dijkstra
std::vector<double> dist(graph::num_vertices(g));
std::vector<size_t> pred(graph::num_vertices(g));
graph::init_shortest_paths(dist, pred);
graph::dijkstra_shortest_paths(g, 0u, dist, pred, weight_fn);
```

### Key Components

| Header | Purpose |
|--------|---------|
| `graph/adaptors/bgl/graph_adaptor.hpp` | Main adaptor class + ADL free functions |
| `graph/adaptors/bgl/bgl_edge_iterator.hpp` | C++20 iterator wrapper for BGL iterators |
| `graph/adaptors/bgl/property_bridge.hpp` | Bridge BGL property maps to graph-v3 value functions |

### Design Notes

- **Non-owning** — `graph_adaptor` stores a pointer to the BGL graph.
- **ADL-based dispatch** — uses `graph_bgl_adl` namespace (outside `namespace graph`) to call BGL functions without CPO interference.
- **Bidirectional support** — `in_edges(g, u)` is available when the BGL graph has `bidirectional_graph_tag`.
- **Build configuration** — requires `-DTEST_BGL_ADAPTOR=ON -DBGL_INCLUDE_DIR=<path-to-boost>` for tests.

---

## When to Use Adaptors

| Scenario | Solution |
|----------|----------|
| Run algorithms on a subgraph | `filtered_graph` with vertex/edge predicates |
| Exclude vertices by condition | `filtered_graph` with vertex predicate |
| Exclude specific edges | `filtered_graph` with edge predicate |
| Use BGL graphs with graph-v3 | `graph_adaptor` |
| Reverse edge direction | `transpose` view (see [Views](views.md)) |
