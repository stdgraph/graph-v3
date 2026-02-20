# Graph Descriptor Project - Implementation Instructions

## Project Overview
This is a C++20 project that implements descriptors as abstractions for vertices and edges in a graph data structure. The descriptors provide type-safe, efficient handles for graph elements.

## RFC 2119 Key Words
The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119.

## Project Configuration

### Build System
- **Build Tool**: CMake (minimum version 3.20)
- **Generator**: Ninja
- **C++ Standard**: C++20
- **Testing Framework**: Catch2 (v3.x recommended)

### Directory Structure
```
desc/
├── CMakeLists.txt           # Root CMake configuration
├── include/
│   └── desc/
│       ├── descriptor.hpp   # Core descriptor interface
│       ├── vertex_descriptor.hpp
│       └── edge_descriptor.hpp
├── src/
│   └── (implementation files if needed)
├── tests/
│   ├── CMakeLists.txt
│   ├── test_main.cpp        # Catch2 main
│   ├── test_vertex_descriptor.cpp
│   └── test_edge_descriptor.cpp
├── examples/
│   └── basic_usage.cpp
└── README.md
```

## Core Requirements

### 1. Underlying Data Representation

#### Vertex Representation
Vertices in the graph can be stored in different container types, which affects how descriptors reference them:

**Random Access Containers (e.g., `std::vector`):**
- Vertices MAY be stored as direct values in a contiguous container
- Each vertex is identified by its index position which is the Vertex ID
- The vertex itself can be any type (simple types, structs, classes)
- Examples: `std::vector<VertexData>` and `std::deque<VertexData>` where vertices are accessed by index

**Associative Containers (e.g., `std::map`, `std::unordered_map`):**
- Vertices MUST be stored as key-value pairs
- The key serves as the vertex ID (MUST be comparable and/or hashable)
- The value contains the vertex data/properties
- Example: `std::map<VertexId, VertexData>` where vertices are accessed by key
- The iterator's `value_type` is `std::pair<const Key, Value>`

#### Edge Representation
Edges in the graph can be stored in various ways depending on graph structure:

**Random Access Containers (e.g., `std::vector`):**
- Edges MAY be stored in contiguous containers
- Each edge is identified by its index position
- Two primary layouts are supported:
  - **Per-vertex adjacency storage** (RECOMMENDED): each vertex owns a random-access container of outgoing edges. The source vertex is implied by the owning container and MUST NOT be duplicated in the edge payload.
  - **Global edge storage**: all edges share a single random-access container. In this scenario the edge payload MUST include the source vertex identifier because the container position does not encode it.
- In both layouts the edge payload MUST include the target vertex identifier and MAY include additional properties.
- Examples:
  - `std::vector<std::vector<int>>` where each inner vector stores target vertex ids for a specific source vertex
  - `std::vector<Edge>` where `Edge` includes source and target vertex identifiers (for global storage)
  - `std::vector<std::pair<int,double>>` where the first element stores the target vertex id and the second stores an optional property
  - `std::vector<std::tuple<int32_t,double>>` where `int32_t` stores the target vertex id and `double` stores an optional property
  - Note: Tuple-like edge payloads MUST have 1 or more elements, and the first element MUST be the target vertex id

**Forward/Bidirectional Containers (e.g., `std::list`, `std::set`, `std::map`, adjacency lists):**
- Each edge is identified by an iterator to its position
- Edges MAY be stored in linked structures or per-vertex adjacency lists
- The same container value types as random access containers apply (simple types, pairs, tuples, or structs)
- The choice between per-vertex adjacency storage and a global container follows the same rules as above: omit the source vertex in per-vertex containers, include it when using a shared/global container.
- Examples:
  - `std::list<int>` where `int` stores the target vertex id
  - `std::list<std::pair<int,double>>` where first element is target vertex id, second is edge property
  - `std::set<int>` where `int` stores the target vertex id (for unweighted graphs)
  - `std::map<int,EdgeData>` where key is target vertex id and value contains edge properties
  - `std::list<Edge>` where `Edge` follows the same source-inclusion rules described above

