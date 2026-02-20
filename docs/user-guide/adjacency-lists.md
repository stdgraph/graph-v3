# Adjacency Lists

> [← Back to Documentation Index](../index.md)

An **adjacency list** is the primary graph abstraction in graph-v3.
It models a graph as a *range of vertices*, where each vertex owns a *range of
outgoing edges*. This "range-of-ranges" view maps directly to standard
containers — a `std::vector<std::vector<int>>` is already a valid adjacency list.

---

## 1. What Is an Adjacency List?

Conceptually:

```
graph
  ├── graph_value       (optional)
  └── vertices          (forward, bidirectional or random-access range)
        └── vertex
              ├── vertex_id     (index or key)
              ├── vertex_value  (optional)
              └── edges         (forward range of outgoing edges)
                    └── edge
                          ├── target_id
                          ├── source_id
                          └── edge_value  (optional)
```

Any data structure that can expose this shape — either natively or through
CPO overrides — satisfies the adjacency list concepts and works with all
graph-v3 algorithms and views.

---

## 2. Core Concepts

graph-v3 defines 9 concepts in `graph::adj_list` (re-exported into `graph::`).

### Edge concepts

| Concept | Description |
|---------|-------------|
| `edge<G, E>` | Edge descriptor with `source_id`, `source`, `target_id`, and `target` |
| `vertex_edge_range<R, G>` | Forward range whose elements satisfy `edge` |

### Vertex concepts

| Concept | Description |
|---------|-------------|
| `vertex<G, V>` | Vertex descriptor with `vertex_id(g, u)` and `find_vertex(g, uid)` |
| `vertex_range<R, G>` | Forward + sized range of vertices |
| `index_vertex_range<G>` | Vertex range backed by a random-access container with integral IDs |

### Graph concepts

| Concept | Description |
|---------|-------------|
| `adjacency_list<G>` | `vertices(g)` returns a `vertex_range`; `edges(g, u)` returns a `vertex_edge_range` |
| `index_adjacency_list<G>` | An `adjacency_list` whose vertex range is also an `index_vertex_range` (O(1) vertex lookup) |
| `ordered_vertex_edges<G>` | An `adjacency_list` whose edge ranges are sorted by `target_id` (enables algorithms like triangle counting) |

Most library algorithms require `index_adjacency_list`. Standard containers
such as `vector<vector<T>>` and `deque<vector<T>>` satisfy it automatically.

---

## 3. Accessing Graph Structure — CPOs

All graph operations are **Customization Point Objects (CPOs)** that resolve in
three tiers: custom member → ADL free function → built-in default.

### Structure CPOs

| CPO | Signature sketch | Returns |
|-----|------------------|---------|
| `vertices(g)` | `g` → vertex range | `vertex_descriptor_view` |
| `edges(g, u)` | `g`, vertex → edge range | `edge_descriptor_view` |
| `vertex_id(g, u)` | `g`, vertex → id | Integral index or key |
| `target_id(g, uv)` | `g`, edge → id | Target vertex ID |
| `target(g, uv)` | `g`, edge → vertex | Target vertex descriptor |
| `source_id(g, uv)` | `g`, edge → id | Source vertex ID |
| `source(g, uv)` | `g`, edge → vertex | Source vertex descriptor |
| `find_vertex(g, uid)` | `g`, id → vertex | Vertex descriptor |
| `num_vertices(g)` | `g` → count | `size_t` |
| `num_edges(g, u)` | `g`, vertex → count | `size_t` |
| `degree(g, u)` | `g`, vertex → count | Number of outgoing edges |

### Query CPOs

| CPO | Description |
|-----|-------------|
| `find_vertex_edge(g, u, v)` | Find edge from vertex `u` to vertex `v` |
| `contains_edge(g, u, v)` | Returns `true` if edge u→v exists |
| `has_edge(g, uid, vid)` | Returns `true` if edge uid→vid exists |

### Partition CPOs

| CPO | Description |
|-----|-------------|
| `partition_id(g, u)` | Partition index of vertex `u` |
| `num_partitions(g)` | Total number of partitions |

