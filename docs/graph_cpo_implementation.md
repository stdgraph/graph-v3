# Graph Container Interface CPO Implementation Guide

## Overview

This document provides a comprehensive guide for implementing the Graph Container Interface (GCI) using Customization Point Objects (CPOs) in `graph_cpo.hpp`. The implementation follows the P1709/D3130 specification, uses the MSVC-style CPO pattern, and integrates with the descriptor system.

**References:**
- [adjacency-list-interface.md](reference/adjacency-list-interface.md) - Adjacency list function signatures and specifications
- [edge-list-interface.md](reference/edge-list-interface.md) - Edge list function signatures and specifications
- [cpo.md](cpo.md) - CPO design patterns and best practices
- Descriptor headers: `vertex_descriptor.hpp`, `edge_descriptor.hpp`

## Project Structure

```
include/
  graph/
    graph_cpo.hpp              # Main CPO implementations
    descriptor_traits.hpp      # Type traits for descriptors
    vertex_descriptor.hpp      # Vertex descriptor template
    edge_descriptor.hpp        # Edge descriptor template
    concepts.hpp               # Graph concepts (optional)
```

## CPO Implementation Strategy

### Inner Value Priority Pattern

**IMPORTANT**: When implementing CPOs that operate on graph elements (vertices or edges), the resolution order MUST check the **inner_value** (the actual element stored in the container) before checking the descriptor wrapper.

#### Rationale
The descriptor is a lightweight wrapper around iterators/indices. The actual graph element (inner_value) should have the highest priority for defining its own behavior, with the descriptor serving only as a fallback when no customization exists on the element itself.

#### Resolution Order for Element-Based CPOs

For CPOs that operate on graph elements through descriptors:
- **Vertex CPOs**: `vertex_id(g, u)`, `edges(g, u)`, etc. where `u` is a vertex_descriptor
- **Edge CPOs**: `target_id(g, uv)`, `source_id(g, uv)`, `edge_value(g, uv)`, etc. where `uv` is an edge_descriptor

1. **Inner value member function** (highest priority)
   - Vertex example: `u.inner_value(g).vertex_id(g)` where `u` is a vertex_descriptor
   - Edge example: `uv.inner_value(g).target_id(g)` where `uv` is an edge_descriptor
   - Checks if the actual element data has a member function

2. **ADL with inner_value** 
   - Vertex example: `vertex_id(g, u.inner_value(g))`
   - Edge example: `target_id(g, uv.inner_value(g))`
   - ADL lookup using the actual element data type

3. **ADL with descriptor**
   - Vertex example: `vertex_id(g, u)`
   - Edge example: `target_id(g, uv)`
   - ADL lookup using the descriptor type

4. **Descriptor default or pattern extraction** (lowest priority)
   - Vertex example: `u.vertex_id()` - built-in descriptor functionality
   - Edge example: Pattern extraction (integral/pair/tuple) from `uv.inner_value(g)`
   - Fallback when no customization exists

Where:
- `u` is a `vertex_descriptor<Iter>` and `u.inner_value(g)` extracts the actual vertex data
  - For vectors: returns `g[u.value()]` (the actual vertex data)
  - For maps: returns `(*iter).second` (the value part of the key-value pair)
- `uv` is an `edge_descriptor<EdgeIter, VertexIter>` and `uv.inner_value(g)` extracts the actual edge data
  - For vector of vectors: returns the target ID (int)
  - For vector of pairs/tuples: returns the pair/tuple containing target and optional weight/data

#### Example: vertex_id(g, u)

```cpp
// User's custom vertex type
struct MyVertex {
    std::string name;
    int custom_id;
    
    // Highest priority: inner value member
    template<typename G>
    int vertex_id(const G& g) const {
        return custom_id;  // Use custom ID instead of index/key
    }
};

// Graph using custom vertices
std::vector<MyVertex> graph = {
    {"Alice", 100},
    {"Bob", 200},
    {"Charlie", 300}
};

// The CPO will call: u.inner_value(g).vertex_id(g)
// Returns: 100, 200, 300 (custom IDs)
// NOT: 0, 1, 2 (indices from descriptor default)
```

#### Example: target_id(g, uv)

```cpp
// User's custom edge type
struct MyEdge {
    int destination;
    double weight;
    
    // Highest priority: inner value member
    template<typename G>
    int target_id(const G& g) const {
        return destination;  // Custom extraction logic
    }
};

// Graph using custom edges
std::vector<std::vector<MyEdge>> graph = {
    {{1, 1.5}, {2, 2.5}},  // Vertex 0 -> edges to 1 and 2
    {{2, 3.5}},            // Vertex 1 -> edge to 2
    {}                     // Vertex 2 -> no edges
};

// The CPO will call: uv.inner_value(g).target_id(g)
// Returns target IDs using custom MyEdge::target_id(g) member
// Alternative: for pair<int, double>, pattern extraction uses .first
```

### MSVC-Style Pattern Summary

All CPOs follow this structure:

```cpp
namespace graph::_cpo {

// Shared choice struct (define once at top of file)
template<typename _Ty>
struct _Choice_t {
    _Ty _Strategy = _Ty{};
    bool _No_throw = false;
};

namespace _function_name {
    // 1. Strategy enum
    enum class _St { _none, _member, _adl, _default };
    
    // 2. Concepts for each path
    template<typename G>
    concept _has_member = requires(G& g) {
        { g.function_name() } -> /* constraint */;
    };
    
    template<typename G>
    concept _has_adl = requires(G& g) {
        { function_name(g) } -> /* constraint */;
    };
    
    // 3. Single compile-time choice evaluation
    template<typename G>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G>) {
            return {_St::_member, noexcept(std::declval<G>().function_name())};
        } else if constexpr (_has_adl<G>) {
            return {_St::_adl, noexcept(function_name(std::declval<G>()))};
        } else {
            return {_St::_default, /* noexcept of default impl */};
        }
    }
    
    // 4. CPO class with single operator()
    class _fn {
    private:
        template<typename G>
        static constexpr _Choice_t<_St> _Choice = _Choose<G>();
        
    public:
        template<typename G>
        [[nodiscard]] constexpr auto operator()(G&& g) const
            noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
        {
            using _G = std::remove_cvref_t<G>;
            if constexpr (_Choice<_G>._Strategy == _St::_member) {
                return std::forward<G>(g).function_name();
            } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                return function_name(std::forward<G>(g));
            } else {
                // Default implementation
            }
        }
    };
} // namespace _function_name

inline namespace _cpos {
    inline constexpr _function_name::_fn function_name{};
}

} // namespace graph::_cpo
```

