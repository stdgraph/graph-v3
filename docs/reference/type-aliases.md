# Type Aliases Reference

> [← Back to Documentation Index](../index.md) · [CPO Reference](cpo-reference.md)

All type aliases below are available in `namespace graph` after
`#include <graph/graph.hpp>`. They are re-exported from
`namespace graph::adj_list`.

---

## Adjacency List Type Aliases

These aliases extract types from a graph `G` that models the adjacency list
concepts.

| Alias | Definition | Required Concept |
|-------|-----------|-----------------|
| `vertex_range_t<G>` | Type of the range returned by `vertices(g)` | `vertex_range` |
| `vertex_iterator_t<G>` | `std::ranges::iterator_t<vertex_range_t<G>>` | `vertex_range` |
| `vertex_t<G>` | `std::ranges::range_value_t<vertex_range_t<G>>` | `vertex_range` |
| `vertex_id_t<G>` | Result type of `vertex_id(g, ui)` — typically `std::size_t` | `index_vertex_range` |
| `out_edge_range_t<G>` | Type of the range returned by `out_edges(g, u)` | `vertex` |
| `out_edge_iterator_t<G>` | `std::ranges::iterator_t<out_edge_range_t<G>>` | `vertex` |
| `out_edge_t<G>` | `std::ranges::range_value_t<out_edge_range_t<G>>` | `vertex` |

> **Convenience aliases:** `vertex_edge_range_t<G>` = `out_edge_range_t<G>`, `vertex_edge_iterator_t<G>` = `out_edge_iterator_t<G>`, `edge_t<G>` = `out_edge_t<G>`. The old names remain available.

### Usage

```cpp
#include <graph/graph.hpp>

template <graph::index_adjacency_list G>
void example(G& g) {
    using VId = graph::vertex_id_t<G>;       // e.g. size_t
    using Edge = graph::edge_t<G>;            // edge value type
    
    for (auto& u : graph::vertices(g)) {
        VId uid = graph::vertex_id(g, std::ranges::begin(graph::vertices(g)));
        for (Edge& uv : graph::edges(g, u)) {
            VId vid = graph::target_id(g, uv);
        }
    }
}
```

---

## Edge List Type Aliases

These aliases extract types from an edge list `EL` that models the edge list
concepts. They live in `namespace graph::edge_list`.

| Alias | Definition | Required Concept |
|-------|-----------|-----------------|
| `edge_range_t<EL>` | The edge list type itself | `basic_sourced_edgelist` |
| `edge_iterator_t<EL>` | `std::ranges::iterator_t<EL>` | `basic_sourced_edgelist` |
| `edge_t<EL>` | `std::ranges::range_value_t<EL>` | `basic_sourced_edgelist` |
| `edge_value_t<EL>` | Result type of `edge_value(el, e)` | `has_edge_value` |
| `vertex_id_t<EL>` | Result type of `target_id(el, e)` | `basic_sourced_edgelist` |

### Usage

```cpp
#include <graph/edge_list/edge_list.hpp>

template <graph::edge_list::basic_sourced_index_edgelist EL>
void process(EL& el) {
    using VId = graph::edge_list::vertex_id_t<EL>;
    using Edge = graph::edge_list::edge_t<EL>;
    
    for (Edge& e : el) {
        VId src = graph::source_id(el, e);
        VId tgt = graph::target_id(el, e);
    }
}
```

---

## Partition Type Aliases

Available when the graph supports partitioning.

| Alias | Definition | Required Concept |
|-------|-----------|-----------------|
| `partition_id_t<G>` | Result type of `partition_id(g, u)` | Graph supports `partition_id` CPO |

---

## Type Alias Relationship to CPOs

```
vertices(g)  ──→  vertex_range_t<G>
                   ├── iterator_t  ──→  vertex_iterator_t<G>
                   └── range_value_t  ──→  vertex_t<G>

vertex_id(g, ui)  ──→  vertex_id_t<G>

edges(g, u)  ──→  vertex_edge_range_t<G>      (primary: out_edge_range_t<G>)
                   ├── iterator_t  ──→  vertex_edge_iterator_t<G>  (primary: out_edge_iterator_t<G>)
                   └── range_value_t  ──→  edge_t<G>               (primary: out_edge_t<G>)
```

---

## See Also

- [CPO Reference](cpo-reference.md) — CPO signatures that produce these types
- [Concepts Reference](concepts.md) — concepts these aliases require
- [Adjacency List Interface](adjacency-list-interface.md) — full GCI spec
- [Edge List Interface](edge-list-interface.md) — edge list GCI spec