### Example

```cpp
#include <graph/graph.hpp>
#include <vector>
#include <iostream>

int main() {
  std::vector<std::vector<int>> g = {
    {1, 2},   // vertex 0
    {2, 3},   // vertex 1
    {3},      // vertex 2
    {}        // vertex 3
  };

  for (auto&& u : graph::vertices(g)) {
    auto uid = graph::vertex_id(g, u);
    std::cout << "vertex " << uid
              << "  degree=" << graph::degree(g, u) << "\n";
    for (auto&& uv : graph::edges(g, u)) {
      std::cout << "  -> " << graph::target_id(g, uv) << "\n";
    }
  }
}
```

---

## 4. Vertex and Edge Values

Three value-access CPOs extract user-defined data from vertices, edges, and
the graph itself:

| CPO | Signature sketch | Description |
|-----|------------------|-------------|
| `vertex_value(g, u)` | `g`, vertex → `VV&` | User data on a vertex (e.g., label, weight) |
| `edge_value(g, uv)` | `g`, edge → `EV&` | User data on an edge (e.g., weight, capacity) |
| `graph_value(g)` | `g` → `GV&` | User data on the whole graph (metadata) |

These are only available when the corresponding value type is non-void in the
container's template parameters. For `dynamic_graph`:

```cpp
using namespace graph::container;

// EV=double, VV=std::string, GV=void
using Graph = vov_graph_traits<double, std::string>::graph_type;
```

Here `edge_value(g, uv)` returns `double&` and `vertex_value(g, u)` returns
`std::string&`. Since `GV=void`, `graph_value(g)` is not available.

---

## 5. Descriptors

Descriptors are lightweight value objects that **identify** a vertex or edge.
They are the currency of graph-v3's interface.

A descriptor is either an **index** or an **iterator**, depending on the
underlying container in the physical graph. Its greatest value is abstracting
the container's details so that calling code is unaffected by the storage
strategy. Because a single descriptor type can represent both an index into a
`vector` and an iterator into a `map`, the library needs far fewer concept
definitions and function overloads than an iterator-centric or reference-centric
design would require.

Index-based vertex descriptors provide an additional level of abstraction:
they can refer to a vertex even when no physical vertex object exists in the
graph (e.g., in `compressed_graph` or an implicit grid). The vertex is
identified purely by its index, so algorithms and views work the same way
regardless of whether the container materializes vertex storage.

### Why descriptors?

| Concern | Raw iterators | Descriptors |
|---------|---------------|-------------|
| Abstraction | Expose container layout | Hide storage strategy — one concept covers indexed and keyed graphs |
| Overload reduction | Separate overloads for index vs. iterator | A single overload accepts the descriptor |
| Invalidation | Invalidated by container mutation | Can be re-resolved via `find_vertex` |
| Copyability | Varies (e.g., `forward_list::iterator`) | Always trivially copyable or cheap |
| Size | May be pointer-sized or larger | Index-based: one integer |

### Descriptor types

Descriptors are value types that identify a single vertex or edge:

| Type | Description |
|------|-------------|
| `vertex_descriptor<VId>` | Holds an index or iterator that identifies a vertex; provides `vertex_id()`, `value()`, `inner_value()` |
| `edge_descriptor<VId, EI>` | Holds an edge iterator + source vertex descriptor; provides `source_id()`, `target_id()`, `value()` |

Descriptor views are range adaptors that wrap an entire physical range and
yield one descriptor per element:

| Type | Description |
|------|-------------|
| `vertex_descriptor_view<G>` | Wraps the physical vertex range, yielding a `vertex_descriptor` per element |
| `edge_descriptor_view<G>` | Wraps the physical edge range for a vertex, yielding an `edge_descriptor` per element |

For **index-based** graphs (e.g., backed by `std::vector`), a vertex descriptor
holds an integer index. For **keyed** graphs it wraps a bidirectional iterator (e.g., for `std::map`)
or forward iterator (e.g., backed by `std::unordered_map`). In all cases the
same CPOs — `vertex_id`, `edges`, `target_id`, etc. — work unchanged, which is
the whole point. This extends to containers defined in external libraries as
well, as long as they satisfy `random_access_range`, `bidirectional_range`, or
`forward_range`.

