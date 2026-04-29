# Implementation Plan: BGL Graph Adaptation

This plan implements `bgl_graph_adapt_strategy.md` in phases that each produce a
compilable, testable increment. Each phase lists the files to create or modify, what to
put in each file, and the verification command.

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Phase 0: Project Scaffolding](#phase-0-project-scaffolding)
- [Phase 1: C++20 Iterator Wrapper](#phase-1-c20-iterator-wrapper)
- [Phase 2: graph_adaptor and Core CPOs](#phase-2-graph_adaptor-and-core-cpos)
- [Phase 3: Concept Satisfaction Verification](#phase-3-concept-satisfaction-verification)
- [Phase 4: Property Bridge](#phase-4-property-bridge)
- [Phase 5: View Integration Tests](#phase-5-view-integration-tests)
- [Phase 6: Dijkstra End-to-End](#phase-6-dijkstra-end-to-end)
- [Phase 7: Bidirectional and Undirected Variants](#phase-7-bidirectional-and-undirected-variants)
- [Phase 8: compressed_sparse_row_graph](#phase-8-compressed_sparse_row_graph)
- [Phase 9: Documentation and Example](#phase-9-documentation-and-example)
- [Status Summary](#status-summary)

---

## Prerequisites

- Boost headers must be available. The project already resolves a Boost include directory
  via the BGL benchmark CMake logic in `benchmark/algorithms/CMakeLists.txt`. The test
  CMake added in Phase 0 reuses the same resolution approach.
- The build must use C++20 (`CMAKE_CXX_STANDARD 20`), which the project already enforces.

---

## Phase 0: Project Scaffolding

**Goal:** Create the directory structure and CMake wiring so that all subsequent phases
have a place to put code and tests. No adaptor logic yet.

### Files to Create

1. **`include/graph/adaptors/bgl/graph_adaptor.hpp`** — Empty placeholder header with
   `#pragma once` and a namespace skeleton:
   ```cpp
   #pragma once
   namespace graph::bgl {
   // BGL graph adaptor — implementation added in Phase 2
   } // namespace graph::bgl
   ```

2. **`include/graph/adaptors/bgl/bgl_edge_iterator.hpp`** — Empty placeholder header.

3. **`include/graph/adaptors/bgl/property_bridge.hpp`** — Empty placeholder header.

4. **`tests/adaptors/CMakeLists.txt`** — Test executable for adaptors.
   ```cmake
   # BGL adaptor tests
   # Requires Boost headers. Reuse BGL_INCLUDE_DIR resolution from benchmark/.
   option(TEST_BGL_ADAPTOR "Build BGL adaptor tests (requires Boost headers)" OFF)

   if(TEST_BGL_ADAPTOR)
     # Resolve Boost include directory (same logic as benchmark/algorithms/)
     if(NOT BGL_INCLUDE_DIR)
       if(DEFINED ENV{BGL_INCLUDE_DIR})
         set(BGL_INCLUDE_DIR "$ENV{BGL_INCLUDE_DIR}")
       elseif(DEFINED ENV{BOOST_ROOT})
         set(BGL_INCLUDE_DIR "$ENV{BOOST_ROOT}")
       else()
         # Platform defaults
         foreach(_path "$ENV{HOME}/dev_graph/boost" "/usr/local/include" "/usr/include")
           if(EXISTS "${_path}/boost/graph/adjacency_list.hpp")
             set(BGL_INCLUDE_DIR "${_path}")
             break()
           endif()
         endforeach()
       endif()
     endif()

     if(NOT BGL_INCLUDE_DIR OR NOT EXISTS "${BGL_INCLUDE_DIR}/boost/graph/adjacency_list.hpp")
       message(WARNING "BGL headers not found — skipping BGL adaptor tests. "
                       "Set BGL_INCLUDE_DIR or BOOST_ROOT.")
       return()
     endif()

     message(STATUS "BGL adaptor tests: using Boost headers from ${BGL_INCLUDE_DIR}")

     add_executable(graph3_bgl_adaptor_tests
       test_bgl_edge_iterator.cpp
       test_bgl_cpo.cpp
       test_bgl_concepts.cpp
       test_bgl_views.cpp
       test_bgl_property_bridge.cpp
       test_bgl_dijkstra.cpp
       test_bgl_bidirectional.cpp
       test_bgl_csr.cpp
     )

     target_link_libraries(graph3_bgl_adaptor_tests
       PRIVATE graph3 Catch2::Catch2WithMain)

     target_include_directories(graph3_bgl_adaptor_tests
       PRIVATE ${BGL_INCLUDE_DIR})

     # Keep the test runner simple: run the whole adaptor suite as one CTest test.
     # The phases in this plan are expected to remain short-running, so per-case
     # Catch discovery is not required.
     add_test(NAME graph3_bgl_adaptor_tests COMMAND graph3_bgl_adaptor_tests)
   endif()
   ```

5. **`tests/adaptors/test_bgl_edge_iterator.cpp`** — Stub:
   `#include <catch2/catch_test_macros.hpp>` + empty test case placeholder.

6. Create stub `.cpp` files for each test listed in the CMakeLists above. Each stub
   includes Catch2 and has one `TEST_CASE("placeholder") { SUCCEED(); }`.

### Files to Modify

7. **`tests/CMakeLists.txt`** — Add `add_subdirectory(adaptors)` at the end.

### Verification

```bash
cmake --preset <preset> -DTEST_BGL_ADAPTOR=ON \
  -DBGL_INCLUDE_DIR=/home/phil/dev_graph/boost
cmake --build build
ctest --test-dir build --output-on-failure -R graph3_bgl_adaptor_tests
```

All stub tests should pass (placeholder `SUCCEED()`).

---

## Phase 1: C++20 Iterator Wrapper

**Goal:** Implement `bgl_edge_iterator` — a thin wrapper that makes a BGL iterator
satisfy `std::forward_iterator`. This is the highest-risk piece and is done early so
that failures surface before any CPO work depends on it.

Before writing the wrapper, inspect the actual `std::iterator_traits<BGL_Iter>::reference`
type for the target BGL iterators (`adjacency_list` and CSR). Do not assume `operator*`
returns a stable lvalue reference.

### File: `include/graph/adaptors/bgl/bgl_edge_iterator.hpp`

Replace the placeholder with:

```cpp
#pragma once
#include <iterator>
#include <type_traits>

namespace graph::bgl {

/// Wraps a pre-C++20 BGL iterator to satisfy std::forward_iterator.
/// BGL's boost::iterator_facade defines iterator_category but not
/// iterator_concept, which C++20 ranges require.
template <typename BGL_Iter>
class bgl_edge_iterator {
  BGL_Iter it_{};

public:
  // C++20 iterator requirements
  using iterator_concept  = std::forward_iterator_tag;
  using value_type        = typename std::iterator_traits<BGL_Iter>::value_type;
  using difference_type   = typename std::iterator_traits<BGL_Iter>::difference_type;
  using reference         = typename std::iterator_traits<BGL_Iter>::reference;
  using pointer           = std::conditional_t<std::is_reference_v<reference>,
                             std::add_pointer_t<std::remove_reference_t<reference>>,
                             void>;

  bgl_edge_iterator() = default;
  explicit bgl_edge_iterator(BGL_Iter it) : it_(it) {}

  reference operator*()  const { return *it_; }

  // Only provide operator-> when dereferencing yields a stable lvalue reference.
  auto operator->() const requires std::is_reference_v<reference> {
    return std::addressof(*it_);
  }

  bgl_edge_iterator& operator++()    { ++it_; return *this; }
  bgl_edge_iterator  operator++(int) { auto tmp = *this; ++it_; return tmp; }

  friend bool operator==(const bgl_edge_iterator& a, const bgl_edge_iterator& b) {
    return a.it_ == b.it_;
  }

  /// Access the underlying BGL iterator (needed for edge property extraction).
  const BGL_Iter& base() const { return it_; }
};

} // namespace graph::bgl
```

If the inspection shows that a target BGL iterator dereferences to a proxy or prvalue,
do not force `operator->`. Keep the wrapper readable via `operator*` only, and adjust the
adaptor code to unwrap through dereference rather than pointer semantics.

### File: `tests/adaptors/test_bgl_edge_iterator.cpp`

Replace the stub with tests that:

1. `static_assert(std::forward_iterator<bgl_edge_iterator<OutEdgeIter>>)` for
   `adjacency_list<vecS, vecS, directedS>` out-edge iterators.
2. `static_assert(std::forward_iterator<bgl_edge_iterator<CSR_OutEdgeIter>>)` for
   `compressed_sparse_row_graph` out-edge iterators.
3. `static_assert` that the wrapper's `reference` type matches the underlying BGL iterator's
  `reference` type.
4. If `reference` is a true reference type for the target iterator, add a test that
  `operator->` is available and points to the same object as `std::addressof(*it)`.
5. Construct a small BGL `adjacency_list<vecS, vecS, directedS>` (3 vertices, 4 edges),
   wrap its `out_edges(0, g)` iterators in `bgl_edge_iterator`, iterate, and verify
   the targets match BGL's own `boost::target(e, g)` results.
6. Verify that `std::ranges::subrange(wrapped_begin, wrapped_end)` models
   `std::ranges::forward_range`.

### Verification

```bash
cmake --build build && ctest --test-dir build --output-on-failure -R graph3_bgl_adaptor_tests
```

---

## Phase 2: graph_adaptor and Core CPOs

**Goal:** Implement the wrapper type and the minimum ADL free functions for
`adjacency_list<vecS, vecS, directedS>` to satisfy `adjacency_list` and
`index_adjacency_list` concepts.

### File: `include/graph/adaptors/bgl/graph_adaptor.hpp`

Replace the placeholder with the full implementation containing:

1. **`graph_adaptor<BGL_Graph>` class** — Non-owning wrapper (pointer to BGL graph),
   with `bgl_graph()` accessors and CTAD deduction guide.

2. **ADL free functions** (all in `namespace graph::bgl`):

   - **`vertices(const graph_adaptor<G>& ga)`** — Returns
     `std::views::iota(vid{0}, boost::num_vertices(ga.bgl_graph()))`.
     The `vertices` CPO will auto-wrap this in `vertex_descriptor_view`.

   - **`num_vertices(const graph_adaptor<G>& ga)`** — Returns
     `boost::num_vertices(ga.bgl_graph())`.

   - **`out_edges(graph_adaptor<G>& ga, const auto& u)`** — Calls
     `boost::out_edges(u.vertex_id(), ga.bgl_graph())`, wraps the returned
     iterator pair in `bgl_edge_iterator`, returns
     `std::ranges::subrange(wrapped_begin, wrapped_end)`.
     Also provide the `const graph_adaptor<G>&` overload.
     The `out_edges` CPO will auto-wrap this in `edge_descriptor_view`.

   - **`target_id(const graph_adaptor<G>& ga, const auto& uv)`** — Dereferences
     `*uv.value()` to get the BGL edge, calls `boost::target(bgl_edge, ga.bgl_graph())`.

   - **`source_id(const graph_adaptor<G>& ga, const auto& uv)`** — Same pattern,
     calls `boost::source(bgl_edge, ga.bgl_graph())`.

### Required Includes

The header needs:
```cpp
#include <boost/graph/graph_traits.hpp>
#include <graph/adaptors/bgl/bgl_edge_iterator.hpp>
#include <ranges>
#include <concepts>
```

Keep this header graph-type-agnostic. Do not include concrete BGL graph family headers
such as `boost/graph/adjacency_list.hpp` or `boost/graph/compressed_sparse_row_graph.hpp`
here unless a BGL API genuinely requires a complete type at the declaration point.
Prefer including concrete BGL graph headers only in tests, examples, or a graph-family-
specific companion header if one becomes necessary.

### Key Implementation Notes for the Agent

- The `out_edges` function receives a `vertex_descriptor<IotaIter>` from the auto-wrapped
  `vertices` result. Extract the BGL vertex ID via `u.vertex_id()`.
- The `target_id` and `source_id` functions receive an
  `edge_descriptor<bgl_edge_iterator<...>, IotaIter>`. The BGL edge is accessed via
  `*uv.value()` which dereferences the wrapped `bgl_edge_iterator` to yield the BGL
  `edge_descriptor`.
- All functions must be `constexpr`-friendly where possible and `noexcept` where the
  underlying BGL calls are noexcept.
- Use `boost::graph_traits<G>` for portable type extraction rather than BGL internal types.

### File: `tests/adaptors/test_bgl_cpo.cpp`

Replace the stub. Build a `adjacency_list<vecS, vecS, directedS, no_property, EdgeProp>`
with 4 vertices, edges: 0→1, 0→2, 1→3, 2→3. Test each CPO:

1. `graph::vertices(g)` returns a range of size 4, vertex IDs 0–3.
2. `graph::num_vertices(g)` returns 4.
3. `graph::out_edges(g, u)` for vertex 0 returns 2 edges.
4. `graph::target_id(g, uv)` on the first out-edge of vertex 0 returns 1 or 2.
5. `graph::source_id(g, uv)` on the same edge returns 0.
6. `graph::find_vertex(g, 2)` returns a valid iterator.
7. `graph::vertex_id(g, *graph::find_vertex(g, 2))` returns 2.

### Verification

```bash
cmake --build build && ctest --test-dir build --output-on-failure -R graph3_bgl_adaptor_tests
```

---

## Phase 3: Concept Satisfaction Verification

**Goal:** Prove with `static_assert` that the adapted type satisfies the graph-v3 concepts.

### File: `tests/adaptors/test_bgl_concepts.cpp`

Replace the stub. Include `<graph/graph.hpp>` and the adaptor header. Define BGL graph
typedefs and test:

```cpp
using bgl_directed_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                              boost::no_property, EdgeProp>;
using adapted_t = graph::bgl::graph_adaptor<bgl_directed_t>;

static_assert(graph::adj_list::adjacency_list<adapted_t>);
static_assert(graph::adj_list::index_adjacency_list<adapted_t>);
```

If either assertion fails, the error message will indicate which sub-requirement failed
(e.g., `vertices(g)` not found, `target_id(g, uv)` not valid, etc.), guiding the fix
back in Phase 2.

Also add a runtime `TEST_CASE` that constructs a graph, wraps it, and iterates
`vertexlist(g)` as a smoke test — if concepts pass but runtime fails, the descriptors
are not wiring up correctly.

### Verification

```bash
cmake --build build && ctest --test-dir build --output-on-failure -R graph3_bgl_adaptor_tests
```

---

## Phase 4: Property Bridge

**Goal:** Implement helper function objects that bridge true BGL property maps into
graph-v3's `edge_value_function` and `vertex_value_function` concepts.

### File: `include/graph/adaptors/bgl/property_bridge.hpp`

Replace the placeholder. Implement:

1. **Readable property-map wrapper** — A function object class (not a lambda, for stable
   type identity) that wraps any BGL readable property map and adapts it to graph-v3's
   function-object calling convention.

   ```cpp
   template <typename PropertyMap, typename KeyExtractor>
   struct bgl_readable_property_map_fn {
     PropertyMap property_map;
     KeyExtractor key_extractor;

     template <typename AdaptedGraph, typename Descriptor>
     decltype(auto) operator()(const AdaptedGraph&, const Descriptor& descriptor) const {
       return get(property_map, key_extractor(descriptor));
     }
   };
   ```

   Provide a factory function:
   ```cpp
   template <typename PropertyMap, typename KeyExtractor>
   auto make_bgl_readable_property_map_fn(PropertyMap pm, KeyExtractor key_extractor);
   ```

2. **Writable property-map wrapper** — A function object class that wraps any BGL property
   map whose `get(pm, key)` yields a stable lvalue reference. This is the phase-1 route for
   true writable property-map adaptation because graph-v3's writable property functions must
   return a reference.

   ```cpp
   template <typename PropertyMap, typename KeyExtractor>
   struct bgl_lvalue_property_map_fn {
     PropertyMap property_map;
     KeyExtractor key_extractor;

     template <typename AdaptedGraph, typename Descriptor>
     decltype(auto) operator()(const AdaptedGraph&, const Descriptor& descriptor) const {
       return get(property_map, key_extractor(descriptor));
     }
   };
   ```

   Constrain or document this wrapper so it is only used for property maps whose `get`
   result is an lvalue reference or otherwise satisfies the graph-v3 writable-property
   requirement. If a property map only supports `put` but not lvalue access, it is not a
   phase-1 writable adaptor target without changing graph-v3 algorithm contracts.

3. **Convenience aliases:**
   - A bundled-property helper built on top of the generic readable-property wrapper.
   - A `make_vertex_id_property_fn<T>(std::vector<T>& storage)` helper for graph-v3-owned
     writable storage, implemented as a thin special case for indexed vertex IDs.

### File: `tests/adaptors/test_bgl_property_bridge.cpp`

Replace the stub. Test:

1. Construct a BGL graph with `struct EdgeProp { double weight; }`, add edges with
   known weights.
2. Create a true BGL readable property map via `boost::get(&EdgeProp::weight, g)`, wrap it
  via `make_bgl_readable_property_map_fn`, iterate edges through graph-v3's `incidence`
  view, and verify weights match.
3. Create a true writable BGL property map that returns lvalue references, wrap it via
  `make_bgl_lvalue_property_map_fn`, write through the adapted function object, and verify
  the underlying property map changes.
4. Also test the graph-v3-owned vector helper `make_vertex_id_property_fn` for distances and
  predecessors, since Dijkstra will use it as the lowest-friction storage path.
5. Verify that the readable edge weight function satisfies
   `graph::basic_edge_weight_function<adapted_t, decltype(weight_fn), double, std::less<>, std::plus<>>`.

### Verification

```bash
cmake --build build && ctest --test-dir build --output-on-failure -R graph3_bgl_adaptor_tests
```

---

## Phase 5: View Integration Tests

**Goal:** Verify all four phase-1 views work on adapted BGL graphs.

### File: `tests/adaptors/test_bgl_views.cpp`

Replace the stub. Use the same 4-vertex directed graph from Phase 2. Test:

1. **`vertexlist(g)`** — Iterate, collect `[uid, u]` pairs, verify UIDs = {0, 1, 2, 3}.
2. **`incidence(g, u)`** — For vertex 0, iterate, collect target IDs, verify they match
   the edges added to the BGL graph.
3. **`neighbors(g, u)`** — For vertex 0, iterate, collect neighbor IDs, verify same set
   as incidence targets. (This exercises the `target` CPO default tier.)
4. **`edgelist(g)`** — Iterate, collect `[sid, tid]` pairs, verify all edges present.
5. **Value-function variants** — `incidence(g, u, evf)` with the edge weight function
   from Phase 4, verify values.

### Verification

```bash
cmake --build build && ctest --test-dir build --output-on-failure -R graph3_bgl_adaptor_tests
```

---

## Phase 6: Dijkstra End-to-End

**Goal:** Run graph-v3's `dijkstra_shortest_paths` on a BGL graph and verify correctness.

### File: `tests/adaptors/test_bgl_dijkstra.cpp`

Replace the stub. Use a graph with known shortest paths (e.g., the CLRS Dijkstra example
already defined in `examples/dijkstra_clrs.hpp`):

1. Build the same graph in BGL: `adjacency_list<vecS, vecS, directedS, no_property, EdgeProp>`.
2. Wrap it: `auto g = graph::bgl::graph_adaptor(bgl_g);`
3. Create distance and predecessor vectors, wrap via `make_vertex_id_property_fn`.
4. Create an edge weight function via the true readable property-map wrapper, using
  `boost::get(&EdgeProp::weight, bgl_g)`.
5. Call `graph::dijkstra_shortest_paths(g, sources, dist_fn, pred_fn, weight_fn)`.
6. Verify distances and predecessors match the known correct values.
7. (Optional) Also run BGL's own Dijkstra and compare results.

### Verification

```bash
cmake --build build && ctest --test-dir build --output-on-failure -R graph3_bgl_adaptor_tests
```

**This phase completes the first acceptance criterion in `bgl_graph_adapt_goal.md`.**

---

## Phase 7: Bidirectional and Undirected Variants

**Goal:** Extend the adaptor to support `bidirectionalS` and `undirectedS` BGL graphs.

### File: `include/graph/adaptors/bgl/graph_adaptor.hpp` (modify)

Add an ADL free function:

- **`in_edges(graph_adaptor<G>& ga, const auto& u)`** — Same pattern as `out_edges`,
  calls `boost::in_edges(u.vertex_id(), ga.bgl_graph())`, wraps iterators.
  Use `if constexpr` or SFINAE to only enable this for BGL graphs that provide in-edges
  (check `boost::graph_traits<G>::traversal_category` is convertible to
  `boost::bidirectional_graph_tag`).

For undirected graphs: BGL's `out_edges` already returns all incident edges. Provide
`in_edges` that delegates to the same `boost::out_edges` call, so the adapted graph
models `bidirectional_adjacency_list`.

### File: `tests/adaptors/test_bgl_bidirectional.cpp`

Replace the stub. Test:

1. `adjacency_list<vecS, vecS, bidirectionalS>` — verify `in_edges` returns correct
   edges, `static_assert(bidirectional_adjacency_list<adapted_t>)`.
2. `adjacency_list<vecS, vecS, undirectedS>` — verify `out_edges` and `in_edges` both
  return the full incident set, verify `edgelist`/degree-related behavior explicitly, then
  run `dijkstra_shortest_paths` and compare to BGL.

### Verification

```bash
cmake --build build && ctest --test-dir build --output-on-failure -R graph3_bgl_adaptor_tests
```

---

## Phase 8: compressed_sparse_row_graph

**Goal:** Verify the adaptor works with BGL's CSR graph type.

### File: `include/graph/adaptors/bgl/graph_adaptor.hpp` (modify, if needed)

The existing `graph_adaptor` template and ADL functions should already work because they
use `boost::graph_traits<G>`, `boost::vertices`, `boost::out_edges`, `boost::source`,
and `boost::target`, which are all defined for `compressed_sparse_row_graph`. If there
are CSR-specific issues (e.g., different edge descriptor layout), add overloads as needed.

Do not add `boost/graph/compressed_sparse_row_graph.hpp` to the core adaptor header by
default. Include it in the CSR test file, and only introduce a CSR-specific adaptor header
if the implementation ends up requiring one.

### File: `tests/adaptors/test_bgl_csr.cpp`

Replace the stub. Test:

1. Build a `compressed_sparse_row_graph<directedS, no_property, EdgeProp>` from sorted
   edge pairs (reuse the pattern from `benchmark/algorithms/bgl_dijkstra_fixtures.hpp`).
2. Wrap it, verify `static_assert(index_adjacency_list<adapted_t>)`.
3. Run `dijkstra_shortest_paths`, compare results against known values.

### Verification

```bash
cmake --build build && ctest --test-dir build --output-on-failure -R graph3_bgl_adaptor_tests
```

---

## Phase 9: Documentation and Example

**Goal:** Create user-facing documentation and a complete example.

### Files to Create

1. **`examples/bgl_adaptor_example.cpp`** — Self-contained example showing:
   - Building a BGL `adjacency_list<vecS, vecS, directedS>` with edge weights.
   - Wrapping it with `graph_adaptor`.
   - Iterating with `vertexlist` and `incidence` views.
   - Running `dijkstra_shortest_paths`.
   - Printing results.

2. **`docs/user-guide/bgl-adaptor.md`** — User guide covering:
   - When to use the adaptor.
   - How to wrap a BGL graph (one-line `graph_adaptor(g)`).
   - How to bridge properties (factory functions).
   - Supported BGL graph types.
   - Known limitations (no listS/setS yet, no undirected concept, etc.).

### Files to Modify

3. **`examples/CMakeLists.txt`** — Add the new example, conditionally compiled when
   Boost headers are available.

4. **`agents/bgl_migration_strategy.md`** — Update Section 12 with a reference to the
   new adaptor headers and example (per acceptance criteria).
5. **Consistency pass across planning docs** — Before closing the work, do one short pass
  over `bgl_graph_adapt_goal.md`, `bgl_graph_adapt_strategy.md`, and this plan to make sure
  phase names, section numbering, and property-map terminology still agree.

### Verification

```bash
cmake --build build && ./build/examples/bgl_adaptor_example
```

---

## Status Summary

| Phase | Description                         | Status      |
|-------|-------------------------------------|-------------|
| 0     | Project scaffolding                 | ✅ Complete |
| 1     | C++20 iterator wrapper              | ✅ Complete |
| 2     | graph_adaptor and core CPOs         | ✅ Complete |
| 3     | Concept satisfaction verification   | ✅ Complete |
| 4     | Property bridge                     | ✅ Complete |
| 5     | View integration tests              | ✅ Complete |
| 6     | Dijkstra end-to-end                 | ✅ Complete |
| 7     | Bidirectional and undirected         | ✅ Complete |
| 8     | compressed_sparse_row_graph         | ✅ Complete |
| 9     | Documentation and example           | ✅ Complete |

**All phases complete.** Last verified: 2026-04-29 — 34 test cases, 107 assertions, all passing.
Build requires `-DTEST_BGL_ADAPTOR=ON` and Boost headers. Example requires `-DBUILD_BGL_EXAMPLES=ON`.
