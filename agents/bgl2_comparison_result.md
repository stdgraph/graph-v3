# BGL2 vs Graph-v3 Comparative Evaluation

**Date:** December 2024 (Updated with BGL2 container selector improvements)  
**BGL2 Location:** `/home/phil/dev_graph/boost/libs/graph/modern/`  
**Graph-v3 Location:** `/home/phil/dev_graph/desc/`

---

## 1. Vertex and Edge Descriptor Differences

### BGL2 Approach

BGL2 uses a **traditional, type-centric** approach to descriptors:

```cpp
// Vertex descriptor varies by VertexListS selector:
// - vecS: std::size_t (index-based)
// - listS/setS: iterator into the container (iterator-based)
using vertex_descriptor = /* varies by selector */;

// Edge descriptor is a lightweight struct
struct adjacency_list_edge<vertex_descriptor> {
    vertex_descriptor source;
    vertex_descriptor target;
    std::size_t edge_index;  // for parallel edge distinction
};
```

**Key Characteristics:**
- **Selector-based vertex descriptors**: `std::size_t` for `vecS`, container iterator for `listS`/`setS`
- **10 container selectors fully implemented**:
  - Sequence: `vecS` (vector), `listS` (list)
  - Ordered associative: `setS`, `mapS`, `multisetS`, `multimapS`
  - Hash-based: `hash_setS`, `hash_mapS`, `hash_multisetS`, `hash_multimapS`
- **Vertex list implementations**: `vecS` (index-based), `listS` (stable iterators), `setS` (ordered, stable)
- **All 10 out-edge selectors**: Work for edge containers
- **Optional strong typing**: `descriptor<vertex_tag, IndexType>` wrapper prevents mixing vertex/edge descriptors at compile time
- **Null vertex convention**: `static_cast<vertex_descriptor>(-1)` for invalid/null vertices (vecS)
- **Extracted via traits**: `vertex_descriptor_t<G>` uses `graph_traits<G>::vertex_descriptor`
- **Parallel edge control**: `parallel_edge_category_for<OutEdgeListS>` auto-detects based on selector's `is_unique` trait

### Graph-v3 Approach

Graph-v3 uses a **conditionally-typed descriptor** pattern:

```cpp
// Vertex descriptor with conditional storage
template<typename VertexIter>
struct vertex_descriptor {
    // Stores size_t for random access containers, iterator otherwise
    using storage_type = std::conditional_t<
        std::random_access_iterator<VertexIter>,
        std::size_t,   // 8 bytes for vector/deque
        VertexIter     // Iterator for map/list/forward_list
    >;
    storage_type storage_;
    
    auto vertex_id() const;   // Returns index or extracts key from iterator
    auto inner_value(Container& c) const; // Access requires container
};
```

**Key Characteristics:**
- **Conditional storage**: Stores `size_t` index for random-access containers (vector, deque), iterator for others (map, list)
- **Lightweight for vectors**: Same 8-byte overhead as BGL2 when using vector-based storage
- **Pattern-aware**: Automatically detects storage patterns (random_access, pair_value, whole_value)
- **Synthesized on iteration**: `vertex_descriptor_view` creates descriptors on-the-fly during traversal
- **Flexible ID types**: Supports non-integral vertex IDs (strings, custom keys) for map-based containers
- **Container access needed**: `inner_value(container)` requires container reference to access vertex data

**Edge Descriptor:**
```cpp
// Edge descriptor with conditional storage + source vertex
template<typename EdgeIter, typename VertexIter>
struct edge_descriptor {
    // Stores size_t for random access edge containers, iterator otherwise
    using edge_storage_type = std::conditional_t<
        std::random_access_iterator<EdgeIter>,
        std::size_t,
        EdgeIter
    >;
    edge_storage_type edge_storage_;
    vertex_descriptor<VertexIter> source_;  // Source vertex descriptor
    
    auto source_id() const;                      // Delegates to source_.vertex_id()
    auto target_id(const VertexData&) const;     // Extracts from edge data
    auto edge_value(const VertexData&) const;    // Edge property access
};
```

**Edge Descriptor Characteristics:**
- **Carries source**: Unlike BGL2 which stores source in edge data, graph-v3 stores source vertex descriptor directly
- **Conditional edge storage**: Index for random-access edge containers (vector), iterator for others (forward_list, list)
- **Flexible target extraction**: Supports simple integral targets, pair-like edges, and custom edge types
- **Size**: 16 bytes minimum (8 for edge + 8 for source) for vector-based; larger for iterator-based

