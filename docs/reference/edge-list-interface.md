# Edge List Interface Reference

> [← Back to Documentation Index](../index.md) · [User Guide: Edge Lists](../user-guide/edge-lists.md)

This is the formal **Graph Container Interface (GCI)** specification for edge
lists, derived from P1709/D3130.

---

## Namespace

```cpp
namespace graph::edge_list { ... }
```

> **Note:** The edge list interface lives in `graph::edge_list`, not
> `graph::container::edgelist` (an older naming that may appear in some
> documents).

---

## Concepts

| Concept | Parameters | Description |
|---------|------------|-------------|
| `basic_sourced_edgelist<EL>` | Edge list `EL` | `input_range` of non-nested elements; `source_id(el, uv)` and `target_id(el, uv)` are valid. Vertex ID type can be any type. |
| `basic_sourced_index_edgelist<EL>` | Edge list `EL` | Refines `basic_sourced_edgelist`; `source_id` and `target_id` return `std::integral` types. |
| `has_edge_value<EL>` | Edge list `EL` | Refines `basic_sourced_edgelist`; `edge_value(el, uv)` is valid. |

### Concept Hierarchy

```
basic_sourced_edgelist<EL>
├── basic_sourced_index_edgelist<EL>   (+ integral vertex IDs)
└── has_edge_value<EL>                 (+ edge_value(el, uv))
```

---

## Type Aliases

| Alias | Constraint | Definition |
|-------|-----------|------------|
| `edge_range_t<EL>` | `basic_sourced_edgelist` | `EL` (the edge list itself) |
| `edge_iterator_t<EL>` | `basic_sourced_edgelist` | `std::ranges::iterator_t<EL>` |
| `edge_t<EL>` | `basic_sourced_edgelist` | `std::ranges::range_value_t<EL>` |
| `edge_value_t<EL>` | `has_edge_value` | `decltype(edge_value(el, uv))` |
| `vertex_id_t<EL>` | `basic_sourced_edgelist` | `decltype(source_id(el, uv))` |

---

## Type Traits

| Trait | Default | Description |
|-------|---------|-------------|
| `is_directed<EL>` | `std::false_type` | Specialize to `std::true_type` for directed edge lists |
| `is_directed_v<EL>` | `false` | Convenience variable template |

**Usage:** During graph construction from an edge list, a constructor may add a
reverse edge (B → A) for each forward edge (A → B) when `is_directed_v<EL>` is
`false`, producing an undirected graph.

---

## Functions (CPOs)

Edge list CPOs use the shared `graph::source_id`, `graph::target_id`, and
`graph::edge_value` CPOs (defined in `graph::detail`), which resolve for both
adjacency list edges and edge list edges.

| Function | Return Type | Complexity | Default Implementation |
|----------|-------------|------------|------------------------|
| `source_id(el, uv)` | `vertex_id_t<EL>` | O(1) | Automatic for tuple and edge_descriptor patterns |
| `target_id(el, uv)` | `vertex_id_t<EL>` | O(1) | Automatic for tuple and edge_descriptor patterns |
| `edge_value(el, uv)` | `edge_value_t<EL>` | O(1) | Automatic for 3+-element tuples and edge_descriptor patterns |
| `num_edges(el)` | integral | O(1) | `std::ranges::size(el)` |
| `has_edge(el)` | bool | O(1) | `num_edges(el) > 0` |
| `contains_edge(el, uid, vid)` | bool | O(E) | Linear search for matching `source_id`/`target_id` |

---

## Pattern Matching for Edge Types

The CPOs automatically recognize these element patterns:

### Tuple Patterns

| Element Type | `source_id` | `target_id` | `edge_value` |
|-------------|-------------|-------------|--------------|
| `tuple<integral, integral>` | `get<0>` | `get<1>` | n/a |
| `tuple<integral, integral, T>` | `get<0>` | `get<1>` | `get<2>` |

### `edge_descriptor` Patterns

| Element Type | `source_id` | `target_id` | `edge_value` |
|-------------|-------------|-------------|--------------|
| `edge_descriptor<VId, void>` | `.source_id` | `.target_id` | n/a |
| `edge_descriptor<VId, EV>` | `.source_id` | `.target_id` | `.value` |

For other edge types, override the CPOs via ADL in the edge type's namespace.

---

## Edge Descriptor

The library provides `graph::edge_descriptor<VId, EV>` as a convenient edge
element type:

```cpp
#include <graph/edge_list/edge_list_descriptor.hpp>

namespace graph {
template <class VId, class EV = void>
struct edge_descriptor {
    VId source_id;
    VId target_id;
    EV  value;       // omitted when EV = void
};
}
```

---

## Example: Using an Edge List

```cpp
#include <graph/edge_list/edge_list.hpp>
#include <vector>
#include <tuple>

// Tuple-based edge list
std::vector<std::tuple<int, int, double>> edges = {
    {0, 1, 1.5}, {1, 2, 2.0}, {2, 0, 0.5}
};

static_assert(graph::edge_list::basic_sourced_index_edgelist<decltype(edges)>);
static_assert(graph::edge_list::has_edge_value<decltype(edges)>);

for (auto& e : edges) {
    auto s = graph::source_id(edges, e);    // get<0>(e)
    auto t = graph::target_id(edges, e);    // get<1>(e)
    auto w = graph::edge_value(edges, e);   // get<2>(e)
}
```

---

## See Also

- [User Guide: Edge Lists](../user-guide/edge-lists.md) — tutorial-style guide
- [Adjacency List Interface](adjacency-list-interface.md) — GCI spec for adjacency lists
- [CPO Reference](cpo-reference.md) — full CPO documentation
- [Concepts Reference](concepts.md) — all concepts in one place