**Edge Data Structure Requirements:**
- An edge representation SHOULD contain at minimum:
  - Target vertex identifier (as the first element if tuple-like, or as a simple value)
  - Optional edge weight or properties (if tuple-like or struct)
- Whether the edge payload stores the source vertex identifier depends on storage layout:
  - For per-vertex adjacency storage, the source vertex identifier MUST NOT be duplicated in the edge payload.
  - For global edge storage, the edge payload MUST include the source vertex identifier because container location does not encode it.
- Examples:
  - Per-vertex adjacency: `struct Edge { vertex_id target; EdgeData data; }`
  - Global storage: `struct Edge { vertex_id source; vertex_id target; EdgeData data; }`
- Note: The edge_descriptor will maintain both the edge location and the source vertex_descriptor, regardless of storage layout

### 2. Descriptor Base Concept/Interface
- Descriptors MUST be lightweight, copyable handles
- Descriptors MUST support comparison operations (==, !=, <, <=, >, >=)
- Descriptors MUST satisfy the C++20 `std::incrementable` concept (support pre/post increment semantics)
- Descriptors MUST be hashable for use in STL containers
- Descriptors MUST be default constructible and trivially copyable
- Descriptors SHOULD provide a null/invalid state representation
- Descriptors MUST be type-safe: vertex and edge descriptors SHALL be distinct types

### 3. Vertex Descriptor
- MUST represent a handle to a vertex in the graph
- MUST be a template with a single parameter for the underlying container's iterator type
- Iterator category constraints:
  - **Random Access Iterator** (e.g., vector, deque): Used for index-based vertex storage
  - **Bidirectional Iterator** (e.g., map, unordered_map): Used for key-value based vertex storage
    - When bidirectional: the iterator's `value_type` MUST satisfy a pair-like concept where:
      - The type MUST have at least 2 members (accessible via tuple protocol or pair interface)
      - The first element serves as the vertex ID (key)
      - This can be checked using `std::tuple_size<value_type>::value >= 2` or by requiring `.first` and `.second` members
- MUST have a single member variable that:
  - MUST be `size_t index` when the iterator is a random access iterator
  - MUST be the iterator type itself when the iterator is a bidirectional iterator (non-random access)
- MUST provide a `vertex_id()` member function that returns the vertex's unique identifier:
  - When the vertex iterator is random access: MUST return the `size_t` member value (the index)
  - When the vertex iterator is bidirectional: MUST return the key (first element) from the pair-like `value_type`
  - Return type SHOULD be deduced appropriately based on iterator category
- MUST provide a public `value()` member function that returns the underlying storage handle:
  - When the vertex iterator is random access: `value()` MUST return the stored `size_t` index
  - When the vertex iterator is bidirectional: `value()` MUST return the stored iterator
  - Return type SHOULD be the exact type of the underlying member (copy by value)
- MUST provide pre-increment and post-increment operators (`operator++` / `operator++(int)`) whose behavior mirrors the underlying storage:
  - For random access iterators: increment operations MUST advance the `size_t` index by one
  - For bidirectional iterators: increment operations MUST advance the stored iterator
- MUST be efficiently passable by value
- SHOULD support conversion to/from underlying index type (for random access case)
- MUST integrate with std::hash for unordered containers

### 4. Edge Descriptor
- MUST represent a handle to an edge in the graph
- MUST be a template with two parameters:
  - First parameter: the underlying edge container's iterator type
  - Second parameter: the vertex iterator type
- MUST have two member variables:
  - First member: MUST be `size_t index` (for edge index) when the edge iterator is a random access iterator, or the edge iterator type itself when the edge iterator is a forward/bidirectional iterator (non-random access)
  - Second member: MUST be a `vertex_descriptor` (instantiated with the vertex iterator type from the second template parameter) representing the source vertex
- The edge descriptor identifies both WHERE the edge is stored (via first member) and WHICH vertex it originates from (via second member)
- MUST provide a public `value()` member function that returns the underlying edge storage handle:
  - When the edge iterator is random access: `value()` MUST return the stored `size_t` index
  - When the edge iterator is forward/bidirectional: `value()` MUST return the stored edge iterator
  - Return type SHOULD be the exact type of the underlying first member (copy by value)