### Comparative Analysis

#### Vertex Descriptors

| Aspect | BGL2 | Graph-v3 |
|--------|------|----------|
| **Type** | Selector-dependent: `size_t` (vecS) or iterator (listS/setS) | `vertex_descriptor<Iter>` template |
| **Internal storage** | Index (8 bytes) for vecS; iterator for listS/setS | Index for random-access (8 bytes), iterator otherwise |
| **Container selectors** | 10 selectors (vecS, listS, setS, mapS, multisetS, multimapS, hash_setS, hash_mapS, hash_multisetS, hash_multimapS); 3 vertex list impls (vecS, listS, setS) | 25 trait combinations (vov, vofl, mofl, mol, dos, uous, etc.) |
| **Non-integral IDs** | Not directly supported (vertex descriptors are index or iterator) | Native support via map-based traits (mofl, mol, uofl, etc.) |
| **Property access** | Requires property map lambda | `inner_value(container)` or CPO `vertex_value(g, u)` |
| **Array indexing** | Direct (`container[descriptor]`) | Via `vertex_id()` for random-access containers |
| **Descriptor stability** | Stable (index-based) | Stable for random-access; iterator-dependent for others |
| **Compile-time safety** | Optional `strong_descriptor<Tag>` wrapper | Inherent (distinct template instantiations) |

#### Edge Descriptors

| Aspect | BGL2 | Graph-v3 |
|--------|------|----------|
| **Type** | `adjacency_list_edge<VD>` struct | `edge_descriptor<EdgeIter, VertexIter>` template |
| **Source storage** | In edge data (`source` field) | Embedded `vertex_descriptor` member |
| **Target storage** | In edge data (`target` field) | Extracted via `target_id(vertex_data)` |
| **Parallel edge support** | `edge_index` field distinguishes duplicates | Iterator position distinguishes duplicates |
| **Size (vector-based)** | 24 bytes (source + target + index) | 16 bytes (edge index + source vertex index) |
| **Size (iterator-based)** | Varies by iterator size (for listS/setS) | Varies by iterator size |
| **Property access** | `g[e].property` with property map | CPO `edge_value(g, uv)` or `inner_value()` |

---

## 2. Ability to Adapt to Pre-existing Graph Data Structures

### BGL2 Adaptation Model

BGL2 uses a **traits specialization + ADL** pattern:

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
auto edges(const MyGraph& g) { return g.get_edges(); }
auto source(const MyGraph& g, edge_descriptor e) { return g.get_source(e); }
auto target(const MyGraph& g, edge_descriptor e) { return g.get_target(e); }
auto out_edges(const MyGraph& g, vertex_descriptor v) { return g.outgoing(v); }
```

**Strengths:**
- One-time traits specialization exposes all required types
- ADL functions provide complete customization control
- Well-established pattern (compatible with legacy Boost.Graph adaptors)
- Works with any descriptor type the user chooses

**Limitations:**
- Must specialize `graph_traits` (potentially invasive to library code)
- All required functions must be provided upfront
- No fallback behavior if a function isn't provided

### Graph-v3 Adaptation Model

Graph-v3 uses a **CPO with member → ADL → default resolution**:

```cpp
// CPO resolution order for vertices(g):
// 1. g.vertices()              // Member function (preferred)
// 2. vertices(g)               // ADL function
// 3. vertex_descriptor_view(g) // Default (container auto-detection)

// CPO resolution order for vertex_id(g, u):
// 1. vertex_id(g, u)           // ADL function
// 2. u.vertex_id()             // Descriptor method (default)
```

**Adaptation Example:**
```cpp
// Option 1: Define a member function
class MyGraph {
    auto vertices() { return my_vertex_range(); }
};

// Option 2: Define ADL friend function  
class MyGraph {
    friend auto vertices(MyGraph& g) { return g.internal_vertices(); }
};

