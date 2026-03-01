<table><tr>
<td><img src="assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Migrating from graph-v2 to graph-v3

</td>
</tr></table>

> [← Back to Documentation Index](index.md)

## Overview

**graph-v3** is a ground-up rewrite of graph-v2, driven by the adoption of **descriptors** as
the primary abstraction for accessing vertices and edges. This shift — from reference-based to
value-based access — simplified the design, reduced concept count, and enabled support for a
much wider range of container types.

This guide summarizes what changed and what you need to update if you're migrating from v2.

---

## Key Changes at a Glance

| Area | graph-v2 | graph-v3 |
|------|----------|----------|
| Access model | Reference-based | Value-based (descriptors) |
| Concepts | 18 | 9 |
| "Sourced" function and view overloads | Required | Eliminated — source vertex always available via edge descriptor |
| Vertex ID-only vs reference overloads | Both needed | Single overload via descriptors |
| Undirected graph tagging | Required | Not required (verified by `undirected_adjacency_list` tests) |
| Vertex containers | `vector` and `deque` only | `vector`, `deque`, `map`, `unordered_map` (sparse vertex ids)|
| Edge containers | `vector` and `deque` only | `vector`, `deque`, `forward_list`, `list`, `set`, `unordered_set`, `map` |
| Non-integral vertex IDs | Not supported | Supported in `dynamic_graph` |
| Bidirectional graphs (incoming edges) | Not supported | `dynamic_graph<...,true,true,...>` and `undirected_adjacency_list` satisfy new `bidirectional_adjacency_list` |
| Namespaces | `graph::`, `graph::edge_list::`, `graph::views::`, `graph::container::` | `graph::adj_list::`, `graph::edge_list::`, `graph::views::`, `graph::container::` |

---

## Detailed Changes

### Descriptors

The most fundamental change. Descriptors are lightweight value types that identify vertices and
edges without holding references to the underlying container.

**What this means for your code:**
- `source_id` is always available on an edge descriptor — no need for separate "sourced" functions.
- Function overloads that took both a `vertex_id` and a vertex reference were consolidated
  into a single overload using descriptors.
- Concepts were reduced from 18 to 9 because the descriptor model eliminates the need
  for separate sourced/unsourced and id/reference concept variants.

### Containers

#### `undirected_adjacency_list` (new)

A dedicated undirected graph container using a **dual-list design**: each edge is physically
stored in two doubly-linked lists — one at each incident vertex. This gives O(1) vertex access
(via contiguous vertex storage) and O(1) edge removal (by unlinking from both lists), without
relying on the general-purpose `dynamic_graph`.

**Template signature:**

```cpp
template <typename EV         = void,      // edge value type
          typename VV         = void,      // vertex value type
          typename GV         = void,      // graph value type
          integral VId        = uint32_t,  // vertex id / index type
          template <typename V, typename A> class VContainer = std::vector,
          typename Alloc      = std::allocator<char>>
class undirected_adjacency_list;
```

**Complexity guarantees:**

| Operation | Complexity |
|-----------|-----------|
| Vertex access by id | O(1) |
| `create_vertex()` | O(1) amortized |
| `create_edge(u, v)` | O(1) |
| `erase_edge(pos)` | O(1) — unlinks from both vertices' lists |
| `degree(v)` | O(1) — cached per vertex |
| Iterate edges from vertex | O(degree) |
| Iterate all edges | O(V + E) |

**Edge iteration semantics:** at graph level each undirected edge is visited **twice** — once
from each endpoint. Use `edges_size() / 2` to get the unique edge count.

**Iterator invalidation:**
- Vertex iterators: invalidated by `create_vertex()` if reallocation occurs, and `clear()`.
- Vertex iterators: **not** invalidated by `create_edge()` or `erase_edge()`.

**Basic usage:**

```cpp
#include <graph/container/undirected_adjacency_list.hpp>
using namespace graph::container;

// Edge value = int (weight), vertex value = std::string (name)
undirected_adjacency_list<int, std::string> g;

auto u = g.create_vertex("Alice");
auto v = g.create_vertex("Bob");
auto e = g.create_edge(u, v, 42);   // weight 42

// Iterate incident edges of u
for (auto&& [uid, vid, uv] : edges(g, *u)) {
    std::cout << graph::vertex_value(g, *u) << " -- "
              << graph::vertex_value(g, vid)
              << " [" << graph::edge_value(g, uv) << "]\n";
}
```

