# Edge Lists

> [← Back to Documentation Index](../index.md)

An **edge list** is a flat range of sourced edges — a simpler alternative to
the adjacency list when you only need to enumerate edges without fast per-vertex
access. Any `std::ranges::input_range` whose elements expose `source_id` and
`target_id` qualifies.

---

## 1. What Is an Edge List?

An edge list is a single range where each element describes one edge:

```
edge_list   (input_range)
  └── edge
        ├── source_id
        ├── target_id
        └── edge_value  (optional)
```

Unlike an adjacency list (a range of vertices, each with a range of edges), an
edge list has **no vertex-level structure**. This makes it ideal for:

- **Input parsing** — CSV, TSV, and Matrix Market files map directly to
  an edge list.
- **Graph construction** — containers like `dynamic_graph` and
  `compressed_graph` accept edge lists as construction input.
- **Edge-centric algorithms** — Kruskal's MST, edge filtering, etc.

> **Note:** The `views::edgelist(g)` view converts an adjacency list into an
> edge list by flattening its vertices and edges into a single range of sourced
> edges. The result satisfies `basic_sourced_edgelist`, so algorithms written
> against edge list concepts work interchangeably with a standalone edge list
> or one derived from an adjacency list.

---

## 2. Core Concepts

All edge list concepts live in `graph::edge_list`.

| Concept | Description |
|---------|-------------|
| `basic_sourced_edgelist<EL>` | An `input_range` whose elements support `source_id(el, uv)` and `target_id(el, uv)`. Explicitly excludes nested ranges (adjacency lists). |
| `basic_sourced_index_edgelist<EL>` | A `basic_sourced_edgelist` whose vertex IDs are `std::integral` (`int`, `size_t`, etc.) |
| `has_edge_value<EL>` | A `basic_sourced_edgelist` whose elements also support `edge_value(el, uv)` |

The distinction between `basic_sourced_edgelist` and
`basic_sourced_index_edgelist` matters: the basic concept allows **any** vertex
ID type (including `std::string`), while the index variant requires integral
IDs. Most graph algorithms require integral IDs for fast vertex lookup.

---

## 3. Edge Patterns

The library's CPOs (`source_id`, `target_id`, `edge_value`) detect common
element types automatically — no overrides needed:

| Element type | `source_id` | `target_id` | `edge_value` |
|--------------|-------------|-------------|--------------|
| `pair<T, T>` | `.first` | `.second` | — |
| `tuple<T, T>` | `get<0>` | `get<1>` | — |
| `tuple<T, T, EV, ...>` | `get<0>` | `get<1>` | `get<2>` |
| `edge_info<VId, true, void, void>` | `.source_id` | `.target_id` | — |
| `edge_info<VId, true, void, EV>` | `.source_id` | `.target_id` | `.value` |
| `edge_list::edge_descriptor<VId, void>` | `.source_id()` | `.target_id()` | — |
| `edge_list::edge_descriptor<VId, EV>` | `.source_id()` | `.target_id()` | `.value()` |

### Examples

```cpp
#include <graph/edge_list/edge_list.hpp>
#include <vector>

// Unweighted edges as pairs
std::vector<std::pair<int,int>> el1 = {{0,1}, {1,2}, {2,0}};

// Weighted edges as 3-tuples
std::vector<std::tuple<int,int,double>> el2 = {{0,1, 1.5}, {1,2, 2.0}};

// String vertex IDs (non-integral — satisfies basic_sourced_edgelist
// but NOT basic_sourced_index_edgelist)
std::vector<std::pair<std::string,std::string>> el3 = {
  {"Alice","Bob"}, {"Bob","Charlie"}
};

static_assert(graph::edge_list::basic_sourced_edgelist<decltype(el1)>);
static_assert(graph::edge_list::basic_sourced_index_edgelist<decltype(el1)>);
static_assert(!graph::edge_list::has_edge_value<decltype(el1)>);

static_assert(graph::edge_list::has_edge_value<decltype(el2)>);

static_assert(graph::edge_list::basic_sourced_edgelist<decltype(el3)>);
static_assert(!graph::edge_list::basic_sourced_index_edgelist<decltype(el3)>);
```

For user-defined edge types, override `source_id`, `target_id`, and optionally
`edge_value` as ADL free functions that take the edge list and edge as
arguments.