## File Header and Namespace Structure

```cpp
/**
 * @file graph_cpo.hpp
 * @brief Graph Container Interface Customization Point Objects
 * 
 * Implements the P1709 Graph Container Interface using MSVC-style CPOs.
 * All functions are customization point objects that can be overridden
 * via member functions, ADL, or use default implementations.
 */

#pragma once

#include "vertex_descriptor.hpp"
#include "edge_descriptor.hpp"
#include <concepts>
#include <ranges>
#include <iterator>

namespace graph {

// Forward declarations for ADL
template<typename G> auto vertices(G&&) -> decltype(auto);
template<typename G, typename U> auto edges(G&&, U&&) -> decltype(auto);
// ... other forward declarations

namespace _cpo {

// Shared choice struct for all CPOs
template<typename _Ty>
struct _Choice_t {
    _Ty _Strategy = _Ty{};
    bool _No_throw = false;
};

// Individual CPO implementations follow...

} // namespace _cpo
} // namespace graph
```

## CPO Implementation Order Reference

**IMPORTANT:** The CPO implementations below are organized for teaching/learning purposes and DO NOT match the canonical implementation order. 

**For the correct implementation order,** see [graph_cpo_order.md](graph_cpo_order.md) which specifies:
- Canonical order: vertices → vertex_id → find_vertex → edges → num_edges → target_id → ...
- Proper dependency resolution
- Type alias placement

### Teaching Order vs Implementation Order Mapping

| Teaching Section | CPO Name | Canonical Order # | Notes |
|-----------------|----------|-------------------|-------|
| Section 1 | `vertices` | 1 | ✓ Matches |
| Section 2 | `edges` | 4 | Move later in actual implementation |
| Section 3 | `vertex_id` | 2 | Should be #2, not #3 |
| Section 4 | `target_id` | 6 | Move later in actual implementation |
| Section 5 | `num_vertices` | 13 | Much later in actual implementation |
| Section 6 | `find_vertex` | 3 | Should be #3, not #6 |
| Section 7 | `degree` | 14 | Much later in actual implementation |
| Section 8 | `target` | 8 | Move later in actual implementation |
| Section 9 | `num_edges` | 5 | Should be #5, not #9 |
| Section 10 | `vertex_value` | 15 | Matches relative position |
| Section 11 | `edge_value` | 16 | Matches relative position |

**When implementing `graph_cpo.hpp`,** follow the order in [graph_cpo_order.md](graph_cpo_order.md), not the section numbers below.

## CPO Implementations

Each CPO below includes complete code with the MSVC-style `_Choice_t` pattern. Use these implementations but place them in the canonical order.

### 1. `vertices(g)` - Get Vertex Range (Implementation Order: #1)

**Signature:** `vertex_range_t<G> vertices(G& g)`  
**Return:** `vertex_descriptor_view` wrapping vertices in the graph  
**Complexity:** O(1)  
**Default:** Returns `vertex_descriptor_view(g)` if `g` follows inner value patterns (is a range with inner value pattern support), otherwise no default

**IMPORTANT:** `vertices(g)` MUST always return a `vertex_descriptor_view`. The implementation should:
1. First check if `g.vertices()` member exists - if so, use it (must return `vertex_descriptor_view`)
2. Then check if ADL `vertices(g)` exists - if so, use it (must return `vertex_descriptor_view`)
3. Otherwise, if `g` follows inner value patterns (has a range of elements with inner value pattern support), return `vertex_descriptor_view(g)` wrapping the graph container itself

```cpp
namespace _vertices {
    enum class _St { _none, _member, _adl, _inner_value_pattern };
    
    template<typename G>
    concept _has_member = requires(G& g) {
        { g.vertices() } -> std::ranges::forward_range;
    };
    
    template<typename G>
    concept _has_adl = requires(G& g) {
        { vertices(g) } -> std::ranges::forward_range;
    };
    
    template<typename G>
    concept _has_inner_value_pattern = std::ranges::forward_range<G> && 
        requires { typename std::ranges::iterator_t<G>; } &&
        has_inner_value_pattern<std::ranges::iterator_t<G>>;
    
    template<typename G>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G>) {
            return {_St::_member, noexcept(std::declval<G&>().vertices())};
        } else if constexpr (_has_adl<G>) {
            return {_St::_adl, noexcept(vertices(std::declval<G&>()))};
        } else if constexpr (_has_inner_value_pattern<G>) {
            return {_St::_inner_value_pattern, true};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G>
        static constexpr _Choice_t<_St> _Choice = _Choose<G>();
        
    public:
        template<typename G>
        [[nodiscard]] constexpr auto operator()(G&& g) const
            noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
            -> decltype(auto)
        {
            using _G = std::remove_cvref_t<G>;
            if constexpr (_Choice<_G>._Strategy == _St::_member) {
                return std::forward<G>(g).vertices();
            } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                return vertices(std::forward<G>(g));
            } else if constexpr (_Choice<_G>._Strategy == _St::_inner_value_pattern) {
                return vertex_descriptor_view(std::forward<G>(g));
            } else {
                static_assert(_Choice<_G>._Strategy != _St::_none,
                    "vertices(g) requires .vertices() member, ADL vertices(), or inner value pattern support");
            }
        }
    };
} // namespace _vertices

inline namespace _cpos {
    inline constexpr _vertices::_fn vertices{};
}
```

### 2. `edges(g, u)` - Get Edge Range for Vertex (Implementation Order: #4)

**Signature:** `vertex_edge_range_t<G> edges(G& g, vertex_t<G> u)`  
**Return:** `edge_descriptor_view` wrapping outgoing edges from vertex `u`  
**Complexity:** O(1)  
**Default:** Returns `edge_descriptor_view(u.inner_value(g), u)` if the vertex descriptor's inner value is a forward range with edge value pattern support, otherwise no default

**IMPORTANT:** `edges(g, u)` MUST always return an `edge_descriptor_view`.

