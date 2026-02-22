# Incoming Edges — Phased Implementation Plan

**Source:** `agents/incoming_edges_design.md`
**Branch:** `incoming`
**Build preset:** `linux-gcc-debug` (full test suite)

---

## How to use this plan

Each phase is a self-contained unit of work that can be completed by an agent in
one session. Phases are strictly ordered — each depends on the prior phase compiling
and passing all tests. Within each phase, steps are listed in execution order.

**Conventions used throughout:**

- "Mirror X" means copy the implementation pattern from X, replacing outgoing
  names/semantics with incoming equivalents.
- "Full test suite passes" means `cmake --build build/linux-gcc-debug -j$(nproc)`
  succeeds and `cd build/linux-gcc-debug && ctest --output-on-failure` reports
  zero failures with no regressions.
- File paths are relative to the repository root.

---

## Phase 1 — `in_edges` and `in_degree` CPOs + type aliases

**Goal:** Add the two core incoming-edge CPOs, their public instances, the
outgoing aliases (`out_edges`, `out_degree`, `find_out_edge`), the six new type
aliases, and a CPO unit-test file proving all resolution tiers work.

**Why first:** Every subsequent phase depends on `in_edges` and `in_edge_t<G>`
existing. This phase touches exactly one production header and adds one test
file, so it is low-risk and easy to validate.

### Files to modify

| File | Action |
|---|---|
| `include/graph/adj_list/detail/graph_cpo.hpp` | Add CPOs, aliases, public instances |
| `tests/adj_list/CMakeLists.txt` | Register new test file |

### Files to create

| File | Content |
|---|---|
| `tests/adj_list/cpo/test_in_edges_cpo.cpp` | Unit tests |

### Steps

#### 1.1 Add `in_edges` CPO (graph_cpo.hpp)

Insert a new section **after** the `edges` public instance and type aliases
(after line ~835, before the `_target` namespace). Follow the exact structural
pattern of `namespace _edges`:

```
namespace _cpo_impls {
  namespace _in_edges {
    // --- (g, u) overload ---
    enum class _St_u { _none, _vertex_member, _adl };

    template <typename G, typename U>
    concept _has_vertex_member_u = is_vertex_descriptor_v<...> &&
        requires(G& g, const U& u) {
          { u.inner_value(g).in_edges() } -> std::ranges::forward_range;
        };

    template <typename G, typename U>
    concept _has_adl_u = requires(G& g, const U& u) {
      { in_edges(g, u) } -> std::ranges::forward_range;
    };

    // NOTE: No _edge_value_pattern tier — in_edges has no implicit default.
    // A graph MUST explicitly provide in_edges() via member or ADL.

    template <typename G, typename U>
    [[nodiscard]] consteval _Choice_t<_St_u> _Choose_u() noexcept { ... }

    // --- (g, uid) overload ---
    enum class _St_uid { _none, _adl, _default };
    // _has_adl_uid, _has_default_uid (find_vertex + in_edges(g, u))
    template <typename G, typename VId>
    [[nodiscard]] consteval _Choice_t<_St_uid> _Choose_uid() noexcept { ... }

    class _fn {
      // operator()(G&& g, const U& u)   — vertex descriptor
      // operator()(G&& g, const VId& uid) — vertex ID
      // Both use _wrap_if_needed() to ensure edge_descriptor_view return
    };
  }
}
```

Key differences from `edges`:
- Only 2 tiers for `(g, u)`: `_vertex_member` and `_adl`. No `_edge_value_pattern`.
- The `(g, uid)` default delegates to `in_edges(g, *find_vertex(g, uid))`.
- The `_has_default_uid` concept checks `_has_adl_u` (not `_has_edge_value_pattern`).

#### 1.2 Add `in_edges` public instance and type aliases

Immediately after the `_in_edges` namespace closing brace:

```cpp
inline namespace _cpo_instances {
  inline constexpr _cpo_impls::_in_edges::_fn in_edges{};
}

template <typename G>
using in_vertex_edge_range_t = decltype(in_edges(std::declval<G&>(), std::declval<vertex_t<G>>()));

template <typename G>
using in_vertex_edge_iterator_t = std::ranges::iterator_t<in_vertex_edge_range_t<G>>;

template <typename G>
using in_edge_t = std::ranges::range_value_t<in_vertex_edge_range_t<G>>;
```

#### 1.3 Add `in_degree` CPO

Insert after the `in_edges` section, mirroring `namespace _degree`:

```
namespace _cpo_impls {
  namespace _in_degree {
    // (g, u): _member (g.in_degree(u)), _adl, _default (size(in_edges(g,u)))
    // (g, uid): _member, _adl, _default (*find_vertex then in_degree(g,u))
    class _fn { ... };
  }
}

inline namespace _cpo_instances {
  inline constexpr _cpo_impls::_in_degree::_fn in_degree{};
}
```

Key difference from `degree`: the `_default` tier calls `in_edges(g, u)` instead
of `edges(g, u)`.

#### 1.4 Add outgoing aliases

