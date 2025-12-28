# BGL2 vs Graph-v3 Comparative Evaluation

**Date:** Based on analysis of source code from both libraries  
**BGL2 Location:** `/home/phil/dev_graph/boost/libs/graph/modern/`  
**Graph-v3 Location:** `/home/phil/dev_graph/desc/`

---

## 1. Vertex and Edge Descriptor Differences

### BGL2 Approach

BGL2 uses a **traditional, type-centric** approach to descriptors:

```cpp
// Vertex descriptor is a simple integral type
using vertex_descriptor = std::size_t;

// Edge descriptor is a lightweight struct
struct adjacency_list_edge<vertex_descriptor> {
    vertex_descriptor source;
    vertex_descriptor target;
    std::size_t edge_index;  // for parallel edge distinction
};
```

**Key Characteristics:**
- **Integral vertex descriptors**: `std::size_t` enables direct array indexing for O(1) property map access
- **Stable on insertion**: Adding vertices doesn't invalidate existing descriptors (index-based)
- **Optional strong typing**: `descriptor<vertex_tag, IndexType>` wrapper prevents mixing vertex/edge descriptors at compile time
- **Null vertex convention**: `static_cast<vertex_descriptor>(-1)` for invalid/null vertices
- **Extracted via traits**: `vertex_descriptor_t<G>` uses `graph_traits<G>::vertex_descriptor`
- **Container selectors defined but not implemented**: `listS` and `setS` are declared as TODOs; `mapS` is not present. Only `vecS` (vector) works currently.

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
| **Type** | `std::size_t` (fixed) | `vertex_descriptor<Iter>` template |
| **Internal storage** | Index (8 bytes) | Index for random-access (8 bytes), iterator otherwise |
| **Container selectors** | Only `vecS` implemented; `listS`, `setS` TODO; no `mapS` | 20+ trait combinations (vov, vofl, mofl, mol, etc.) |
| **Non-integral IDs** | Not supported (no `mapS`) | Native support via map-based traits (mofl, mol, etc.) |
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
| **Size (iterator-based)** | N/A (only vector implemented) | Varies by iterator size |
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

5. **Lightweight descriptors**: 8-byte vertex descriptors enable cache-efficient traversal.

### BGL2 Weaknesses

1. **Fixed container structure**: `adjacency_list` currently only supports vector-based storage (`vecS`). Container selectors `listS` and `setS` are declared but marked TODO; `mapS` (which BGL1 supported for map-based vertex storage with non-integral keys) is not present. This means vertex descriptors are always `std::size_t`.

2. **Integral descriptor requirement**: Since only vector storage is implemented, vertex descriptors are always integral. Non-integral vertex IDs (strings, custom types) would require either:
   - A separate vertex-to-index mapping layer
   - Using `unordered_map` for property storage instead of vectors
   - Future implementation of `mapS` selector

3. **Traits boilerplate**: Adapting external graphs requires explicit `graph_traits` specialization.

4. **Single-partition assumption**: No built-in support for partitioned/distributed graphs.

### Graph-v3 Strengths

1. **Exceptional container flexibility**: 20+ trait combinations (vofl, vol, vov, vod, dol, dofl, dov, dod, mofl, mol, mov, mod, uov, uod, uol, uofl, mos, mous, dos, vos):
   ```cpp
   // Same dynamic_graph, different containers
   using g1 = dynamic_graph<EV, VV, GV, VId, Sourced, vov_graph_traits<...>>;  // vector<vector>
   using g2 = dynamic_graph<EV, VV, GV, VId, Sourced, mofl_graph_traits<...>>; // map<forward_list>
   ```

2. **Non-invasive adaptation**: CPO resolution with fallbacks means standard containers work without adaptation.

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

### Graph-v3 Weaknesses

1. **No algorithms implemented**: This is a known limitation. All traversal, shortest-path, spanning tree, and flow algorithms are missing.

2. **Iterator-based descriptors for non-random-access containers**: Map/list storage requires storing iterators (larger than 8 bytes).

3. **Iterator stability concerns**: Descriptors holding iterators may be invalidated by container modifications.

4. **Learning curve**: CPO resolution chain and pattern detection are less familiar than traditional traits.

5. **Property map story unclear**: No explicit property map abstraction; properties accessed via descriptors or container access.

---

## 4. Other Design and Flexibility Considerations

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

## 5. Summary Recommendations

### When to Choose BGL2

- You need **working algorithms now** (BFS, DFS, Dijkstra, etc.)
- Your graphs use **integral vertex IDs**
- You're migrating from **legacy Boost.Graph** code
- You want **familiar patterns** with established documentation
- You need algorithms immediately without building your own

### When to Choose Graph-v3

- You need **maximum container flexibility** (vector, deque, map, set, list, forward_list combinations)
- Your graphs use **non-integral vertex IDs** (strings, custom types)
- You want **non-invasive adaptation** of existing containers
- You prefer **self-contained descriptors** that carry values
- You're building a **new codebase** and can wait for algorithms
- You need **partitioned graph support**

### Potential Synergies

Both libraries share C++20 foundations and could potentially:
1. Share concept definitions (both have similar `Graph`, `IncidenceGraph` concepts)
2. Use BGL2 algorithms with Graph-v3 containers via adapter layer
3. Adopt BGL2's property map abstraction in Graph-v3 for algorithm integration

---

## 6. Code Examples Side-by-Side

### Creating a Graph

**BGL2:**
```cpp
struct VertexData { std::string name; };
struct EdgeData { double weight; };
adjacency_list<VertexData, EdgeData> g;

auto v0 = add_vertex(g, {"A"});
auto v1 = add_vertex(g, {"B"});
add_edge(g, v0, v1, {1.5});
```

**Graph-v3:**
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

**Graph-v3:**
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