**Resolution Order** (follows inner_value priority pattern):
1. `u.inner_value(g).edges(g)` - Inner value member (highest priority)
2. `edges(g, u.inner_value(g))` - ADL with inner_value
3. `edges(g, u)` - ADL with descriptor
4. `edge_descriptor_view(u.inner_value(g), u)` - Default if inner_value has edge pattern (lowest priority)

```cpp
namespace _edges {
    enum class _St { _none, _inner_value_member, _adl_inner_value, _adl_descriptor, _edge_value_pattern };
    
    // Check for inner_value.edges(g) member function
    template<typename G, typename U>
    concept _has_inner_value_member = 
        is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
        requires(G& g, const U& u) {
            { u.inner_value(g).edges(g) } -> std::ranges::forward_range;
        };
    
    // Check for ADL edges(g, inner_value)
    template<typename G, typename U>
    concept _has_adl_inner_value = 
        is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
        requires(G& g, const U& u) {
            { edges(g, u.inner_value(g)) } -> std::ranges::forward_range;
        };
    
    // Check for ADL edges(g, descriptor)
    template<typename G, typename U>
    concept _has_adl_descriptor = 
        is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
        requires(G& g, const U& u) {
            { edges(g, u) } -> std::ranges::forward_range;
        };
    
    // Check if vertex descriptor's inner value is a range of edge values
    template<typename G, typename U>
    concept _has_edge_value_pattern = 
        is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
        requires(G& g, const U& u) {
            { u.inner_value(g) } -> std::ranges::forward_range;
        } && requires(G& g, const U& u) {
            typename std::ranges::range_value_t<decltype(u.inner_value(g))>;
        } && edge_value_type<std::ranges::range_value_t<decltype(std::declval<U>().inner_value(std::declval<G&>()))>>;
    
    template<typename G, typename U>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_inner_value_member<G, U>) {
            return {_St::_inner_value_member, 
                    noexcept(std::declval<U&>().inner_value(std::declval<G&>()).edges(std::declval<G&>()))};
        } else if constexpr (_has_adl_inner_value<G, U>) {
            return {_St::_adl_inner_value, 
                    noexcept(edges(std::declval<G&>(), std::declval<U&>().inner_value(std::declval<G&>())))};
        } else if constexpr (_has_adl_descriptor<G, U>) {
            return {_St::_adl_descriptor, 
                    noexcept(edges(std::declval<G&>(), std::declval<const U&>()))};
        } else if constexpr (_has_edge_value_pattern<G, U>) {
            return {_St::_edge_value_pattern, true};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G, typename U>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, U>();
        
    public:
        template<typename G, typename U>
        [[nodiscard]] constexpr auto operator()(G& g, const U& u) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
            -> decltype(auto)
        {
            using _G = std::remove_cvref_t<G>;
            using _U = std::remove_cvref_t<U>;
            
            if constexpr (_Choice<_G, _U>._Strategy == _St::_inner_value_member) {
                return u.inner_value(g).edges(g);
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl_inner_value) {
                return edges(g, u.inner_value(g));
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl_descriptor) {
                return edges(g, u);
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_edge_value_pattern) {
                return edge_descriptor_view(u.inner_value(g), u);
            } else {
                static_assert(_Choice<_G, _U>._Strategy != _St::_none,
                    "edges(g,u) requires inner_value.edges(g), ADL edges(), or edge value pattern support");
            }
        }
    };
} // namespace _edges

inline namespace _cpos {
    inline constexpr _edges::_fn edges{};
}
```

### 3. `vertex_id(g, u)` - Get Vertex ID (Implementation Order: #2)

**Signature:** `vertex_id_t<G> vertex_id(G& g, vertex_t<G> u)`  
**Return:** Unique identifier for vertex `u`  
**Complexity:** O(1)  
**Default:** Returns descriptor's index for random-access, or key from pair for associative

**Resolution Order** (follows inner_value priority pattern):
1. `u.inner_value(g).vertex_id(g)` - Inner value member (highest priority)
2. `vertex_id(g, u.inner_value(g))` - ADL with inner_value
3. `vertex_id(g, u)` - ADL with descriptor
4. `u.vertex_id()` - Descriptor default (lowest priority)

```cpp
namespace _vertex_id {
    enum class _St { _none, _inner_value_member, _adl_inner_value, _adl_descriptor, _descriptor };
    
    // Check for inner_value.vertex_id(g) member function
    template<typename G, typename U>
    concept _has_inner_value_member = 
        is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
        requires(G& g, const U& u) {
            { u.inner_value(g).vertex_id(g) };
        };
    
    // Check for ADL vertex_id(g, inner_value)
    template<typename G, typename U>
    concept _has_adl_inner_value = 
        is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
        requires(G& g, const U& u) {
            { vertex_id(g, u.inner_value(g)) };
        };
    
    // Check for ADL vertex_id(g, descriptor)
    template<typename G, typename U>
    concept _has_adl_descriptor = 
        is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
        requires(const G& g, const U& u) {
            { vertex_id(g, u) };
        };
    
    // Check if descriptor has vertex_id() member (default)
    template<typename U>
    concept _has_descriptor = is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
        requires(const U& u) {
            { u.vertex_id() };
        };
    
    template<typename G, typename U>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_inner_value_member<G, U>) {
            return {_St::_inner_value_member, 
                    noexcept(std::declval<U&>().inner_value(std::declval<G&>()).vertex_id(std::declval<const G&>()))};
        } else if constexpr (_has_adl_inner_value<G, U>) {
            return {_St::_adl_inner_value, 
                    noexcept(vertex_id(std::declval<G&>(), std::declval<U&>().inner_value(std::declval<G&>())))};
        } else if constexpr (_has_adl_descriptor<G, U>) {
            return {_St::_adl_descriptor, 
                    noexcept(vertex_id(std::declval<const G&>(), std::declval<const U&>()))};
        } else if constexpr (_has_descriptor<U>) {
            return {_St::_descriptor, 
                    noexcept(std::declval<const U&>().vertex_id())};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G, typename U>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, U>();
        
    public:
        template<typename G, typename U>
        [[nodiscard]] constexpr auto operator()(G& g, const U& u) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
            -> decltype(auto)
        {
            using _G = std::remove_cvref_t<G>;
            using _U = std::remove_cvref_t<U>;
            
            if constexpr (_Choice<_G, _U>._Strategy == _St::_inner_value_member) {
                return u.inner_value(g).vertex_id(g);
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl_inner_value) {
                return vertex_id(g, u.inner_value(g));
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl_descriptor) {
                return vertex_id(g, u);
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_descriptor) {
                return u.vertex_id();
            } else {
                static_assert(_Choice<_G, _U>._Strategy != _St::_none,
                    "vertex_id(g,u) requires inner_value.vertex_id(g), ADL vertex_id(), or descriptor.vertex_id()");
            }
        }
    };
} // namespace _vertex_id

inline namespace _cpos {
    inline constexpr _vertex_id::_fn vertex_id{};
}
```