**v2 migration note:** In v2 undirected graphs required tagging the graph type with an
"undirected" marker. In v3 `undirected_adjacency_list` is its own concrete type — no tagging
needed. Replace any v2 undirected `adjacency_list` instantiation with
`undirected_adjacency_list` and remove the tag.

**When to prefer over `dynamic_graph`:**
- Frequent edge removal (O(1) vs O(degree) for `dynamic_graph`).
- Edges with values that need to be updated in a single place.
- Algorithms that walk incident edges of both endpoints frequently.

**When to prefer alternatives:**
- Read-only or write-once graphs → use `compressed_graph` (lower memory overhead).
- Directed graphs → use `dynamic_graph`.
- Very high per-vertex degrees (thousands of edges) → cache locality of `dynamic_graph`
  edge vectors may outweigh the pointer overhead here.

---

#### `dynamic_graph` (extended)

- **`dynamic_graph`** now supports:
  - Vertex storage in `map` and `unordered_map` (for sparse vertex IDs).
  - Edge storage in `map`, `set`, `unordered_set` (for sorted or deduplicated edges).
  - Non-integral vertex IDs.
  - 27 vertex×edge container combinations via traits (see
    [Containers](user-guide/containers.md)).

### Bidirectional Graphs (new)

graph-v3 introduces the `bidirectional_adjacency_list` concept, which adds incoming-edge
support on top of the standard `adjacency_list` interface. Two containers satisfy it:

| Container | How to enable |
|-----------|---------------|
| `dynamic_graph` | Set `Bidirectional = true` (5th template parameter; `Sourced` was removed in v3) |
| `undirected_adjacency_list` | Always satisfied — every edge is its own reverse |

#### Enabling bidirectional on `dynamic_graph`

```cpp
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>

using namespace graph::container;

// EV=void, VV=void, GV=void, VId=uint32_t, Bidirectional=true
using BiDiGraph = dynamic_graph<void, void, void, uint32_t, true,
                                vov_graph_traits<void, void, void, uint32_t, true>>;

BiDiGraph g({{0, 1}, {0, 2}, {1, 3}, {2, 3}});
```

When `Bidirectional = true`, each vertex automatically maintains an **incoming-edge list**
alongside its outgoing-edge list. Adding an edge `u → v` atomically inserts an in-edge
record at `v`.

#### Incoming-edge CPOs

Available on any graph satisfying `bidirectional_adjacency_list<G>`:

| CPO | Signature | Complexity |
|-----|-----------|------------|
| `in_edges(g, u)` | vertex descriptor → range | O(1) |
| `in_edges(g, uid)` | vertex id → range | O(1) |
| `in_degree(g, u)` | vertex descriptor → integral | O(1) |
| `in_degree(g, uid)` | vertex id → integral | O(1) |
| `find_in_edge(g, uid, vid)` | target id, source id → iterator | O(in-degree) |
| `contains_in_edge(g, uid, vid)` | target id, source id → bool | O(in-degree) |
| `source_id(g, e)` | in-edge descriptor → vertex id | O(1) |

```cpp
#include <graph/graph.hpp>

auto v3 = *find_vertex(g, 3u);

// Iterate incoming edges to vertex 3
for (auto&& e : in_edges(g, v3)) {
    std::cout << source_id(g, e) << " -> 3\n";
}

std::cout << "in_degree(3) = " << in_degree(g, v3) << "\n";

// Find a specific incoming edge
auto it = find_in_edge(g, 3u, 1u);  // edge from vertex 1 to vertex 3
bool exists = contains_in_edge(g, 3u, 1u);
```

#### Incoming-edge views

The `incidence` and `neighbors` views have incoming-edge variants, parameterized via
the `in_edge_accessor` policy. These are constrained on `bidirectional_adjacency_list`.

| View | Direction | Structured binding yields |
|------|-----------|---------------------------|
| `incidence(g, u)` | outgoing (default) | `[tid, uv]` |
| `out_incidence(g, u)` | outgoing (explicit) | `[tid, uv]` |
| `in_incidence(g, u)` | incoming | `[sid, uv]` |
| `basic_incidence(g, uid)` | outgoing | `[tid]` |
| `basic_out_incidence(g, uid)` | outgoing (explicit) | `[tid]` |
| `basic_in_incidence(g, uid)` | incoming | `[sid]` |
| `neighbors(g, u)` | outgoing (default) | `[tid, v]` |
| `in_neighbors(g, u)` | incoming | `[sid, v]` |