- MUST provide pre-increment and post-increment operators whose behavior mirrors the underlying edge storage:
  - For random access iterators: increment operations MUST advance the `size_t` index by one while leaving the source `vertex_descriptor` unchanged
  - For forward/bidirectional iterators: increment operations MUST advance the stored edge iterator while leaving the source `vertex_descriptor` unchanged
- Directed/undirected semantics are determined by the graph structure, not by the descriptor itself
- MUST be efficiently passable by value
- MUST integrate with std::hash for unordered containers

### 4.1 Descriptor Value Access Functions

Both `vertex_descriptor` and `edge_descriptor` MUST provide specialized functions for accessing the underlying container data:

#### `underlying_value()` Function
- **Purpose**: Returns the complete data stored in the container at the descriptor's position
- **For vertex_descriptor**:
  - Random access containers (vector): returns reference to the element at index
  - Bidirectional containers (map): returns reference to the pair<key, value>
- **For edge_descriptor**:
  - Random access containers: returns reference to the edge element at index
  - Forward/bidirectional containers: returns reference to the edge element via iterator dereference
- **Return type**: MUST use `decltype(auto)` to preserve cv-qualifiers and reference category
- **Signature**: `template<typename Container> decltype(auto) underlying_value(Container& container) const noexcept`
- **Const overload**: MUST provide const version accepting `const Container&`

#### `inner_value()` Function
- **Purpose**: Returns the "property" or "data" part of the container element, excluding identifiers
- **For vertex_descriptor**:
  - Random access containers (vector): returns reference to the entire element (same as underlying_value)
  - Bidirectional containers (map): returns reference to `.second` only (the value, excluding the key)
- **For edge_descriptor** (behavior depends on edge value type):
  - **Simple integral type** (e.g., `int`): returns the value itself (it IS the target ID, no separate property)
  - **Pair type** `<target, property>`: returns reference to `.second` (the property part, excluding target)
  - **Tuple type** `<target, ...>`:
    - 1 element: returns reference to element [0]
    - 2 elements: returns reference to element [1]
    - 3+ elements: returns tuple of references to elements [1, N) using `std::forward_as_tuple`
  - **Custom struct/class**: returns reference to the whole value (user manages which fields are properties)
- **Return type**: MUST use `decltype(auto)` to preserve reference semantics
- **Signature**: `template<typename Container> decltype(auto) inner_value(Container& container) const noexcept`
- **Const overload**: MUST provide const version accepting `const Container&`

#### **CRITICAL IMPLEMENTATION DETAIL: Parentheses with `decltype(auto)`**

When implementing functions that return via `decltype(auto)`, ALL return statements MUST wrap their expressions in parentheses to preserve lvalue reference semantics:

**❌ INCORRECT** (returns by value, breaks modification):
```cpp
template<typename Container>
decltype(auto) inner_value(Container& container) const noexcept {
    return container[storage_].second;  // decltype treats this as member type (double)
}
```

**✅ CORRECT** (returns lvalue reference, allows modification):
```cpp
template<typename Container>
decltype(auto) inner_value(Container& container) const noexcept {
    return (container[storage_].second);  // decltype treats this as expression (double&)
}
```

**Rationale**: With `decltype(auto)`:
- `decltype(expr)` when `expr` is an id-expression or member access gives the **declared type** (strips references)
- `decltype((expr))` with extra parentheses treats it as an **expression** and preserves value category (lvalue → T&, xvalue → T&&, prvalue → T)

This applies to ALL return statements in value-access functions:
- `return (container[i]);` not `return container[i];`
- `return ((*iterator).second);` not `return (*iterator).second;`
- `return (std::get<1>(tuple));` not `return std::get<1>(tuple);`

**Exception**: Lambda return values and `std::forward_as_tuple` results do NOT need extra parentheses as they already create the correct reference type.

### 5. Descriptor Views 