// Option 3: Works automatically if MyGraph is a container
// (vector, list, map, etc. with inner_value_pattern)
```

**Strengths:**
- Non-invasive: No traits specialization required
- Progressive: Only provide what you need, defaults handle the rest
- Container-friendly: Standard containers work automatically
- Member functions preferred (encapsulation respected)

**Limitations:**
- Default behavior requires container to satisfy `inner_value_pattern` concepts
- Descriptor view wrapping may add overhead for simple adaptations
- Less explicit about what a graph type provides (determined at instantiation)

### Comparative Analysis

| Aspect | BGL2 | Graph-v3 |
|--------|------|----------|
| **Primary mechanism** | `graph_traits<G>` specialization | CPO with fallback chain |
| **Function customization** | ADL only | Member → ADL → default |
| **Invasiveness** | Must specialize traits | Non-invasive |
| **Standard containers** | Requires adapter wrapper | Automatic detection |
| **Discoverability** | Explicit traits documentation | Concept-based detection |
| **Compile-time checking** | Via concepts on traits | Via CPO `requires` clauses |
| **Legacy compatibility** | High (BGL1 pattern) | New pattern (C++20) |

---

## 3. Strengths and Weaknesses

### BGL2 Strengths

1. **Complete algorithm library**: BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal, topological sort, connected components are implemented and tested.

2. **Simple property maps**: Lambda-based `PropertyMap<F, Key, Value>` concept using `std::invocable` - any callable works:
   ```cpp
   auto weight = [&g](edge_descriptor e) { return g[e].weight; };
   dijkstra_shortest_paths(g, source, distances, weight);
   ```

3. **Established patterns**: Familiar to Boost.Graph users, extensive documentation pattern.

4. **Algorithm composability**: Modern visitor callbacks with designated initializers:
   ```cpp
   bfs(g, source, {
       .on_discover_vertex = [](auto v, auto& g) { ... },
       .on_tree_edge = [](auto e, auto& g) { ... }
   });
   ```

5. **Lightweight descriptors**: 8-byte vertex descriptors for `vecS` enable cache-efficient traversal.

6. **Selector-based configuration**: Compile-time container selection via template parameters with concept-based constraints:
   ```cpp
   template<typename S>
   concept ContainerSelector = 
       std::same_as<S, vecS> || std::same_as<S, listS> || 
       std::same_as<S, setS> || /* ... 7 more selectors */;
   ```

7. **Automatic parallel edge detection**: `parallel_edge_category_for<OutEdgeListS>` determines `allow_parallel_edge_tag` vs `disallow_parallel_edge_tag` based on selector's `is_unique` trait.

### BGL2 Weaknesses

1. **No direct non-integral vertex ID support**: While `mapS` is defined for edge containers, vertex IDs remain index-based (for `vecS`) or iterator-based (for `listS`/`setS`). Non-integral vertex IDs (strings, custom types) would require:
   - A separate vertex-to-index mapping layer
   - External property maps for string/custom key lookup

2. **Traits boilerplate**: Adapting external graphs requires explicit `graph_traits` specialization.

3. **Single-partition assumption**: No built-in support for partitioned/distributed graphs.

4. **Vertex list selector limitation**: While 10 out-edge selectors are supported, only 3 vertex list selectors are implemented (`vecS`, `listS`, `setS`). Hash-based vertex storage is not yet available.

### Graph-v3 Strengths

1. **Multi-layered container flexibility**:
   - **Zero-config**: Standard containers like `vector<vector<int>>` work directly as graphs
   - **Concept-based adaptation**: Any structure with random-access vertices + forward-range edges works automatically
   - **Trait-based**: 25 explicit trait combinations for `dynamic_graph`
   - **CSR format**: `compressed_graph` for high-performance static graphs
   
   Example - standard container as graph:
   ```cpp
   std::vector<std::vector<int>> g = {{1,2}, {2,3}, {3}};
   for (auto u : graph::vertices(g)) {
       for (auto e : graph::edges(g, u)) {
           auto tid = graph::target_id(g, e);  // Just works!
       }
   }
   ```

2. **Non-invasive adaptation**: CPO resolution with fallbacks means:
   - Standard containers work without any wrapper
   - Custom graphs just need to satisfy `adjacency_list` concept
   - No traits specialization required

3. **Rich descriptor pattern detection**: Automatic handling of:
   - Random-access patterns (vector, deque) → index-based IDs
   - Pair-value patterns (map) → key-based IDs  
   - Whole-value patterns (set) → whole-element IDs

4. **Efficient descriptors for random-access**: When using vector-based storage, `vertex_descriptor` stores only a `size_t` (same as BGL2):
   ```cpp
   for (auto v : graph::vertices(g)) {
       auto id = v.vertex_id();              // Returns index (no graph needed)
       auto& val = v.inner_value(vertices);  // Requires container reference
   }
   ```

5. **Partition support**: CPOs support `vertices(g, partition_id)` for multi-partition graphs.

6. **Non-integral vertex ID support**: Native support for string keys, custom types, etc.

7. **Compressed Sparse Row support**: `compressed_graph` provides optimal memory layout for static graphs.

### Graph-v3 Weaknesses

1. **No algorithms implemented**: This is a known limitation. All traversal, shortest-path, spanning tree, and flow algorithms are missing.

2. **Iterator-based descriptors for non-random-access containers**: Map/list storage requires storing iterators (larger than 8 bytes).

3. **Iterator stability concerns**: Descriptors holding iterators may be invalidated by container modifications.

4. **Learning curve**: CPO resolution chain and pattern detection are less familiar than traditional traits.

5. **Property map story unclear**: No explicit property map abstraction; properties accessed via descriptors or container access.

---

## 4. Container Flexibility: Selector-Based vs Trait-Based

### BGL2 Selector-Based Approach

BGL2 uses **container selector tags** that map to standard containers:

```cpp
// Selector tags with compile-time properties
struct vecS {
    static constexpr bool is_random_access = true;
    static constexpr bool has_stable_iterators = false;
    static constexpr bool is_ordered = false;
    static constexpr bool is_unique = false;
};