### 4. `target_id(g, uv)` - Get Target Vertex ID from Edge (Implementation Order: #6)

**Signature:** `vertex_id_t<G> target_id(G& g, edge_t<G> uv)`  
**Return:** Target vertex ID of edge `uv`  
**Complexity:** O(1)  
**Default:** Pattern-based extraction for integral, pair, or tuple edge values from inner_value

**Resolution Order** (follows inner_value priority pattern):
1. `uv.inner_value(g).target_id(g)` - Inner value member (highest priority)
2. `target_id(g, uv.inner_value(g))` - ADL with inner_value
3. `target_id(g, uv)` - ADL with descriptor
4. Pattern extraction from `uv.inner_value(g)` - Default (lowest priority):
   - Integral: returns the value itself
   - Pair: returns `.first`
   - Tuple: returns `std::get<0>(...)`

Where:
- `uv` is an `edge_descriptor<EdgeIter, VertexIter>`
- `uv.inner_value(g)` extracts the actual edge data from the graph

```cpp
namespace _target_id {
    enum class _St { _none, _inner_value_member, _adl_inner_value, _adl_descriptor, 
                     _integral, _pair, _tuple };
    
    // Check for inner_value.target_id(g) member function
    template<typename G, typename E>
    concept _has_inner_value_member = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { uv.inner_value(g).target_id(g) };
        };
    
    // Check for ADL target_id(g, inner_value)
    template<typename G, typename E>
    concept _has_adl_inner_value = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { target_id(g, uv.inner_value(g)) };
        };
    
    // Check for ADL target_id(g, descriptor)
    template<typename G, typename E>
    concept _has_adl_descriptor = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(const G& g, const E& uv) {
            { target_id(g, uv) };
        };
    
    // Check if edge inner_value is integral (simple target_id)
    template<typename G, typename E>
    concept _is_integral = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { uv.inner_value(g) } -> std::integral;
        };
    
    // Check if edge inner_value is pair-like
    template<typename G, typename E>
    concept _is_pair = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { uv.inner_value(g).first };
            { uv.inner_value(g).second };
        };
    
    // Check if edge inner_value is tuple-like
    template<typename G, typename E>
    concept _is_tuple = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            typename std::tuple_size<std::remove_cvref_t<decltype(uv.inner_value(g))>>::type;
        } && (std::tuple_size_v<std::remove_cvref_t<decltype(std::declval<E>().inner_value(std::declval<G&>()))>> >= 1);
    
    template<typename G, typename E>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_inner_value_member<G, E>) {
            return {_St::_inner_value_member, 
                    noexcept(std::declval<E&>().inner_value(std::declval<G&>()).target_id(std::declval<const G&>()))};
        } else if constexpr (_has_adl_inner_value<G, E>) {
            return {_St::_adl_inner_value, 
                    noexcept(target_id(std::declval<G&>(), std::declval<E&>().inner_value(std::declval<G&>())))};
        } else if constexpr (_has_adl_descriptor<G, E>) {
            return {_St::_adl_descriptor, 
                    noexcept(target_id(std::declval<const G&>(), std::declval<const E&>()))};
        } else if constexpr (_is_integral<G, E>) {
            return {_St::_integral, true};
        } else if constexpr (_is_pair<G, E>) {
            return {_St::_pair, noexcept(std::declval<E&>().inner_value(std::declval<G&>()).first)};
        } else if constexpr (_is_tuple<G, E>) {
            return {_St::_tuple, noexcept(std::get<0>(std::declval<E&>().inner_value(std::declval<G&>())))};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G, typename E>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, E>();
        
    public:
        template<typename G, typename E>
        [[nodiscard]] constexpr auto operator()(G& g, const E& uv) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._No_throw)
            -> decltype(auto)
        {
            using _G = std::remove_cvref_t<G>;
            using _E = std::remove_cvref_t<E>;
            
            if constexpr (_Choice<_G, _E>._Strategy == _St::_inner_value_member) {
                return uv.inner_value(g).target_id(g);
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl_inner_value) {
                return target_id(g, uv.inner_value(g));
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl_descriptor) {
                return target_id(g, uv);
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_integral) {
                return uv.inner_value(g);  // Edge value itself is the target_id
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_pair) {
                return uv.inner_value(g).first;  // First element is target_id
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_tuple) {
                return std::get<0>(uv.inner_value(g));  // First tuple element is target_id
            } else {
                static_assert(_Choice<_G, _E>._Strategy != _St::_none,
                    "target_id(g,uv) requires inner_value.target_id(g), ADL target_id(), "
                    "or edge inner_value as integral/pair/tuple with target_id as first element");
            }
        }
    };
} // namespace _target_id

inline namespace _cpos {
    inline constexpr _target_id::_fn target_id{};
}
```

## Priority 2: Vertex Query Functions

### 5. `num_vertices(g)` - Count Vertices (Implementation Order: #13)

**Signature:** `integral num_vertices(G& g)`  
**Return:** Number of vertices in the graph  
**Complexity:** O(1)  
**Default:** `size(vertices(g))`