***IMPORTANT LIMITATION***: Descriptor views are restricted to forward iteration only. This is because the descriptor is synthesized by the iterator during traversal (the descriptor contains the iterator itself), making it incompatible with random access operations. Specifically, operator[] would need to return a reference to a descriptor, but since the descriptor is created on-the-fly by the iterator, there is no stable object to reference. This design enables the interface to work uniformly across different storage strategies (vectors, maps, custom containers) while maintaining descriptor-based abstraction.

#### Vertex Descriptor View
- MUST provide a view over the underlying vertex storage that yields `vertex_descriptor` values
- MUST model `std::ranges::view` (lightweight, copyable, reference semantics)
- View iterator requirements:
  - Iterator value_type MUST be `vertex_descriptor`
  - Iterator MUST satisfy the `std::forward_iterator` concept. Random-access operations (e.g., `operator[]`, `operator+=`) MUST NOT be provided because descriptors are synthesized on the fly during traversal.
  - Increment operations MUST advance the underlying index/iterator consistent with the corresponding descriptor semantics
- MUST expose `begin()`/`end()` (const and non-const) returning forward iterators that satisfy the above requirements
- MUST NOT copy vertex storage; underlying containers MUST be referenced via pointer/reference semantics
- MAY provide `size()`/`empty()` helpers when the underlying container supports them
- Implementations SHOULD derive from `std::ranges::view_interface` unless a compelling reason prevents it
- Rationale: constraining the iterator to forward traversal ensures the view works uniformly across storage strategies (vectors, maps, custom containers) while maintaining descriptor-based abstraction and avoiding invalid references

#### Edge Descriptor View
- MUST provide a view over the underlying edge storage that yields `edge_descriptor` values
- MUST model `std::ranges::view`
- View iterator requirements:
  - Iterator value_type MUST be `edge_descriptor`
  - Iterator MUST satisfy the `std::forward_iterator` concept. Random-access operations MUST NOT be provided for the same reasons as the vertex descriptor view (descriptors are synthesized per step).
  - Increment operations MUST advance the underlying edge index/iterator and preserve the source `vertex_descriptor`
- MUST support both per-vertex adjacency storage and global edge storage configurations
- MUST expose `begin()`/`end()` (const and non-const) returning forward iterators that satisfy the above requirements
- MUST NOT copy edge storage; underlying containers MUST be referenced via pointer/reference semantics
- MAY provide helpers (e.g., `size()`, `empty()`) when supported by the underlying storage
- Implementations SHOULD derive from `std::ranges::view_interface` unless a compelling reason prevents it
- Rationale: restricting to forward iteration guarantees compatibility with all supported storage layouts while keeping descriptors ephemeral and preventing invalid references

### 6. Design Principles
- **Zero-cost abstraction**: Implementation MUST NOT introduce runtime overhead compared to raw indices
- **Type safety**: Implementation MUST prevent mixing vertex and edge descriptors
- **STL compatibility**: Descriptors MUST work seamlessly with standard containers
- **Const-correctness**: Implementation MUST maintain proper const semantics throughout
- **Modern C++20**: Implementation SHOULD utilize concepts, requires clauses, and other C++20 features where appropriate

## Implementation Guidelines

### Phase 1: Foundation
1. Create CMake build configuration with C++20 support
2. Set up Catch2 integration for testing
3. Define descriptor concept using C++20 concepts, including:
   - Pair-like concept for bidirectional iterator value_type (using `std::tuple_size >= 2` or `.first`/`.second` members)
   - Random access and bidirectional iterator concepts
4. Implement basic vertex descriptor template with:
   - Template parameter for container iterator type
   - Concept constraints: random access OR (bidirectional AND pair-like value_type)
   - Conditional member type based on iterator category (size_t for random access, iterator for bidirectional)
   - `vertex_id()` member function with conditional implementation based on iterator category
  - `value()` member function returning the underlying index or iterator
  - Pre/post increment operators that forward to index/iterator increments
5. Implement `vertex_descriptor_view` that wraps the underlying vertex storage, yields descriptors, models `std::ranges::view`, exposes forward-only iterators, and (SHOULD) derives from `std::ranges::view_interface`
6. Write comprehensive unit tests for vertex descriptor and vertex descriptor view with both random access and bidirectional iterators (including map-based containers), covering `vertex_id()`, `value()`, increment semantics, and verifying the view satisfies `std::forward_range`