After the `in_degree` public instance:

```cpp
inline namespace _cpo_instances {
  inline constexpr auto& out_edges     = edges;
  inline constexpr auto& out_degree    = degree;
  inline constexpr auto& find_out_edge = find_vertex_edge;
}
```

#### 1.5 Add outgoing type aliases

```cpp
template <typename G>
using out_vertex_edge_range_t = vertex_edge_range_t<G>;

template <typename G>
using out_vertex_edge_iterator_t = vertex_edge_iterator_t<G>;

template <typename G>
using out_edge_t = edge_t<G>;
```

#### 1.6 Create test file `tests/adj_list/cpo/test_in_edges_cpo.cpp`

Test the following scenarios (mirror `test_edges_cpo.cpp` structure):

1. **Stub graph with `in_edges()` member** — a minimal struct whose vertex
   `inner_value(g).in_edges()` returns a `vector<int>`. Verify the CPO
   dispatches to `_vertex_member` tier and the return is an
   `edge_descriptor_view`.

2. **Stub graph with ADL `in_edges(g, u)` friend** — verify `_adl` tier.

3. **`(g, uid)` overload with default** — verify `_default` tier delegates
   through `find_vertex` + `in_edges(g, u)`.

4. **Type alias verification** — `in_vertex_edge_range_t<G>`,
   `in_vertex_edge_iterator_t<G>`, `in_edge_t<G>` all compile and produce
   expected types.

5. **Mixed-type test** — a stub graph where `edge_t<G>` is
   `pair<int, double>` but `in_edge_t<G>` is just `int`. Verify both
   aliases are independently deduced.

6. **`out_edges` / `out_degree` / `find_out_edge` aliases** — verify they
   are the exact same object as `edges` / `degree` / `find_vertex_edge`
   (use `static_assert(&out_edges == &edges)`).

7. **`in_degree` CPO** — test member, ADL, and default (`size(in_edges)`)
   resolution tiers.

8. **Outgoing type aliases** — verify `out_edge_t<G>` is `edge_t<G>` etc.

#### 1.7 Register test in CMakeLists

Add to `tests/adj_list/CMakeLists.txt` under the CPOs section:

```cmake
    cpo/test_in_edges_cpo.cpp
```

#### 1.8 Build and run full test suite

```bash
cmake --build build/linux-gcc-debug -j$(nproc)
cd build/linux-gcc-debug && ctest --output-on-failure
```

### Merge gate

- [ ] Full test suite passes with zero regressions.
- [ ] All 8 test scenarios pass.
- [ ] `in_edge_t<G> != edge_t<G>` mixed-type test passes.
- [ ] `out_edges` alias identity check passes.

---

## Phase 2 — `find_in_edge` and `contains_in_edge` CPOs + traits

**Goal:** Add the remaining incoming-edge CPOs (`find_in_edge`,
`contains_in_edge`), the new traits (`has_in_degree`, `has_find_in_edge`,
`has_contains_in_edge`), and their unit tests.

**Why second:** These CPOs depend on `in_edges` (Phase 1). They complete the
full incoming-edge CPO surface before concepts are defined.

### Files to modify

| File | Action |
|---|---|
| `include/graph/adj_list/detail/graph_cpo.hpp` | Add `find_in_edge`, `contains_in_edge` CPOs |
| `include/graph/adj_list/adjacency_list_traits.hpp` | Add 3 new traits |
| `tests/adj_list/CMakeLists.txt` | Register new test files |

### Files to create

| File | Content |
|---|---|
| `tests/adj_list/cpo/test_find_in_edge_cpo.cpp` | Unit tests for `find_in_edge` |
| `tests/adj_list/cpo/test_contains_in_edge_cpo.cpp` | Unit tests for `contains_in_edge` |
| `tests/adj_list/traits/test_incoming_edge_traits.cpp` | Trait tests |

### Steps

#### 2.1 Add `find_in_edge` CPO (graph_cpo.hpp)

Mirror `namespace _find_vertex_edge` with three overload groups:

1. `find_in_edge(g, u, v)` — both descriptors. Default iterates
   `in_edges(g, u)` and matches `source_id(g, ie)` against `vertex_id(g, v)`.
2. `find_in_edge(g, u, vid)` — descriptor + ID. Default iterates
   `in_edges(g, u)` and matches `source_id(g, ie)` against `vid`.
3. `find_in_edge(g, uid, vid)` — both IDs. Default delegates via
   `*find_vertex(g, uid)`.

Key difference from `find_vertex_edge`: iterates `in_edges` and compares
`source_id` instead of `target_id`.

```cpp
inline namespace _cpo_instances {
  inline constexpr _cpo_impls::_find_in_edge::_fn find_in_edge{};
}
```

#### 2.2 Add `contains_in_edge` CPO (graph_cpo.hpp)

Mirror `namespace _contains_edge` with two overload groups:

1. `contains_in_edge(g, u, v)` — both descriptors.
2. `contains_in_edge(g, uid, vid)` — both IDs.

Default implementations iterate `in_edges(g, u)` and match `source_id`.