```cpp
namespace _num_vertices {
    enum class _St { _none, _member, _adl, _default };
    
    template<typename G>
    concept _has_member = requires(const G& g) {
        { g.num_vertices() } -> std::integral;
    };
    
    template<typename G>
    concept _has_adl = requires(const G& g) {
        { num_vertices(g) } -> std::integral;
    };
    
    template<typename G>
    concept _has_default = requires(G& g) {
        { std::ranges::size(vertices(g)) } -> std::integral;
    };
    
    template<typename G>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G>) {
            return {_St::_member, noexcept(std::declval<const G&>().num_vertices())};
        } else if constexpr (_has_adl<G>) {
            return {_St::_adl, noexcept(num_vertices(std::declval<const G&>()))};
        } else if constexpr (_has_default<G>) {
            return {_St::_default, 
                    noexcept(std::ranges::size(vertices(std::declval<G&>())))};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G>
        static constexpr _Choice_t<_St> _Choice = _Choose<G>();
        
    public:
        template<typename G>
        [[nodiscard]] constexpr auto operator()(const G& g) const
            noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
        {
            using _G = std::remove_cvref_t<G>;
            if constexpr (_Choice<_G>._Strategy == _St::_member) {
                return g.num_vertices();
            } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                return num_vertices(g);
            } else if constexpr (_Choice<_G>._Strategy == _St::_default) {
                return std::ranges::size(vertices(g));
            } else {
                static_assert(_Choice<_G>._Strategy != _St::_none,
                    "num_vertices(g) requires .num_vertices() member, ADL num_vertices(), "
                    "or sized vertices(g) range");
            }
        }
    };
} // namespace _num_vertices

inline namespace _cpos {
    inline constexpr _num_vertices::_fn num_vertices{};
}
```

### 6. `find_vertex(g, uid)` - Find Vertex by ID (Implementation Order: #3)

**Signature:** `vertex_iterator_t<G> find_vertex(G& g, vertex_id_t<G> uid)`  
**Return:** Iterator to vertex with ID `uid`  
**Complexity:** O(1) for random-access, O(log n) for ordered, O(1) average for hash-based  
**Default:** `begin(vertices(g)) + uid` if random-access

```cpp
namespace _find_vertex {
    enum class _St { _none, _member, _adl, _random_access };
    
    template<typename G, typename VId>
    concept _has_member = requires(G& g, const VId& uid) {
        { g.find_vertex(uid) };
    };
    
    template<typename G, typename VId>
    concept _has_adl = requires(G& g, const VId& uid) {
        { find_vertex(g, uid) };
    };
    
    template<typename G>
    concept _has_random_access = requires(G& g) {
        { vertices(g) } -> std::ranges::random_access_range;
    };
    
    template<typename G, typename VId>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G, VId>) {
            return {_St::_member, 
                    noexcept(std::declval<G&>().find_vertex(std::declval<const VId&>()))};
        } else if constexpr (_has_adl<G, VId>) {
            return {_St::_adl, 
                    noexcept(find_vertex(std::declval<G&>(), std::declval<const VId&>()))};
        } else if constexpr (_has_random_access<G>) {
            return {_St::_random_access, 
                    noexcept(std::ranges::begin(vertices(std::declval<G&>())))};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G, typename VId>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, VId>();
        
    public:
        template<typename G, typename VId>
        [[nodiscard]] constexpr auto operator()(G&& g, const VId& uid) const
            noexcept(_Choice<std::remove_cvref_t<G>, VId>._No_throw)
            -> decltype(auto)
        {
            using _G = std::remove_cvref_t<G>;
            if constexpr (_Choice<_G, VId>._Strategy == _St::_member) {
                return std::forward<G>(g).find_vertex(uid);
            } else if constexpr (_Choice<_G, VId>._Strategy == _St::_adl) {
                return find_vertex(std::forward<G>(g), uid);
            } else if constexpr (_Choice<_G, VId>._Strategy == _St::_random_access) {
                return std::ranges::begin(vertices(std::forward<G>(g))) + uid;
            } else {
                static_assert(_Choice<_G, VId>._Strategy != _St::_none,
                    "find_vertex(g,uid) requires .find_vertex() member, ADL find_vertex(), "
                    "or random_access vertices(g) range");
            }
        }
    };
} // namespace _find_vertex

inline namespace _cpos {
    inline constexpr _find_vertex::_fn find_vertex{};
}
```

### 7. `degree(g, u)` - Get Vertex Degree (Implementation Order: #14)

**Signature:** `integral degree(G& g, vertex_t<G> u)`  
**Return:** Number of outgoing edges from vertex `u`  
**Complexity:** O(1) if edges are sized, O(degree) otherwise  
**Default:** `size(edges(g, u))` if sized_range

```cpp
namespace _degree {
    enum class _St { _none, _member, _adl, _default };
    
    template<typename G, typename U>
    concept _has_member = requires(const G& g, const U& u) {
        { g.degree(u) } -> std::integral;
    };
    
    template<typename G, typename U>
    concept _has_adl = requires(const G& g, const U& u) {
        { degree(g, u) } -> std::integral;
    };
    
    template<typename G, typename U>
    concept _has_default = requires(G& g, U&& u) {
        { std::ranges::size(edges(g, std::forward<U>(u))) } -> std::integral;
    };
    
    template<typename G, typename U>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G, U>) {
            return {_St::_member, 
                    noexcept(std::declval<const G&>().degree(std::declval<const U&>()))};
        } else if constexpr (_has_adl<G, U>) {
            return {_St::_adl, 
                    noexcept(degree(std::declval<const G&>(), std::declval<const U&>()))};
        } else if constexpr (_has_default<G, U>) {
            return {_St::_default, 
                    noexcept(std::ranges::size(edges(std::declval<G&>(), std::declval<U>())))};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G, typename U>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, U>();
        
    public:
        template<typename G, typename U>
        [[nodiscard]] constexpr auto operator()(const G& g, const U& u) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
        {
            using _G = std::remove_cvref_t<G>;
            using _U = std::remove_cvref_t<U>;
            
            if constexpr (_Choice<_G, _U>._Strategy == _St::_member) {
                return g.degree(u);
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl) {
                return degree(g, u);
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_default) {
                return std::ranges::size(edges(g, u));
            } else {
                static_assert(_Choice<_G, _U>._Strategy != _St::_none,
                    "degree(g,u) requires .degree() member, ADL degree(), or sized edges(g,u)");
            }
        }
    };
} // namespace _degree

inline namespace _cpos {
    inline constexpr _degree::_fn degree{};
}
```

## Priority 3: Edge Query Functions

### 8. `target(g, uv)` - Get Target Vertex Descriptor (Implementation Order: #8)

**Signature:** `vertex_t<G> target(G& g, edge_t<G> uv)`  
**Return:** Target vertex descriptor for edge `uv`  
**Complexity:** O(1)  
**Default:** `*find_vertex(g, target_id(g, uv))` if available

