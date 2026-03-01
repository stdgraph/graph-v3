<table><tr>
<td><img src="../assets/logo.svg" width="120" alt="graph-v3 logo"></td>
<td>

# Edge Value Type Concepts and Pattern Detection

This document describes the concepts and type traits added to `descriptor.hpp` for detecting and categorizing edge value types used in graph descriptors.

</td>
</tr></table>

## Concepts

### Edge Value Type Patterns

The library defines four mutually exclusive concepts for edge value patterns:

#### 1. `simple_edge_type<T>`
Identifies simple integral edge types where the value itself is the target vertex ID.

**Example:**
```cpp
std::vector<int> edges = {1, 2, 3, 4};  // Each int is a target vertex ID
static_assert(simple_edge_type<int>);
```

#### 2. `pair_edge_type<T>`
Identifies pair-like edge types with `.first` (target ID) and `.second` (properties).

**Example:**
```cpp
std::vector<std::pair<int, double>> edges = {
    {1, 1.5},  // Target: 1, Weight: 1.5
    {2, 2.5}   // Target: 2, Weight: 2.5
};
static_assert(pair_edge_type<std::pair<int, double>>);
```

#### 3. `tuple_edge_type<T>`
Identifies tuple-like edge types where the first element is the target ID.

**Example:**
```cpp
std::vector<std::tuple<int, double, std::string>> edges = {
    {1, 1.5, "road"},  // Target: 1, Weight: 1.5, Type: "road"
    {2, 2.5, "rail"}   // Target: 2, Weight: 2.5, Type: "rail"
};
static_assert(tuple_edge_type<std::tuple<int, double, std::string>>);
```

#### 4. `custom_edge_type<T>`
Identifies custom struct/class edge types (fallback pattern).

**Example:**
```cpp
struct CustomEdge {
    int target;
    double weight;
    std::string label;
};
static_assert(custom_edge_type<CustomEdge>);
```

### Comprehensive Concept

#### `edge_value_type<T>`
Accepts any valid edge value type (matches at least one of the above patterns).

**Example:**
```cpp
static_assert(edge_value_type<int>);
static_assert(edge_value_type<std::pair<int, double>>);
static_assert(edge_value_type<std::tuple<int, double>>);
static_assert(edge_value_type<CustomEdge>);
```

## Type Traits

### Pattern Detection Struct

#### `edge_value_pattern<T>`
Provides boolean flags for each pattern:

```cpp
template<typename T>
struct edge_value_pattern {
    static constexpr bool is_simple;
    static constexpr bool is_pair;
    static constexpr bool is_tuple;
    static constexpr bool is_custom;
};
```

**Usage:**
```cpp
using Pattern = edge_value_pattern<std::pair<int, double>>;
static_assert(Pattern::is_pair);
static_assert(!Pattern::is_simple);
```

### Pattern Detection Variable Template

#### `edge_value_pattern_v<T>`
Convenient variable template for pattern detection:

```cpp
auto pattern = edge_value_pattern_v<std::tuple<int, double>>;
if (pattern.is_tuple) {
    // Handle tuple edge type
}
```

### Pattern Enum

#### `edge_pattern` Enumeration
Represents the four edge patterns:

```cpp
enum class edge_pattern {
    simple,   ///< Simple integral type
    pair,     ///< Pair-like with .first/.second
    tuple,    ///< Tuple-like with std::get<N>
    custom    ///< Custom struct/class
};
```

#### `edge_pattern_type_v<T>`
Returns the pattern enum value for a type:

```cpp
static_assert(edge_pattern_type_v<int> == edge_pattern::simple);
static_assert(edge_pattern_type_v<std::pair<int, double>> == edge_pattern::pair);
static_assert(edge_pattern_type_v<std::tuple<int, double>> == edge_pattern::tuple);
static_assert(edge_pattern_type_v<CustomEdge> == edge_pattern::custom);
```

## Pattern Matching in target_id()

The `target_id()` function in `edge_descriptor` uses these patterns to extract the target vertex ID:

```cpp
template<typename EdgeContainer>
constexpr auto target_id(const EdgeContainer& edges) const noexcept {
    using edge_value_type = typename std::iterator_traits<EdgeIter>::value_type;
    
    const auto& edge_value = /* get edge from container */;
    
    if constexpr (std::integral<edge_value_type>) {
        return edge_value;  // Simple: value is target ID
    }
    else if constexpr (requires { edge_value.first; }) {
        return edge_value.first;  // Pair: .first is target ID
    }
    else if constexpr (requires { std::get<0>(edge_value); }) {
        return std::get<0>(edge_value);  // Tuple: first element is target ID
    }
    else {
        return edge_value;  // Custom: return whole value
    }
}
```

## Use Cases

### 1. Compile-Time Type Checking

```cpp
template<typename EdgeType>
void process_edges() {
    static_assert(edge_value_type<EdgeType>, "Invalid edge type");
    
    if constexpr (simple_edge_type<EdgeType>) {
        // Handle simple integer edges
    } else if constexpr (pair_edge_type<EdgeType>) {
        // Handle pair edges with properties
    } else if constexpr (tuple_edge_type<EdgeType>) {
        // Handle tuple edges with multiple properties
    } else {
        // Handle custom edge types
    }
}
```