```cpp
inline namespace _cpo_instances {
  inline constexpr _cpo_impls::_contains_in_edge::_fn contains_in_edge{};
}
```

#### 2.3 Add traits (adjacency_list_traits.hpp)

Follow the existing `detail::has_X_impl` → `has_X` → `has_X_v` pattern:

```cpp
// --- has_in_degree ---
namespace detail {
  template <typename G>
  concept has_in_degree_impl = requires(G& g, vertex_t<G> u, vertex_id_t<G> uid) {
    { in_degree(g, u) } -> std::integral;
    { in_degree(g, uid) } -> std::integral;
  };
}
template <typename G> concept has_in_degree = detail::has_in_degree_impl<G>;
template <typename G> inline constexpr bool has_in_degree_v = has_in_degree<G>;

// --- has_find_in_edge ---
namespace detail {
  template <typename G>
  concept has_find_in_edge_impl = requires(G& g, vertex_t<G> u, vertex_t<G> v,
                                           vertex_id_t<G> uid, vertex_id_t<G> vid) {
    find_in_edge(g, u, v);
    find_in_edge(g, u, vid);
    find_in_edge(g, uid, vid);
  };
}
template <typename G> concept has_find_in_edge = detail::has_find_in_edge_impl<G>;
template <typename G> inline constexpr bool has_find_in_edge_v = has_find_in_edge<G>;

// --- has_contains_in_edge ---
namespace detail {
  template <typename G>
  concept has_contains_in_edge_impl = requires(G& g, vertex_t<G> u, vertex_t<G> v,
                                               vertex_id_t<G> uid, vertex_id_t<G> vid) {
    { contains_in_edge(g, u, v) } -> std::convertible_to<bool>;
    { contains_in_edge(g, uid, vid) } -> std::convertible_to<bool>;
  };
}
template <typename G> concept has_contains_in_edge = detail::has_contains_in_edge_impl<G>;
template <typename G> inline constexpr bool has_contains_in_edge_v = has_contains_in_edge<G>;
```

#### 2.4 Create test files

**`test_find_in_edge_cpo.cpp`** — test all 3 overloads:
- Stub graph with ADL `in_edges()` friend returning known edges.
- Verify `find_in_edge(g, u, v)` finds correct incoming edge by `source_id`.
- Verify `find_in_edge(g, u, vid)` works.
- Verify `find_in_edge(g, uid, vid)` delegates through `find_vertex`.
- Verify not-found case.

**`test_contains_in_edge_cpo.cpp`** — test both overloads:
- Verify `contains_in_edge(g, u, v)` returns true/false correctly.
- Verify `contains_in_edge(g, uid, vid)` returns true/false correctly.

**`test_incoming_edge_traits.cpp`** — test all 3 traits:
- Stub graph that models `in_edges`/`in_degree` → `has_in_degree_v` is true.
- Plain `vector<vector<int>>` → `has_in_degree_v` is false.
- Stub graph with `in_edges` + `find_in_edge` → `has_find_in_edge_v` is true.
- Stub graph with `in_edges` + `contains_in_edge` → `has_contains_in_edge_v` is true.

#### 2.5 Register tests in CMakeLists

Add to `tests/adj_list/CMakeLists.txt`:

```cmake
    cpo/test_find_in_edge_cpo.cpp
    cpo/test_contains_in_edge_cpo.cpp
    traits/test_incoming_edge_traits.cpp
```

#### 2.6 Build and run full test suite

### Merge gate

- [ ] Full test suite passes.
- [ ] All `find_in_edge` overloads work (member, ADL, default).
- [ ] All `contains_in_edge` overloads work.
- [ ] Traits correctly detect presence/absence of incoming-edge CPOs.

---

## Phase 3 — Concepts + namespace re-exports

**Goal:** Define `in_vertex_edge_range`, `bidirectional_adjacency_list`,
`index_bidirectional_adjacency_list` concepts; update `graph.hpp` with all
re-exports for Phases 1-3.

**Why third:** Concepts depend on the CPOs and type aliases from Phases 1-2.
Re-exports should be done once after the complete CPO surface exists.

### Files to modify

| File | Action |
|---|---|
| `include/graph/adj_list/adjacency_list_concepts.hpp` | Add 3 concepts |
| `include/graph/graph.hpp` | Add re-exports for all new CPOs, aliases, traits, concepts |
| `tests/adj_list/CMakeLists.txt` | Register new test file |

### Files to create

| File | Content |
|---|---|
| `tests/adj_list/concepts/test_bidirectional_concepts.cpp` | Concept tests |

### Steps

#### 3.1 Add concepts (adjacency_list_concepts.hpp)

After the `index_adjacency_list` concept:

```cpp
template <class R, class G>
concept in_vertex_edge_range = std::ranges::forward_range<R>;

template <class G>
concept bidirectional_adjacency_list =
    adjacency_list<G> &&
    requires(G& g, vertex_t<G> u, in_edge_t<G> ie) {
      { in_edges(g, u) } -> in_vertex_edge_range<G>;
      { source_id(g, ie) } -> std::convertible_to<vertex_id_t<G>>;
    };

template <class G>
concept index_bidirectional_adjacency_list =
    bidirectional_adjacency_list<G> && index_vertex_range<G>;
```

