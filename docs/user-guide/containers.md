# Graph Containers

> [← Back to Documentation Index](../index.md)

graph-v3 ships three purpose-built graph containers. Each satisfies the
adjacency list concepts so all CPOs, views, and algorithms work interchangeably.

| Container | Storage | Mutability | Best for |
|-----------|---------|------------|----------|
| [`dynamic_graph`](#1-dynamic_graph) | Traits-configured vertex + edge containers | Mutable | General purpose, flexible container choice |
| [`compressed_graph`](#2-compressed_graph) | CSR (Compressed Sparse Row) | Immutable after construction | Read-only, high performance, memory-compact |
| [`undirected_adjacency_list`](#3-undirected_adjacency_list) | Dual doubly-linked lists per edge | Mutable, O(1) edge removal | Undirected graphs, frequent edge insertion/removal |

All three live in `graph::container`.

You can also use graphs without any library container:

- **[Range-of-Ranges Graphs](#4-range-of-ranges-graphs-no-library-graph-container-required)** — use standard containers (e.g. `vector<vector<int>>`) directly as graphs, zero-copy
- **[Custom Graphs](#5-custom-graphs)** — adapt your own graph data structure by overriding graph CPOs via ADL
- **[Container Selection Guide](#6-container-selection-guide)** — decision tree and comparison matrix

---

## 1. `dynamic_graph`

```cpp
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>  // pick your trait

namespace graph::container {
template <class EV     = void,       // edge value type
          class VV     = void,       // vertex value type
          class GV     = void,       // graph value type
          class VId    = uint32_t,   // vertex id type
          bool Sourced = false,      // store source_id on edges?
          class Traits = vofl_graph_traits<EV, VV, GV, VId, Sourced>>
class dynamic_graph;
}
```

`dynamic_graph` is the most flexible container. Its vertex and edge storage are
determined entirely by the **Traits** parameter, which names the concrete
`vertices_type` and `edges_type` (e.g., `std::vector<vertex_type>` and
`std::forward_list<edge_type>`).

### Properties

| Property | Value |
|----------|-------|
| Vertex ID assignment | Contiguous (0 .. N-1) for `v`/`d` traits; sparse user-defined keys for `m`/`u` traits |
| Vertex range | Random access (`v`/`d`), bidirectional (`m`), forward (`u`) |
| Edge range per vertex | Random access (`v`/`d`), forward (`fl`/`us`), bidirectional (`l`/`s`/`em`) |
| Partitions | No |
| Append vertices/edges | Yes |

### Complexity guarantees

Complexity depends on the vertex container, the edge container, or neither.

**Vertex-container-dependent operations:**

| Operation | `v`/`d` | `m` | `u` |
|-----------|---------|-----|-----|
| `find_vertex(g, uid)` | O(1) | O(log V) | O(1) avg |
| Add vertex | O(1) amortized | O(log V) | O(1) avg |

**Edge-container-dependent operations:**

| Operation | `v`/`d`/`l` | `fl` | `s`/`em` | `us` |
|-----------|-------------|------|----------|------|
| `find_vertex_edge(g, u, vid)` | O(degree) | O(degree) | O(log degree) | O(1) avg |
| Add edge | O(1) amortized† | O(1) | O(log degree) | O(1) avg |
| `degree(g, u)` | O(1) | O(degree) | O(1) | O(1) |

† O(1) amortized for `v`/`d` (reallocation); O(1) for `l` (no reallocation).

**Container-independent operations:**

| Operation | Complexity |
|-----------|------------|
| `num_vertices(g)` | O(1) |
| `num_edges(g)` | O(1) |
| `has_edge(g)` | O(1) |

### Template parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `EV` | `void` | Edge value type (`void` → no edge values) |
| `VV` | `void` | Vertex value type (`void` → no vertex values) |
| `GV` | `void` | Graph value type (`void` → no graph value) |
| `VId` | `uint32_t` | Vertex ID type (integral for indexed traits, any ordered/hashable type for map-based traits) |
| `Sourced` | `false` | When `true`, each edge stores a source vertex ID. This does not affect the ability to use `source_id(g,uv)`. |
| `Traits` | `vofl_graph_traits<…>` | Trait struct that defines the vertex and edge container types |

### Quick usage

```cpp
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>

using namespace graph::container;

// vector of vertices with vector of edges
// Weighted edges (double), labeled vertices (string), no graph value
using Traits = vov_graph_traits<double, std::string>;
using G      = dynamic_adjacency_graph<Traits>;

G g;
// ... populate with load_edges / load_vertices ...
```

The `dynamic_adjacency_graph<Traits>` alias extracts `EV`, `VV`, `GV`, `VId`,
and `Sourced` from the traits struct, so you only need one template argument.

### Trait combinations

Traits follow the naming convention **`{vertex}o{edge}_graph_traits`** — vertex
container abbreviation, the letter `o`, edge container abbreviation.

#### Vertex container abbreviations

| Container | Iterator type | Vertex ID type | Other  | Abbrev |
|-----------|---------------|----------------|--------|--------|
| `std::vector` | Random access | Integral index | | `v` |
| `std::deque` | Random access | Integral index | | `d` |
| `std::map` | Bidirectional | Ordered key (any `operator<` type) | deduplicated vertex id | `m` |
| `std::unordered_map` | Forward | Hashable key (any with `std::hash`) | deduplicated vertex id | `u` |

#### Edge container abbreviations

| Container | Iterator type | Properties | Abbrev |
|-----------|---------------|------------|--------|
| `std::vector` | Random access | Cache-friendly, allows duplicates | `v` |
| `std::deque` | Random access | Efficient front/back insertion | `d` |
| `std::forward_list` | Forward | Minimal memory overhead. Edges added to the front. | `fl` |
| `std::list` | Bidirectional | O(1) insertion/removal anywhere. Edges added to the back. | `l` |
| `std::set` | Bidirectional | Sorted, deduplicated target id | `s` |
| `std::unordered_set` | Forward | Hash-based, O(1) avg lookup, deduplicated target id| `us` |
| `std::map` | Bidirectional | Sorted by target_id key, deduplicated target id | `em` |

#### Full 26-combination matrix

Each trait struct is in `graph::container` and has its own header in
`include/graph/container/traits/`.

| Trait | Vertices | Edges | Header |
|-------|----------|-------|--------|
| `vov_graph_traits` | `vector` | `vector` | `traits/vov_graph_traits.hpp` |
| `vod_graph_traits` | `vector` | `deque` | `traits/vod_graph_traits.hpp` |
| `vofl_graph_traits` | `vector` | `forward_list` | `traits/vofl_graph_traits.hpp` |
| `vol_graph_traits` | `vector` | `list` | `traits/vol_graph_traits.hpp` |
| `vos_graph_traits` | `vector` | `set` | `traits/vos_graph_traits.hpp` |
| `vous_graph_traits` | `vector` | `unordered_set` | `traits/vous_graph_traits.hpp` |
| `voem_graph_traits` | `vector` | `map` | `traits/voem_graph_traits.hpp` |
| `dov_graph_traits` | `deque` | `vector` | `traits/dov_graph_traits.hpp` |
| `dod_graph_traits` | `deque` | `deque` | `traits/dod_graph_traits.hpp` |
| `dofl_graph_traits` | `deque` | `forward_list` | `traits/dofl_graph_traits.hpp` |
| `dol_graph_traits` | `deque` | `list` | `traits/dol_graph_traits.hpp` |
| `dos_graph_traits` | `deque` | `set` | `traits/dos_graph_traits.hpp` |
| `dous_graph_traits` | `deque` | `unordered_set` | `traits/dous_graph_traits.hpp` |
| `mov_graph_traits` | `map` | `vector` | `traits/mov_graph_traits.hpp` |
| `mod_graph_traits` | `map` | `deque` | `traits/mod_graph_traits.hpp` |
| `mofl_graph_traits` | `map` | `forward_list` | `traits/mofl_graph_traits.hpp` |
| `mol_graph_traits` | `map` | `list` | `traits/mol_graph_traits.hpp` |
| `mos_graph_traits` | `map` | `set` | `traits/mos_graph_traits.hpp` |
| `mous_graph_traits` | `map` | `unordered_set` | `traits/mous_graph_traits.hpp` |
| `moem_graph_traits` | `map` | `map` | `traits/moem_graph_traits.hpp` |
| `uov_graph_traits` | `unordered_map` | `vector` | `traits/uov_graph_traits.hpp` |
| `uod_graph_traits` | `unordered_map` | `deque` | `traits/uod_graph_traits.hpp` |
| `uofl_graph_traits` | `unordered_map` | `forward_list` | `traits/uofl_graph_traits.hpp` |
| `uol_graph_traits` | `unordered_map` | `list` | `traits/uol_graph_traits.hpp` |
| `uos_graph_traits` | `unordered_map` | `set` | `traits/uos_graph_traits.hpp` |
| `uous_graph_traits` | `unordered_map` | `unordered_set` | `traits/uous_graph_traits.hpp` |

#### Common trait parameters

All trait structs share the same template parameters:

```cpp
template <class EV = void, class VV = void, class GV = void,
          class VId = uint32_t, bool Sourced = false>
struct vov_graph_traits { ... };
```

Each defines:
- `edge_value_type`, `vertex_value_type`, `graph_value_type`, `vertex_id_type`
- `static constexpr bool sourced`
- `edge_type`, `vertex_type`, `graph_type`
- `vertices_type` (e.g., `std::vector<vertex_type>`)
- `edges_type` (e.g., `std::vector<edge_type>`)

#### Using non-default traits

```cpp
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/mofl_graph_traits.hpp>

using namespace graph::container;

// Sparse graph with string vertex IDs, double edge weights
using Traits = mofl_graph_traits<double, std::string, void, std::string>;
using G      = dynamic_adjacency_graph<Traits>;
// Vertices: std::map<std::string, vertex_type>
// Edges:    std::forward_list<edge_type>
```

> **Note:** Map-based vertex containers (`m*`, `u*` traits) require vertices to
> be explicitly created — they do not auto-extend when edges reference undefined
> vertex IDs, unlike `vector`/`deque`-based traits.

### Defining custom traits

You can define your own traits struct to use containers not in the standard
library (e.g., Boost, Abseil, or your own). A traits struct must provide exactly
the type aliases and constant shown below:

```cpp
#include <graph/container/dynamic_graph.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>

namespace myapp {

// Forward declarations required by dynamic_edge / dynamic_vertex / dynamic_graph
using namespace graph::container;

template <class EV = void, class VV = void, class GV = void,
          class VId = uint32_t, bool Sourced = false>
struct flat_map_small_vec_traits {
  // --- Required type aliases ---
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced = Sourced;

  // --- Edge, vertex, and graph types (always use dynamic_*) ---
  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced,
                                   flat_map_small_vec_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced,
                                     flat_map_small_vec_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced,
                                    flat_map_small_vec_traits>;

  // --- Storage types (your custom containers) ---
  using vertices_type = boost::container::flat_map<VId, vertex_type>;
  using edges_type    = boost::container::small_vector<edge_type, 8>;
};

} // namespace myapp
```

**Container requirements:**

| Member | Container must satisfy |
|--------|-----------------------|
| `vertices_type` | `std::ranges::forward_range` with `value_type` = `vertex_type`. For indexed vertex IDs, must also be a `std::ranges::sized_range` supporting `operator[]`. For keyed vertex IDs, must support `find(key)` and `end()`. |
| `edges_type` | `std::ranges::forward_range` with `value_type` = `edge_type`. Must support insertion (`push_back`, `push_front`, `insert`, or `emplace`). |

**Usage:**

```cpp
using Traits = myapp::flat_map_small_vec_traits<double, std::string>;
using G      = graph::container::dynamic_adjacency_graph<Traits>;

G g;
// All CPOs, views, and algorithms work as normal
```

> **Tip:** Model your traits struct on one of the 26 built-in traits headers in
> `include/graph/container/traits/`. The simplest starting point is
> `vov_graph_traits.hpp`.

---

## 2. `compressed_graph`

```cpp
#include <graph/container/compressed_graph.hpp>

namespace graph::container {
template <class EV        = void,            // edge value type
          class VV        = void,            // vertex value type
          class GV        = void,            // graph value type
          integral VId    = uint32_t,        // vertex id type
          integral EIndex = uint32_t,        // edge index type
          class Alloc     = std::allocator<VId>>
class compressed_graph;
}
```

`compressed_graph` uses CSR (Compressed Sparse Row) format for maximum memory
density and cache locality. Vertices and edges cannot be added or removed after
construction, but values on them can be modified.

### Properties

| Property | Value |
|----------|-------|
| Vertex ID assignment | Contiguous (0 .. N-1) |
| Vertex range | Contiguous |
| Edge range | Contiguous per vertex |
| Partitions | Optional (multi-partite support) |
| Append vertices/edges | No |

### Complexity guarantees

| Operation | Complexity |
|-----------|------------|
| Vertex access by ID | O(1) |
| `find_vertex(g, uid)` | O(1) |
| `find_vertex_edge(g, u, vid)` | O(degree) |
| `num_vertices(g)` | O(1) |
| `num_edges(g)` | O(1) |
| `degree(g, u)` | O(1) |
| Iterate edges from vertex | O(degree) |

### Memory layout

$$|V| \times (\text{sizeof}(\texttt{EIndex}) + \text{sizeof}(\texttt{VV})) + |E| \times (\text{sizeof}(\texttt{VId}) + \text{sizeof}(\texttt{EV})) + \text{sizeof}(\texttt{GV})$$

When `VV`, `EV`, or `GV` is `void`, that term contributes zero.

### Quick usage

```cpp
#include <graph/container/compressed_graph.hpp>

using namespace graph::container;

// Construct from an edge list (tuple<source, target, weight>)
std::vector<std::tuple<int,int,double>> edges = {
  {0, 1, 1.5}, {1, 2, 2.0}, {2, 0, 0.5}
};
compressed_graph<double> g(edges, 3);  // 3 vertices, EV=double

// Values on edges are mutable even though structure is not
for (auto&& u : graph::vertices(g)) {
  for (auto&& uv : graph::edges(g, u)) {
    graph::edge_value(g, uv) *= 2.0;
  }
}
```

### Template parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `EV` | `void` | Edge value type |
| `VV` | `void` | Vertex value type |
| `GV` | `void` | Graph value type |
| `VId` | `uint32_t` | Vertex ID type (must be integral; size must hold \|V\|+1) |
| `EIndex` | `uint32_t` | Edge index type (must be integral; size must hold \|E\|+1) |
| `Alloc` | `std::allocator<VId>` | Allocator (rebound for internal containers) |

---

## 3. `undirected_adjacency_list`

```cpp
#include <graph/container/undirected_adjacency_list.hpp>

namespace graph::container {
template <typename VV                                        = void,
          typename EV                                        = void,
          typename GV                                        = void,
          integral VId                                       = uint32_t,
          template <typename V, typename A> class VContainer = std::vector,
          typename Alloc                                     = std::allocator<char>>
class undirected_adjacency_list;
}
```

Each undirected edge appears in two doubly-linked lists — one at each endpoint.
This gives O(1) edge removal from both vertices and efficient iteration of
incident edges. Edges are not duplicated, so this is ideal when edge properties 
need to be changed or when there are many edge properties.

### Complexity guarantees

| Operation | Complexity |
|-----------|------------|
| Vertex access by ID | O(1) |
| `find_vertex(g, uid)` | O(1) |
| `find_vertex_edge(g, u, vid)` | O(degree) |
| Add vertex | O(1) amortized |
| Add edge | O(1) |
| Remove edge | O(degree) find + O(1) unlink |
| Degree query | O(1) (cached) |
| Iterate edges from vertex | O(degree) |
| Iterate all edges | O(V + E) |

### Memory overhead

- Per vertex: ~24-32 bytes (list head pointers + value)
- Per edge: ~48-64 bytes (4 list pointers, 2 vertex IDs, value, allocation overhead)

### Iteration note

Edge iteration at graph level visits each edge **twice** (once from each
endpoint). Use `edges_size() / 2` for the unique edge count.

### Template parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `VV` | `void` | Vertex value type |
| `EV` | `void` | Edge value type |
| `GV` | `void` | Graph value type |
| `VId` | `uint32_t` | Vertex ID type (integral) |
| `VContainer` | `std::vector` | Vertex storage container template |
| `Alloc` | `std::allocator<char>` | Allocator |

> **Note:** The template parameter order is `VV, EV, GV` (vertex-first), which
> differs from `dynamic_graph` and `compressed_graph` (`EV, VV, GV`).

---

## 4. Range-of-Ranges Graphs (No Library Graph Container Required)

You do not need `dynamic_graph`, `compressed_graph`, or any library container
to use graph-v3. Any **range-of-ranges** whose elements follow a recognised
pattern works directly with all CPOs, views, and algorithms:

```cpp
#include <graph/graph.hpp>

// A plain vector-of-vectors IS a graph — no wrapper needed
std::vector<std::vector<std::pair<int,double>>> g = {
  {{1, 1.5}, {2, 0.5}},   // vertex 0 → edges to 1 (weight 1.5) and 2 (0.5)
  {{2, 2.0}},              // vertex 1 → edge to 2 (weight 2.0)
  {}                       // vertex 2 → no outgoing edges
};

for (auto&& u : graph::vertices(g)) {
  for (auto&& uv : graph::edges(g, u)) {
    auto vid = graph::target_id(g, uv);
    auto w   = graph::edge_value(g, uv);   // returns pair.second (the double)
  }
}
```

The library detects the graph structure from the **range categories** and
**element types** — not from specific container names. Any container satisfying
the requirements below will work, including third-party containers
(Boost, Abseil, etc.).

### Outer range (vertex container) requirements

The outer range holds vertices. Each element represents one vertex; its position
or key becomes the vertex ID.

| Outer range category | Vertex ID type | Examples |
|----------------------|----------------|----------|
| Random-access + sized | Integral index (0 .. N-1) | `std::vector<…>`, `std::deque<…>`, `std::array<…, N>` |
| Associative (pair-valued) | Key type | `std::map<K, …>`, `std::unordered_map<K, …>` |

The outer range must be a `forward_range`. For `num_vertices(g)` to work, it
must also be a `sized_range`.

When the outer range is an associative container, vertex IDs are keys rather
than indices:

```cpp
std::map<std::string, std::vector<std::pair<std::string, double>>> g = {
  {"A", {{"B", 1.0}, {"C", 2.0}}},
  {"B", {{"C", 0.5}}},
  {"C", {}}
};

for (auto&& u : graph::vertices(g)) {
  auto vid = graph::vertex_id(g, u);        // std::string ("A", "B", "C")
  for (auto&& uv : graph::edges(g, u)) {
    auto tid = graph::target_id(g, uv);     // std::string
    auto w   = graph::edge_value(g, uv);    // double
  }
}
```

| Outer container | `vertex_id_t<G>` | `find_vertex` complexity |
|----------------|-------------------|--------------------------|
| `std::map<K, …>` | `K` | O(log V) |
| `std::unordered_map<K, …>` | `K` | O(1) avg |

In a plain range-of-ranges, the "vertex value" returned by `vertex_value(g, u)`
**is the inner range itself** (the edge list). There is no separate vertex data
slot. If you need per-vertex data alongside edges, use `dynamic_graph` with a
vertex value type, or store vertex data in a separate parallel container.

### Inner range (edge container) requirements

Each vertex's value must be a `forward_range` whose elements are the edges
leaving that vertex. The element type determines how `target_id` and
`edge_value` are extracted.

### Edge element patterns

The library recognises four edge element patterns, checked in priority order:

#### 1. Simple integral — target ID only

```cpp
std::vector<std::vector<int>> g = {
  {1, 2},    // vertex 0 → edges to vertices 1 and 2
  {0},       // vertex 1 → edge to vertex 0
  {}
};
auto vid = graph::target_id(g, uv);   // the int value itself
auto ev  = graph::edge_value(g, uv);  // also the int value (same as target_id)
```

| `target_id` | `edge_value` |
|-------------|-------------|
| The value itself | The value itself |

Any `std::integral` type (`int`, `uint32_t`, `size_t`, …).

#### 2. Pair — target ID + one property

```cpp
std::vector<std::vector<std::pair<int, double>>> g = {
  {{1, 1.5}, {2, 0.5}},
  {{2, 2.0}},
  {}
};
auto vid = graph::target_id(g, uv);   // pair.first
auto w   = graph::edge_value(g, uv);  // pair.second (mutable reference)
```

| `target_id` | `edge_value` |
|-------------|-------------|
| `.first` | `.second` |

Works with `std::pair` and any type with `.first` and `.second` members.
`edge_value` returns a reference, so the property is mutable in-place.

#### 3. Tuple — target ID + one or more properties

```cpp
// Two-element tuple (same semantics as pair)
std::vector<std::vector<std::tuple<int, double>>> g = { ... };
auto vid = graph::target_id(g, uv);   // get<0>
auto w   = graph::edge_value(g, uv);  // get<1> (single value reference)

// Three-or-more-element tuple
std::vector<std::vector<std::tuple<int, double, std::string>>> g = { ... };
auto vid = graph::target_id(g, uv);   // get<0>
auto ev  = graph::edge_value(g, uv);  // tuple of references: {get<1>, get<2>, ...}
```

| Elements | `target_id` | `edge_value` |
|----------|-------------|-------------|
| 2 | `get<0>` | `get<1>` (single reference) |
| 3+ | `get<0>` | `forward_as_tuple(get<1>, get<2>, …)` |

Any type satisfying `std::tuple_size` (e.g., `std::tuple`, `std::array`).

#### 4. Custom struct — user-defined extraction

```cpp
struct MyEdge {
  int    target;
  double weight;
  // Option A: provide a target_id() member
  int target_id() const { return target; }
};

std::vector<std::vector<MyEdge>> g = { ... };
auto vid = graph::target_id(g, uv);   // calls MyEdge::target_id()
auto ev  = graph::edge_value(g, uv);  // returns MyEdge& (whole struct)
```

| Has `target_id()` member? | `target_id` | `edge_value` |
|---------------------------|-------------|-------------|
| Yes | `.target_id()` | Whole struct reference |
| No | Must provide ADL `target_id(g, uv)` | Whole struct reference |

If your struct does not have a `target_id()` member, provide a free function
found by ADL.

### `source_id(g, uv)` — always available

Every edge descriptor stores its source vertex, so `source_id(g, uv)` works
for all range-of-ranges graphs without any extra annotation.

### Quick reference

| Graph type | `vertex_id` type | `target_id` source | `edge_value` type |
|-----------|-----------------|-------------------|------------------|
| `vector<vector<int>>` | `size_t` | value | `int` |
| `vector<vector<pair<int,double>>>` | `size_t` | `.first` | `double&` |
| `vector<vector<tuple<int,double>>>` | `size_t` | `get<0>` | `double&` |
| `vector<vector<tuple<int,double,string>>>` | `size_t` | `get<0>` | `tuple<double&,string&>` |
| `vector<vector<MyEdge>>` | `size_t` | `.target_id()` | `MyEdge&` |
| `deque<vector<int>>` | `size_t` | value | `int` |
| `map<K, vector<pair<K,double>>>` | `K` | `.first` | `double&` |
| `vector<set<int>>` | `size_t` | value (key) | `int` |

---

## 5. Custom Graphs

If you have an existing graph data structure that does not model a range-of-ranges, you can
still use it with all library views and algorithms by overriding the graph CPOs for your type.
This gives maximum flexibility — your storage layout, indexing scheme, and memory management
remain unchanged while the library operates on them through the CPO interface.

### Which CPOs to override

At minimum, provide ADL overloads for these core CPOs in your type's namespace:

| CPO | Signature | Purpose |
|-----|-----------|------------------|
| `vertices(g)` | Returns a range of vertices | Vertex iteration |
| `edges(g, u)` | Returns a range of edges for vertex `u` | Edge iteration |
| `target_id(g, uv)` | Returns the target vertex id of edge `uv` | Edge traversal |
| `vertex_id(g, u)` | Returns the id of vertex `u` | Vertex identification |

Optional CPOs to override for richer functionality:

| CPO | Purpose |
|-----|------------------|
| `source_id(g, uv)` | Required by algorithms that need edge source |
| `vertex_value(g, u)` | Expose vertex properties |
| `edge_value(g, uv)` | Expose edge properties (weights, labels, etc.) |
| `num_vertices(g)` | Vertex count (defaults to `size(vertices(g))`) |
| `find_vertex(g, uid)` | Sub-linear vertex lookup |
| `find_vertex_edge(g, u, vid)` | Sub-linear edge lookup |

### Example

```cpp
namespace mylib {

struct my_graph {
  struct vertex { int id; std::string name; std::vector<int> neighbors; };
  std::vector<vertex> verts;
};

// Core CPOs — found via ADL
auto vertices(const my_graph& g) { return std::ranges::ref_view(g.verts); }
auto edges(const my_graph& g, const my_graph::vertex& u) {
  return std::ranges::ref_view(u.neighbors);
}
auto target_id(const my_graph&, int tid) { return tid; }
auto vertex_id(const my_graph&, const my_graph::vertex& u) { return u.id; }

// Optional: expose vertex value
auto vertex_value(const my_graph&, const my_graph::vertex& u) {
  return std::string_view(u.name);
}

} // namespace mylib
```

With these overloads in place, `my_graph` works directly with library views
(`vertices_breadth_first_search`, `edges_depth_first_search`, etc.) and algorithms
(`dijkstra_shortest_paths`, `bellman_ford_shortest_paths`, etc.).

See [Graph CPO Implementation](../graph_cpo_implementation.md) for the full CPO resolution
order and advanced customization patterns.

---

## 6. Container Selection Guide

```
              ┌─ Already have data in          → range-of-ranges
              │   standard containers?            (zero-copy, no conversion)
              │
              ├─ Have an existing graph         → custom graph
              │   data structure?                  (override CPOs for full control)
              │
              ├─ Need undirected edges          → undirected_adjacency_list
              │   with O(1) removal or
              │   mutable edge properties?
Start ────────┤
              │
              ├─ Graph is read-only             → compressed_graph
              │   after construction?              (smallest memory, best cache)
              │
              └─ Need mutable vertices/edges?   → dynamic_graph
                                                   (pick traits for your needs)
```

When `compressed_graph` or `dynamic_graph` is used to represent an undirected graph, 
edges must be duplicated (e.g. edges A/B and B/A for vertices A and B). Properties must also be
duplicated, or handled in a way that avoids duplication (e.g. a `std::shared_ptr` to a property 
struct), if they are mutable.

| Criterion | `dynamic_graph` | `compressed_graph` | `undirected_adjacency_list` | range-of-ranges |
|-----------|-----------------|--------------------|-----------------------------|------------------|
| Add/remove vertices | Yes | No | Yes | Depends on outer container |
| Add/remove edges | Yes | No | Yes (O(1) remove) | Depends on inner container |
| Directed | Yes | Yes | No (undirected) | Yes |
| Undirected | Yes (duplicated edges) | Yes (duplicated edges) | Yes | Yes (duplicated edges) |
| Mutable Properties | Directed (Yes), Undirected (No) | Directed (Yes), Undirected (No) | Yes | Directed (Yes), Undirected (No); your data |
| Memory efficiency | Medium | Best (CSR) | Highest overhead | Zero overhead (existing data) |
| Cache locality | Depends on trait | Excellent | Poor (linked-list) | Depends on containers used |
| Multi-partite | No | Yes | No | No |
| Container flexibility | 26 trait combos | Fixed (CSR) | Configurable random access vertex container | Any forward_range of forward_ranges |

**Custom graphs.** See [Section 5 (Custom Graphs)](#5-custom-graphs) for how to use your own
graph data structure with all library views and algorithms by overriding graph CPOs.

---

## See Also

- [Adjacency Lists User Guide](adjacency-lists.md) — concepts, CPOs, descriptors
- [Edge Lists User Guide](edge-lists.md) — edge list input for graph construction
- [Getting Started](../getting-started.md) — quick-start examples with all three containers
- [Adjacency List Interface](../reference/adjacency-list-interface.md) — formal adjacency list specification
- [Edge List Interface](../reference/edge-list-interface.md) — formal edge list specification