### 2. Generic Edge Processing

```cpp
template<typename EdgeIter>
void analyze_edge_structure() {
    using EdgeType = typename std::iterator_traits<EdgeIter>::value_type;
    
    constexpr auto pattern = edge_pattern_type_v<EdgeType>;
    
    std::cout << "Edge pattern: ";
    switch (pattern) {
        case edge_pattern::simple:
            std::cout << "simple (target ID only)\n";
            break;
        case edge_pattern::pair:
            std::cout << "pair (target + property)\n";
            break;
        case edge_pattern::tuple:
            std::cout << "tuple (target + multiple properties)\n";
            break;
        case edge_pattern::custom:
            std::cout << "custom struct/class\n";
            break;
    }
}
```

### 3. Constraining Function Templates

```cpp
// Function that only accepts pair-like edges
template<typename EdgeType>
    requires pair_edge_type<EdgeType>
auto get_edge_weight(const EdgeType& edge) {
    return edge.second;
}

// Function that only accepts edges with properties
template<typename EdgeType>
    requires (!simple_edge_type<EdgeType>)
void process_weighted_edge(const EdgeType& edge) {
    // Edge has properties beyond just target ID
}
```

## Design Rationale

1. **Mutual Exclusivity**: Each edge type matches exactly one pattern, preventing ambiguity.

2. **Priority Order**: The pattern matching follows a specific order:
   - Simple (integral) ? Pair (.first/.second) ? Tuple (std::get) ? Custom (fallback)

3. **Compile-Time Detection**: All concepts and traits are evaluated at compile-time, maintaining zero-cost abstraction.

4. **Extensibility**: The pattern system can be extended with new patterns without breaking existing code.

## Testing

Comprehensive tests are provided in `test_edge_concepts.cpp`:
- ? 64 tests covering all concepts and traits
- ? Pattern detection for all edge types
- ? Mutual exclusivity verification
- ? Runtime pattern queries
- ? Type safety guarantees

## Integration with edges(g, u) CPO

The edge value concepts are crucial for the default implementation of `edges(g, u)`:

### Default Implementation Strategy

`edges(g, u)` MUST always return an `edge_descriptor_view`. The CPO uses the following resolution order:

1. **Override with member function**: If `g.edges(u)` exists, use it (must return `edge_descriptor_view`)
2. **Override with ADL**: If ADL `edges(g, u)` exists, use it (must return `edge_descriptor_view`)
3. **Default to edge value pattern**: If the vertex descriptor's inner value is a forward range with elements satisfying `edge_value_type`, return `edge_descriptor_view(u.inner_value(), u)`

### Why Edge Value Patterns Enable Default Implementation

When a vertex's inner value (edge container) contains elements that follow one of the edge value patterns, `edge_descriptor_view` can automatically:
- Iterate over the edge elements
- Create `edge_descriptor` instances that know how to extract target IDs and edge values
- Provide a uniform interface regardless of the underlying edge storage type

### Example: Automatic edges(g, u) Support

```cpp
// No custom edges() needed for these containers:

// Simple edge pattern: vector of integers
std::vector<std::vector<int>> adj_list;
auto verts = vertices(adj_list);
for (auto u : verts) {
    auto edges = edges(adj_list, u);  // Returns edge_descriptor_view
    // Each edge descriptor wraps an int (target ID)
}

// Pair edge pattern: vector of pairs
std::vector<std::vector<std::pair<int, double>>> weighted_graph;
auto verts2 = vertices(weighted_graph);
for (auto u : verts2) {
    auto edges = edges(weighted_graph, u);  // Returns edge_descriptor_view
    // Each edge descriptor wraps a pair<int, double>
}

// Tuple edge pattern: map with tuple edges
std::map<int, std::vector<std::tuple<int, double, std::string>>> complex_graph;
auto verts3 = vertices(complex_graph);
for (auto u : verts3) {
    auto edges = edges(complex_graph, u);  // Returns edge_descriptor_view
    // Each edge descriptor wraps a tuple<int, double, string>
}

// Custom override when needed:
class MyGraph {
public:
    auto edges(vertex_descriptor<VertexIter> u) const { 
        return edge_descriptor_view(get_edges_for(u.vertex_id()), u); 
    }
private:
    std::vector<Edge> get_edges_for(int vid) const;
};
```

This design means that most simple graph containers automatically support `edges(g, u)` without any additional code, as long as the vertex's inner value contains elements following one of the four edge value patterns.

## Summary

These concepts and traits provide:
- **Type Safety**: Compile-time guarantees about edge value types
- **Clear Intent**: Explicit naming of edge patterns
- **Easy Detection**: Simple APIs for pattern matching
- **Zero Cost**: All checks happen at compile-time
- **Future Proof**: Extensible design for new patterns
- **Automatic CPO Support**: Enable default `edges(g, u)` implementation for standard containers