#### 3.2 Update graph.hpp re-exports

In the `namespace graph { ... }` using-declaration block, add:

```cpp
// Incoming-edge CPOs
using adj_list::in_edges;
using adj_list::in_degree;
using adj_list::find_in_edge;
using adj_list::contains_in_edge;

// Outgoing aliases
using adj_list::out_edges;
using adj_list::out_degree;
using adj_list::find_out_edge;

// Incoming-edge type aliases
using adj_list::in_vertex_edge_range_t;
using adj_list::in_vertex_edge_iterator_t;
using adj_list::in_edge_t;

// Outgoing type aliases
using adj_list::out_vertex_edge_range_t;
using adj_list::out_vertex_edge_iterator_t;
using adj_list::out_edge_t;

// Incoming-edge concepts
using adj_list::in_vertex_edge_range;
using adj_list::bidirectional_adjacency_list;
using adj_list::index_bidirectional_adjacency_list;

// Incoming-edge traits
using adj_list::has_in_degree;
using adj_list::has_in_degree_v;
using adj_list::has_find_in_edge;
using adj_list::has_find_in_edge_v;
using adj_list::has_contains_in_edge;
using adj_list::has_contains_in_edge_v;
```

#### 3.3 Create test file `tests/adj_list/concepts/test_bidirectional_concepts.cpp`

1. **Stub bidirectional graph** — a struct with both `edges()` and `in_edges()`
   ADL friends. `static_assert(bidirectional_adjacency_list<StubGraph>)`.

2. **Outgoing-only graph** — a `vector<vector<int>>`.
   `static_assert(!bidirectional_adjacency_list<VVGraph>)`.

3. **Index variant** — stub graph with random-access vertices.
   `static_assert(index_bidirectional_adjacency_list<StubGraph>)`.

4. **Mixed-type concept** — stub where `in_edge_t<G>` is just `int`
   (lightweight back-pointer) but `source_id(g, ie)` works.
   `static_assert(bidirectional_adjacency_list<MixedGraph>)`.

5. **Re-export test** — verify `graph::bidirectional_adjacency_list` is
   accessible (compile-time only).

#### 3.4 Register test and build

### Merge gate

- [ ] Full test suite passes.
- [ ] `bidirectional_adjacency_list` satisfied by stub bidirectional graph.
- [ ] `bidirectional_adjacency_list` not satisfied by `vector<vector<int>>`.
- [ ] Mixed-type (`in_edge_t != edge_t`) graph satisfies concept.
- [ ] All re-exports compile via `#include <graph/graph.hpp>`.

---

## Phase 4 — `undirected_adjacency_list` incoming-edge support

**Goal:** Add `in_edges()` and `in_degree()` ADL friends to
`undirected_adjacency_list` that return the same ranges as `edges()` and
`degree()`, making undirected graphs model `bidirectional_adjacency_list`
at zero cost.

**Why fourth:** This is the simplest container change (just forwarding to
existing functions) and provides a real container for integration testing.

### Files to modify

| File | Action |
|---|---|
| `include/graph/container/undirected_adjacency_list.hpp` | Add `in_edges`, `in_degree` friends |
| `tests/container/CMakeLists.txt` | Register new test file |

### Files to create

| File | Content |
|---|---|
| `tests/container/test_undirected_bidirectional.cpp` | Integration tests |

### Steps

#### 4.1 Add ADL friends (undirected_adjacency_list.hpp)

In `base_undirected_adjacency_list`, immediately after the existing `edges()`
friend functions (around line 1030), add:

```cpp
  // in_edges(g, u) — for undirected graphs, same as edges(g, u)
  template <typename U>
  requires adj_list::vertex_descriptor_type<U>
  friend constexpr auto in_edges(graph_type& g, const U& u) noexcept {
    auto uid = static_cast<vertex_id_type>(u.vertex_id());
    return g.vertices_[uid].edges(g, uid);
  }
  template <typename U>
  requires adj_list::vertex_descriptor_type<U>
  friend constexpr auto in_edges(const graph_type& g, const U& u) noexcept {
    auto uid = static_cast<vertex_id_type>(u.vertex_id());
    return g.vertices_[uid].edges(g, uid);
  }
```

Also add `in_degree` friends if the class has a `degree()` member/friend.
Otherwise, the `in_degree` CPO's default tier (`size(in_edges(g, u))`) will
handle it automatically.

#### 4.2 Create test file

Test using actual `undirected_adjacency_list` instances (e.g., `vov_graph`):

1. **`in_edges` returns same edges as `edges`** — for each vertex, verify
   `in_edges(g, u)` and `edges(g, u)` yield the same edge set.

2. **`in_degree` equals `degree`** — for each vertex.

3. **Concept satisfaction** — `static_assert(bidirectional_adjacency_list<G>)`.