### Phase 2: Edge Descriptors
1. Implement edge descriptor template with:
   - Two template parameters (edge container iterator and vertex iterator)
   - First member variable: conditional based on edge iterator category (size_t for random access, iterator for forward/bidirectional)
   - Second member variable: vertex_descriptor instantiated with the vertex iterator type (represents source vertex)
   - Proper std::random_access_iterator and std::forward_iterator concept constraints
   - Pre/post increment operators consistent with underlying storage semantics
   - `value()` member function returning the underlying edge index or iterator
   - `source()` member function returning the source vertex_descriptor
   - `target_id()` member function that extracts target vertex ID from edge data (handles int, pair.first, tuple[0], custom types)
2. Implement `edge_descriptor_view` that adapts both per-vertex adjacency storage and global edge storage while yielding descriptors, modelling a forward-only view, and (SHOULD) deriving from `std::ranges::view_interface`
3. Write comprehensive unit tests for edge descriptor and edge descriptor view with both random access and forward iterators
4. Test `value()`/`source()`/`target_id()`/increment behavior, confirm the view satisfies `std::forward_range`, and cover various edge data types (simple integers, pairs, tuples, structs)
5. Ensure proper comparison and hashing

### Phase 3: Advanced Features

**Step 1: Type Traits and Introspection**
1. Create `descriptor_traits.hpp` with comprehensive type utilities:
   - `is_vertex_descriptor<T>` / `is_edge_descriptor<T>` type traits
   - `vertex_descriptor_type<T>` / `edge_descriptor_type<T>` concepts
   - `descriptor_iterator_type<D>` to extract underlying iterator type
   - `descriptor_storage_type<D>` to get internal storage type (size_t or iterator)
   - Storage category traits: `is_random_access_descriptor`, `is_bidirectional_descriptor`, `is_forward_descriptor`
   - `random_access_descriptor<D>` concept for descriptors with random access storage
2. Write unit tests verifying all traits/concepts work with various descriptor instantiations

**Step 2: Value Access Functions**
1. Add `underlying_value(Container&)` to both vertex_descriptor and edge_descriptor:
   - Returns complete container element at descriptor position
   - Return type: `decltype(auto)` to preserve cv-qualifiers and references
   - Provide const overload: `underlying_value(const Container&) const`
   - **CRITICAL**: Wrap ALL return expressions in parentheses: `return (container[i]);` not `return container[i];`
2. Add `inner_value(Container&)` to both descriptors:
   - **For vertex_descriptor**: returns data part (maps: `.second`; vectors: whole element)
   - **For edge_descriptor**: returns property part excluding target ID:
     - Simple int: returns the int itself (no separate property)
     - Pair `<target, prop>`: returns `.second`
     - Tuple 1-elem: returns element [0]
     - Tuple 2-elem: returns element [1]
     - Tuple 3+ elem: returns `std::forward_as_tuple` of elements [1, N)
     - Custom struct: returns whole value
   - Return type: `decltype(auto)`
   - Provide const overload accepting `const Container&`
   - **CRITICAL**: Wrap ALL return expressions in parentheses to preserve lvalue references
3. Write comprehensive tests:
   - Test reading through inner_value/underlying_value
   - Test MODIFYING through inner_value/underlying_value (verify reference semantics)
   - Test const correctness
   - Test all edge data types: int, pair, 2-tuple, 3-tuple, custom struct

**Step 3: Additional Features (Optional)**
1. Implement descriptor property maps if needed
2. Add specialized range adaptors
3. Performance benchmarks comparing descriptor overhead to raw indices
4. Extended documentation and usage examples

## Testing Requirements

### Unit Tests (Catch2)
Each component MUST have comprehensive tests covering:
- Default construction and initialization
- Copy and move semantics
- Comparison operations (all six comparison operators)
- Increment operations (pre/post increment semantics)
- `value()` accessors returning the correct underlying handle types
- Hash function consistency
- Edge cases (null/invalid descriptors)
- Container usage (vector, set, unordered_map)
- Descriptor views (vertex and edge): forward-range compliance, iterator value_type requirements, absence of random-access operations, and behavior of `view_interface` helpers when used
- Type safety (compilation failures for invalid operations)

