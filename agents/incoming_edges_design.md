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

---

## 3. Naming Convention

### 3.1 Existing names (no rename required)

The current names remain as-is and **continue to mean "outgoing edges"**:

| Current Name | Stays | Meaning |
|---|---|---|
| `edges(g, u)` | Yes | Outgoing edges from vertex `u` |
| `degree(g, u)` | Yes | Out-degree of `u` |
| `target_id(g, uv)` | Yes | Target vertex of edge `uv` |
| `source_id(g, uv)` | Yes | Source vertex of edge `uv` |
| `target(g, uv)` | Yes | Target vertex descriptor |
| `source(g, uv)` | Yes | Source vertex descriptor |
| `num_edges(g)` | Yes | Total edge count (direction-agnostic) |
| `find_vertex_edge(g, u, v)` | Yes | Find outgoing edge from `u` to `v` |
| `contains_edge(g, u, v)` | Yes | Check outgoing edge from `u` to `v` |
| `has_edge(g, uid, vid)` | Yes | Check outgoing edge from `uid` to `vid` |
| `edge_value(g, uv)` | Yes | Edge property (direction-agnostic) |

### 3.2 New outgoing aliases (optional clarity)

For code that wants to be explicit, introduce **aliases** that forward to the
existing CPOs. These are convenience only — not required:

| New Alias | Forwards To |
|---|---|
| `out_edges(g, u)` | `edges(g, u)` |
| `out_degree(g, u)` | `degree(g, u)` |

These are defined as inline constexpr CPO aliases or thin wrapper functions in
`graph::adj_list` and re-exported to `graph::`.

### 3.3 New incoming names

| New Name | Meaning |
|---|---|
| `in_edges(g, u)` / `in_edges(g, uid)` | Incoming edges to vertex `u` |
| `in_degree(g, u)` / `in_degree(g, uid)` | In-degree of `u` |
| `find_vertex_in_edge(g, u, v)` | Find incoming edge from `v` to `u` |
| `contains_in_edge(g, u, v)` | Check incoming edge from `v` to `u` |
| `has_in_edge(g, uid, vid)` | Check incoming edge from `vid` to `uid` |

---

## 4. New CPOs

### 4.1 `in_edges(g, u)` / `in_edges(g, uid)`

**File:** `include/graph/adj_list/detail/graph_cpo.hpp` (new section)

**Resolution order** (mirrors `edges()`):

| Priority | Strategy | Expression |
|---|---|---|
| 1 | `_vertex_member` | `u.inner_value(g).in_edges()` |
| 2 | `_adl` | ADL `in_edges(g, u)` |
| 3 | `_edge_value_pattern` | (not applicable — no implicit reverse structure) |

For the `(g, uid)` overload:

| Priority | Strategy | Expression |
|---|---|---|
| 1 | `_adl` | ADL `in_edges(g, uid)` |
| 2 | `_default` | `in_edges(g, *find_vertex(g, uid))` |

**Return type:** `in_vertex_edge_range_t<G>` — an `edge_descriptor_view` wrapping
the reverse edge container's iterators.

### 4.2 `in_degree(g, u)` / `in_degree(g, uid)`

**File:** `include/graph/adj_list/detail/graph_cpo.hpp` (new section)

**Resolution order** (mirrors `degree()`):

| Priority | Strategy | Expression |
|---|---|---|
| 1 | `_member` | `g.in_degree(u)` |
| 2 | `_adl` | ADL `in_degree(g, u)` |
| 3 | `_default` | `size(in_edges(g, u))` or `distance(in_edges(g, u))` |

### 4.3 `out_edges` / `out_degree` (aliases)

**File:** `include/graph/adj_list/detail/graph_cpo.hpp`

```cpp
inline constexpr auto& out_edges  = edges;
inline constexpr auto& out_degree = degree;
```

### 4.4 `find_vertex_in_edge`, `contains_in_edge`, `has_in_edge`

These mirror `find_vertex_edge` / `contains_edge` / `has_edge` but operate on
the reverse adjacency structure. Implementation follows the same member → ADL → default
cascade pattern.

---

## 5. New Type Aliases

**File:** `include/graph/adj_list/detail/graph_cpo.hpp` (after `in_edges` CPO)

| Alias | Definition |
|---|---|
| `in_vertex_edge_range_t<G>` | `decltype(in_edges(g, vertex_t<G>))` |
| `in_vertex_edge_iterator_t<G>` | `ranges::iterator_t<in_vertex_edge_range_t<G>>` |
| `in_edge_t<G>` | `ranges::range_value_t<in_vertex_edge_range_t<G>>` |

Also add outgoing aliases for explicitness:

| Alias | Definition |
|---|---|
| `out_vertex_edge_range_t<G>` | Same as `vertex_edge_range_t<G>` |
| `out_vertex_edge_iterator_t<G>` | Same as `vertex_edge_iterator_t<G>` |
| `out_edge_t<G>` | Same as `edge_t<G>` |

---

## 6. New Concepts

**File:** `include/graph/adj_list/adjacency_list_concepts.hpp`

### 6.1 `in_vertex_edge_range<R, G>`

```cpp
template <class R, class G>
concept in_vertex_edge_range =
    std::ranges::forward_range<R> && edge<G, std::ranges::range_value_t<R>>;
```

Identical constraint to `vertex_edge_range` — the distinction is semantic (incoming
vs outgoing), enforced by which CPO returns the range.

### 6.2 `bidirectional_adjacency_list<G>`