4. **`find_in_edge` works** — since `in_edges` returns the same range.

5. **`contains_in_edge` works** — mirrors `contains_edge`.

6. Run against multiple trait types (at least `vov` and `vol`).

#### 4.3 Register test and build

### Merge gate

- [ ] Full test suite passes.
- [ ] `undirected_adjacency_list` models `bidirectional_adjacency_list`.
- [ ] `in_edges(g, u)` and `edges(g, u)` produce identical results.

---

## Phase 5 — `in_incidence` and `in_neighbors` views

**Goal:** Create the incoming-edge view files, their factory functions,
and tests.

**Why fifth:** Views consume the CPOs from Phases 1-3 and can now also be
tested against the undirected container from Phase 4.

### Files to modify

| File | Action |
|---|---|
| `tests/views/CMakeLists.txt` | Register new test files |

### Files to create

| File | Content |
|---|---|
| `include/graph/views/in_incidence.hpp` | `in_incidence_view`, `basic_in_incidence_view`, factory functions |
| `include/graph/views/in_neighbors.hpp` | `in_neighbors_view`, `basic_in_neighbors_view`, factory functions |
| `tests/views/test_in_incidence.cpp` | View tests |
| `tests/views/test_in_neighbors.cpp` | View tests |

### Steps

#### 5.1 Create `include/graph/views/in_incidence.hpp`

Copy `incidence.hpp` and make these changes:

- Rename all classes: `incidence_view` → `in_incidence_view`,
  `basic_incidence_view` → `basic_in_incidence_view`.
- Change edge iteration from `edges(g, u)` to `in_edges(g, u)`.
- Change neighbor ID extraction from `target_id(g, e)` to `source_id(g, e)`.
- Change `edge_t<G>` references to `in_edge_t<G>`.
- Change concept constraint from `adjacency_list` to `bidirectional_adjacency_list`.
- Update all doxygen comments.
- Update the `edge_info` yield: `{sid, uv}` / `{sid, uv, val}` / `{sid}` / `{sid, val}`.

#### 5.2 Create `include/graph/views/in_neighbors.hpp`

Copy `neighbors.hpp` and apply the same transformation:

- Rename: `neighbors_view` → `in_neighbors_view`.
- Iterate `in_edges` instead of `edges`.
- Extract `source_id` instead of `target_id`.
- Retrieve source vertex via `source(g, e)` instead of `target(g, e)`.
- Constrain with `bidirectional_adjacency_list`.

#### 5.3 Add outgoing view aliases

At the bottom of `in_incidence.hpp`:

```cpp
namespace graph::views {
  inline constexpr auto& out_incidence       = incidence;
  inline constexpr auto& basic_out_incidence = basic_incidence;
}
```

At the bottom of `in_neighbors.hpp`:

```cpp
namespace graph::views {
  inline constexpr auto& out_neighbors       = neighbors;
  inline constexpr auto& basic_out_neighbors = basic_neighbors;
}
```

#### 5.4 Update `graph.hpp`

Add includes:

```cpp
#include <graph/views/in_incidence.hpp>
#include <graph/views/in_neighbors.hpp>
```

#### 5.5 Create test files

**`test_in_incidence.cpp`** — mirror `test_incidence.cpp`:
- Use undirected_adjacency_list (from Phase 4) as the bidirectional graph.
- Verify `in_incidence(g, u)` yields `{source_id, edge}` tuples.
- Verify `in_incidence(g, u, evf)` yields `{source_id, edge, value}`.
- Verify `basic_in_incidence(g, uid)` yields `{source_id}`.
- Verify factory function overloads (descriptor and ID).
- Verify `out_incidence` alias is the same as `incidence`.

**`test_in_neighbors.cpp`** — mirror `test_neighbors.cpp`:
- Verify `in_neighbors(g, u)` yields `{source_id, vertex}` tuples.
- Verify `basic_in_neighbors(g, uid)` yields `{source_id}`.
- Verify `out_neighbors` alias.

#### 5.6 Register tests and build

### Merge gate

- [ ] Full test suite passes.
- [ ] `in_incidence_view` iterates incoming edges, yields `source_id`.
- [ ] `in_neighbors_view` iterates source vertices.
- [ ] All factory function overloads work.
- [ ] Outgoing aliases are identity references.

---

## Phase 6 — Pipe-syntax adaptors

**Goal:** Add pipe-syntax closures for `in_incidence`, `in_neighbors`,
`basic_in_incidence`, `basic_in_neighbors`, and their `out_` aliases
to `adaptors.hpp`.

**Why sixth:** Adaptors depend on the view headers from Phase 5.

### Files to modify

| File | Action |
|---|---|
| `include/graph/views/adaptors.hpp` | Add adaptor closures and factory fns |
| `tests/views/test_adaptors.cpp` | Add pipe-syntax tests for new views |

### Steps

#### 6.1 Add adaptor closures (adaptors.hpp)

For each new view, follow the existing pattern:

1. `in_incidence_adaptor_closure<UID, EVF>` — holds `uid` and optional `evf`.
   `operator|(G&& g, closure)` calls `graph::views::in_incidence(g, uid)` or
   `graph::views::in_incidence(g, uid, evf)`.

2. `in_incidence_adaptor_fn` — has `operator()(uid)`, `operator()(uid, evf)`,
   `operator()(g, uid)`, `operator()(g, uid, evf)`.

3. Same for `basic_in_incidence`, `in_neighbors`, `basic_in_neighbors`.

4. Outgoing aliases:
   ```cpp
   inline constexpr auto& out_incidence       = incidence;
   inline constexpr auto& basic_out_incidence = basic_incidence;
   inline constexpr auto& out_neighbors       = neighbors;
   inline constexpr auto& basic_out_neighbors = basic_neighbors;
   ```

Add to the adaptor namespace block at the bottom:

```cpp
inline constexpr in_incidence_adaptor_fn       in_incidence{};
inline constexpr basic_in_incidence_adaptor_fn basic_in_incidence{};
inline constexpr in_neighbors_adaptor_fn       in_neighbors{};
inline constexpr basic_in_neighbors_adaptor_fn basic_in_neighbors{};
```

#### 6.2 Add tests to `test_adaptors.cpp`

Append new test cases:

```cpp
TEST_CASE("pipe: g | in_incidence(uid)", "[adaptors][in_incidence]") { ... }
TEST_CASE("pipe: g | in_incidence(uid, evf)", "[adaptors][in_incidence]") { ... }
TEST_CASE("pipe: g | basic_in_incidence(uid)", "[adaptors][basic_in_incidence]") { ... }
TEST_CASE("pipe: g | in_neighbors(uid)", "[adaptors][in_neighbors]") { ... }
TEST_CASE("pipe: g | basic_in_neighbors(uid)", "[adaptors][in_neighbors]") { ... }
TEST_CASE("pipe: out_ aliases forward", "[adaptors][aliases]") { ... }
```

#### 6.3 Build and run full test suite

### Merge gate

- [ ] Full test suite passes.
- [ ] `g | in_incidence(uid)` produces same results as `in_incidence(g, uid)`.
- [ ] All 8 pipe expressions from design doc §8.4 work.

---

## Phase 7 — BFS/DFS/topological-sort `EdgeAccessor` parameterization

**Goal:** Add an `EdgeAccessor` template parameter to all 6 traversal view
classes and their factory functions; define `out_edges_accessor` and
`in_edges_accessor` functors.

**Why seventh:** This is a cross-cutting change to 3 existing view headers.
It must not break any existing call sites (the default accessor preserves
current behavior). Having the undirected bidirectional container (Phase 4)
enables testing reverse traversal on a real graph.

### Files to modify

| File | Action |
|---|---|
| `include/graph/views/bfs.hpp` | Add `EdgeAccessor` param to `vertices_bfs_view`, `edges_bfs_view`; replace 5 hardcoded `adj_list::edges()` calls |
| `include/graph/views/dfs.hpp` | Same for `vertices_dfs_view`, `edges_dfs_view`; replace 5 calls |
| `include/graph/views/topological_sort.hpp` | Same for `vertices_topological_sort_view`, `edges_topological_sort_view`; replace 7 calls |
| `tests/views/CMakeLists.txt` | Register new test file |

### Files to create

| File | Content |
|---|---|
| `include/graph/views/edge_accessor.hpp` | `out_edges_accessor`, `in_edges_accessor` definitions |
| `tests/views/test_reverse_traversal.cpp` | Reverse BFS/DFS tests |

### Steps

#### 7.1 Create `edge_accessor.hpp`

```cpp
#pragma once
#include <graph/adj_list/detail/graph_cpo.hpp>

namespace graph::views {

struct out_edges_accessor {
  template <class G>
  constexpr auto operator()(G& g, adj_list::vertex_t<G> u) const {
    return adj_list::edges(g, u);
  }
};

struct in_edges_accessor {
  template <class G>
  constexpr auto operator()(G& g, adj_list::vertex_t<G> u) const {
    return adj_list::in_edges(g, u);
  }
};

} // namespace graph::views
```

#### 7.2 Parameterize BFS views (bfs.hpp)

For each of `vertices_bfs_view` and `edges_bfs_view`:

1. Add `class EdgeAccessor = out_edges_accessor` as the last template parameter
   (before `Alloc` if present, or after the value function type).

2. Store `[[no_unique_address]] EdgeAccessor edge_accessor_;` member.

3. Replace every `adj_list::edges(g, v)` or `adj_list::edges(*g_, v)` with
   `edge_accessor_(g, v)` or `edge_accessor_(*g_, v)`.

4. Add factory function overloads accepting `EdgeAccessor`:
   ```cpp
   vertices_bfs(g, seed, vvf, accessor)
   edges_bfs(g, seed, evf, accessor)
   ```

5. Existing signatures remain unchanged (default `EdgeAccessor`).

#### 7.3 Parameterize DFS views (dfs.hpp)