### Test Organization
- Tests MUST use Catch2 TEST_CASE and SECTION macros
- Tests SHOULD group related tests logically
- Tests MUST include both positive and negative test cases
- Tests SHOULD test compile-time constraints where applicable

## CMake Configuration Template

```cmake
cmake_minimum_required(VERSION 3.20)
project(graph_descriptors VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Library target (header-only or compiled)
add_library(descriptors INTERFACE)
target_include_directories(descriptors INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(descriptors INTERFACE cxx_std_20)

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
```

## Code Style Guidelines

- Implementation MUST use snake_case for functions and variables
- Implementation MUST use snake_case for types and classes
- Implementation SHOULD prefer `constexpr` where possible
- Implementation SHOULD use `[[nodiscard]]` for functions that return important values
- Public APIs MUST be documented with Doxygen-style comments
- Header files MUST be self-contained (include all dependencies)

## Build Commands

```bash
# Configure
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# Or using ninja directly
cd build && ninja && ninja test
```

## Incremental Update Process

When updating this project:
1. Agent MUST review existing tests to understand current behavior
2. Agent MUST add new tests for new features BEFORE implementation (TDD)
3. Agent MUST implement features to make tests pass
4. Agent MUST run full test suite to ensure no regressions
5. Agent SHOULD update documentation and examples
6. Agent SHOULD update this descriptor.md file with any new requirements or design decisions

## Success Criteria

A successful implementation:
- ✓ MUST compile with C++20 compliant compilers (GCC 10+, Clang 10+, MSVC 2019+)
- ✓ MUST pass all unit tests with 100% success rate
- ✓ MUST provide zero runtime overhead compared to raw index usage
- ✓ MUST prevent common programming errors through type safety
- ✓ MUST integrate seamlessly with STL containers
- ✓ MUST have clear, well-documented public APIs
- ✓ SHOULD include working examples demonstrating usage

## Future Considerations

- Integration with graph adjacency structures
- Property maps for associating data with descriptors
- Descriptor ranges and views
- Custom allocator support
- Serialization support
- Thread-safety considerations for concurrent graph operations

## Common Implementation Pitfalls

### 1. decltype(auto) Without Parentheses
**Problem**: Using `decltype(auto)` without parentheses on return expressions causes value semantics instead of reference semantics.

**Symptoms**:
- Compilation error: "lvalue required as left operand of assignment"
- Compilation error: "cannot bind non-const lvalue reference to rvalue"
- Code compiles but modification through returned value doesn't affect container

**Solution**: Wrap ALL return expressions in parentheses:
```cpp
// ❌ WRONG - returns by value
return container[index].member;

// ✅ CORRECT - returns lvalue reference
return (container[index].member);
```

**Why**: `decltype(expr)` gives the declared type of members (strips references), while `decltype((expr))` treats it as an expression and preserves value category (lvalue → `T&`).

### 2. Lambda Reference Binding Issues with decltype(auto)
**Problem**: Using immediately-invoked lambdas that extract container members (like `.second` from map pairs) and binding the result to `auto&&` with `decltype(auto)` return type causes reference binding issues and dangling references.

**Symptoms**:
- Methods return references to wrong memory locations
- Values read through methods are garbage data
- Tests fail specifically for map-based containers while working for vectors
- Debug shows memory addresses of returned values differ from actual container elements

**Incorrect Pattern** (causes bugs with map graphs):
```cpp
template<typename EdgeContainer>
[[nodiscard]] constexpr decltype(auto) inner_value(EdgeContainer& edges) const noexcept {
    // ❌ WRONG - lambda creates reference binding issues
    auto&& edge_container = [&]() -> decltype(auto) {
        if constexpr (requires { edges.edges(); }) {
            return edges.edges();
        } else if constexpr (requires { edges.first; edges.second; }) {
            return edges.second;  // Extracting .second from map pair
        } else {
            return edges;
        }
    }();  // Immediately invoked
    
    // Using edge_container here may reference wrong memory
    return (edge_container[edge_storage_]);
}
```