```cpp
namespace _target {
    enum class _St { _none, _member, _adl, _default };
    
    template<typename G, typename E>
    concept _has_member = requires(G& g, const E& uv) {
        { g.target(uv) };
    };
    
    template<typename G, typename E>
    concept _has_adl = requires(G& g, const E& uv) {
        { target(g, uv) };
    };
    
    template<typename G, typename E>
    concept _has_default = requires(G& g, const E& uv) {
        { find_vertex(g, target_id(g, uv)) };
    };
    
    template<typename G, typename E>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G, E>) {
            return {_St::_member, 
                    noexcept(std::declval<G&>().target(std::declval<const E&>()))};
        } else if constexpr (_has_adl<G, E>) {
            return {_St::_adl, 
                    noexcept(target(std::declval<G&>(), std::declval<const E&>()))};
        } else if constexpr (_has_default<G, E>) {
            constexpr bool nothrow_target_id = 
                noexcept(target_id(std::declval<const G&>(), std::declval<const E&>()));
            constexpr bool nothrow_find = 
                noexcept(find_vertex(std::declval<G&>(), 
                         target_id(std::declval<const G&>(), std::declval<const E&>())));
            return {_St::_default, nothrow_target_id && nothrow_find};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G, typename E>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, E>();
        
    public:
        template<typename G, typename E>
        [[nodiscard]] constexpr auto operator()(G&& g, const E& uv) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._No_throw)
            -> decltype(auto)
        {
            using _G = std::remove_cvref_t<G>;
            using _E = std::remove_cvref_t<E>;
            
            if constexpr (_Choice<_G, _E>._Strategy == _St::_member) {
                return std::forward<G>(g).target(uv);
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl) {
                return target(std::forward<G>(g), uv);
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_default) {
                return *find_vertex(std::forward<G>(g), target_id(g, uv));
            } else {
                static_assert(_Choice<_G, _E>._Strategy != _St::_none,
                    "target(g,uv) requires .target() member, ADL target(), "
                    "or available target_id() and find_vertex()");
            }
        }
    };
} // namespace _target

inline namespace _cpos {
    inline constexpr _target::_fn target{};
}
```

### 9. `num_edges(g)` - Count Total Edges (Implementation Order: #5)

**Signature:** `integral num_edges(G& g)`  
**Return:** Total number of edges in the graph  
**Complexity:** O(|E|) for default, can be O(1) with member override  
**Default:** Sum of `distance(edges(g, u))` for all vertices

```cpp
namespace _num_edges {
    enum class _St { _none, _member, _adl, _default };
    
    template<typename G>
    concept _has_member = requires(const G& g) {
        { g.num_edges() } -> std::integral;
    };
    
    template<typename G>
    concept _has_adl = requires(const G& g) {
        { num_edges(g) } -> std::integral;
    };
    
    template<typename G>
    concept _has_default = requires(G& g) {
        { vertices(g) } -> std::ranges::forward_range;
    };
    
    template<typename G>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member<G>) {
            return {_St::_member, noexcept(std::declval<const G&>().num_edges())};
        } else if constexpr (_has_adl<G>) {
            return {_St::_adl, noexcept(num_edges(std::declval<const G&>()))};
        } else if constexpr (_has_default<G>) {
            return {_St::_default, false};  // Default impl may throw
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G>
        static constexpr _Choice_t<_St> _Choice = _Choose<G>();
        
    public:
        template<typename G>
        [[nodiscard]] constexpr auto operator()(const G& g) const
            noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
        {
            using _G = std::remove_cvref_t<G>;
            if constexpr (_Choice<_G>._Strategy == _St::_member) {
                return g.num_edges();
            } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                return num_edges(g);
            } else if constexpr (_Choice<_G>._Strategy == _St::_default) {
                std::size_t count = 0;
                for (auto&& u : vertices(g)) {
                    count += std::ranges::distance(edges(g, u));
                }
                return count;
            } else {
                static_assert(_Choice<_G>._Strategy != _St::_none,
                    "num_edges(g) requires .num_edges() member, ADL num_edges(), or vertices(g)");
            }
        }
    };
} // namespace _num_edges

inline namespace _cpos {
    inline constexpr _num_edges::_fn num_edges{};
}
```

## Priority 4: Optional Value Functions

These functions provide access to optional user-defined values.

### 10. `vertex_value(g, u)` - Get Vertex Value (Implementation Order: #15)

**Signature:** `vertex_value_t<G> vertex_value(G& g, vertex_t<G> u)`  
**Return:** User-defined value associated with vertex `u`  
**Complexity:** O(1)  
**Default:** None (must be provided by graph implementation)

```cpp
namespace _vertex_value {
    enum class _St { _none, _member_g_u, _member_u, _adl };
    
    template<typename G, typename U>
    concept _has_member_g_u = requires(G& g, U&& u) {
        { g.vertex_value(std::forward<U>(u)) };
    };
    
    template<typename U>
    concept _has_member_u = requires(U&& u) {
        { u.value() };
    };
    
    template<typename G, typename U>
    concept _has_adl = requires(G& g, U&& u) {
        { vertex_value(g, std::forward<U>(u)) };
    };
    
    template<typename G, typename U>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_member_g_u<G, U>) {
            return {_St::_member_g_u, 
                    noexcept(std::declval<G&>().vertex_value(std::declval<U>()))};
        } else if constexpr (_has_member_u<U>) {
            return {_St::_member_u, 
                    noexcept(std::declval<U>().value())};
        } else if constexpr (_has_adl<G, U>) {
            return {_St::_adl, 
                    noexcept(vertex_value(std::declval<G&>(), std::declval<U>()))};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G, typename U>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, U>();
        
    public:
        template<typename G, typename U>
        [[nodiscard]] constexpr auto operator()(G&& g, U&& u) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
            -> decltype(auto)
        {
            using _G = std::remove_cvref_t<G>;
            using _U = std::remove_cvref_t<U>;
            
            if constexpr (_Choice<_G, _U>._Strategy == _St::_member_g_u) {
                return std::forward<G>(g).vertex_value(std::forward<U>(u));
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_member_u) {
                return std::forward<U>(u).value();
            } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl) {
                return vertex_value(std::forward<G>(g), std::forward<U>(u));
            } else {
                static_assert(_Choice<_G, _U>._Strategy != _St::_none,
                    "vertex_value(g,u) is optional and requires explicit implementation");
            }
        }
    };
} // namespace _vertex_value

inline namespace _cpos {
    inline constexpr _vertex_value::_fn vertex_value{};
}
```

