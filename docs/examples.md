<table><tr>
<td><img src="assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Examples

> Complete working programs that demonstrate graph-v3 features — from simple traversals to full algorithm usage and third-party graph adaptation.

</td>
</tr></table>

> [← Back to Documentation Index](index.md)

---

## Quick Reference

| Example | What it demonstrates |
|---------|---------------------|
| [Adapting a Third-Party Graph](#adapting-a-third-party-graph) | Wrapping an existing type with CPO friend functions |
| [CppCon 2021](#cppcon-2021) | BFS, Dijkstra, path reconstruction on real-world graph data |
| [CppCon 2022](#cppcon-2022) | Range-of-ranges adaptor, Dijkstra, Graphviz output |
| [PageRank](#pagerank) | PageRank algorithm (placeholder) |
| [Basic Usage](#basic-usage) | Minimal vertex/edge traversal |
| [Dijkstra CLRS](#dijkstra-clrs) | Dijkstra example from CLRS textbook |
| [MST Usage](#mst-usage) | Minimum spanning tree |
| [BGL Adaptor](#bgl-adaptor) | Using graph-v3 on Boost.Graph data structures |

---

## Adapting a Third-Party Graph

**Directory:** `examples/AdaptingThirdPartyGraph/`

Demonstrates how to adapt an existing third-party graph container for use with
graph-v3 without modifying the original type.  You provide a small set of
ADL-findable free functions in the type's namespace:

- `vertices(g)` — vertex range (wrapped into `vertex_descriptor_view`)
- `edges(g, u)` — out-edge range for a vertex descriptor
- `target_id(g, uv)` — target vertex ID from an edge descriptor
- `find_vertex(g, uid)` — vertex descriptor from a vertex ID

Once these are defined, the type satisfies `graph::adjacency_list<G>` and works
with all views and algorithms.

**Key concepts demonstrated:**
- CPO friend-function pattern with SFINAE trailing return types
- Vertex descriptor vs. vertex reference distinction
- Optional `vertex_value` / `edge_value` overrides

**Build target:** `adapting_third_party_graph`

---

## CppCon 2021

**Directory:** `examples/CppCon2021/`

Four standalone programs from the CppCon 2021 presentation, refactored from
graph-v2 to graph-v3.  Each program is self-contained with its own graph data.

### graphs.cpp

Basic graph construction and traversal using `vertexlist` and `incidence` views.
Shows structured bindings `[uid, u]` and `[vid, uv]`.

**Build target:** `cppcon21_graphs`

### bacon.cpp

The Kevin Bacon "six degrees of separation" problem.  Uses BFS
(`vertices_breadth_first_search`) to find shortest paths in a social graph.

**Build target:** `cppcon21_bacon`

### ospf.cpp

OSPF-style shortest-path routing.  Runs `dijkstra_shortest_paths` on a
weighted network graph and reconstructs the shortest-path tree.

**Build target:** `cppcon21_ospf`

### imdb.cpp

Builds an actor/movie bipartite graph from the IMDB dataset, then uses BFS to
find paths between actors.

**Build target:** `cppcon21_imdb`

**Supporting files:**
- `graphs/` — graph data headers (karate, ospf, spice, imdb)
- `include/utilities.hpp` — shared output helpers

---

## CppCon 2022

**Directory:** `examples/CppCon2022/`

Germany routes graph demonstration from the CppCon 2022 presentation, refactored
from graph-v2 to graph-v3.

### germany_routes_example.cpp

Builds a Germany inter-city routes graph using a generic `rr_adaptor` (range-of-ranges
adaptor), traverses vertices and edges, then runs Dijkstra shortest paths twice:
1. **Segment count** — unweighted (each edge costs 1)
2. **Kilometers** — weighted by route distance

Prints the shortest-path tree and reconstructs the path to the farthest city.

**Build target:** `cppcon22_germany_routes`

### rr_adaptor.hpp

A reusable generic adaptor that wraps any random-access range-of-ranges (e.g.
`vector<list<route>>`) plus an optional parallel vertex-value vector and exposes
the full graph-v3 CPO interface.

**Key implementation details:**
- Data members declared *before* friend function trailing-return-type declarations
  (C++ class scope rule: trailing return types only see members declared above)
- `vertex_value` uses `decltype(auto)` without trailing return type
- Template friend functions with SFINAE guards for `edges`, `target_id`, `edge_value`
- `find_vertex` constructs a `vertex_descriptor_view::iterator` directly from a `size_t` index

### graphviz_output.hpp

Three function templates that write Graphviz `.gv` files:
- `output_routes_graphviz` — full graph via `vertexlist` + `incidence`
- `output_routes_graphviz_dfs` — DFS tree via `edges_dfs`

Uses `vertex_value(g, u)` for city names and `edge_value(g, uv)` for distances.

---

## PageRank

**Directory:** `examples/PageRank/`

Placeholder for a PageRank implementation.  PageRank was removed from the
standard algorithm list because there is no single universally-agreed
implementation — convergence criteria, damping factor, and normalization vary
across textbooks and production systems.

---

## Basic Usage

**File:** `examples/basic_usage.cpp`

Minimal example: constructs a `vector<vector<int>>` graph, iterates vertices and
edges using CPOs.  Good starting point for understanding the descriptor model.

**Build target:** `basic_usage`

---

## Dijkstra CLRS

**File:** `examples/dijkstra_clrs_example.cpp`

Dijkstra's algorithm on the textbook graph from *Introduction to Algorithms*
(Cormen, Leiserson, Rivest, Stein).  Demonstrates `dijkstra_shortest_paths` with
weight function, distance, and predecessor containers.

**Build target:** `dijkstra_example`

---

## MST Usage

**File:** `examples/mst_usage_example.cpp`

Minimum spanning tree (Kruskal's algorithm) on a weighted graph.  Shows
`kruskal_minimum_spanning_tree` with edge-value weight extraction.

**Build target:** `mst_usage_example`

---

## BGL Adaptor

**File:** `examples/bgl_adaptor_example.cpp`

Demonstrates using graph-v3 views and algorithms on Boost.Graph `adjacency_list`
via the BGL adaptor (`graph::bgl::graph_adaptor`).  Shows property bridge
functions for mapping BGL property maps to graph-v3 value functions.

**Build target:** `bgl_adaptor_example`

---

## Building the Examples

All examples are built as part of the default CMake build:

```bash
cmake --preset linux-gcc-debug
cmake --build build/linux-gcc-debug
```

To build a specific example:

```bash
cmake --build build/linux-gcc-debug --target cppcon22_germany_routes
```

Examples link against the `graph3` library target and require no external
dependencies beyond the library itself (Boost is needed only for
`bgl_adaptor_example`).