**Correct Pattern** (direct conditional logic):
```cpp
template<typename EdgeContainer>
[[nodiscard]] constexpr decltype(auto) inner_value(EdgeContainer& edges) const noexcept {
    // ✅ CORRECT - direct if constexpr eliminates lambda lifetime issues
    if constexpr (requires { edges.edges(); }) {
        auto& edge_container = edges.edges();
        return (edge_container[edge_storage_]);
    } else if constexpr (requires { edges.first; edges.second; }) {
        auto& edge_container = edges.second;
        return (edge_container[edge_storage_]);
    } else {
        return (edges[edge_storage_]);
    }
}
```

**Why This Happens**: When a lambda extracts a member (like `.second`) from a parameter and returns it via `decltype(auto)`, the binding to `auto&&` can create subtle issues:
- The lambda captures by reference `[&]` and accesses the parameter
- It returns `decltype(auto)` which should preserve references
- But the binding of the lambda result to `auto&&` causes the reference to point to unexpected memory
- This specifically manifests with map iterators whose `.second` extraction through lambdas breaks

**Solution**: 
1. **Always use direct `if constexpr` branches** instead of lambdas for extracting container members
2. Use simple reference bindings: `auto& edge_container = edges.second;` not lambda-based extraction
3. Apply this pattern consistently to ALL methods that extract container members: `underlying_value()`, `inner_value()`, etc.
4. Test with map-based containers to verify correct memory addresses

**Important Exception**: This bug ONLY affects methods that return references (using `decltype(auto)`). Methods that return by value (like `target_id()`, `vertex_id()`, and `source_id()` which return `auto`) can safely use lambdas for extraction because no reference binding issues occur. The distinction:
- `vertex_id()`: Returns `auto` (by value) → lambdas are safe **for now** (see note below)
- `source_id()`: Returns `auto` (by value) → lambdas are safe **for now** (see note below)
- `target_id()`: Returns `auto` (by value) → lambdas are safe **for now** (see note below)
- `underlying_value()`: Returns `decltype(auto)` (reference) → lambdas cause bugs, use direct `if constexpr`
- `inner_value()`: Returns `decltype(auto)` (reference) → lambdas cause bugs, use direct `if constexpr`

**Future Consideration for ID Accessor Methods**: Currently `vertex_id()`, `source_id()`, and `target_id()` return `auto` (by value), which is acceptable for integral vertex IDs (int, uint32_t, etc.). However, when support for non-trivial vertex ID types is added (e.g., `std::string`, custom types), these methods will need to be updated to:
1. Change return type from `auto` to `decltype(auto)` to return by reference for non-trivial types
2. Replace any lambda-based extraction patterns with direct `if constexpr` branches (same fix applied to `underlying_value()` and `inner_value()`)
3. Wrap return expressions in parentheses: `return (storage_);` or `return (edge_val.first);`
4. Consider using `std::conditional_t` or type traits to decide between value/reference return based on whether the vertex ID type is trivially copyable
5. Update tests to cover non-trivial vertex ID types with reference semantics

The current implementations in these ID accessors will exhibit the same reference binding bugs once changed to return `decltype(auto)` for non-trivial types. Specifically:
- `vertex_descriptor::vertex_id()` - extracts ID from storage or map pair
- `edge_descriptor::source_id()` - calls `vertex_descriptor::vertex_id()` on source
- `edge_descriptor::target_id()` - uses lambdas to extract edge container, then accesses target ID

**Critical Testing**: When implementing value extraction methods:
- Test with `std::vector` (random access)
- Test with `std::map` (bidirectional, pair-like value_type) 
- Verify returned references point to correct memory addresses
- Test both const and non-const overloads
- Include modification tests to ensure references work correctly

### 3. Forgetting Const Overloads
**Problem**: Only implementing non-const versions of value access functions.

