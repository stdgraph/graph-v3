# BGL2 vs Graph-v3 Comparative Evaluation

**Date:** April 2025 (Updated with edge_descriptor simplification and vertices(g) iota_view changes)
**BGL2 Location:** `/home/phil/dev_graph/boost/libs/graph/modern/`
**Graph-v3 Location:** `/home/phil/dev_graph/graph-v3/`

---

## 1. Vertex and Edge Descriptor Differences

### BGL2 Approach

BGL2 uses a **selector-dependent descriptor** model where the vertex descriptor type is determined by the vertex list selector:

```cpp
// vecS: vertex_descriptor = std::size_t (index-based)
// listS: vertex_descriptor = std::list<vertex_data>::iterator
// setS: vertex_descriptor = std::set<vertex_wrapper>::iterator
```

**Edge descriptor** is a lightweight struct carrying source, target, and a global edge index:

```cpp
template<typename VertexDescriptor>
struct adjacency_list_edge {
    VertexDescriptor source;
    VertexDescriptor target;
    std::size_t edge_index;  // Global index into edges_ vector
    
    bool operator==(const adjacency_list_edge&) const = default;
    auto operator<=>(const adjacency_list_edge&) const = default;
};
```

**Additional edge descriptor variants** exist for other graph containers:
- `matrix_edge<VD>` — source + target only (adjacency matrix)
- `csr_edge_descriptor` — source_vertex + edge_index + target_vertex (CSR graph)
- `grid_edge<Dims>` — source/target coordinates + dimension + direction (grid graph)

**Optional strong typing** via `descriptor<Tag, IndexType>` wrapper (`strong_descriptor.hpp`):
```cpp
template<typename Tag, typename IndexType = std::size_t>
class descriptor {
    index_type index_ = null_value;
public:
    static constexpr index_type null_value = static_cast<index_type>(-1);
    [[nodiscard]] constexpr bool valid() const noexcept { return index_ != null_value; }
    [[nodiscard]] constexpr bool null() const noexcept { return index_ == null_value; }
};
using strong_vertex_descriptor = descriptor<vertex_tag>;
using strong_edge_index = descriptor<edge_tag>;
```
This prevents mixing vertex and edge descriptors at compile time.