// container_gen maps selectors to containers
template<typename ValueType>
struct container_gen<listS, ValueType> {
    using type = std::list<ValueType>;
};

// Usage: adjacency_list<OutEdgeListS, VertexListS, DirectedS, ...>
adjacency_list<setS, listS, directed_tag, VertexProp, EdgeProp> g;
//             ^^^^^  ^^^^^
//             edges  vertices
```

**Available Selectors (10 total):**

| Selector | Container | Use Case |
|----------|-----------|----------|
| `vecS` | `std::vector` | Fast access, cache-friendly traversal |
| `listS` | `std::list` | Stable iterators, O(1) insertion/removal |
| `setS` | `std::set` | Unique edges, ordered iteration |
| `mapS` | `std::set` (for edges) | Prevent parallel edges |
| `multisetS` | `std::multiset` | Ordered with parallel edges |
| `multimapS` | `std::multiset` | Key-value with parallel edges |
| `hash_setS` | `std::unordered_set` | Fast unique lookup |
| `hash_mapS` | `std::unordered_set` | Fast key-value access |
| `hash_multisetS` | `std::unordered_multiset` | Fast with duplicates |
| `hash_multimapS` | `std::unordered_multiset` | Fast key-value with duplicates |

**Vertex List Implementations (3):** `vecS`, `listS`, `setS`

### Graph-v3 Multi-Layered Approach

Graph-v3 provides **three levels of graph support**:

#### Level 1: Standard Containers as Graphs (Zero Configuration)

Standard containers work directly as graphs via CPOs and concepts:

```cpp
// std::vector<std::vector<int>> works directly as an adjacency list
std::vector<std::vector<int>> g = {
    {1, 2},    // vertex 0 -> edges to 1, 2
    {2, 3},    // vertex 1 -> edges to 2, 3
    {3}        // vertex 2 -> edges to 3
};

// CPOs work automatically
for (auto u : graph::vertices(g)) {
    auto id = graph::vertex_id(g, u);  // Returns index (size_t)
    for (auto e : graph::edges(g, u)) {
        auto tid = graph::target_id(g, e);  // Returns target vertex ID
    }
}

// Weighted graphs: vector<vector<pair<int, double>>>
std::vector<std::vector<std::pair<int, double>>> weighted = {
    {{1, 1.5}, {2, 2.0}},  // vertex 0 with weights
    {{2, 0.5}}              // vertex 1
};
```

**Supported patterns:**
- `vector<vector<VId>>` - simple adjacency list
- `vector<vector<pair<VId, Weight>>>` - weighted adjacency
- `vector<forward_list<VId>>` - forward_list edges
- `deque<vector<VId>>` - deque-based vertices
- Any combination with random-access vertices + forward-range edges

#### Level 2: Adapt Existing Graphs (Concept-Based)

Any graph structure satisfying the `adjacency_list` concept works automatically:

```cpp
// Requirements: random-access vertices, forward-range edges
template<typename G>
concept adjacency_list = 
    vertex_range<G> &&                          // vertices(g) returns a range
    requires(G& g, vertex_t<G> u) {
        { edges(g, u) } -> targeted_edge_range<G>;  // forward range of edges
    };
