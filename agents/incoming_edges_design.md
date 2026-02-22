# Incoming Edge Support — Design Document

## 1. Problem Statement

The graph-v3 library currently provides **outgoing edge access only**. All edge CPOs
(`edges()`, `degree()`, `target_id()`), views (`incidence`, `edgelist`, `neighbors`),
concepts (`adjacency_list`, `vertex_edge_range`), and algorithms are implicitly outgoing.
There is no way to query "which edges point **to** a vertex."

This document proposes extending the library to support **incoming edges** while
preserving full backward compatibility with existing code.

---

## 2. Design Principles

1. **Mirror, don't replace.** Every new incoming-edge facility mirrors an existing
   outgoing-edge facility with a consistent naming convention.
2. **Backward compatible.** Existing code compiles unchanged. The current `edges()`,
   `degree()`, etc. continue to mean "outgoing."
3. **Opt-in.** Graphs that don't store reverse adjacency data simply don't model the
   new concepts — algorithms gracefully fall back to outgoing-only behavior.
4. **Consistent naming.** Incoming counterparts use an `in_` prefix; where ambiguity
   arises, outgoing counterparts gain an `out_` alias.
5. **Independent edge types.** `in_edge_t<G>` and `edge_t<G>` (i.e. `out_edge_t<G>`)
   are independently deduced from their respective ranges. They are commonly the same
   type but this is **not** required. For example, in a distributed graph the outgoing
   edges may carry full property data while incoming edges are lightweight back-pointers
   (just a source vertex ID). In a CSR+CSC compressed graph, the CSC incoming entries
   may store only a column index plus a back-reference into the CSR value array.

---

## 3. Naming Convention

### 3.1 Primary outgoing names

The explicit directional names are the **primary** CPO definitions:

| Primary Name | Meaning |
|---|---|
| `out_edges(g, u)` | Outgoing edges from vertex `u` |
| `out_degree(g, u)` | Out-degree of `u` |
| `target_id(g, uv)` | Target vertex of edge `uv` |
| `source_id(g, uv)` | Source vertex of edge `uv` |
| `target(g, uv)` | Target vertex descriptor |
| `source(g, uv)` | Source vertex descriptor |
| `num_edges(g)` | Total edge count (direction-agnostic) |
| `find_out_edge(g, u, v)` | Find outgoing edge from `u` to `v` |
| `contains_edge(g, u, v)` | Check outgoing edge from `u` to `v` |
| `has_edge(g)` | Check whether graph has at least one edge |
| `edge_value(g, uv)` | Edge property (direction-agnostic) |

### 3.2 Convenience aliases (shorter names)

For code that prefers brevity, **aliases** forward to the primary CPOs:

| Alias | Forwards To |
|---|---|
| `edges(g, u)` | `out_edges(g, u)` |
| `degree(g, u)` | `out_degree(g, u)` |
| `find_vertex_edge(g, u, v)` | `find_out_edge(g, u, v)` |

These are defined as inline constexpr CPO aliases in
`graph::adj_list` and re-exported to `graph::`.

