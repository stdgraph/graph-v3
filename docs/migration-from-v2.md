# Migrating from graph-v2 to graph-v3

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
| "Sourced" function overloads | Required | Eliminated — source vertex always available via edge descriptor |
| Vertex ID-only vs reference overloads | Both needed | Single overload via descriptors |
| Undirected graph tagging | Required | Not required (verified by `undirected_adjacency_list` tests) |
| Vertex containers | `vector` only | `vector`, `deque`, `map`, `unordered_map` |
| Edge containers | `vector` only | `vector`, `deque`, `forward_list`, `list`, `set`, `unordered_set`, `map` |
| Non-integral vertex IDs | Not supported | Supported in `dynamic_graph` |
| Namespaces | Flat `graph::` | `graph::adj_list::`, `graph::edge_list::`, `graph::views::`, `graph::container::` |

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

- **`undirected_adjacency_list`** added for undirected graph use cases with O(1) edge removal.
- **`dynamic_graph`** now supports:
  - Vertex storage in `map` and `unordered_map` (for sparse vertex IDs).
  - Edge storage in `map`, `set`, `unordered_set` (for sorted or deduplicated edges).
  - Non-integral vertex IDs.
  - 27 vertex×edge container combinations via traits (see
    [Containers](user-guide/containers.md)).

### Graph Container Interface

- Added support for non-integral vertex IDs.
- Extended range types for vertices and edges:
  - **Vertices:** bidirectional (e.g. `map`) and forward (e.g. `unordered_map`) ranges.
  - **Edges:** bidirectional (`map`, `set`), forward (`unordered_map`, `unordered_set`).
  - **Impact:** GCI (P3130), Views (P3129), `dynamic_graph`. **Not** supported by algorithms (P3128).

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
| `graph::` | `graph::` | Root — re-exports `adj_list` types/CPOs for convenience |
| *(mixed into `graph::`)* | `graph::adj_list::` | Adjacency list CPOs, descriptors, concepts, traits |
| *(N/A)* | `graph::edge_list::` | Edge list concepts, traits, descriptors |
| *(mixed into `graph::`)* | `graph::views::` | Graph views |
| *(N/A)* | `graph::container::` | Concrete graph containers |

> **Backward compatibility:** Core `adj_list` types and CPOs are re-exported into `graph::`
> via `using` declarations, so most v2 code using `graph::vertices(g)` etc. will continue to
> compile without changes.

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
- [ ] If using custom containers: check trait support in the
      [container matrix](user-guide/containers.md)

---

> [← Back to Documentation Index](index.md)