```

Custom graphs just need to provide `vertices()` and `edges()` functions/methods.

#### Level 3: dynamic_graph with Trait Structs

For full-featured graphs with properties, use trait-based configuration:

```cpp
// Trait struct defines container types directly
template <class EV, class VV, class GV, class VId, bool Sourced>
struct mofl_graph_traits {
    using vertices_type = std::map<VId, vertex_type>;      // map for vertices
    using edges_type    = std::forward_list<edge_type>;    // forward_list for edges
};

// Usage: dynamic_graph<EV, VV, GV, VId, Sourced, Traits>
using G = dynamic_graph<double, std::string, void, uint32_t, false, 
                        mofl_graph_traits<double, std::string, void, uint32_t, false>>;
```

**Available Trait Combinations (25 total):**

| Vertices | Edges | Trait |
|----------|-------|-------|
| vector | vector, list, forward_list, deque, set, unordered_set, edge_multimap | vov, vol, vofl, vod, vos, vous, voem |
| deque | vector, list, forward_list, deque, set, unordered_set | dov, dol, dofl, dod, dos, dous |
| map | vector, list, forward_list, deque, set, unordered_set, edge_multimap | mov, mol, mofl, mod, mos, mous, moem |
| unordered_map | vector, list, forward_list, deque, unordered_set | uov, uol, uofl, uod, uous |

#### Level 4: compressed_graph (CSR Format)

High-performance static graph with Compressed Sparse Row storage:

```cpp
compressed_graph<EdgeValue, VertexValue, GraphValue, VId, EIndex> csr;
// Optimal for read-heavy workloads, minimal memory footprint
```

### Comparative Analysis

| Aspect | BGL2 Selectors | Graph-v3 |
|--------|---------------|----------|
| **Configuration** | Tag-based template params | Multi-layered (zero-config to traits) |
| **Standard containers** | Requires `adjacency_list` wrapper | Direct use via concepts |
| **Total built-in options** | 10 × 3 = 30 potential (edge × vertex) | 25 traits + unlimited container combos |
| **Extensibility** | Specialize `container_gen<S, T>` | Satisfy concept or create trait |
| **CSR/static graphs** | `compressed_sparse_row_graph` | `compressed_graph` |
| **Adaptation complexity** | graph_traits specialization | Just satisfy concept |
| **Non-integral vertex IDs** | Not directly supported | Native (map/unordered_map vertices) |

---

## 5. Other Design and Flexibility Considerations

### Customization Point Objects (Graph-v3) vs ADL (BGL2)

**Graph-v3 CPO Pattern:**
```cpp
// Guaranteed to find the right function, prevents ADL hijacking
auto verts = graph::vertices(my_graph);  // Qualified call to CPO
```

**BGL2 ADL Pattern:**
```cpp
// Relies on unqualified lookup
auto verts = vertices(my_graph);  // ADL finds correct overload
```

CPOs provide stronger encapsulation and avoid ADL hijacking issues, but add complexity to the library implementation.

### Direction and Edge Parallel Categories

**BGL2**: Uses tag types (`directed_tag`, `bidirectional_tag`, `allow_parallel_edge_tag`) extracted via `graph_traits`.

**Graph-v3**: Uses template parameters directly (`bool Sourced`) and trait-based container selection. Direction is implicit in which CPOs are available (`source_id` requires `Sourced=true`).

### Property Access Paradigms

**BGL2**: Unified property map concept with `get(pmap, key)` and `put(pmap, key, value)`:
```cpp
template<typename PMap, typename Key, typename Value>
concept PropertyMap = std::invocable<PMap, Key> && 
    std::convertible_to<std::invoke_result_t<PMap, Key>, Value>;