`vertex_descriptor_view<G>` and `edge_descriptor_view<G>` are forward ranges,
like most views. This doesn't limit graph algorithms because they only need
forward iteration on vertices and edges after the range is created.


### Automatic pattern recognition

You don't usually need to spell out descriptor types. The CPOs detect
containers automatically:

- **Random-access** containers (`vector`, `deque`) → index-based IDs
- **Associative** containers (`map`, `unordered_map`) → key-based IDs
- **Edge patterns** → common element types such as `int`, `pair<int, W>`, `tuple<int, ...>`, or structs are recognized to automatically extract edge properties (target ID, edge value, etc.)

---

## 6. Working with Views

Views provide structured-binding-friendly iteration over graph structure.
Instead of calling CPOs manually inside loops, a view bundles the results
into a tuple you can destructure:

```cpp
#include <graph/graph.hpp>   // includes vertexlist + incidence views

using namespace graph;

// CPO style (manual extraction):
for (auto&& u : vertices(g)) {
  auto uid = vertex_id(g, u);
  for (auto&& uv : edges(g, u)) {
    auto vid = target_id(g, uv);
    // ...
  }
}

// View style (structured bindings):
for (auto&& [uid, u] : views::vertexlist(g)) {
  for (auto&& [vid, uv] : views::incidence(g, u)) {
    // uid, vid, uv are ready to use
  }
}
```

Views also support **value functions** that project custom values:

```cpp
// With an edge value function:
for (auto&& [tid, uv, wt] : views::incidence(g, u, [](auto& g, auto& uv) {
       return edge_value(g, uv).weight; // The edge value is a struct with multiple members
     })) {
  std::cout << tid << " weight=" << wt << "\n";
}
```

Available views: `vertexlist`, `edgelist`, `incidence`, `neighbors`, `bfs`,
`dfs`, `topological_sort`. See the [Views User Guide](views.md) for details.

---

## 7. Compile-Time Traits

Traits let you query a graph type's capabilities at compile time:

| Trait | What it checks |
|-------|----------------|
| `has_degree<G>` | `degree(g, u)` and `degree(g, uid)` are valid |
| `has_find_vertex<G>` | `find_vertex(g, uid)` returns `vertex_t<G>` |
| `has_find_vertex_edge<G>` | All three `find_vertex_edge` overloads work |
| `has_contains_edge<G, V>` | `contains_edge(g, u, v)` returns `bool` |
| `has_basic_queries<G>` | `has_degree` ∧ `has_find_vertex` ∧ `has_find_vertex_edge` |
| `has_full_queries<G>` | `has_basic_queries` ∧ `has_contains_edge` |

Each trait also has a `_v` variable template (e.g., `has_degree_v<G>`).

---

## 8. Type Aliases

Convenience aliases extracted from a graph type `G`:

| Alias | Meaning |
|-------|---------|
| `vertex_range_t<G>` | Return type of `vertices(g)` (a `vertex_descriptor_view`) |
| `vertex_iterator_t<G>` | Iterator type of the vertex range |
| `vertex_t<G>` | Value type of the vertex range (a vertex descriptor) |
| `vertex_id_t<G>` | Vertex ID type (`size_t` for indexed, key type for map-based) |
| `vertex_edge_range_t<G>` | Return type of `edges(g, u)`  (an `edge_descriptor_view`)|
| `vertex_edge_iterator_t<G>` | Iterator type of the edge range |
| `edge_t<G>` | Value type of the edge range (an edge descriptor) |

---

## See Also

- [Container Interface (GCI spec)](../container_interface.md) — formal adjacency list specification
- [Edge Lists User Guide](edge-lists.md) — the peer edge-centric ADT
- [Views User Guide](views.md) — BFS, DFS, incidence, neighbors, etc.
- [CPO Implementation Guide](../graph_cpo_implementation.md) — how to write/override CPOs
- [Getting Started](../getting-started.md) — installation and first examples