```cpp
#include <graph/graph.hpp>

using namespace graph::views;

// Incoming edges to vertex 3
for (auto [sid, uv] : in_incidence(g, 3u)) {
    std::cout << "Edge from " << sid << "\n";
}

// Predecessor vertices of vertex 3
for (auto [sid, v] : in_neighbors(g, 3u)) {
    std::cout << "Predecessor: " << sid << "\n";
}

// With edge value function
auto evf = [](const auto& g, auto& uv) { return edge_value(g, uv); };
for (auto [sid, uv, w] : in_incidence(g, 3u, evf)) {
    std::cout << sid << " weight " << w << "\n";
}
```

#### Algorithms using bidirectional graphs

- **`strongly_connected_components`** (Kosaraju's algorithm) requires
  `index_bidirectional_adjacency_list` to perform the reverse pass over incoming edges.
- Reverse BFS/DFS is achieved by swapping `incidence` for `in_incidence` in the
  traversal loop — no separate reverse-graph construction needed.

> See the full reference in [Bidirectional Edge Access](user-guide/bidirectional-access.md).

---

### Graph Container Interface

- Added support for non-integral vertex IDs.
- Extended range types for vertices and edges:
  - **Vertices:** bidirectional (e.g. `map`) and forward (e.g. `unordered_map`) ranges, enabling sparse vertex ids.
  - **Edges:** bidirectional (`map`, `set`), forward (`unordered_map`, `unordered_set`).
  - **Impact:** GCI (P3130), Views (P3129), `dynamic_graph`. **Not** supported by algorithms (P3128) at this time.

### Views

- **`topological_sort_view`** implemented, including a "safe" version with cycle detection.
- **BFS views** (`vertices_bfs`, `edges_bfs`): added cancellation and `depth()`.
- **DFS views** (`vertices_dfs`, `edges_dfs`): implemented with visitor support.
- **View chaining** added (pipe syntax, e.g. `vertices(g) | views::filter(...)`).
- **Value functions** (`VVF`, `EVF`) now require a graph parameter `g` — this enables
  valueless lambdas for full flexibility.

### Algorithms

- **Topological sort algorithm** implemented for vertices and edges, with "safe" versions
  for cycle detection.
- Algorithm documentation follows C++ standard description conventions.
- 13 algorithms now implemented (see [Algorithms](user-guide/algorithms.md)).

### Namespaces

Definitions specific to adjacency lists moved into `graph::adj_list::` to reflect that
`graph::edge_list::` is a peer abstract data structure, not a subset.

| v2 namespace | v3 namespace | Contents |
|--------------|--------------|----------|
| `graph::` | `graph::` | Root — re-exports `adj_list` types/CPOs for convenience. Common edge list definitions. |
| *(mixed into `graph::`)* | `graph::adj_list::` | Adjacency list CPOs, descriptors, concepts, traits |
| `graph::edge_list::` | `graph::edge_list::` | Edge list concepts, traits, descriptors. Separate from adjacency lists. |
| `graph::views` | `graph::views::` | Graph views |
| `graph::container::` | `graph::container::` | Concrete graph containers |

> **Backward compatibility:** Core `adj_list` types and CPOs are re-exported into `graph::`
> via `using` declarations, so most v2 code using `graph::vertices(g)` etc. will continue to
> compile without changes. This may be removed in the future.

---

## C++ Standard Note

graph-v3 targets C++20. However, `std::expected` from C++23 is used by `topological_sort_view`
for cycle detection. A third-party library (`tl::expected`) provides this until C++23 is enabled
project-wide. There is no target date for that transition.

---

## Migration Checklist

- [ ] Replace `#include <graph/...>` paths with updated header locations
- [ ] Remove "sourced" function overloads — use edge descriptors instead
- [ ] Remove vertex ID / vertex reference dual overloads — use descriptors
- [ ] Update namespace qualifications if using explicit `graph::` prefixes
- [ ] Update value function lambdas to accept `(const auto& g, ...)` as first parameter
- [ ] If using undirected graphs: remove "undirected" tagging, use `undirected_adjacency_list`
- [ ] If you need predecessor/incoming-edge queries: use `dynamic_graph` with `Bidirectional=true`, or `undirected_adjacency_list`; replace manual reverse-graph builds with `in_edges`, `in_degree`, `in_incidence`, or `in_neighbors`
- [ ] If using custom containers: check trait support in the
      [container matrix](user-guide/containers.md)

---

> [← Back to Documentation Index](index.md)