**Solution**: Always provide both:
```cpp
template<typename Container>
decltype(auto) inner_value(Container& container) const noexcept;

template<typename Container>
decltype(auto) inner_value(const Container& container) const noexcept;
```

**Note**: Both overloads must use the same implementation pattern (direct `if constexpr`, not lambdas).

### 4. Incorrect Edge Type Detection
**Problem**: Using simple type checks that don't account for `std::pair` also satisfying tuple requirements.

**Solution**: Check pair-like BEFORE tuple-like using `else if` chains, since pairs satisfy both concepts.

### 5. Not Testing Modification
**Problem**: Only testing reads through value access functions, missing reference semantics bugs.

**Solution**: Always include tests that MODIFY values through returned references and verify the container was updated.

### 6. Insufficient Container Type Coverage in Tests
**Problem**: Only testing with vector-based containers, missing bugs that only appear with map-based or other container types.

**Solution**: Test value access methods with multiple container types:
- Random access: `std::vector`, `std::deque`
- Map-based: `std::map<int, std::vector<Edge>>`
- Forward/bidirectional: `std::list`, custom containers
- Include both mutable and const container references

## Notes for the Agent

- Agent MUST run tests after making changes
- Agent MUST wrap ALL return expressions with parentheses when using `decltype(auto)`
- Agent MUST NEVER use immediately-invoked lambdas to extract container members (like `.second` from maps) - always use direct `if constexpr` branches instead
- Agent MUST provide both const and non-const overloads for container access functions
- Agent MUST test both reading AND writing through returned references
- Agent MUST test with multiple container types including `std::vector` AND `std::map<int, Container>`
- Agent MUST verify returned references point to correct memory addresses (especially for map-based containers)
- Agent SHOULD consider return-by-reference for non-trivial types (strings, custom types) instead of return-by-value
- Agent MUST update ID accessor implementations when adding non-trivial vertex ID support:
  - `vertex_descriptor::vertex_id()` - change return type from `auto` to `decltype(auto)`
  - `edge_descriptor::source_id()` - change return type from `auto` to `decltype(auto)` 
  - `edge_descriptor::target_id()` - change return type from `auto` to `decltype(auto)` and replace lambda extraction with direct `if constexpr` pattern
  - Wrap all return expressions in parentheses for proper reference semantics
  - Add tests for non-trivial vertex ID types (strings, custom types)
- Agent SHOULD keep the implementation header-only unless there's a compelling reason not to
- Agent SHOULD prioritize simplicity and clarity over premature optimization
- Agent MUST favor stronger type safety when in doubt
- Agent MUST document design decisions and trade-offs in code comments
- Agent SHOULD update this file when requirements evolve or design decisions are made

## Implementation Status

### Completed Features
- ✅ Phase 1: Vertex descriptors with conditional storage (index for random access, iterator for bidirectional)
- ✅ Phase 1: Vertex descriptor views with forward-only iteration
- ✅ Phase 1: Comprehensive vertex descriptor tests (all passing)
- ✅ Phase 2: Edge descriptors with source tracking and conditional storage
- ✅ Phase 2: Edge descriptor views with forward-only iteration
- ✅ Phase 2: `target_id()` function for extracting target vertex IDs from edge data
- ✅ Phase 2: Comprehensive edge descriptor tests (all passing)
- ✅ Phase 3 Step 1: Complete descriptor traits and type utilities system
- ✅ Phase 3 Step 1: Comprehensive traits tests (all passing)
- ✅ Phase 3 Step 2: `underlying_value()` functions for direct container access
- ✅ Phase 3 Step 2: `inner_value()` functions for property-only access
- ✅ Phase 3 Step 2: Full test coverage including modification tests (all passing)

### Test Results
- **Total Test Cases**: 55
- **Total Assertions**: 287
- **Pass Rate**: 100%

### Pending Features
- ⏳ Phase 3 Step 3: Property maps (optional, low priority)
- ⏳ Phase 3: Performance benchmarks
- ⏳ Phase 3: Extended documentation and usage examples

---

**Last Updated**: December 1, 2025
**Version**: 0.2.1
**Status**: Core implementation complete; value access functions fully implemented and tested; lambda lifetime bug documented
 