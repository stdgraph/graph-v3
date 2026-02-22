# Concepts Reference

> [← Back to Documentation Index](../index.md) · [CPO Reference](cpo-reference.md)

All graph concepts live in `namespace graph::adj_list` (adjacency lists) or
`namespace graph::edge_list` (edge lists) and are re-exported into
`namespace graph` via `<graph/graph.hpp>`.

---

## Adjacency List Concepts

Header: `<graph/adj_list/adjacency_list_concepts.hpp>`

### Concept Hierarchy

```
edge<G, E>
└── out_edge_range<R, G>
    └── vertex<G, V>
        └── vertex_range<R, G>
            └── index_vertex_range<G>
                └── adjacency_list<G>
                    └── index_adjacency_list<G>
                        └── ordered_vertex_edges<G>
```

### `edge<G, E>`

An edge exposes at least a target vertex ID.

```cpp
template <class G, class E = void>
concept edge = requires(G& g, E& e) {
    { target_id(g, e) } -> std::convertible_to<vertex_id_t<G>>;
};
```

### `out_edge_range<R, G>`

A forward range of outgoing edges adjacent to a vertex.

```cpp
template <class R, class G>
concept out_edge_range =
    std::ranges::forward_range<R> &&
    edge<G, std::ranges::range_value_t<R>>;
```

### `vertex<G, V>`

A vertex exposes an edge range via the `edges` CPO.

```cpp
template <class G, class V = void>
concept vertex = requires(G& g, V& u) {
    { edges(g, u) } -> out_edge_range<G>;
};
```

### `vertex_range<R, G>`

A forward range of vertices, each exposing an edge range.

```cpp
template <class R, class G>
concept vertex_range =
    std::ranges::forward_range<R> &&
    vertex<G, std::ranges::range_value_t<R>>;
```

### `index_vertex_range<G>`

Vertices are in a random-access, sized container addressable by integral IDs.

```cpp
template <class G>
concept index_vertex_range = requires(G& g) {
    // vertices(g) is random_access_range and sized_range
    // vertex_id(g, ui) is integral
    // find_vertex(g, uid) returns random_access_iterator
};
```

| Requirement | Expression |
|-------------|-----------|
| Random-access vertices | `std::ranges::random_access_range<decltype(vertices(g))>` |
| Sized vertex range | `std::ranges::sized_range<decltype(vertices(g))>` |
| Integral vertex IDs | `std::integral<vertex_id_t<G>>` |
| O(1) vertex lookup | `find_vertex(g, uid)` returns `std::random_access_iterator` |

### `adjacency_list<G>`

Full adjacency list: random-access indexed vertices, each with a forward edge range.

```cpp
template <class G>
concept adjacency_list =
    index_vertex_range<G> &&
    vertex<G, vertex_t<G>> &&
    edge<G, edge_t<G>>;
```

### `index_adjacency_list<G>`

Adjacency list with O(1) vertex lookup by integral target IDs on edges.

```cpp
template <class G>
concept index_adjacency_list =
    adjacency_list<G> &&
    std::integral<decltype(target_id(g, uv))>;
```

> **This is the concept required by most algorithms.**

### `ordered_vertex_edges<G>`

Edges for each vertex are ordered, enabling efficient binary search.

```cpp
template <class G>
concept ordered_vertex_edges =
    index_adjacency_list<G> &&
    requires(G& g, vertex_t<G>& u, vertex_id_t<G> vid) {
        { find_vertex_edge(g, u, vid) } -> /* valid iterator */;
    };
```

---

## Edge List Concepts

Header: `<graph/edge_list/edge_list.hpp>`

### `basic_sourced_edgelist<EL>`

An edge list — a flat `forward_range` where each element has both source and
target vertex IDs.

```cpp
template <class EL>
concept basic_sourced_edgelist =
    std::ranges::forward_range<EL> &&
    requires(EL& el, std::ranges::range_value_t<EL>& e) {
        { source_id(el, e) } -> std::convertible_to<vertex_id_t<EL>>;
        { target_id(el, e) } -> std::convertible_to<vertex_id_t<EL>>;
    };
```

### `basic_sourced_index_edgelist<EL>`

Refines `basic_sourced_edgelist` — vertex IDs are integral.

```cpp
template <class EL>
concept basic_sourced_index_edgelist =
    basic_sourced_edgelist<EL> &&
    std::integral<vertex_id_t<EL>>;
```

### `has_edge_value<EL>`

Refines `basic_sourced_edgelist` — edges carry associated values.

```cpp
template <class EL>
concept has_edge_value =
    basic_sourced_edgelist<EL> &&
    requires(EL& el, std::ranges::range_value_t<EL>& e) {
        edge_value(el, e);
    };
```

---

## Traversal Concepts

Header: `<graph/algorithm/traversal_common.hpp>`

### `edge_weight_function<G, WF, DistanceValue>`

A callable that extracts a numeric weight from an edge, compatible with
standard shortest-path semantics (`std::less<>` + `std::plus<>`).

```cpp
template <class G, class WF, class DistanceValue>
concept edge_weight_function =
    std::invocable<WF, G&, edge_t<G>&> &&
    std::is_arithmetic_v<DistanceValue> &&
    basic_edge_weight_function<G, WF, DistanceValue, std::less<>, std::plus<>>;
```

### `basic_edge_weight_function<G, WF, DistanceValue, Compare, Combine>`

Generalized edge weight function with custom comparison and combination
operators. Used by advanced Dijkstra/Bellman-Ford overloads.

```cpp
template <class G, class WF, class DistanceValue, class Compare, class Combine>
concept basic_edge_weight_function = requires {
    // WF(g, e) returns arithmetic value
    // Compare is strict_weak_order on DistanceValue
    // Combine(distance, wf(g, e)) is assignable to DistanceValue&
};
```

---

## Descriptor Concepts

Header: `<graph/descriptor/descriptor.hpp>` (internal)

These concepts classify vertex/edge storage patterns. See
[Vertex Patterns](vertex-patterns.md) and [Edge Value Concepts](edge-value-concepts.md)
for full documentation.

| Concept | Purpose |
|---------|---------|
| `direct_vertex_type<Iter>` | Random-access, index-based vertex |
| `keyed_vertex_type<Iter>` | Bidirectional, key-based vertex |
| `vertex_iterator<Iter>` | Either direct or keyed |
| `random_access_vertex_pattern<Iter>` | Inner value: return whole element |
| `pair_value_vertex_pattern<Iter>` | Inner value: return `.second` |
| `whole_value_vertex_pattern<Iter>` | Inner value: return `*iter` |
| `has_inner_value_pattern<Iter>` | Any inner value pattern |
| `simple_edge_type<T>` | Edge is a bare integral (target ID only) |
| `pair_edge_type<T>` | Edge is pair-like (target + property) |
| `tuple_edge_type<T>` | Edge is tuple-like |
| `custom_edge_type<T>` | Edge is a custom struct/class |
| `edge_value_type<T>` | Any edge value pattern |

---

## See Also

- [CPO Reference](cpo-reference.md) — CPO signatures and behavior
- [Adjacency List Interface](adjacency-list-interface.md) — full GCI spec
- [Edge List Interface](edge-list-interface.md) — edge list GCI spec
- [Vertex Patterns](vertex-patterns.md) — inner value and storage concepts
- [Edge Value Concepts](edge-value-concepts.md) — edge type pattern detection