---

## 4. Vertex ID Types

| Vertex ID kind | Concept satisfied | Example types |
|----------------|-------------------|---------------|
| Integral | `basic_sourced_index_edgelist` | `int`, `size_t`, `uint32_t` |
| Non-integral | `basic_sourced_edgelist` only | `std::string`, user-defined keys |

Most graph-v3 algorithms and containers require **integral** vertex IDs because
they use the ID as an index into a random-access container. If your data uses
string keys, you can build a mapping from strings to integers and store the
edges as `pair<int,int>` or `tuple<int,int,EV>`.

---

## 5. CPOs and Type Aliases

### Edge CPOs

Edge CPOs take the edge list and an edge element:

| CPO | Returns |
|-----|---------|
| `source_id(el, uv)` | Source vertex ID |
| `target_id(el, uv)` | Target vertex ID |
| `edge_value(el, uv)` | Edge value (only when `has_edge_value<EL>`) |

### Type aliases

Convenience aliases in `graph::edge_list`:

| Alias | Meaning |
|-------|---------|
| `edge_range_t<EL>` | `EL` itself |
| `edge_iterator_t<EL>` | Iterator type of the edge range |
| `edge_t<EL>` | Value type of elements |
| `edge_value_t<EL>` | Type returned by `edge_value` (requires `has_edge_value`) |
| `vertex_id_t<EL>` | Type returned by `source_id` |

### Traits

| Trait | Description |
|-------|-------------|
| `is_directed<EL>` | Specialize to derive from `std::true_type` for directed edge lists |
| `is_directed_v<EL>` | Variable template shorthand |

`is_directed` affects graph construction: when `false` (the default),
containers may insert a reverse edge for each input edge to represent an
undirected graph.

---

## 6. Working with Edge Lists

### Iterating edges

```cpp
#include <graph/edge_list/edge_list.hpp>
#include <vector>
#include <iostream>

std::vector<std::tuple<int,int,double>> edges = {
  {0, 1, 1.5},
  {1, 2, 2.0},
  {2, 0, 0.5}
};

for (auto&& uv : edges) {
  std::cout << graph::source_id(edges, uv)
            << " -> " << graph::target_id(edges, uv)
            << " w=" << graph::edge_value(edges, uv) << "\n";
}
```

### Writing a generic algorithm

Constrain on the edge list concepts to write algorithms that work with any
edge representation:

```cpp
template <typename EL>
requires graph::edge_list::basic_sourced_edgelist<EL>
int count_self_loops(EL&& el) {
  int count = 0;
  for (auto&& uv : el) {
    if (graph::source_id(el, uv) == graph::target_id(el, uv))
      ++count;
  }
  return count;
}
```

This single function works with `pair`, `tuple`, `edge_info`, and
`edge_list::edge_descriptor` — all transparently.

### Constructing a graph from an edge list

Edge lists are the primary input for constructing graph containers:

```cpp
#include <graph/container/compressed_graph.hpp>

std::vector<std::tuple<int,int,double>> edges = {
  {0, 1, 1.5}, {1, 2, 2.0}, {2, 0, 0.5}
};

// compressed_graph accepts an edge list + vertex count
graph::container::compressed_graph<int> g(edges, 3);
```

---

## 7. Edge Lists vs. Adjacency Lists

| Property | Edge list | Adjacency list |
|----------|-----------|----------------|
| Structure | Flat range of edges | Range of vertices, each with a range of edges |
| Per-vertex access | O(n) scan | O(1) by index |
| Memory | One allocation | Nested allocations (or CSR) |
| Typical use | Input, construction, edge-centric algorithms | Most graph algorithms |
| Namespace | `graph::edge_list` | `graph::adj_list` (re-exported to `graph::`) |

In practice, you often **read** data as an edge list, **construct** an adjacency
list from it, and **run** algorithms on the adjacency list.

---

## See Also

- [Adjacency Lists User Guide](adjacency-lists.md) — the vertex-centric ADT
- [Containers User Guide](containers.md) — `dynamic_graph`, `compressed_graph` construction from edge lists
- [Container Interface (GCI spec)](../container_interface.md) — formal edge list specification
- [Getting Started](../getting-started.md) — edge list quick-start example