```

**Graph-v3**: Properties accessed via CPOs with graph reference:
```cpp
for (auto u : graph::vertices(g)) {
    graph::vertex_value(g, u) = new_value;  // Via CPO (requires graph)
}
```

### Algorithm Integration Readiness

| Aspect | BGL2 | Graph-v3 |
|--------|------|----------|
| **Color maps** | `vector_property_map<Color>` | Would need external map |
| **Distance maps** | `vector_property_map<Dist>` | Would need external map |
| **Predecessor maps** | `vector_property_map<VD>` | Would need external map |
| **Weight access** | Lambda property map | `edge_value(g, e)` CPO |
| **Visitor callbacks** | Designated initializer struct | Not defined yet |

### Compile-Time Guarantees

**BGL2**: Concepts check that graphs satisfy required operations:
```cpp
template<typename G>
concept VertexListGraph = Graph<G> && requires(G& g) {
    { vertices(g) } -> std::ranges::input_range;
    { num_vertices(g) } -> std::integral;
};
```

**Graph-v3**: CPO `requires` clauses check individual operations:
```cpp
template<typename G>
[[nodiscard]] constexpr auto operator()(G&& g) const
    requires (_Choice<G>._Strategy != _St::_none)
{ ... }
```

---

## 6. Summary Recommendations

### When to Choose BGL2

- You need **working algorithms now** (BFS, DFS, Dijkstra, Bellman-Ford, Prim, Kruskal, etc.)
- Your graphs use **integral or iterator-based vertex descriptors**
- You're migrating from **legacy Boost.Graph** code
- You want **familiar patterns** with established documentation
- You need **container selector flexibility** with 10 selectors for edges and 3 for vertices
- You want **automatic parallel edge detection** based on container uniqueness

### When to Choose Graph-v3

- You want to use **standard containers directly** (`vector<vector<int>>`, etc.) without wrappers
- You need **non-integral vertex IDs** (strings, custom types) as first-class citizens
- You want to **adapt existing graph structures** by just satisfying concepts
- You need **maximum container flexibility** (25 traits + unlimited container combos)
- You prefer **self-contained descriptors** that carry vertex IDs
- You're building a **new codebase** and can wait for algorithms
- You need **partitioned graph support** or **CSR format** (`compressed_graph`)

### Potential Synergies

Both libraries share C++20 foundations and could potentially:
1. Share concept definitions (both have similar `Graph`, `IncidenceGraph` concepts)
2. Use BGL2 algorithms with Graph-v3 containers via adapter layer
3. Adopt BGL2's property map abstraction in Graph-v3 for algorithm integration

---

## 7. Code Examples Side-by-Side

### Creating a Graph

**BGL2:**
```cpp
struct VertexData { std::string name; };
struct EdgeData { double weight; };

// Using selectors: setS for edges (no parallel), listS for vertices (stable)
adjacency_list<setS, listS, directed_tag, VertexData, EdgeData> g;

auto v0 = g.add_vertex({"A"});
auto v1 = g.add_vertex({"B"});
auto [e, inserted] = g.add_edge(v0, v1, {1.5});
```

**Graph-v3 (using standard containers directly):**
```cpp
// Simple adjacency list - no library types needed!
std::vector<std::vector<int>> g = {
    {1, 2},    // vertex 0 -> edges to 1, 2
    {2},       // vertex 1 -> edge to 2
    {}         // vertex 2 -> no outgoing edges
};

// Weighted version
std::vector<std::vector<std::pair<int, double>>> weighted = {
    {{1, 1.5}, {2, 2.0}},
    {{2, 0.5}},
    {}
};
```

**Graph-v3 (using dynamic_graph with traits):**
```cpp
using G = dynamic_graph<double, std::string, void, uint32_t, false, vov_graph_traits<...>>;
G g;
g.vertices().resize(2);
graph::vertex_value(g, 0) = "A";
graph::vertex_value(g, 1) = "B";
g.vertices()[0].emplace_back(1, 1.5);  // edge 0->1 with weight 1.5
```

### Traversing Vertices

**BGL2:**
```cpp
for (auto v : vertices(g)) {
    std::cout << g[v].name << "\n";  // Requires graph for property access
}
```

**Graph-v3 (standard container):**
```cpp
std::vector<std::vector<int>> g = {{1,2}, {2}, {}};
for (auto u : graph::vertices(g)) {
    auto vid = graph::vertex_id(g, u);
    std::cout << "Vertex " << vid << " has " 
              << std::ranges::size(graph::edges(g, u)) << " edges\n";
}
```

**Graph-v3 (dynamic_graph):**
```cpp
for (auto v : graph::vertices(g)) {
    std::cout << graph::vertex_value(g, v) << "\n";  // Via CPO with graph reference
}
```

### Running BFS

**BGL2:**
```cpp
auto [distances, predecessors] = bfs(g, source);
// or with callbacks:
bfs(g, source, {
    .on_discover_vertex = [](auto v, auto&) { std::cout << v << " "; }
});
```

**Graph-v3:**
```cpp
// Not implemented - algorithms are a known limitation
```

---

*Note: Algorithms have not been implemented for graph-v3 and this is a known limitation.*