### 11. `edge_value(g, uv)` - Get Edge Value (Implementation Order: #16)

**Signature:** `edge_value_t<G> edge_value(G& g, edge_t<G> uv)`  
**Return:** User-defined value associated with edge `uv`  
**Complexity:** O(1)  
**Default:** Returns `uv.inner_value(g)` (the edge data itself)

**Resolution Order** (follows inner_value priority pattern):
1. `uv.inner_value(g).edge_value(g)` - Inner value member (highest priority)
2. `edge_value(g, uv.inner_value(g))` - ADL with inner_value
3. `edge_value(g, uv)` - ADL with descriptor
4. `uv.inner_value(g)` - Default (returns edge data itself, lowest priority)

Where:
- `uv` is an `edge_descriptor<EdgeIter, VertexIter>`
- `uv.inner_value(g)` extracts the actual edge data

```cpp
namespace _edge_value {
    enum class _St { _none, _inner_value_member, _adl_inner_value, _adl_descriptor, _inner_value };
    
    // Check for inner_value.edge_value(g) member function
    template<typename G, typename E>
    concept _has_inner_value_member = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { uv.inner_value(g).edge_value(g) };
        };
    
    // Check for ADL edge_value(g, inner_value)
    template<typename G, typename E>
    concept _has_adl_inner_value = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { edge_value(g, uv.inner_value(g)) };
        };
    
    // Check for ADL edge_value(g, descriptor)
    template<typename G, typename E>
    concept _has_adl_descriptor = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { edge_value(g, uv) };
        };
    
    // Default: return inner_value itself
    template<typename G, typename E>
    concept _has_inner_value = 
        is_edge_descriptor_v<std::remove_cvref_t<E>> &&
        requires(G& g, const E& uv) {
            { uv.inner_value(g) };
        };
    
    template<typename G, typename E>
    [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
        if constexpr (_has_inner_value_member<G, E>) {
            return {_St::_inner_value_member, 
                    noexcept(std::declval<E&>().inner_value(std::declval<G&>()).edge_value(std::declval<const G&>()))};
        } else if constexpr (_has_adl_inner_value<G, E>) {
            return {_St::_adl_inner_value, 
                    noexcept(edge_value(std::declval<G&>(), std::declval<E&>().inner_value(std::declval<G&>())))};
        } else if constexpr (_has_adl_descriptor<G, E>) {
            return {_St::_adl_descriptor, 
                    noexcept(edge_value(std::declval<G&>(), std::declval<const E&>()))};
        } else if constexpr (_has_inner_value<G, E>) {
            return {_St::_inner_value, true};
        } else {
            return {_St::_none, false};
        }
    }
    
    class _fn {
    private:
        template<typename G, typename E>
        static constexpr _Choice_t<_St> _Choice = _Choose<G, E>();
        
    public:
        template<typename G, typename E>
        [[nodiscard]] constexpr auto operator()(G& g, const E& uv) const
            noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._No_throw)
            -> decltype(auto)
        {
            using _G = std::remove_cvref_t<G>;
            using _E = std::remove_cvref_t<E>;
            
            if constexpr (_Choice<_G, _E>._Strategy == _St::_inner_value_member) {
                return uv.inner_value(g).edge_value(g);
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl_inner_value) {
                return edge_value(g, uv.inner_value(g));
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl_descriptor) {
                return edge_value(g, uv);
            } else if constexpr (_Choice<_G, _E>._Strategy == _St::_inner_value) {
                return uv.inner_value(g);  // Return edge data itself
            } else {
                static_assert(_Choice<_G, _E>._Strategy != _St::_none,
                    "edge_value(g,uv) requires inner_value.edge_value(g), ADL edge_value(), or edge descriptor");
            }
        }
    };
} // namespace _edge_value

inline namespace _cpos {
    inline constexpr _edge_value::_fn edge_value{};
}
```

## Implementation Checklist

### Phase 1: Foundation (Priority 1)
- [x] Set up file structure and namespaces
- [x] Define `_Choice_t<_Ty>` struct
- [x] Implement `vertices(g)` ✅ **COMPLETE** - 18 tests passing (including 4 custom implementation tests)
- [x] Define type aliases: `vertex_range_t<G>`, `vertex_iterator_t<G>`, `vertex_t<G>` ✅ **COMPLETE** - 5 tests passing
- [x] Implement `vertex_id(g, u)` ✅ **COMPLETE** - 14 tests passing (including custom member and ADL tests)
- [x] Define type alias: `vertex_id_t<G>` ✅ **COMPLETE** - 2 tests passing
- [ ] Implement `find_vertex(g, uid)`
- [ ] Implement `edges(g, u)`
- [ ] Define type aliases: `vertex_edge_range_t<G>`, `vertex_edge_iterator_t<G>`, `edge_descriptor_t<G>`, `edge_t<G>`
- [ ] Implement `target_id(g, uv)`
- [x] Create basic tests for `vertices(g)` ✅ **COMPLETE**
- [x] Create tests for `vertex_id(g, u)` ✅ **COMPLETE**
- [x] Create tests for type aliases ✅ **COMPLETE**

### Phase 2: Vertex Queries (Priority 2)
- [ ] Implement `num_vertices(g)`
- [ ] Implement `find_vertex(g, uid)`
- [ ] Implement `degree(g, u)`
- [ ] Create tests with various adjacency list types

### Phase 3: Edge Queries (Priority 3)
- [ ] Implement `target(g, uv)`
- [ ] Implement `num_edges(g)`
- [ ] Implement `find_vertex_edge(g, u, vid)`
- [ ] Implement `contains_edge(g, uid, vid)`
- [ ] Create comprehensive edge query tests

### Phase 4: Optional Values (Priority 4)
- [ ] Implement `vertex_value(g, u)`
- [ ] Implement `edge_value(g, uv)`
- [ ] Implement `graph_value(g)`
- [ ] Create tests with valued graphs

### Phase 5: Advanced Features (Future)
- [ ] Partition support: `num_partitions(g)`, `partition_id(g, u)`
- [ ] Sourced edges: `source_id(g, uv)`, `source(g, uv)`
- [ ] Convenience overloads: `edges(g, uid)`, `degree(g, uid)`, etc.
- [ ] Adjacency matrix optimizations

## Testing Strategy

### Test with All Adjacency List Types