Same transformation: 5 `adj_list::edges()` calls → `edge_accessor_()`.

#### 7.4 Parameterize topological sort views (topological_sort.hpp)

Same transformation: 7 `adj_list::edges()` calls → `edge_accessor_()`.

#### 7.5 Create `test_reverse_traversal.cpp`

Using an undirected_adjacency_list (where `in_edges == edges`):

1. **Forward BFS** — `vertices_bfs(g, seed)` produces expected order.
2. **Reverse BFS** — `vertices_bfs(g, seed, void_fn, in_edges_accessor{})`
   produces expected order (same as forward for undirected).
3. **Forward DFS** vs **Reverse DFS** — same pattern.
4. **Source-compatibility** — existing call sites `vertices_bfs(g, seed)` and
   `vertices_bfs(g, seed, vvf)` still compile unchanged.

#### 7.6 Build and run full test suite

### Merge gate

- [ ] Full test suite passes (zero regressions in existing BFS/DFS/topo tests).
- [ ] Reverse BFS/DFS with `in_edges_accessor` produces correct traversal.
- [ ] Existing factory signatures compile without changes.

---

## Phase 8 — `dynamic_graph` bidirectional support

**Goal:** Add `bool Bidirectional = false` template parameter to
`dynamic_graph_base`, maintain reverse adjacency lists when enabled,
provide `in_edges()` ADL friend.

**Why eighth:** This is the most complex container change. All prerequisite
infrastructure (CPOs, concepts, views) is already proven by earlier phases.

### Files to modify

| File | Action |
|---|---|
| `include/graph/container/dynamic_graph.hpp` | Add `Bidirectional` param, reverse storage, in_edges friend, mutator updates |
| `tests/container/CMakeLists.txt` | Register new test file |

### Files to create

| File | Content |
|---|---|
| `tests/container/test_dynamic_graph_bidirectional.cpp` | Comprehensive tests |

### Steps

#### 8.1 Add `Bidirectional` template parameter

Change `dynamic_graph_base` signature:

```cpp
template <class EV, class VV, class GV, class VId,
          bool Sourced, class Traits, bool Bidirectional = false>
class dynamic_graph_base;
```

Propagate through `dynamic_graph` and all using-aliases.

#### 8.2 Add reverse edge storage (conditional)

When `Bidirectional = true`, each vertex needs a second edge container.
Use `if constexpr` or a conditional base class:

```cpp
// In the vertex type:
if constexpr (Bidirectional) {
  edges_type in_edges_;  // reverse adjacency container
}
```

Add `in_edges()` accessor to the vertex type (conditional on `Bidirectional`).

#### 8.3 Update mutators

Update `create_edge(u, v, ...)`:
```cpp
if constexpr (Bidirectional) {
  // Insert reverse entry in v.in_edges_ with source = u
}
```

Similarly update `erase_edge`, `erase_vertex`, `clear_vertex`, `clear`,
copy constructor, move constructor, and swap. Each must maintain
forward↔reverse consistency.

#### 8.4 Add `in_edges()` ADL friend

```cpp
template <typename U>
requires adj_list::vertex_descriptor_type<U>
friend constexpr auto in_edges(graph_type& g, const U& u) noexcept
  requires (Bidirectional)
{
  auto uid = static_cast<vertex_id_type>(u.vertex_id());
  return g.vertices_[uid].in_edges(g, uid);
}
```

#### 8.5 Create comprehensive test file

1. **Basic bidirectional graph construction** — create edges, verify `in_edges`
   returns expected source vertices.

2. **`in_degree` matches expected** — for each vertex.

3. **Concept satisfaction** — `static_assert(bidirectional_adjacency_list<G>)`.

4. **Mutator invariants** — after `create_edge(u, v)`:
   - `contains_edge(g, u, v) == true`
   - `contains_in_edge(g, v, u) == true`
   
   After `erase_edge(...)`:
   - Both forward and reverse entries removed.

5. **`erase_vertex`** — removes all incident edges from both directions.

6. **`clear_vertex`** — removes all edges for vertex in both directions.

7. **Copy/move** — verify reverse adjacency is preserved.

8. **Non-bidirectional unchanged** — `Bidirectional=false` graph compiles and
   works identically to before. `static_assert(!bidirectional_adjacency_list<G>)`.

9. **Views integration** — `in_incidence(g, u)` and `in_neighbors(g, u)` work
   on bidirectional dynamic_graph.

10. Test with at least 2 trait types (e.g., `vov` and `vol`).

#### 8.6 Register test and build

### Merge gate

- [ ] Full test suite passes.
- [ ] All 6 mutator invariants from design doc §10.5 are tested and pass.
- [ ] Non-bidirectional mode has identical behavior (no regressions).
- [ ] `bidirectional_adjacency_list` concept satisfied.

---

## Phase 9 — Algorithm updates (Kosaraju + transpose_view)

**Goal:** Optimize Kosaraju's SCC to use `in_edges()` when available;
add `transpose_view` as a zero-cost adaptor.