> **Design decision (2026-02-21):** These aliases are **retained**. See
> [Appendix D](#appendix-d-out_-alias-retention-decision) for the full trade-off
> analysis.

### 3.3 New incoming names

| New Name | Meaning |
|---|---|
| `in_edges(g, u)` / `in_edges(g, uid)` | Incoming edges to vertex `u` |
| `in_degree(g, u)` / `in_degree(g, uid)` | In-degree of `u` |
| `find_in_edge(g, u, v)` | Find incoming edge from `v` to `u` (both descriptors) |
| `find_in_edge(g, u, vid)` | Find incoming edge from `vid` to `u` (descriptor + ID) |
| `find_in_edge(g, uid, vid)` | Find incoming edge from `vid` to `uid` (both IDs) |
| `contains_in_edge(g, u, v)` | Check incoming edge from `v` to `u` (both descriptors) |
| `contains_in_edge(g, uid, vid)` | Check incoming edge from `vid` to `uid` (both IDs) |

Parameter convention for incoming-edge queries:

- `u`/`uid` = **target** vertex (the vertex receiving incoming edges)
- `v`/`vid` = **source** vertex (the vertex the edge comes from)

Example:

```cpp
// true if edge 7 -> 3 exists
bool b = contains_in_edge(g, /*target=*/3, /*source=*/7);
```

---

## 4. New CPOs

### 4.1 `in_edges(g, u)` / `in_edges(g, uid)`

**File:** `include/graph/adj_list/detail/graph_cpo.hpp` (new section)

**Resolution order** (mirrors `edges()`):

| Priority | Strategy | Expression |
|---|---|---|
| 1 | `_vertex_member` | `u.inner_value(g).in_edges()` |
| 2 | `_adl` | ADL `in_edges(g, u)` |

For the `(g, uid)` overload:

| Priority | Strategy | Expression |
|---|---|---|
| 1 | `_adl` | ADL `in_edges(g, uid)` |
| 2 | `_default` | `in_edges(g, *find_vertex(g, uid))` |

**Return type:** `in_edge_range_t<G>` — i.e. the exact range type returned by
`in_edges(g, u)` after CPO resolution. This may be an `edge_descriptor_view`, but
custom member/ADL implementations may return a different valid range type.

### 4.2 `in_degree(g, u)` / `in_degree(g, uid)`

**File:** `include/graph/adj_list/detail/graph_cpo.hpp` (new section)

**Resolution order** (mirrors `degree()`):

| Priority | Strategy | Expression |
|---|---|---|
| 1 | `_member` | `g.in_degree(u)` |
| 2 | `_adl` | ADL `in_degree(g, u)` |
| 3 | `_default` | `size(in_edges(g, u))` or `distance(in_edges(g, u))` |

### 4.3 `edges` / `degree` / `find_vertex_edge` (convenience aliases)

**File:** `include/graph/adj_list/detail/graph_cpo.hpp`

> **Note:** `out_edges`, `out_degree`, and `find_out_edge` are the primary
> CPO instances. The shorter names are convenience aliases.

```cpp
inline constexpr auto& edges            = out_edges;
inline constexpr auto& degree           = out_degree;
inline constexpr auto& find_vertex_edge = find_out_edge;
```

### 4.4 `find_in_edge`, `contains_in_edge`

These mirror `find_out_edge` / `contains_edge` but operate
on the reverse adjacency structure. Implementation follows the same
member → ADL → default cascade pattern.

`find_in_edge` has three overloads mirroring `find_out_edge`:
- `find_in_edge(g, u, v)` — both vertex descriptors
- `find_in_edge(g, u, vid)` — descriptor + vertex ID
- `find_in_edge(g, uid, vid)` — both vertex IDs

`contains_in_edge` has two overloads mirroring `contains_edge`:
- `contains_in_edge(g, u, v)` — both vertex descriptors
- `contains_in_edge(g, uid, vid)` — both vertex IDs

The default implementation of `find_in_edge` iterates `in_edges(g, u)` and
matches `source_id(g, ie)` against the target vertex (the in-neighbor). The default
`contains_in_edge` calls `find_in_edge` and checks the result.

> **Note:** There is no `has_in_edge` counterpart. The existing `has_edge(g)` takes
> only the graph and means "graph has at least one edge" — a direction-agnostic
> query. A `has_in_edge(g)` variant would be redundant with `has_edge(g)`, and
> `has_in_edge(g, uid, vid)` would duplicate `contains_in_edge(g, uid, vid)`.

---

## 5. New Type Aliases

**File:** `include/graph/adj_list/detail/graph_cpo.hpp` (after `in_edges` CPO)

| Alias | Definition |
|---|---|
| `in_edge_range_t<G>` | `decltype(in_edges(g, vertex_t<G>))` |
| `in_edge_iterator_t<G>` | `ranges::iterator_t<in_edge_range_t<G>>` |
| `in_edge_t<G>` | `ranges::range_value_t<in_edge_range_t<G>>` |

`in_edge_t<G>` is **independently deduced** from the `in_edges()` return range.
It is commonly the same type as `out_edge_t<G>` but this is not required. See
Design Principle 5 and Appendix C for rationale.

The outgoing type aliases are the primary definitions:

| Primary Alias | Definition |
|---|---|
| `out_edge_range_t<G>` | `decltype(out_edges(g, vertex_t<G>))` |
| `out_edge_iterator_t<G>` | `ranges::iterator_t<out_edge_range_t<G>>` |
| `out_edge_t<G>` | `ranges::range_value_t<out_edge_range_t<G>>` |

Convenience aliases:

| Alias | Definition |
|---|---|
| `vertex_edge_range_t<G>` | Same as `out_edge_range_t<G>` |
| `vertex_edge_iterator_t<G>` | Same as `out_edge_iterator_t<G>` |
| `edge_t<G>` | Same as `out_edge_t<G>` |

---

## 6. New Concepts

**File:** `include/graph/adj_list/adjacency_list_concepts.hpp`

### 6.1 `in_edge_range<R, G>`

```cpp
template <class R, class G>
concept in_edge_range =
    std::ranges::forward_range<R>;
```

Note: this is intentionally **less constrained** than `vertex_edge_range` (which
requires `edge<G, range_value_t<R>>`). Incoming edges need only be iterable —
their element type (`in_edge_t<G>`) may be a lightweight back-pointer that does
not satisfy the full `edge<G, E>` concept. The minimum requirement is that
`source_id(g, ie)` can extract the neighbor vertex ID (see §6.2).

When `in_edge_t<G>` and `edge_t<G>` are the same type, the range will naturally
model `out_edge_range` as well.

### 6.2 `bidirectional_adjacency_list<G>`

```cpp
template <class G>
concept bidirectional_adjacency_list =
    adjacency_list<G> &&
    requires(G& g, vertex_t<G> u, in_edge_t<G> ie) {
      { in_edges(g, u) } -> in_edge_range<G>;
      { source_id(g, ie) } -> std::convertible_to<vertex_id_t<G>>;
      // Note: no edge_value() requirement on incoming edges
    };
```

A graph that models `bidirectional_adjacency_list` supports both `edges(g, u)`
(outgoing) and `in_edges(g, u)` (incoming). The only requirement on incoming
edge elements is that the neighbor vertex ID (the edge's source) can be extracted
via `source_id()`. Notably, `edge_value(g, ie)` is **not** required — incoming
edges may be valueless back-pointers.

When `in_edge_t<G> == edge_t<G>`, `edge_value()` will work on incoming edges
automatically.

### 6.3 `index_bidirectional_adjacency_list<G>`

```cpp
template <class G>
concept index_bidirectional_adjacency_list =
    bidirectional_adjacency_list<G> && index_vertex_range<G>;
```

---

## 7. New Traits

**File:** `include/graph/adj_list/adjacency_list_traits.hpp`

| Trait | Constraint |
|---|---|
| `has_in_degree<G>` | `in_degree(g, u)` and `in_degree(g, uid)` both return integral |
| `has_in_degree_v<G>` | `bool` shorthand |
| `has_find_in_edge<G>` | `find_in_edge` CPO resolves for graph type `G` |
| `has_find_in_edge_v<G>` | `bool` shorthand |
| `has_contains_in_edge<G>` | `contains_in_edge` CPO resolves for graph type `G` |
| `has_contains_in_edge_v<G>` | `bool` shorthand |

---

## 8. New Views

### 8.1 `in_incidence` view

**File:** `include/graph/views/in_incidence.hpp` (new file)

Mirrors `incidence_view` from `include/graph/views/incidence.hpp`.

| Class | Yields | Description |
|---|---|---|
| `in_incidence_view<G, void>` | `edge_info{sid, uv}` | Iterates incoming edges to a vertex, yielding **source_id** + edge descriptor |
| `in_incidence_view<G, EVF>` | `edge_info{sid, uv, val}` | Same, with value function |
| `basic_in_incidence_view<G, void>` | `edge_info{sid}` | Source ID only |
| `basic_in_incidence_view<G, EVF>` | `edge_info{sid, val}` | Source ID + value function |

**Factory functions:**
```cpp
namespace graph::views {
  in_incidence(g, u)           // → in_incidence_view<G, void>
  in_incidence(g, uid)         // → in_incidence_view<G, void>
  in_incidence(g, u, evf)     // → in_incidence_view<G, EVF>
  in_incidence(g, uid, evf)   // → in_incidence_view<G, EVF>
  basic_in_incidence(g, uid)   // → basic_in_incidence_view<G, void>
  basic_in_incidence(g, uid, evf) // → basic_in_incidence_view<G, EVF>
}
```

**Key difference from `incidence_view`:** The standard `incidence_view` iterates
`edges(g, u)` and yields `target_id` per edge. The `in_incidence_view` iterates
`in_edges(g, u)` and yields `source_id` per edge (the vertex the edge comes from).

**Edge type note:** The `EVF` value function receives `in_edge_t<G>`, which may
differ from `edge_t<G>`. When incoming edges are valueless back-pointers, the
`void` (no value function) variants are the natural choice.

### 8.2 `in_neighbors` view

**File:** `include/graph/views/in_neighbors.hpp` (new file)

Mirrors `neighbors_view` from `include/graph/views/neighbors.hpp`.

| Class | Yields | Description |
|---|---|---|
| `in_neighbors_view<G, void>` | `neighbor_info{sid, n}` | Iterates source vertices of incoming edges |
| `in_neighbors_view<G, VVF>` | `neighbor_info{sid, n, val}` | Same, with vertex value function |
| `basic_in_neighbors_view<G, void>` | `neighbor_info{sid}` | Source ID only |
| `basic_in_neighbors_view<G, VVF>` | `neighbor_info{sid, val}` | Source ID + value function |

### 8.3 Outgoing aliases (optional)

For symmetry, add aliases in the `graph::views` namespace:

```cpp
namespace graph::views {
  inline constexpr auto& out_incidence       = incidence;
  inline constexpr auto& basic_out_incidence = basic_incidence;
  inline constexpr auto& out_neighbors       = neighbors;
  inline constexpr auto& basic_out_neighbors = basic_neighbors;
}
```

### 8.4 Pipe-syntax adaptors

**File:** `include/graph/views/adaptors.hpp` (extend existing)

The existing `adaptors.hpp` provides pipe-syntax closures for all outgoing views
(`g | incidence(uid)`, `g | neighbors(uid)`, etc.). Corresponding closures must
be added for the new incoming views:

| Pipe expression | Expands to |
|---|---|
| `g \| in_incidence(uid)` | `in_incidence(g, uid)` |
| `g \| in_incidence(uid, evf)` | `in_incidence(g, uid, evf)` |
| `g \| basic_in_incidence(uid)` | `basic_in_incidence(g, uid)` |
| `g \| basic_in_incidence(uid, evf)` | `basic_in_incidence(g, uid, evf)` |
| `g \| in_neighbors(uid)` | `in_neighbors(g, uid)` |
| `g \| in_neighbors(uid, vvf)` | `in_neighbors(g, uid, vvf)` |
| `g \| basic_in_neighbors(uid)` | `basic_in_neighbors(g, uid)` |
| `g \| basic_in_neighbors(uid, vvf)` | `basic_in_neighbors(g, uid, vvf)` |

Outgoing aliases for symmetry:

| Pipe alias | Forwards to |
|---|---|
| `g \| out_incidence(uid)` | `g \| incidence(uid)` |
| `g \| out_neighbors(uid)` | `g \| neighbors(uid)` |
| `g \| basic_out_incidence(uid)` | `g \| basic_incidence(uid)` |
| `g \| basic_out_neighbors(uid)` | `g \| basic_neighbors(uid)` |

---

## 9. BFS/DFS/Topological-Sort Parameterization

### Problem

`bfs.hpp`, `dfs.hpp`, and `topological_sort.hpp` hardcode `adj_list::edges(g, vertex)`
(5 locations in BFS, 5 in DFS, 7 in topological sort).
To support reverse traversal (following incoming edges), they need parameterization.

### Solution

Add an **edge accessor** template parameter defaulting to the current outgoing behavior:

```cpp
// Default edge accessor — current behavior
struct out_edges_accessor {
  template <class G>
  constexpr auto operator()(G& g, vertex_t<G> u) const {
    return adj_list::edges(g, u);
  }
};

// Reverse edge accessor
struct in_edges_accessor {
  template <class G>
  constexpr auto operator()(G& g, vertex_t<G> u) const {
    return adj_list::in_edges(g, u);
  }
};

template <adjacency_list G, class VVF = void, class EdgeAccessor = out_edges_accessor>
class vertices_bfs_view { ... };

template <adjacency_list G, class EVF = void, class EdgeAccessor = out_edges_accessor>
class edges_bfs_view { ... };
```

Factory functions add overloads accepting the accessor:
```cpp
// Existing (unchanged)
vertices_bfs(g, seed)
vertices_bfs(g, seed, vvf)

// New
vertices_bfs(g, seed, vvf, in_edges_accessor{})  // reverse BFS
edges_bfs(g, seed, evf, in_edges_accessor{})      // reverse BFS
```

Same pattern applies to DFS views (`vertices_dfs_view`, `edges_dfs_view`) and
topological sort views (`vertices_topological_sort_view`, `edges_topological_sort_view`).
All three view families receive identical `EdgeAccessor` parameterization.

---

## 10. Container Support

### 10.1 `dynamic_graph` — Reverse Adjacency Storage

The simplest approach is a **separate reverse adjacency list** stored alongside the
forward list. This is a well-known pattern (e.g., Boost.Graph's `bidirectionalS`).

**Option A: Built-in template parameter**

Add a `bool Bidirectional = false` template parameter to `dynamic_graph_base`:

```cpp
template <typename EV, typename VV, typename GV, typename VId,
          bool Sourced = false, typename Traits = ...,
          bool Bidirectional = false>
class dynamic_graph_base;
```

When `Bidirectional = true`:
- Each vertex stores two edge containers: `out_edges_` and `in_edges_`
- `create_edge(u, v)` inserts into `u.out_edges_` and `v.in_edges_`
- `erase_edge` removes from both lists
- The `edges(g, u)` CPO returns `u.out_edges_` (unchanged)
- The `in_edges(g, u)` CPO returns `u.in_edges_`

The in-edge container element type may differ from the out-edge element type.
The default stores the same `EV` (so `in_edge_t<G> == edge_t<G>`), but an
additional template parameter (e.g., `InEV = EV`) can allow lightweight
back-pointer in-edges (e.g., just a source vertex ID) for reduced storage.

When `Bidirectional = false`:
- Behavior is identical to today (no storage overhead)

**Option B: External reverse index wrapper**

A `reverse_adjacency<G>` adaptor that wraps an existing graph and provides
`in_edges()` via a separately-constructed reverse index. This avoids modifying
`dynamic_graph` at all but adds a construction cost:

```cpp
auto rev = graph::reverse_adjacency(g);
for (auto [sid, uv] : views::in_incidence(rev, target_vertex)) { ... }
```

**Recommendation:** Option A for tight integration. Option B as an additional
utility for read-only reverse queries on existing graphs.

### 10.2 `compressed_graph` — CSC (Compressed Sparse Column) Format

The CSR format stores outgoing edges efficiently. The CSC (Compressed Sparse Column)
format stores incoming edges. A bidirectional `compressed_graph` would store both:

- `row_index_` + `col_index_` (CSR, outgoing — existing)
- `col_ptr_` + `row_index_csc_` (CSC, incoming — new)

This is standard in sparse matrix libraries (e.g., Eigen stores both CSR and CSC).

**Implementation:** Add a `bool Bidirectional = false` template parameter. When enabled,
the constructor builds the CSC representation from the input edges alongside the CSR.

### 10.3 `undirected_adjacency_list` — Already bidirectional

The `undirected_adjacency_list` already stores edges in both endpoint vertex lists
via `inward_list`/`outward_list` links. For undirected graphs, `edges(g, u)` and
`in_edges(g, u)` return the **same** range — every edge is both incoming and outgoing.

**Implementation:** Provide `in_edges()` as an ADL friend that returns the same
`edge_descriptor_view` as `edges()`. Similarly, `in_degree()` returns the same value
as `degree()`. This allows undirected graphs to model `bidirectional_adjacency_list`
at zero cost.

### 10.4 New trait type combinations for `dynamic_graph`

For each existing trait file (27 total in `include/graph/container/traits/`), a
corresponding `b` (bidirectional) variant may be added:

| Existing | Bidirectional Variant | Description |
|---|---|---|
| `vov_graph_traits` | `bvov_graph_traits` | Bidirectional vector-of-vectors |
| `vol_graph_traits` | `bvol_graph_traits` | Bidirectional vector-of-lists |
| ... | ... | ... |

Alternatively, a single set of traits with `Bidirectional` as a template parameter
avoids the combinatorial explosion.

### 10.5 Mutator Invariants & Storage/Complexity Budget

To keep the implementation predictable, reverse adjacency support must define and
enforce container invariants explicitly.

**Mutator invariants (Bidirectional = true):**

1. `create_edge(u, v)` inserts one forward entry in `u.out_edges_` and one reverse
   entry in `v.in_edges_` representing the same logical edge.
2. `erase_edge(u, v)` removes both forward and reverse entries.
3. `erase_vertex(x)` removes all incident edges and corresponding mirrored entries
   in opposite adjacency lists.
4. `clear_vertex(x)` removes both outgoing and incoming incidence for `x`.
5. `clear()` empties both forward and reverse structures.
6. Copy/move/swap preserve forward↔reverse consistency.

**Complexity/storage targets:**

- Outgoing-only mode (`Bidirectional = false`) remains unchanged.
- Bidirectional mode targets O(1) additional work per edge insertion/erasure in
  adjacency-list containers (plus container-specific costs).
- Storage overhead target is roughly one additional reverse entry per edge
  (`O(E)` extra), with optional reduced footprint via lightweight `in_edge_t`.

---

## 11. Algorithm Updates

Most algorithms only need outgoing edges and require **no changes**. The following
would benefit from incoming edge support:

| Algorithm | Benefit |
|---|---|
| **Kosaraju's SCC** (`connected_components.hpp`) | Currently simulates reverse graph by rebuilding edges. With `in_edges()`, the second DFS pass can use `in_edges_accessor` directly. |
| **PageRank** (future) | Requires iterating incoming edges for each vertex. |
| **Reverse BFS/DFS** | Enable with `in_edges_accessor` parameter (§9). |
| **Transpose graph** | Can be implemented as a zero-cost view that swaps `edges()`↔`in_edges()`. |
| **Dominator trees** | Require reverse control flow graph (incoming edges). |

No existing algorithm needs to be **renamed**. They all operate on outgoing
edges by default and remain unchanged.

---

## 12. Namespace & Re-export Updates

**File:** `include/graph/graph.hpp`

Add to the `graph::` re-exports:

```cpp
// New incoming-edge CPOs
using adj_list::in_edges;
using adj_list::in_degree;
using adj_list::out_edges;
using adj_list::out_degree;
using adj_list::find_out_edge;
using adj_list::find_in_edge;
using adj_list::contains_in_edge;

// New concepts
using adj_list::in_edge_range;
using adj_list::bidirectional_adjacency_list;
using adj_list::index_bidirectional_adjacency_list;

// New traits
using adj_list::has_in_degree;
using adj_list::has_in_degree_v;

// New type aliases
using adj_list::in_edge_range_t;
using adj_list::in_edge_iterator_t;
using adj_list::in_edge_t;
using adj_list::out_edge_range_t;
using adj_list::out_edge_iterator_t;
using adj_list::out_edge_t;
```

**File:** `include/graph/graph.hpp` — add includes:

```cpp
#include <graph/views/in_incidence.hpp>
#include <graph/views/in_neighbors.hpp>
```

---

## 13. Documentation Updates

### 13.1 Existing docs to update

| File | Change |
|---|---|
| `docs/index.md` | Add "Bidirectional edge access" to feature list |
| `docs/getting-started.md` | Add section on incoming edges; update "outgoing edges" mentions to be explicit |
| `docs/user-guide/views.md` | Add `in_incidence` and `in_neighbors` views; add `out_incidence`/`out_neighbors` aliases |
| `docs/user-guide/algorithms.md` | Note which algorithms benefit from bidirectional access |
| `docs/user-guide/containers.md` | Document bidirectional dynamic_graph and compressed_graph |
| `docs/reference/concepts.md` | Add `bidirectional_adjacency_list`, `in_edge_range` |
| `docs/reference/cpo-reference.md` | Add `in_edges`, `in_degree`, `out_edges`, `out_degree`, `find_in_edge`, `contains_in_edge` |
| `docs/reference/type-aliases.md` | Add `in_edge_range_t`, `in_edge_t`, etc. |
| `docs/reference/adjacency-list-interface.md` | Add incoming edge section |
| `docs/contributing/cpo-implementation.md` | Add `in_edges` as an example of the CPO pattern |
| `README.md` | Update feature highlights with bidirectional support |

### 13.2 New docs

| File | Content |
|---|---|
| `docs/user-guide/bidirectional-access.md` | Tutorial-style guide on using incoming edges |

---

## 14. Implementation Phases

### Phase 1: Core CPOs & Concepts (~2-3 days)

1. Implement `in_edges` CPO in `graph_cpo.hpp` (mirror `edges` CPO)
2. Implement `in_degree` CPO (mirror `degree` CPO)
3. Add `out_edges` / `out_degree` aliases
4. Define `in_edge_range_t`, `in_edge_t`, etc.
5. Define `bidirectional_adjacency_list` concept
6. Add `has_in_degree` trait
7. Update `graph.hpp` re-exports
8. Add unit tests for CPO resolution (stub graph with `in_edges()` member)
9. Add compile-time concept tests for `bidirectional_adjacency_list`
10. Add mixed-type tests where `in_edge_t<G> != edge_t<G>` (source-only incoming edges)

### Phase 2: Views (~2 days)

1. Create `in_incidence.hpp` (copy+adapt `incidence.hpp`)
2. Create `in_neighbors.hpp` (copy+adapt `neighbors.hpp`)
3. Add `out_incidence` / `out_neighbors` aliases
4. Add pipe-syntax adaptors to `adaptors.hpp` (§8.4)
5. Unit tests for all new views (including pipe-syntax adaptor tests)

### Phase 3: Container Support (~3-4 days)

1. Add `Bidirectional` parameter to `dynamic_graph_base`
2. Update `create_edge` / `erase_edge` / `clear` to maintain reverse lists
3. Update `erase_vertex` / `clear_vertex` / copy-move-swap paths to preserve reverse invariants
4. Add ADL `in_edges()` friend to bidirectional dynamic_graph
5. Add CSC support to `compressed_graph`
6. Add `in_edges()` to `undirected_adjacency_list` (returns same as `edges()`)
7. Unit tests for all three containers (including mutator invariant checks)

### Phase 4: BFS/DFS Parameterization (~1-2 days)

1. Add `EdgeAccessor` template parameter to BFS/DFS/topological sort views
2. Define `out_edges_accessor` and `in_edges_accessor` functors
3. Add factory function overloads
4. Unit tests for reverse BFS/DFS

### Phase 5: Algorithm Updates (~1-2 days)

1. Refactor Kosaraju's SCC to use `in_edges()` when available
2. Add `transpose_view` (zero-cost view swapping `edges()`↔`in_edges()`)
3. Unit tests

### Phase 6: Documentation (~2 days)

1. Update all docs listed in §13.1
2. Create `bidirectional-access.md` tutorial
3. Update CHANGELOG.md

### Recommended execution order (risk-first)

To reduce integration risk and keep PRs reviewable, execute phases with strict
merge gates and defer high-churn container internals until API contracts are
proven by tests.

1. **Phase 1 (CPOs/concepts/traits) first, in one or two PRs max.**
2. **Phase 2 (views) second**, consuming only the new public CPOs.
3. **Phase 4 (BFS/DFS parameterization) third**, before container internals,
   to validate the accessor approach on existing graphs.
4. **Phase 3 (container support) fourth**, split by container:
   `undirected_adjacency_list` → `dynamic_graph` → `compressed_graph`.
5. **Phase 5 (algorithm updates) fifth**, after reverse traversal behavior is
   stable and benchmarked.
6. **Phase 6 (docs/changelog) last**, plus updates in each phase PR where needed.

### Merge gates (go/no-go criteria)

Each phase must satisfy all gates before proceeding:

- `linux-gcc-debug` full test suite passes.
- New compile-time concept tests pass (`requires`/`static_assert` coverage).
- No regressions in existing outgoing-edge APIs (`edges`, `degree`, `find_vertex_edge`).
- New incoming APIs are documented in reference docs for that phase.

Additional phase-specific gates:

- **After Phase 1:**
  - Mixed-type incoming-edge test (`in_edge_t<G> != edge_t<G>`) passes.
  - `in_edges`/`in_degree` ADL and member dispatch paths are covered.
- **After Phase 4:**
  - Reverse BFS/DFS outputs validated against expected traversal sets.
  - Existing BFS/DFS/topological-sort signatures remain source-compatible.
- **After Phase 3:**
  - Mutator invariant tests pass (`create_edge`, `erase_edge`, `erase_vertex`,
    `clear_vertex`, `clear`, copy/move/swap consistency).

### Fallback and scope control

- If CSC integration in `compressed_graph` slips, ship `dynamic_graph` +
  `undirected_adjacency_list` incoming support first and keep CSC behind a
  follow-up milestone.
- Keep `transpose_view` optional for initial release; prioritize core CPOs,
  views, and reverse traversal correctness.
- If API pressure grows, keep only the decided aliases (`out_*`) and avoid
  introducing additional directional synonyms.

---

## 15. Summary of All Changes

| Category | New | Modified | Deleted |
|---|---|---|---|
| **CPOs** | `in_edges`, `in_degree`, `out_edges` (alias), `out_degree` (alias), `find_out_edge` (alias), `find_in_edge`, `contains_in_edge` | — | — |
| **Concepts** | `in_edge_range`, `bidirectional_adjacency_list`, `index_bidirectional_adjacency_list` | — | — |
| **Traits** | `has_in_degree`, `has_in_degree_v`, `has_find_in_edge`, `has_contains_in_edge` | — | — |
| **Type Aliases** | `in_edge_range_t`, `in_edge_iterator_t`, `in_edge_t`, `out_edge_range_t`, `out_edge_iterator_t`, `out_edge_t` | — | — |
| **Views** | `in_incidence.hpp`, `in_neighbors.hpp` | `bfs.hpp`, `dfs.hpp`, `topological_sort.hpp` (add `EdgeAccessor` param), `adaptors.hpp` (add pipe closures) | — |
| **Traversal policies** | `out_edges_accessor`, `in_edges_accessor` | — | — |
| **Containers** | — | `dynamic_graph.hpp` (add `Bidirectional`), `compressed_graph.hpp` (add CSC), `undirected_adjacency_list.hpp` (add `in_edges` friend) | — |
| **Algorithms** | `transpose_view` | `connected_components.hpp` (Kosaraju optimization) | — |
| **Headers** | — | `graph.hpp` (new includes & re-exports) | — |
| **Docs** | `bidirectional-access.md` | 11 existing docs (§13.1) | — |
| **Tests** | CPO tests, view tests, container tests, BFS/DFS reverse tests | — | — |

**Estimated effort:** 11–15 days

---

## Appendix A: Full CPO Resolution Table

| CPO | Tier 1 | Tier 2 | Tier 3 |
|---|---|---|---|
| `edges(g, u)` | `u.inner_value(g).edges()` | ADL `edges(g, u)` | inner_value is forward_range → wrap |
| `in_edges(g, u)` | `u.inner_value(g).in_edges()` | ADL `in_edges(g, u)` | *(no default)* |
| `degree(g, u)` | `g.degree(u)` | ADL `degree(g, u)` | `size(edges(g, u))` |
| `in_degree(g, u)` | `g.in_degree(u)` | ADL `in_degree(g, u)` | `size(in_edges(g, u))` |
| `find_vertex_edge(g, u, v)` | `g.find_vertex_edge(u, v)` | ADL `find_vertex_edge(g, u, v)` | iterate `edges(g, u)`, match `target_id` |
| `find_in_edge(g, u, v)` | `g.find_in_edge(u, v)` | ADL `find_in_edge(g, u, v)` | iterate `in_edges(g, u)`, match `source_id` |
| `contains_edge(g, u, v)` | `g.contains_edge(u, v)` | ADL `contains_edge(g, u, v)` | iterate `edges(g, u)`, match `target_id` |
| `contains_in_edge(g, u, v)` | `g.contains_in_edge(u, v)` | ADL `contains_in_edge(g, u, v)` | iterate `in_edges(g, u)`, match `source_id` |
| `has_edge(g)` | `g.has_edge()` | ADL `has_edge(g)` | iterate vertices, find non-empty edges |
| `out_edges(g, u)` | → `edges(g, u)` | | |
| `out_degree(g, u)` | → `degree(g, u)` | | |
| `find_out_edge(g, u, v)` | → `find_vertex_edge(g, u, v)` | | |

## Appendix B: No-Rename Justification

Several graph libraries (Boost.Graph, NetworkX, LEDA) distinguish `out_edges` /
`in_edges` explicitly. In graph-v3, `edges()` has **always** meant outgoing (this is
documented and all algorithms depend on it). Renaming `edges()` to `out_edges()` would:

- Break every existing user's code
- Require renaming `vertex_edge_range_t` → `out_edge_range_t` across 50+ files
- Create churn in all 14 algorithms

Instead we keep `edges()` as the primary outgoing CPO (matching the "default is outgoing"
convention) and add `out_edges` as a convenience alias for codebases that want explicit
directionality.

## Appendix C: Independent In/Out Edge Types

`in_edge_t<G>` and `edge_t<G>` are independently deduced:

```cpp
template <class G> using edge_t    = ranges::range_value_t<vertex_edge_range_t<G>>;    // from edges(g,u)
template <class G> using in_edge_t = ranges::range_value_t<in_edge_range_t<G>>; // from in_edges(g,u)
```

They are commonly the same type, but this is **not required**. Scenarios where
they differ:

| Scenario | `edge_t<G>` | `in_edge_t<G>` |
|---|---|---|
| Symmetric (common case) | `pair<VId, Weight>` | `pair<VId, Weight>` (same) |
| Distributed graph | `pair<VId, PropertyBundle>` | `VId` (back-pointer only) |
| CSR + CSC compressed | Full edge with value | Column index + CSR back-ref |
| Lightweight reverse index | `pair<VId, EV>` | `VId` (source ID only) |

The `bidirectional_adjacency_list` concept (§6.2) only requires `source_id(g, ie)`
on incoming edges — not `edge_value()`. This means:
- Algorithms that only need the graph structure (BFS, DFS, SCC) work with any
  `in_edge_t<G>`.
- Algorithms that need edge properties on incoming edges (rare) can add their own
  `requires edge_value(g, in_edge_t<G>{})` constraint.
- The undirected case (`in_edge_t<G> == edge_t<G>`) is the zero-cost happy path.

Minimum incoming-edge contract for algorithms:

- **Always required:** `in_edges(g, u)`, `source_id(g, ie)`
- **Conditionally required:** `edge_value(g, ie)` only for algorithms/views that
  explicitly consume incoming edge properties
- **Not required:** `target_id(g, ie)` for incoming-only traversal algorithms

## Appendix D: `out_` Alias Retention Decision

**Date:** 2026-02-21

### Question

Should the library provide `out_edges`, `out_degree`, `find_out_edge`,
`out_incidence`, `out_neighbors` (and their `basic_` variants) as aliases for the
existing outgoing CPOs/views, or should it omit them entirely?

### Arguments for removing the aliases

| # | Argument |
|---|---|
| R1 | `edges()` already means "outgoing" and always has — adding `out_edges()` is redundant and inflates the API surface. |
| R2 | Two spellings for the same operation create ambiguity: users must learn that `out_edges(g, u)` and `edges(g, u)` are identical. |
| R3 | Aliases clutter autocomplete, documentation tables, and error messages. |
| R4 | No existing user code ever spells `out_edges()` today, so removing the aliases breaks nobody. |
| R5 | If a future rename is ever desired (`edges` → `out_edges`), aliases make that rename harder because both names are already established. |

### Arguments for keeping the aliases

| # | Argument |
|---|---|
| K1 | **Symmetry with `in_edges`:** When a codebase uses `in_edges()` alongside `edges()`, the lack of an `out_` counterpart is visually jarring. `out_edges` / `in_edges` reads as a matched pair. |
| K2 | **Self-documenting code:** `out_edges(g, u)` makes directionality explicit at the call site; `edges(g, u)` requires the reader to know the convention. |
| K3 | **Familiar vocabulary:** Boost.Graph, NetworkX, LEDA, and the P1709 proposal all provide an `out_edges` name. Users migrating from those libraries expect it. |
| K4 | **Zero runtime cost:** The aliases are `inline constexpr` references to the existing CPO objects — no code duplication, no template bloat, no additional overload resolution. |
| K5 | **Grep-ability:** Searching a codebase for `out_edges` immediately reveals all outgoing-edge access; searching for `edges` produces false positives from `in_edges`, `num_edges`, `has_edge`, etc. |
| K6 | **Non-breaking:** Aliases are purely additive. Users who prefer `edges()` continue to use it unchanged. |

### Resolution

**Keep all `out_` aliases** (CPOs and view factory functions) as designed in §3.2,
§4.3, and §8.3.

The symmetry (K1), self-documentation (K2), and familiarity (K3) benefits outweigh
the API-surface concern (R1–R3). The aliases are zero-cost (K4) and non-breaking (K6).

To mitigate confusion (R2), documentation will:
- Note that `out_edges(g, u)` is an alias for `edges(g, u)` wherever it appears.
- Use `edges()` as the primary spelling in algorithm implementations.
- Use `out_edges()` in examples that also use `in_edges()`, to keep the pairing
  visually clear.