Use the 23 adjacency list types from `adjacency-list-interface.md` for comprehensive testing:

1. **Types 1-5**: Basic patterns (unweighted, weighted pair/tuple, custom struct)
2. **Types 6-11**: Forward/bidirectional edge iterators
3. **Types 12-13**: Deque-based storage
4. **Types 14-17**: Map-based sparse vertices
5. **Types 18-21**: Unordered map-based hash vertices
6. **Types 22-23**: List-based linked storage

### Test Cases per CPO

For each CPO, test:
1. **Member function customization**: Graph with custom `.function()` member
2. **ADL customization**: Free function `function(g, ...)` found via ADL
3. **Default behavior**: Standard container patterns (vector, map, etc.)
4. **Pattern matching**: Integral, pair, tuple edge values for `target_id`
5. **Noexcept propagation**: Verify correct noexcept specification
6. **Constexpr evaluation**: Ensure compile-time evaluation where possible

### Example Test Structure

```cpp
#include <graph/graph_cpo.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <map>

TEST_CASE("vertices(g) - basic adjacency list", "[graph_cpo][vertices]") {
    using AdjList = std::vector<std::vector<int>>;
    AdjList g = {{1, 2}, {0, 2}, {0, 1}};
    
    auto verts = graph::vertices(g);
    REQUIRE(std::ranges::size(verts) == 3);
}

TEST_CASE("edges(g,u) - vector of lists", "[graph_cpo][edges]") {
    using AdjList = std::vector<std::list<int>>;
    AdjList g(3);
    g[0] = {1, 2};
    
    auto edge_range = graph::edges(g, 0);
    REQUIRE(std::ranges::distance(edge_range) == 2);
}

TEST_CASE("target_id(g,uv) - pair pattern", "[graph_cpo][target_id]") {
    using AdjList = std::vector<std::vector<std::pair<int, double>>>;
    AdjList g = {{{1, 1.5}, {2, 2.5}}};
    
    auto edges = graph::edges(g, 0);
    auto it = std::ranges::begin(edges);
    REQUIRE(graph::target_id(g, *it) == 1);
}
```

## Integration with Descriptors

The CPOs work seamlessly with the descriptor system:

```cpp
// Using vertex_descriptor
using VIter = std::vector<VertexData>::iterator;
using VDesc = graph::vertex_descriptor<VIter>;

std::vector<VertexData> vertices = /* ... */;
VDesc vd{2};  // Vertex at index 2

// CPO automatically extracts vertex_id from descriptor
auto vid = graph::vertex_id(g, vd);  // Calls vd.vertex_id()

// Using edge_descriptor
using EIter = std::vector<int>::iterator;
using EDesc = graph::edge_descriptor<EIter, VIter>;

auto edges_range = graph::edges(g, vd);
auto edge_it = std::ranges::begin(edges_range);
EDesc ed{0, vd};  // Edge at index 0 from vertex vd

auto target_vid = graph::target_id(g, ed);
```

## Best Practices

1. **Always use `std::remove_cvref_t`** when caching choices or checking strategies
2. **Forward parameters appropriately** in member/ADL calls
3. **Use `-> decltype(auto)`** for flexible return types that preserve references
4. **Test noexcept propagation** - verify `_No_throw` is correct
5. **Document complexity** in comments for each CPO
6. **Provide clear static_assert messages** for unsupported types
7. **Follow MSVC pattern consistently** for maintainability
8. **Use pattern matching** for common edge value types (integral, pair, tuple)
9. **Consider constexpr** for all CPOs where possible
10. **Test with sanitizers** (address, undefined behavior) during development

## Future Extensions

### Edgelist Interface
Separate namespace `graph::edgelist::` for edgelist CPOs:
- `source_id(e)`, `target_id(e)`, `edge_value(e)`
- Operates on edge ranges rather than adjacency lists

### Partition Support
Additional CPOs for multipartite graphs:
- `num_partitions(g)` - number of partitions
- `partition_id(g, u)` - get vertex partition
- `vertices(g, pid)` - vertices in partition
- `edges(g, u, pid)` - edges to specific partition

### Advanced Query Functions
- `in_edges(g, u)` - incoming edges (requires separate view)
- `find_edge(g, uid, vid)` - find specific edge
- `edge_id(g, uv)` - unique edge identifier

---

**Document Status:** Implementation guide for Phase 2 development

**Current Progress:**
- ✅ `vertices(g)` - COMPLETE (18 tests passing, handles both descriptor view and raw container returns)
- ✅ Type aliases based on `vertices(g)` - COMPLETE (5 additional tests passing)
  - `vertex_range_t<G>` - Range type returned by vertices(g)
  - `vertex_iterator_t<G>` - Iterator over vertex range
  - `vertex_t<G>` - Vertex descriptor type for graph G
- ✅ `vertex_id(g, u)` - COMPLETE (15 tests passing, four-tier resolution with inner_value priority)
  - Resolution order:
    1. `u.inner_value(g).vertex_id(g)` - Inner value member (highest priority)
    2. `vertex_id(g, u.inner_value(g))` - ADL with inner_value
    3. `vertex_id(g, u)` - ADL with descriptor
    4. `u.vertex_id()` - Descriptor default (lowest priority)
  - Where `u` is a vertex_descriptor and `u.inner_value(g)` extracts the actual vertex data
- ✅ Type alias based on `vertex_id(g, u)` - COMPLETE (2 additional tests passing)
  - `vertex_id_t<G>` - Vertex ID type (size_t for vector, key type for map)
- Location: `include/graph/detail/graph_cpo.hpp`
- Tests: `tests/test_vertices_cpo.cpp`, `tests/test_vertex_id_cpo.cpp`, `tests/test_type_aliases.cpp`
- Total: 114 tests passing (75 descriptors + 18 vertices + 15 vertex_id + 7 type aliases - 1 overlap)
- Features: Three/four-tier resolution (member/ADL/default), automatic wrapping via `_wrap_if_needed()`, inner_value pattern support

**Next Steps:**
1. Implement `find_vertex(g, uid)` CPO (uses vertex_id_t<G> for lookup)
2. Implement `edges(g, u)` CPO (follow parallel pattern to vertices)
3. Implement `target_id(g, uv)` CPO (extract target ID from edge)
4. Create comprehensive test suite for each CPO
5. Validate with GCC, Clang, and MSVC compilers
6. Proceed to Priority 2 functions