**Why ninth:** Algorithms depend on container support (Phase 8) and the
edge accessor infrastructure (Phase 7).

### Files to modify

| File | Action |
|---|---|
| `include/graph/algorithm/connected_components.hpp` | Add `if constexpr (bidirectional_adjacency_list<G>)` branch using `in_edges_accessor` |
| `tests/algorithms/CMakeLists.txt` | Register new test file |

### Files to create

| File | Content |
|---|---|
| `include/graph/views/transpose.hpp` | `transpose_view` wrapper |
| `tests/algorithms/test_scc_bidirectional.cpp` | SCC tests with bidirectional graph |
| `tests/views/test_transpose.cpp` | Transpose view tests |

### Steps

#### 9.1 Add `transpose_view`

A lightweight wrapper that swaps `edges()` ↔ `in_edges()`:

```cpp
template <bidirectional_adjacency_list G>
class transpose_view {
  G* g_;
public:
  // ADL friends: edges(tv, u) → in_edges(*g_, u)
  //              in_edges(tv, u) → edges(*g_, u)
  // Forward all other CPOs to underlying graph
};
```

#### 9.2 Optimize Kosaraju's SCC

Add a compile-time branch:
```cpp
if constexpr (bidirectional_adjacency_list<G>) {
  // Use in_edges_accessor for second DFS pass
} else {
  // Keep existing edge-rebuild approach
}
```

#### 9.3 Create tests and build

### Merge gate

- [ ] Full test suite passes.
- [ ] SCC produces correct results on bidirectional graph.
- [ ] `transpose_view` swaps edge directions correctly.

---

## Phase 10 — Documentation

**Goal:** Update all documentation per design doc §13.

### Files to modify

Per the table in design doc §13.1 (11 files), plus:

| File | Action |
|---|---|
| `docs/index.md` | Add "Bidirectional edge access" feature |
| `docs/getting-started.md` | Add incoming edges section |
| `docs/user-guide/views.md` | Add `in_incidence`, `in_neighbors` |
| `docs/user-guide/algorithms.md` | Note bidirectional benefits |
| `docs/user-guide/containers.md` | Document `Bidirectional` parameter |
| `docs/reference/concepts.md` | Add `bidirectional_adjacency_list` |
| `docs/reference/cpo-reference.md` | Add all incoming CPOs |
| `docs/reference/type-aliases.md` | Add `in_edge_t` etc. |
| `docs/reference/adjacency-list-interface.md` | Add incoming edge section |
| `docs/contributing/cpo-implementation.md` | Add `in_edges` example |
| `README.md` | Update feature highlights |
| `CHANGELOG.md` | Add entry |

### Files to create

| File | Content |
|---|---|
| `docs/user-guide/bidirectional-access.md` | Tutorial guide |

### Steps

1. Update each file per the table.
2. Create the tutorial guide with examples using `in_edges`, `in_incidence`,
   reverse BFS, and bidirectional `dynamic_graph`.
3. Add CHANGELOG entry.
4. Build docs and verify no broken links.

### Merge gate

- [ ] All docs updated.
- [ ] Tutorial compiles (code examples are tested).
- [ ] No broken internal links.

---

## Phase summary

| Phase | Title | New files | Modified files | Key deliverable |
|---|---|---|---|---|
| 1 | `in_edges`/`in_degree` CPOs + aliases | 1 test | 2 | Core CPOs + type aliases |
| 2 | `find_in_edge`/`contains_in_edge` + traits | 3 tests | 3 | Complete CPO surface |
| 3 | Concepts + re-exports | 1 test | 2 | `bidirectional_adjacency_list` concept |
| 4 | Undirected container support | 1 test | 1 | First real bidirectional container |
| 5 | `in_incidence`/`in_neighbors` views | 2 headers + 2 tests | 1 | Incoming-edge views |
| 6 | Pipe-syntax adaptors | — | 2 | `g \| in_incidence(uid)` |
| 7 | BFS/DFS/topo EdgeAccessor | 1 header + 1 test | 3 | Reverse traversal |
| 8 | `dynamic_graph` bidirectional | 1 test | 1 | Directed bidirectional container |
| 9 | Algorithms | 2 headers + 2 tests | 1 | Kosaraju + transpose |
| 10 | Documentation | 1 guide | 12 | Complete docs |

**Total estimated effort:** 11-15 days (same as design doc estimate)

---

## Safety principles

1. **Each phase adds only; nothing is removed or renamed.**
   Existing tests pass between every phase.

2. **Default template arguments preserve source compatibility.**
   `EdgeAccessor = out_edges_accessor` means no existing call site changes.

3. **Stub graphs in early phases isolate CPO testing from container code.**
   Phases 1-3 don't touch any container — they use lightweight test stubs.

4. **The simplest container change comes first (Phase 4).**
   Undirected just forwards to the existing `edges()` function.

5. **The most complex change (Phase 8) comes last in the implementation
   sequence**, after all the infrastructure it depends on is proven.

6. **Each phase has an explicit merge gate** — a checklist of conditions
   that must pass before proceeding.
