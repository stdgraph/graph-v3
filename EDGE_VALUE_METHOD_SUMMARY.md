# Edge Value CPO - `.value()` Method Support

## Summary

Successfully updated the `edge_value` CPO to recognize and support the `.value()` member function pattern as a fallback strategy for accessing edge values.

## Changes Made

### 1. CPO Strategy Extension ([include/graph/detail/graph_cpo.hpp](include/graph/detail/graph_cpo.hpp))

**Lines 2368-2420**: Updated `edge_value` CPO namespace

- Added `_value_fn` to the `enum class _St` strategy enumeration
- New resolution order: `_member` → `_adl` → `_value_fn` → `_default`

**New Concept**: `_has_value_fn`
```cpp
template<typename G, typename E>
concept _has_value_fn = 
    (!is_edge_descriptor_v<std::remove_cvref_t<E>>) && 
    requires(const E& uv) { { uv.value() }; };
```

**Key Design Decision**: The concept explicitly excludes edge descriptors using `!is_edge_descriptor_v` to prevent conflicts with descriptor's `inner_value()` default behavior.

**Updated `_Choose()` Function**: Added check for `_value_fn` strategy between ADL and default

**Updated `operator()`**: Handles `_value_fn` case with:
```cpp
else if constexpr (_Choice_t<G&, E>::_Strategy == _St::_value_fn) {
    return std::forward<E>(uv).value();
}
```

### 2. Edge/Vertex Value Classes ([include/graph/container/undirected_adjacency_list.hpp](include/graph/container/undirected_adjacency_list.hpp))

**Lines 244-257**: `ual_edge_value` class
- Changed public member `value` to private `value_`
- Added public accessor: `constexpr value_type& value() noexcept { return value_; }`
- Updated constructor: `: value_(val)`
- Fixed ADL friend functions to access `value_` instead of `value`

**Lines 294-307**: `ual_vertex_value` class  
- Applied same pattern as `ual_edge_value`
- Private `value_` with public `.value()` accessor

### 3. Implementation File Updates ([include/graph/container/detail/undirected_adjacency_list_impl.hpp](include/graph/container/detail/undirected_adjacency_list_impl.hpp))

**Line 1184**: Copy constructor - vertex value access
```cpp
vertices_.push_back(vertex_type(vertices_, static_cast<vertex_id_type>(vertices_.size()), v.value()));
```

**Line 1203**: Copy constructor - edge value access
```cpp
g.create_edge(src_key, tgt_key, uv->value());
```

### 4. Test File Updates

**[tests/test_undirected_adjacency_list.cpp](tests/test_undirected_adjacency_list.cpp)**: 
- Applied regex transformation to convert all `.value` member accesses to `.value()` function calls
- Pattern: `r'->value(?!\()'` → `'->value()'` and `r'\.value(?!\()'` → `'.value()'`
- Negative lookahead ensures no double parentheses on already-correct code

**[tests/test_edge_value_cpo_value_method.cpp](tests/test_edge_value_cpo_value_method.cpp)** (NEW):
- Created comprehensive tests for `.value()` CPO pattern recognition
- Verified mutable and const access through CPO
- Confirmed resolution priority order

## Resolution Priority

The `edge_value` CPO now resolves in this order:

1. **Member function** (`g.edge_value(uv)`) - Highest priority
2. **ADL** (`edge_value(g, uv)`) - High priority  
3. **`.value()` method** (`uv.value()`) - **NEW** - Medium priority
4. **Descriptor default** (`uv.inner_value(edges)`) - Lowest priority

## Testing

All tests pass: **36012 assertions in 3868 test cases**

Specific new tests:
- `edge_value CPO with .value() method` - Verifies CPO recognizes `.value()` pattern
- `edge_value CPO resolution priority` - Confirms correct resolution order

## Usage Example

```cpp
undirected_adjacency_list<int, int> g;
auto v1 = g.create_vertex(10);
auto v2 = g.create_vertex(20);

auto k1 = vertex_id(v1, g);
auto k2 = vertex_id(v2, g);

g.create_edge(k1, k2, 100);

// Get edge through vertex edge list
auto& v = g.vertices()[k1];
auto edge_it = v.edges(g, k1).begin();

// All three access methods work:
edge_it->value() = 999;              // Direct method call
edge_value(g, *edge_it) = 999;       // CPO (uses _value_fn strategy)
auto& ev = edge_value(g, *edge_it);  // CPO returns reference
```

## Technical Notes

1. **Private member pattern**: Changed from public `value` to private `value_` to enforce accessor usage
2. **Non-descriptor constraint**: Critical to prevent CPO from selecting `.value()` on edge descriptors which should use `inner_value()` default
3. **Perfect forwarding**: CPO uses `std::forward<E>(uv).value()` to preserve value category
4. **Reference semantics**: CPO returns references allowing modification through the abstraction

## Files Modified

- `include/graph/detail/graph_cpo.hpp` - CPO definition
- `include/graph/container/undirected_adjacency_list.hpp` - Value class refactoring
- `include/graph/container/detail/undirected_adjacency_list_impl.hpp` - Implementation updates
- `tests/test_undirected_adjacency_list.cpp` - Test updates
- `tests/test_edge_value_cpo_value_method.cpp` - New test file
- `tests/CMakeLists.txt` - Added new test to build

## Build Status

✅ Clean build with GCC 15.1.0
✅ All 3868 tests passing
✅ No compilation errors
⚠️ Minor warnings (pre-existing, unrelated to changes)