**Key Characteristics:**
- Selector-based: `vecS` → `size_t`, `listS`/`setS` → iterator
- 10 container selectors available for out-edge lists; 3 for vertex lists (`vecS`, `listS`, `setS`)
- Null vertex convention: `static_cast<vertex_descriptor>(-1)` for vecS
- Parallel edge control via `parallel_edge_category_for<OutEdgeListS>` (based on selector's `is_unique` trait)
- Extracted via `graph_traits<G>::vertex_descriptor`

### Graph-v3 Approach

Graph-v3 uses a **template-based descriptor** model with conditional storage for vertices:

```cpp
template <vertex_iterator VertexIter>
class vertex_descriptor {
    using storage_type = std::conditional_t<
        std::random_access_iterator<VertexIter>,
        std::size_t,    // Index for vector/deque (8 bytes)
        VertexIter>;    // Iterator for map/list
    storage_type storage_;
public:
    auto vertex_id() const;              // Returns index or key
    auto underlying_value(Container&) const;  // Get vertex data
    auto inner_value(Container&) const;       // Get value (map: .second, vector: whole)
};
```

For **random-access containers** (vector, deque), `vertices(g)` returns an `iota_view<size_t, size_t>` wrapped in a `vertex_descriptor_view`, so vertex descriptors hold a lightweight `size_t` index.

**Edge descriptors** always store the edge iterator directly — no conditional storage:

```cpp
template <edge_iterator EdgeIter, vertex_iterator VertexIter, 
          class EdgeDirection = out_edge_tag>
class edge_descriptor {
    using edge_storage_type = EdgeIter;  // Always iterator, never conditional
    EdgeIter edge_storage_;
    vertex_descriptor<VertexIter> source_;
public:
    auto source_id() const;                    // Owning vertex ID
    auto target_id(VertexData&) const;         // Extracted from edge data
    auto inner_value(VertexData&) const;       // Edge property value
};
```

**Direction tags** (`out_edge_tag`, `in_edge_tag`) determine source/target semantics for bidirectional graphs.

**Key Characteristics:**
- Template-based: distinct type per container combination
- Vertex descriptor: `size_t` for random-access, iterator for forward-only containers
- Edge descriptor: always stores edge iterator + source vertex descriptor
- Flexible ID types: supports non-integral vertex IDs (strings, custom keys) via map-based containers
- Synthesized on iteration: `vertex_descriptor_view` / `edge_descriptor_view` create descriptors lazily
- No explicit null vertex convention (no null sentinel)

### Comparative Analysis

| Aspect | BGL2 | Graph-v3 |
|--------|------|----------|
| **Vertex descriptor type** | `size_t` (vecS) or iterator (listS/setS) | `vertex_descriptor<Iter>` with conditional storage |
| **Edge descriptor type** | `adjacency_list_edge<VD>` struct | `edge_descriptor<EdgeIter, VertexIter>` template |
| **Vertex descriptor size** | 8 bytes (vecS) or iterator-sized | 8 bytes (random-access) or iterator-sized |
| **Edge descriptor fields** | source + target + edge_index | edge_iterator + source vertex_descriptor |
| **Edge size (vector-based)** | 24 bytes (source + target + index) | ~16 bytes (edge iter + source index) |
| **Non-integral vertex IDs** | Not directly supported | Native (map/unordered_map vertices) |
| **Compile-time safety** | Optional `descriptor<Tag>` wrapper | Inherent (distinct template instantiations) |
| **Null descriptor** | `static_cast<VD>(-1)` for vecS | No built-in convention |
| **Parallel edge control** | `is_unique` trait on selector | Container choice (set vs vector) |
| **Property access** | `g[v].prop` or property map lambda | CPO `vertex_value(g, u)` / `edge_value(g, uv)` |

---

## 2. Ability to Adapt Pre-existing Graph Data Structures

### BGL2 Adaptation Model

BGL2 uses **`graph_traits` specialization + ADL free functions**:

```cpp
// Step 1: Specialize graph_traits
template<>
struct graph_traits<MyGraph> {
    using vertex_descriptor = my_vertex_handle;
    using edge_descriptor = my_edge_handle;
    using directed_category = directed_tag;
    // ... other types
};

// Step 2: Provide ADL free functions
auto vertices(const MyGraph& g) { return g.get_vertices(); }
auto num_vertices(const MyGraph& g) { return g.vertex_count(); }
auto out_edges(vertex_descriptor v, const MyGraph& g) { return g.outgoing(v); }
auto source(edge_descriptor e, const MyGraph& g) { return g.get_source(e); }
auto target(edge_descriptor e, const MyGraph& g) { return g.get_target(e); }
```

**Strengths:**
- Well-established pattern familiar to Boost.Graph users
- Complete control over all type definitions via traits
- Works with any descriptor type
- Explicit about what a graph type provides

**Limitations:**
- Requires `graph_traits<G>` specialization (potentially invasive)
- All required functions must be provided upfront
- No fallback behavior if a function isn't provided
- Standard containers need an adapter wrapper

### Graph-v3 Adaptation Model

Graph-v3 uses **CPOs with a member → ADL → default resolution chain**:

```cpp
// CPO resolution order for vertices(g):
// 1. g.vertices()              — Member function (preferred)
// 2. vertices(g)               — ADL function
// 3. vertex_descriptor_view(g) — Default (container auto-detection)

// CPO resolution order for target_id(g, uv):
// 1. (*uv.value()).target_id()    — Edge data method
// 2. ADL target_id(g, uv)        — ADL function
// 3. uv.target_id()              — Descriptor method
// 4. Tuple get<1>                 — Structured binding fallback
```

**Three levels of adaptation:**

1. **Zero configuration** — Standard containers work directly:
   ```cpp
   std::vector<std::vector<int>> g = {{1,2}, {2,3}, {3}};
   for (auto u : graph::vertices(g))
       for (auto e : graph::edges(g, u))
           auto tid = graph::target_id(g, e);  // Just works
   ```

2. **Concept satisfaction** — Any type satisfying `adjacency_list` concept:
   ```cpp
   template<typename G>
   concept adjacency_list = 
       vertex_range<G> &&
       requires(G& g, vertex_t<G> u) {
           { edges(g, u) } -> targeted_edge_range<G>;
       };
   ```

3. **Member or friend functions** — Custom graphs provide methods:
   ```cpp
   class MyGraph {
       friend auto vertices(MyGraph& g) { return g.internal_vertices(); }
   };
   ```

**Strengths:**
- Non-invasive: no traits specialization required
- Progressive: only provide what you need; defaults handle the rest
- Standard containers work automatically via pattern detection
- Member functions respected (encapsulation preserved)

**Limitations:**
- Default behavior requires containers to satisfy `inner_value_pattern` concepts
- Less explicit about what a graph type must provide (determined at instantiation)
- CPO resolution chain adds library implementation complexity

### Comparative Analysis

| Aspect | BGL2 | Graph-v3 |
|--------|------|----------|
| **Primary mechanism** | `graph_traits<G>` specialization | CPO with fallback chain |
| **Function customization** | ADL only | Member → ADL → default |
| **Invasiveness** | Must specialize traits template | Non-invasive |
| **Standard containers** | Require adapter wrapper | Automatic detection |
| **Compile-time checking** | Concepts on traits | CPO `requires` clauses |
| **Discoverability** | Explicit traits documentation | Concept-based detection |
| **Legacy compatibility** | High (BGL1 pattern) | New pattern (C++20) |

---

## 3. Strengths and Weaknesses

### BGL2 Strengths

1. **Complete algorithm library with rich result types.** BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal, topological sort, connected components, and strongly connected components are all implemented. Algorithm results are first-class objects with query methods:
   ```cpp
   auto result = dijkstra_shortest_paths(g, source, weight);
   if (result.is_reachable(target))
       auto path = result.path_to(target);  // Returns vector<vertex_descriptor>
   ```

2. **Composable, parallel, and coroutine algorithms.** Beyond traditional algorithms:
   - **Composable:** `component_view`, `find_distances_from()`, `find_k_core()`, `find_neighbors()`, `degree_distribution()` — ranges-based composition
   - **Parallel:** `parallel_bfs()` (level-synchronous), `parallel_connected_components()` (Shiloach-Vishkin) via `std::execution` policies
   - **Coroutine:** `bfs_traverse()`, `dfs_traverse()` as lazy generators with early termination support

3. **Named parameters via C++20 designated initializers.** Algorithms accept parameter structs with sensible defaults:
   ```cpp
   auto result = dijkstra_shortest_paths(g, source, {
       .weight_map = [&g](auto e) { return g[e].weight; },
       .distance_compare = std::less<int>{},
       .distance_infinity = std::numeric_limits<int>::max()
   });
   ```

4. **Graph validation framework.** `validate_graph()` checks structural invariants:
   - Dangling edges (edges to non-existent vertices)
   - Self-loops and parallel edges (where disallowed)
   - Bidirectional consistency (in-edges match out-edges)
   - Vertex/edge count mismatches

5. **MutableGraph concept.** Individual `add_vertex()`, `remove_vertex()`, `add_edge()`, `remove_edge()` operations with well-defined invalidation semantics per selector.

6. **Simple lambda-based property maps.** Any callable satisfying `std::invocable<F, Key>` works as a property map — no special types needed:
   ```cpp
   auto weight = [&g](edge_descriptor e) { return g[e].weight; };
   ```

7. **Automatic parallel edge detection.** `parallel_edge_category_for<OutEdgeListS>` determines `allow_parallel_edge_tag` vs `disallow_parallel_edge_tag` based on the selector's `is_unique` trait.

8. **Type-safe strong descriptors.** Optional `descriptor<Tag>` wrapper prevents accidental vertex/edge descriptor mix-ups at compile time, with built-in null semantics.

### BGL2 Weaknesses

1. **No direct non-integral vertex ID support.** Vertex descriptors are `size_t` (vecS) or iterators (listS/setS). Non-integral vertex IDs (strings, UUIDs, custom types) require an external mapping layer.

2. **Traits boilerplate for adaptation.** External graph types require explicit `graph_traits<G>` specialization — more invasive than concept-based adaptation.

3. **Limited vertex list selectors.** Only 3 vertex list implementations (`vecS`, `listS`, `setS`) despite 10 out-edge selectors. Hash-based vertex storage is not available.

4. **No zero-config container support.** Standard containers like `vector<vector<int>>` cannot be used directly — they must be wrapped in `adjacency_list`.

5. **No CSR in-place construction from edge ranges.** The `compressed_sparse_row_graph` requires explicit construction rather than loading from arbitrary edge ranges.

### Graph-v3 Strengths

1. **Multi-layered container flexibility.** Three levels of graph support:
   - **Zero-config:** `vector<vector<int>>` works directly as an adjacency list via CPOs
   - **Concept-based:** Any type satisfying `adjacency_list` concept works automatically
   - **Trait-based:** 27+ trait combinations for `dynamic_graph` (vov, vofl, mol, etc.)
   - **CSR:** `compressed_graph` for high-performance static graphs

2. **Non-invasive adaptation.** CPO resolution with member → ADL → default fallbacks means:
   - Standard containers work without wrappers
   - Custom graphs just need to satisfy concepts
   - No traits specialization required

3. **Native non-integral vertex IDs.** Map-based containers (mol, mofl, uofl, etc.) provide string keys, custom types, or sparse integer IDs as first-class vertex identifiers.

4. **Rich traversal views.** 13 view headers providing lazy range adaptors:
   - `vertices_dfs()`, `edges_dfs()` — DFS traversal as input_range
   - `vertices_bfs()`, `edges_bfs()` — BFS traversal as input_range
   - `vertexlist()`, `incidence()`, `neighbors()`, `edgelist()` — structural views
   - `transpose()` — reversed-direction adaptor
   - Pipe syntax support: `g | vertexlist() | ...`

5. **Comprehensive algorithm set.** 14 algorithms including several not in BGL2:
   - Triangle counting (directed + undirected)
   - Articulation points (iterative Hopcroft-Tarjan)
   - Biconnected components (Hopcroft-Tarjan with edge stack)
   - Jaccard similarity (per-edge coefficient)
   - Label propagation (community detection)
   - Maximum independent set (greedy)

6. **Adaptive property maps.** `vertex_property_map<G, T>` automatically selects `vector<T>` for index-based graphs or `unordered_map<VId, T>` for mapped graphs.

7. **Bidirectional graph support.** `undirected_adjacency_list` with dual-list design provides O(1) edge removal from both endpoints. `dynamic_graph` supports `Bidirectional=true` template parameter.

8. **Edge list module.** Standalone `edge_list` support for `pair<T,T>`, `tuple<T,T,EV,...>`, and `edge_data` structs with dedicated CPOs and concepts.

### Graph-v3 Weaknesses

1. **Batch-only mutation.** `dynamic_graph` provides `load_edges()` and `load_vertices()` for bulk loading but no individual `add_vertex()` / `add_edge()` / `remove_vertex()` / `remove_edge()`. This limits interactive graph construction.

2. **No composable, parallel, or coroutine algorithms.** Graph-v3 has traditional algorithms but lacks the higher-level paradigms (ranges composition, `std::execution` policies, lazy generators).

3. **No named parameter pattern.** Algorithms use positional parameters with defaults rather than C++20 designated initializer structs.

4. **No algorithm result types.** Algorithms write to output parameters (distance maps, predecessor maps) rather than returning rich result objects with query methods like `path_to()` or `is_reachable()`.

5. **No graph validation framework.** No equivalent to BGL2's `validate_graph()` for checking structural invariants.

6. **No strong descriptor typing.** Vertex and edge descriptors are distinct template instantiations (providing some safety), but there is no explicit `descriptor<Tag>` wrapper with null semantics.

7. **Iterator stability concerns.** Descriptors holding iterators for non-random-access containers may be invalidated by container modifications.

8. **Learning curve.** CPO resolution chains and automatic pattern detection are powerful but less familiar than traditional traits-based approaches.

---

## 4. Container Flexibility: Selector-Based vs Trait-Based

### BGL2 Selector-Based Approach

BGL2 uses **container selector tags** that map to standard containers via `container_gen<S, T>`:

```cpp
// Each selector carries compile-time traits
struct vecS {
    static constexpr bool is_random_access = true;
    static constexpr bool has_stable_iterators = false;
    static constexpr bool is_ordered = false;
    static constexpr bool is_unique = false;
};

// container_gen maps selectors to containers
template<typename ValueType>
struct container_gen<vecS, ValueType> {
    using type = std::vector<ValueType>;
};

// Usage: adjacency_list<OutEdgeListS, VertexListS, DirectedS, VP, EP>
adjacency_list<setS, listS, directed_tag, VertexProp, EdgeProp> g;
```

**All 10 Selectors with Traits:**

| Selector | Container | random_access | stable_iters | ordered | unique |
|----------|-----------|:---:|:---:|:---:|:---:|
| `vecS` | `vector` | ✓ | ✗ | ✗ | ✗ |
| `listS` | `list` | ✗ | ✓ | ✗ | ✗ |
| `setS` | `set` | ✗ | ✓ | ✓ | ✓ |
| `mapS` | `set` | ✗ | ✓ | ✓ | ✓ |
| `multisetS` | `multiset` | ✗ | ✓ | ✓ | ✗ |
| `multimapS` | `multiset` | ✗ | ✓ | ✓ | ✗ |
| `hash_setS` | `unordered_set` | ✗ | ✗ | ✗ | ✓ |
| `hash_mapS` | `unordered_set` | ✗ | ✗ | ✗ | ✓ |
| `hash_multisetS` | `unordered_multiset` | ✗ | ✗ | ✗ | ✗ |
| `hash_multimapS` | `unordered_multiset` | ✗ | ✗ | ✗ | ✗ |

> **Note:** Despite "map" in their names, `mapS`/`multimapS`/`hash_mapS`/`hash_multimapS` all map to **set** containers. The "map" name reflects the vertex→edge *mapping semantic*, not the container type.

**Selector concepts** enforce constraints:
- `ContainerSelector` — union of all 10 types
- `SequenceSelector` — vecS, listS
- `AssociativeSelector` — set/map variants
- `UnorderedSelector` — hash_* variants
- `StableIteratorSelector` — selectors where `has_stable_iterators == true`
- `RandomAccessSelector` — selectors where `is_random_access == true`

**Vertex list implementations:** Only `vecS`, `listS`, and `setS` are supported for vertex storage. All 10 selectors work for out-edge lists.

**Additional graph containers:**
- `adjacency_matrix` — O(V²) space, O(1) edge lookup, fixed vertex count
- `compressed_sparse_row_graph` — CSR format, static, cache-efficient
- `grid_graph<Dims, Wrapped>` — N-dimensional implicit grid, O(1) space, optional torus wrapping

### Graph-v3 Multi-Layered Approach

Graph-v3 provides **four levels** of graph support with increasing configuration:

**Level 1: Standard Containers as Graphs (Zero Configuration)**

Any combination of random-access vertex container + forward-range edge container works directly:

```cpp
// Simple adjacency list
std::vector<std::vector<int>> g = {{1,2}, {2,3}, {3}};

// Weighted adjacency list
std::vector<std::vector<std::pair<int, double>>> weighted = {
    {{1, 1.5}, {2, 2.0}}, {{2, 0.5}}, {}
};

// CPOs work automatically on both
for (auto u : graph::vertices(g))
    for (auto e : graph::edges(g, u))
        auto tid = graph::target_id(g, e);
```

Supported patterns include `vector<vector<VId>>`, `vector<vector<pair<VId, W>>>`, `vector<forward_list<VId>>`, `deque<vector<VId>>`, and any similar combination.

**Level 2: Concept-Based Adaptation**

Any type satisfying the `adjacency_list` concept works:
```cpp
template<typename G>
concept adjacency_list = 
    vertex_range<G> &&
    requires(G& g, vertex_t<G> u) {
        { edges(g, u) } -> targeted_edge_range<G>;
    };
```

**Level 3: `dynamic_graph` with Trait Structs**

For full-featured mutable graphs with properties:
```cpp
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_graph;

// Traits define vertex and edge container types
using G = dynamic_graph<double, string, void, uint32_t, false, 
                        vofl_graph_traits<double, string, void, uint32_t, false>>;
```

**27+ trait combinations:**

| Vertices | Edge Containers | Trait Names |
|----------|----------------|-------------|
| `vector` | vector, list, forward_list, deque, set, unordered_set, map | vov, vol, vofl, vod, vos, vous, vom |
| `deque` | vector, list, forward_list, deque, set, unordered_set | dov, dol, dofl, dod, dos, dous |
| `map` | vector, list, forward_list, deque, set, unordered_set, map | mov, mol, mofl, mod, mos, mous, mom |
| `unordered_map` | vector, list, forward_list, deque, unordered_set | uov, uol, uofl, uod, uous |

Separate undirected-specific traits also exist (uios, uous, uov, uod, uol, uofl).

**Level 4: `compressed_graph` (CSR)**

High-performance static graph:
```cpp
template <class EV, class VV, class GV, integral VId, integral EIndex, class Alloc>
class compressed_graph;
// row_index_ + col_index_ + optional values — optimal for read-heavy workloads
```

**Level 5: `undirected_adjacency_list`**

Dual-list design with O(1) edge removal from both endpoints:
```cpp
template <typename EV, typename VV, typename GV, integral VId,
          template<typename V, typename A> class VContainer, typename Alloc>
class undirected_adjacency_list;
```

### Comparative Analysis

| Aspect | BGL2 Selectors | Graph-v3 |
|--------|---------------|----------|
| **Configuration style** | Tag-based template params | Multi-layered (zero-config to traits) |
| **Standard containers** | Requires `adjacency_list` wrapper | Direct use via concepts |
| **Edge container options** | 10 selectors | 7+ edge containers per vertex type |
| **Vertex container options** | 3 (vecS, listS, setS) | 4 (vector, deque, map, unordered_map) |
| **Total built-in combos** | 10 × 3 = 30 (edge × vertex) | 27+ traits + unlimited zero-config |
| **Extensibility** | Specialize `container_gen<S, T>` | Satisfy concept or create trait |
| **CSR/static graphs** | `compressed_sparse_row_graph` | `compressed_graph` |
| **Dense graphs** | `adjacency_matrix` | Not available |
| **Implicit graphs** | `grid_graph<Dims>` | Not available |
| **Non-integral vertex IDs** | Not supported | Native (map/unordered_map vertices) |
| **Undirected O(1) removal** | Via bidirectional + listS | `undirected_adjacency_list` |

---

## 5. Other Design and Flexibility Considerations

### Customization Point Objects vs ADL

**Graph-v3** uses qualified CPO calls that prevent ADL hijacking:
```cpp
auto verts = graph::vertices(my_graph);  // Qualified — CPO dispatches correctly
```

**BGL2** uses unqualified ADL lookup:
```cpp
auto verts = vertices(my_graph);  // ADL finds correct overload in graph's namespace
```

CPOs provide stronger encapsulation and prevent name collisions, but add complexity to the library implementation. BGL2's ADL pattern is simpler and more familiar.

### Concept Hierarchies

**BGL2 concepts** follow a traditional graph theory hierarchy:

| Concept | Requires |
|---------|----------|
| `Graph` | `vertex_descriptor`, `edge_descriptor`, `directed_category` |
| `IncidenceGraph` | Graph + `out_edges()`, `out_degree()`, `source()`, `target()` |
| `BidirectionalGraph` | IncidenceGraph + `in_edges()`, `in_degree()` |
| `VertexListGraph` | Graph + `vertices()`, `num_vertices()` |
| `EdgeListGraph` | Graph + `edges()`, `num_edges()` |
| `AdjacencyGraph` | Graph + `adjacent_vertices()` |
| `MutableGraph` | Graph + `add_vertex()`, `remove_vertex()`, `add_edge()`, `remove_edge()` |

**Graph-v3 concepts** are organized around container patterns:

| Concept | Requires |
|---------|----------|
| `vertex_range<G>` | Forward, sized range with `vertex_id` |
| `index_vertex_range<G>` | Integral `vertex_id_t`, random-access container |
| `mapped_vertex_range<G>` | Hashable `vertex_id_t`, `find_vertex()` |
| `adjacency_list<G>` | `vertex_range` + `out_edges()` → `targeted_edge_range` |
| `index_adjacency_list<G>` | `adjacency_list` + `index_vertex_range` |
| `mapped_adjacency_list<G>` | `adjacency_list` + `mapped_vertex_range` |
| `bidirectional_adjacency_list<G>` | `adjacency_list` + `in_edges()` |
| `ordered_vertex_edges<G>` | Adjacency lists sorted by target_id |

Key difference: Graph-v3 distinguishes index-based vs mapped vertex ranges at the concept level, enabling algorithms to select optimal data structures (e.g., vector vs unordered_map for property maps).

### Property Access

**BGL2:** Lambda-based property maps — any `std::invocable<F, Key>` works:
```cpp
auto weight = [&g](edge_descriptor e) { return g[e].weight; };
auto color = [&g](vertex_descriptor v) -> Color& { return g[v].color; };
dijkstra_shortest_paths(g, source, weight);
```

**Graph-v3:** CPO-based access with adaptive property maps:
```cpp
// CPO access (requires graph reference)
graph::vertex_value(g, u) = new_value;
graph::edge_value(g, uv);

// Adaptive property map (auto-selects vector or unordered_map)
auto dist = make_vertex_property_map(g, std::numeric_limits<double>::max());
// Uses vector<double> for index graphs, unordered_map<VId, double> for mapped graphs
```

Graph-v3's `vertex_property_map<G, T>` is adaptive but requires the graph type at creation. BGL2's lambda approach is simpler and more flexible.

### Direction and Edge Categories

**BGL2:** Tag types extracted via `graph_traits`:
- `directed_tag`, `undirected_tag`, `bidirectional_tag` (inherits from `directed_tag`)
- `allow_parallel_edge_tag` / `disallow_parallel_edge_tag` (derived from selector's `is_unique`)

**Graph-v3:** Template parameters and direction tags:
- `bool Bidirectional` template parameter on `dynamic_graph`
- `out_edge_tag` / `in_edge_tag` on edge descriptors determine source/target semantics
- No explicit parallel edge category — controlled by container choice (set vs vector)

### Visitor/Callback Patterns

**BGL2 callbacks** use C++20 designated initializers with 9 event points:
```cpp
auto result = breadth_first_search(g, source, {
    .on_discover_vertex = [](auto v) { ... },
    .on_tree_edge = [](auto e, const auto& g) { ... },
    .on_finish_vertex = [](auto v) { ... }
});
// Unspecified callbacks default to null_callback{} (zero overhead)
```

**Graph-v3 visitors** use concept-checked optional callbacks:
```cpp
// Algorithms check for visitor methods via has_on_* concepts
void dijkstra_shortest_paths(G&& g, Sources& sources, 
    DistanceFn&&, PredecessorFn&&, WF&&, Visitor&& visitor = {});
// Visitor events: on_initialize_vertex, on_discover_vertex, on_examine_vertex,
//   on_examine_edge, on_edge_relaxed, on_finish_vertex
```

Both approaches have zero overhead for unused callbacks. BGL2's designated initializer pattern is more ergonomic; Graph-v3's concept-checked approach is more extensible.

---

## 6. Algorithm Comparison

### Algorithm Inventory

| Algorithm | BGL2 | Graph-v3 | Notes |
|-----------|:----:|:--------:|-------|
| **BFS** | ✅ | ✅ | Both support visitors/callbacks |
| **DFS** | ✅ | ✅ | Both with edge classification (tree/back/forward/cross) |
| **Dijkstra shortest paths** | ✅ | ✅ | Graph-v3 supports multi-source; BGL2 has `dijkstra_result` |
| **Bellman-Ford shortest paths** | ✅ | ✅ | Both detect negative cycles |
| **Topological sort** | ✅ | ✅ | Graph-v3: full-graph, single-source, multi-source variants |
| **Connected components** | ✅ | ✅ | Graph-v3: DFS-based + parallel afforest |
| **Strongly connected components** | ✅ (Tarjan) | ✅ (Kosaraju) | Different algorithms; Graph-v3 has transpose + bidirectional overloads |
| **MST — Kruskal** | ✅ | ✅ | Graph-v3 includes in-place Kruskal variant |
| **MST — Prim** | ✅ | ✅ | Both priority-queue based |
| **Triangle counting** | — | ✅ | Undirected + directed variants |
| **Articulation points** | — | ✅ | Iterative Hopcroft-Tarjan |
| **Biconnected components** | — | ✅ | Hopcroft-Tarjan with edge stack |
| **Jaccard similarity** | — | ✅ | Per-edge coefficient |
| **Label propagation** | — | ✅ | Community detection |
| **Maximum independent set** | — | ✅ | Greedy algorithm |
| **Composable algorithms** | ✅ | — | k-core, degree distribution, component view, find_neighbors |
| **Parallel algorithms** | ✅ | — | parallel_bfs, parallel_connected_components (std::execution) |
| **Coroutine traversal** | ✅ | — | Lazy BFS/DFS generators with early termination |

**Count:** BGL2 has 9 traditional + 3 paradigm categories. Graph-v3 has 14 traditional algorithms.

### Algorithm Design Differences

**Result types vs output parameters:**
- BGL2 returns rich result objects (`dijkstra_result`, `bfs_result`, `dfs_result`, etc.) with query methods: `path_to()`, `is_reachable()`, `distance_to()`, `has_cycle()`
- Graph-v3 writes to caller-provided output functions/maps (distance function, predecessor function, component function)

**Named parameters vs positional:**
- BGL2 uses designated initializer structs with `use_default_t` sentinels:
  ```cpp
  dijkstra_shortest_paths(g, source, {.weight_map = w, .distance_compare = cmp});
  ```
- Graph-v3 uses positional parameters with template defaults:
  ```cpp
  dijkstra_shortest_paths(g, sources, distance_fn, predecessor_fn, weight_fn, visitor, compare, combine);
  ```

**Multi-source support:**
- Graph-v3 algorithms (Dijkstra, BFS, DFS, topological sort) accept a `Sources&` range for multi-source variants
- BGL2 coroutine traversals support multi-source; traditional algorithms are single-source

**Traversal views (Graph-v3 only):**
Graph-v3 provides traversal as lazy ranges via views, complementing the algorithm functions:
```cpp
for (auto&& [uid, vid, uv] : graph::views::edges_dfs(g, source))
    process(uid, vid, uv);

for (auto&& [uid] : graph::views::vertices_bfs(g, source))
    process(uid);

// Topological sort as view
for (auto&& [uid] : graph::views::vertices_topological_sort(g))
    process(uid);
```

**Coroutine traversal (BGL2 only):**
BGL2 provides lazy generators for incremental traversal with early termination:
```cpp
for (auto v : bfs_traverse(graph, start)) {
    if (v == target) break;  // Early exit
}
```

Both approaches enable lazy, on-demand traversal but with different mechanisms.

---

## 7. Summary and Recommendations

### When to Choose BGL2

- You need **working algorithms with rich result types** — `path_to()`, `is_reachable()`, `distance_to()`, `has_cycle()` out of the box
- You need **parallel algorithms** (`std::execution` policies) or **coroutine-based traversal** (lazy generators)
- You want **composable algorithms** (k-core decomposition, degree distribution, component views)
- Your graphs use **integral vertex descriptors** and you want **MutableGraph** operations (`add_vertex`, `add_edge`, `remove_vertex`, `remove_edge`)
- You want **graph validation** to check structural invariants programmatically
- You need **dense graph support** (`adjacency_matrix`) or **implicit grid graphs** (`grid_graph`)
- You're migrating from **legacy Boost.Graph** code (familiar patterns, compatible traits)
- You want **named parameters** via C++20 designated initializers

### When to Choose Graph-v3

- You want to use **standard containers directly** (`vector<vector<int>>`) without wrappers or boilerplate
- You need **non-integral vertex IDs** (strings, UUIDs, custom types) as first-class citizens
- You want to **adapt existing graph structures** by satisfying concepts rather than specializing traits
- You need **maximum container flexibility** (27+ trait combinations + unlimited zero-config containers)
- You need algorithms not in BGL2: **triangle counting**, **articulation points**, **biconnected components**, **Jaccard similarity**, **label propagation**, **maximum independent set**
- You want **traversal views** as lazy ranges with pipe syntax
- You need **multi-source** Dijkstra, BFS, or topological sort
- You want **adaptive property maps** that auto-select vector vs unordered_map based on graph type
- You need a **CSR graph** loadable from arbitrary edge ranges

### Potential Synergies

Both libraries share C++20 foundations and could potentially:

1. **Share concept definitions** — both have similar vertex list, incidence, and bidirectional graph concepts
2. **Cross-library algorithms** — BGL2 algorithms could operate on Graph-v3 containers via a thin adapter satisfying `graph_traits`
3. **Adopt BGL2 result types** — Graph-v3 could benefit from `dijkstra_result`-style return values with query methods
4. **Adopt Graph-v3 CPOs** — BGL2 could adopt CPO dispatch for stronger encapsulation
5. **Share validation** — BGL2's `validate_graph()` framework could be adapted for Graph-v3 containers

### Quick Reference: Feature Matrix

| Feature | BGL2 | Graph-v3 |
|---------|:----:|:--------:|
| Zero-config standard containers | ✗ | ✓ |
| Non-integral vertex IDs | ✗ | ✓ |
| Trait/selector container config | ✓ (10 selectors) | ✓ (27+ traits) |
| MutableGraph (individual ops) | ✓ | ✗ (batch only) |
| Batch edge loading | ✗ | ✓ |
| Adjacency matrix | ✓ | ✗ |
| Implicit grid graph | ✓ | ✗ |
| CSR graph | ✓ | ✓ |
| Undirected O(1) removal | partial | ✓ |
| Strong typed descriptors | ✓ | ✗ |
| Graph validation | ✓ | ✗ |
| Algorithm result objects | ✓ | ✗ |
| Named algorithm parameters | ✓ | ✗ |
| Traversal views (lazy ranges) | ✗ | ✓ |
| Coroutine traversal | ✓ | ✗ |
| Parallel algorithms | ✓ | ✗ |
| Composable algorithms | ✓ | ✗ |
| Visitor/callback support | ✓ | ✓ |
| Multi-source algorithms | partial | ✓ |
| Adaptive property maps | ✗ | ✓ |
| CPO-based dispatch | ✗ | ✓ |
| Traditional algorithms | 9 | 14 |

---

## 8. Code Examples Side-by-Side

### Creating a Graph

**BGL2:**
```cpp
struct VertexData { std::string name; };
struct EdgeData { double weight; };

adjacency_list<setS, listS, directed_tag, VertexData, EdgeData> g;

auto v0 = add_vertex({"A"}, g);
auto v1 = add_vertex({"B"}, g);
auto [e, inserted] = add_edge(v0, v1, {1.5}, g);
```

**Graph-v3 (standard container):**
```cpp
std::vector<std::vector<std::pair<int, double>>> g = {
    {{1, 1.5}, {2, 2.0}},  // vertex 0: edges to 1 (w=1.5), 2 (w=2.0)
    {{2, 0.5}},              // vertex 1: edge to 2 (w=0.5)
    {}                       // vertex 2: no outgoing edges
};
```

**Graph-v3 (dynamic_graph):**
```cpp
using G = dynamic_graph<double, std::string, void, uint32_t, false,
                        vofl_graph_traits<double, std::string, void, uint32_t, false>>;
G g;
g.load_edges(edges_data, edge_projection);
g.load_vertices(vertex_data, vertex_projection);
```

### Traversing Vertices and Edges

**BGL2:**
```cpp
for (auto v : vertices(g)) {
    std::cout << g[v].name << ": ";
    for (auto e : out_edges(v, g))
        std::cout << target(e, g) << " ";
}
```

**Graph-v3:**
```cpp
for (auto u : graph::vertices(g)) {
    auto uid = graph::vertex_id(g, u);
    for (auto e : graph::edges(g, u))
        std::cout << graph::target_id(g, e) << " ";
}
```

### Running Dijkstra

**BGL2:**
```cpp
auto weight = [&g](auto e) { return g[e].weight; };
auto result = dijkstra_shortest_paths(g, source, {.weight_map = weight});

if (result.is_reachable(target)) {
    auto path = result.path_to(target);
    auto dist = result.distance_to(target);
}
```

**Graph-v3:**
```cpp
auto dist = make_vertex_property_map(g, std::numeric_limits<double>::max());
auto pred = make_vertex_property_map(g, graph::vertex_id(g, *begin(graph::vertices(g))));
auto weight = [](auto& e) { return graph::edge_value(g, e); };

std::vector sources = {source};
graph::dijkstra_shortest_paths(g, sources, dist, pred, weight);
// Query results directly from dist and pred maps
```

### BFS with Callbacks

**BGL2:**
```cpp
auto result = breadth_first_search(g, source, {
    .on_discover_vertex = [](auto v) { std::cout << v << " "; },
    .on_tree_edge = [](auto e, const auto& g) {
        std::cout << source(e, g) << "->" << target(e, g) << "\n";
    }
});
```

**Graph-v3 (view-based):**
```cpp
for (auto&& [uid, vid, uv] : graph::views::edges_bfs(g, source)) {
    std::cout << uid << "->" << vid << "\n";
}
```

**Graph-v3 (algorithm with visitor):**
```cpp
struct my_visitor {
    void on_discover_vertex(const G&, vertex_id_t<G> uid) {
        std::cout << uid << " ";
    }
};
graph::breadth_first_search(g, sources, my_visitor{});
```