```cpp
template <class G>
concept bidirectional_adjacency_list =
    adjacency_list<G> &&
    requires(G& g, vertex_t<G> u) {
      { in_edges(g, u) } -> in_vertex_edge_range<G>;
    };
```

A graph that models `bidirectional_adjacency_list` supports both `edges(g, u)`
(outgoing) and `in_edges(g, u)` (incoming).

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
| `has_find_vertex_in_edge<G>` | All `find_vertex_in_edge` overloads return `in_edge_t<G>` |
| `has_find_vertex_in_edge_v<G>` | `bool` shorthand |
| `has_contains_in_edge<G>` | `contains_in_edge(g, u, v)` returns bool |
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

---

## 9. BFS/DFS Parameterization

### Problem

`bfs.hpp` and `dfs.hpp` hardcode `adj_list::edges(g, vertex)` in ~6 locations each.
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

Same pattern applies to DFS and topological sort views.

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

For each existing trait file (26 total in `include/graph/container/traits/`), a
corresponding `b` (bidirectional) variant may be added:

| Existing | Bidirectional Variant | Description |
|---|---|---|
| `vov_graph_traits` | `bvov_graph_traits` | Bidirectional vector-of-vectors |
| `vol_graph_traits` | `bvol_graph_traits` | Bidirectional vector-of-lists |
| ... | ... | ... |

Alternatively, a single set of traits with `Bidirectional` as a template parameter
avoids the combinatorial explosion.

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
using adj_list::find_vertex_in_edge;
using adj_list::contains_in_edge;
using adj_list::has_in_edge;

// New concepts
using adj_list::in_vertex_edge_range;
using adj_list::bidirectional_adjacency_list;
using adj_list::index_bidirectional_adjacency_list;

// New traits
using adj_list::has_in_degree;
using adj_list::has_in_degree_v;

// New type aliases
using adj_list::in_vertex_edge_range_t;
using adj_list::in_vertex_edge_iterator_t;
using adj_list::in_edge_t;
using adj_list::out_vertex_edge_range_t;
using adj_list::out_vertex_edge_iterator_t;
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
| `docs/reference/concepts.md` | Add `bidirectional_adjacency_list`, `in_vertex_edge_range` |
| `docs/reference/cpo-reference.md` | Add `in_edges`, `in_degree`, `out_edges`, `out_degree`, `find_vertex_in_edge`, `contains_in_edge`, `has_in_edge` |
| `docs/reference/type-aliases.md` | Add `in_vertex_edge_range_t`, `in_edge_t`, etc. |
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
4. Define `in_vertex_edge_range_t`, `in_edge_t`, etc.
5. Define `bidirectional_adjacency_list` concept
6. Add `has_in_degree` trait
7. Update `graph.hpp` re-exports
8. Add unit tests for CPO resolution (stub graph with `in_edges()` member)

### Phase 2: Views (~2 days)

1. Create `in_incidence.hpp` (copy+adapt `incidence.hpp`)
2. Create `in_neighbors.hpp` (copy+adapt `neighbors.hpp`)
3. Add `out_incidence` / `out_neighbors` aliases
4. Unit tests for all new views

### Phase 3: Container Support (~3-4 days)

1. Add `Bidirectional` parameter to `dynamic_graph_base`
2. Update `create_edge` / `erase_edge` / `clear` to maintain reverse lists
3. Add ADL `in_edges()` friend to bidirectional dynamic_graph
4. Add CSC support to `compressed_graph`
5. Add `in_edges()` to `undirected_adjacency_list` (returns same as `edges()`)
6. Unit tests for all three containers

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

---

## 15. Summary of All Changes

| Category | New | Modified | Deleted |
|---|---|---|---|
| **CPOs** | `in_edges`, `in_degree`, `out_edges` (alias), `out_degree` (alias), `find_vertex_in_edge`, `contains_in_edge`, `has_in_edge` | — | — |
| **Concepts** | `in_vertex_edge_range`, `bidirectional_adjacency_list`, `index_bidirectional_adjacency_list` | — | — |
| **Traits** | `has_in_degree`, `has_in_degree_v`, `has_find_vertex_in_edge`, `has_contains_in_edge` | — | — |
| **Type Aliases** | `in_vertex_edge_range_t`, `in_vertex_edge_iterator_t`, `in_edge_t`, `out_vertex_edge_range_t`, `out_vertex_edge_iterator_t`, `out_edge_t` | — | — |
| **Views** | `in_incidence.hpp`, `in_neighbors.hpp`, `out_edges_accessor`, `in_edges_accessor` | `bfs.hpp`, `dfs.hpp`, `topological_sort.hpp` (add `EdgeAccessor` param) | — |
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
| `out_edges(g, u)` | → `edges(g, u)` | | |
| `out_degree(g, u)` | → `degree(g, u)` | | |

## Appendix B: No-Rename Justification

Several graph libraries (Boost.Graph, NetworkX, LEDA) distinguish `out_edges` /
`in_edges` explicitly. In graph-v3, `edges()` has **always** meant outgoing (this is
documented and all algorithms depend on it). Renaming `edges()` to `out_edges()` would:

- Break every existing user's code
- Require renaming `vertex_edge_range_t` → `out_vertex_edge_range_t` across 50+ files
- Create churn in all 14 algorithms

Instead we keep `edges()` as the primary outgoing CPO (matching the "default is outgoing"
convention) and add `out_edges` as a convenience alias for codebases that want explicit
directionality.
