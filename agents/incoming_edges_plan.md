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

## Phase 1 — `in_edges` and `in_degree` CPOs + type aliases  ✅ Done

**Status:** Complete. All 11 test cases pass; 4316/4316 tests pass with zero
regressions. Committed on branch `incoming`.

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
using in_edge_range_t = decltype(in_edges(std::declval<G&>(), std::declval<vertex_t<G>>()));

template <typename G>
using in_edge_iterator_t = std::ranges::iterator_t<in_edge_range_t<G>>;

template <typename G>
using in_edge_t = std::ranges::range_value_t<in_edge_range_t<G>>;
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

Key difference from `out_degree`: the `_default` tier calls `in_edges(g, u)` instead
of `out_edges(g, u)`.

#### 1.4 Add convenience aliases

> **Note:** As of the primary/alias swap, `out_edges`, `out_degree`, and
> `find_out_edge` are the primary CPO instances. The shorter names `edges`,
> `degree`, and `find_vertex_edge` are convenience aliases.

```cpp
inline namespace _cpo_instances {
  inline constexpr auto& edges            = out_edges;
  inline constexpr auto& degree           = out_degree;
  inline constexpr auto& find_vertex_edge = find_out_edge;
}
```

#### 1.5 Type aliases (primary → alias)

> **Note:** `out_edge_range_t`, `out_edge_iterator_t`, and `out_edge_t` are now
> the primary type alias definitions. The old names are convenience aliases.

```cpp
template <typename G>
using out_edge_range_t = decltype(out_edges(std::declval<G&>(), ...));

template <typename G>
using out_edge_iterator_t = std::ranges::iterator_t<out_edge_range_t<G>>;

template <typename G>
using out_edge_t = std::ranges::range_value_t<out_edge_range_t<G>>;

// Convenience aliases:
template <typename G> using vertex_edge_range_t    = out_edge_range_t<G>;
template <typename G> using vertex_edge_iterator_t  = out_edge_iterator_t<G>;
template <typename G> using edge_t                  = out_edge_t<G>;
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

4. **Type alias verification** — `in_edge_range_t<G>`,
   `in_edge_iterator_t<G>`, `in_edge_t<G>` all compile and produce
   expected types.

5. **Mixed-type test** — a stub graph where `out_edge_t<G>` is
   `pair<int, double>` but `in_edge_t<G>` is just `int`. Verify both
   aliases are independently deduced.

6. **`edges` / `degree` / `find_vertex_edge` aliases** — verify they
   are the exact same object as `out_edges` / `out_degree` / `find_out_edge`
   (use `static_assert(&edges == &out_edges)`).

7. **`in_degree` CPO** — test member, ADL, and default (`size(in_edges)`)
   resolution tiers.

8. **Alias type verification** — verify `edge_t<G>` is `out_edge_t<G>` etc.

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

- [x] Full test suite passes with zero regressions (4316/4316).
- [x] All 8 test scenarios pass (11 test cases total).
- [x] `in_edge_t<G> != edge_t<G>` mixed-type test passes.
- [x] `out_edges` alias identity check passes.

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

**Goal:** Define `in_edge_range`, `bidirectional_adjacency_list`,
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
concept in_edge_range = std::ranges::forward_range<R>;

template <class G>
concept bidirectional_adjacency_list =
    adjacency_list<G> &&
    requires(G& g, vertex_t<G> u, in_edge_t<G> ie) {
      { in_edges(g, u) } -> in_edge_range<G>;
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
using adj_list::in_edge_range_t;
using adj_list::in_edge_iterator_t;
using adj_list::in_edge_t;

// Outgoing type aliases
using adj_list::out_edge_range_t;
using adj_list::out_edge_iterator_t;
using adj_list::out_edge_t;

// Incoming-edge concepts
using adj_list::in_edge_range;
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

## Phase 4 — `undirected_adjacency_list` incoming-edge support  ✅ Done

**Status:** Complete. 10 new test cases pass (concept, in_edges, in_degree,
find_in_edge, contains_in_edge, integration); 4352/4352 tests pass with zero
regressions. Committed on branch `incoming`.

**Goal:** Add `in_edges()` ADL friends to `undirected_adjacency_list` that
return the same ranges as `edges()`, making undirected graphs model
`bidirectional_adjacency_list` at zero cost. `in_degree` handled automatically
by CPO default tier (`size(in_edges(g, u))`).

**Why fourth:** This is the simplest container change (just forwarding to
existing functions) and provides a real container for integration testing.

### Files modified

| File | Action |
|---|---|
| `include/graph/container/undirected_adjacency_list.hpp` | Added `in_edges` ADL friends (mutable + const) |
| `tests/container/CMakeLists.txt` | Registered new test file |

### Files created

| File | Content |
|---|---|
| `tests/container/undirected_adjacency_list/test_undirected_bidirectional.cpp` | Integration tests |

### Merge gate

- [x] Full test suite passes (4352/4352).
- [x] `undirected_adjacency_list` models `bidirectional_adjacency_list`.
- [x] `undirected_adjacency_list` models `index_bidirectional_adjacency_list`.
- [x] `in_edges(g, u)` and `edges(g, u)` produce identical results.
- [x] `in_degree(g, u) == degree(g, u)` for all vertices.
- [x] `find_in_edge` and `contains_in_edge` work correctly on undirected graphs.

---

## Phase 5 — Edge accessor + parameterized `incidence` / `neighbors` views

**Goal:** Introduce `out_edge_accessor` / `in_edge_accessor` structs in a
new header, then add an `Accessor` template parameter to `incidence_view`,
`basic_incidence_view`, `neighbors_view`, and `basic_neighbors_view`.  A
single parameterized class serves both outgoing and incoming iteration —
no class duplication.  Factory functions provide the ergonomic names
(`out_incidence`, `in_incidence`, `incidence` alias).

**Design rationale (edge accessor):**

An *edge accessor* is a stateless policy object that bundles three operations:

| Method | `out_edge_accessor` | `in_edge_accessor` |
|---|---|---|
| `edges(g, u)` | `adj_list::edges(g, u)` | `adj_list::in_edges(g, u)` |
| `neighbor_id(g, e)` | `adj_list::target_id(g, e)` | `adj_list::source_id(g, e)` |
| `neighbor(g, e)` | `adj_list::target(g, e)` | `*adj_list::find_vertex(g, adj_list::source_id(g, e))` |

By defaulting the accessor to `out_edge_accessor`, existing code is
source-compatible.  The same accessor type will be reused for BFS / DFS /
topological-sort parameterization in Phase 7.

**Naming convention** (matches CPO pattern):

| Factory (primary) | Factory (alias) | View type |
|---|---|---|
| `out_incidence(g, u)` | `incidence(g, u)` | `incidence_view<G, void, out_edge_accessor>` |
| `in_incidence(g, u)` | — | `incidence_view<G, void, in_edge_accessor>` |
| `out_neighbors(g, u)` | `neighbors(g, u)` | `neighbors_view<G, void, out_edge_accessor>` |
| `in_neighbors(g, u)` | — | `neighbors_view<G, void, in_edge_accessor>` |

Same pattern for `basic_*` variants and EVF / VVF overloads.

**Why fifth:** Views consume the CPOs from Phases 1-3 and can be tested
against the undirected container from Phase 4.

### Files to create

| File | Content |
|---|---|
| `include/graph/views/edge_accessor.hpp` | `out_edge_accessor`, `in_edge_accessor` |
| `tests/views/test_in_incidence.cpp` | View tests for `in_incidence` |
| `tests/views/test_in_neighbors.cpp` | View tests for `in_neighbors` |

### Files to modify

| File | Action |
|---|---|
| `include/graph/views/incidence.hpp` | Add `Accessor` template param to all view classes; derive types from accessor; update iterators |
| `include/graph/views/neighbors.hpp` | Same transformation |
| `include/graph/graph.hpp` | Include `edge_accessor.hpp`; re-export accessor types |
| `tests/views/CMakeLists.txt` | Register new test files |

### Steps

#### 5.1 Create `include/graph/views/edge_accessor.hpp`

```cpp
#pragma once
#include <graph/adj_list/detail/graph_cpo.hpp>
#include <graph/adj_list/adjacency_list_concepts.hpp>

namespace graph::views {

/// Policy for outgoing-edge iteration (default).
struct out_edge_accessor {
  template <adj_list::adjacency_list G>
  [[nodiscard]] constexpr auto edges(G& g, adj_list::vertex_t<G> u) const {
    return adj_list::edges(g, u);
  }

  template <adj_list::adjacency_list G>
  [[nodiscard]] constexpr auto neighbor_id(G& g, adj_list::edge_t<G> e) const {
    return adj_list::target_id(g, e);
  }

  template <adj_list::adjacency_list G>
  [[nodiscard]] constexpr auto neighbor(G& g, adj_list::edge_t<G> e) const {
    return adj_list::target(g, e);
  }
};

/// Policy for incoming-edge iteration.
struct in_edge_accessor {
  template <adj_list::bidirectional_adjacency_list G>
  [[nodiscard]] constexpr auto edges(G& g, adj_list::vertex_t<G> u) const {
    return adj_list::in_edges(g, u);
  }

  template <adj_list::bidirectional_adjacency_list G>
  [[nodiscard]] constexpr auto neighbor_id(G& g, adj_list::in_edge_t<G> e) const {
    return adj_list::source_id(g, e);
  }

  template <adj_list::bidirectional_adjacency_list G>
  [[nodiscard]] constexpr auto neighbor(G& g, adj_list::in_edge_t<G> e) const {
    return *adj_list::find_vertex(g, adj_list::source_id(g, e));
  }
};

} // namespace graph::views
```

Notes:
- `out_edge_accessor` is constrained on `adjacency_list` (the minimum).
- `in_edge_accessor` is constrained on `bidirectional_adjacency_list`.
- Both are stateless — `[[no_unique_address]]` in the view makes them zero-cost.
- `in_edge_accessor::neighbor()` uses `find_vertex(g, source_id(g, e))`
  because there is no `source()` CPO.

#### 5.2 Parameterize `incidence_view` with `Accessor` (incidence.hpp)

Add `#include <graph/views/edge_accessor.hpp>` to the includes.

Change every view class template to accept a trailing `class Accessor`:

```cpp
template <adj_list::adjacency_list G, class EVF = void,
          class Accessor = out_edge_accessor>
class incidence_view;
```

In the `void` specialization (`incidence_view<G, void, Accessor>`):

1. **Derive types from accessor:**
   ```cpp
   using edge_range_type = decltype(
       std::declval<const Accessor&>().edges(
           std::declval<G&>(), std::declval<vertex_type>()));
   using edge_type = std::ranges::range_value_t<edge_range_type>;
   ```

2. **Store accessor:**
   ```cpp
   [[no_unique_address]] Accessor accessor_{};
   ```

3. **Update iterator** — add `const Accessor* acc_` member; use
   `acc_->neighbor_id(*g_, current_)` in `operator*()`.

4. **Update `begin()` / `end()` / `size()`** — call
   `accessor_.edges(*g_, source_)` instead of `adj_list::edges(...)`.

5. **Pass `&accessor_`** to iterator construction.

Same transformation for the non-void EVF specialization and both
`basic_incidence_view` specializations.

Deduction guides gain the `Accessor` parameter with default:
```cpp
template <adj_list::adjacency_list G>
incidence_view(G&, adj_list::vertex_t<G>)
    -> incidence_view<G, void, out_edge_accessor>;

template <adj_list::adjacency_list G, class EVF>
incidence_view(G&, adj_list::vertex_t<G>, EVF)
    -> incidence_view<G, EVF, out_edge_accessor>;
```

#### 5.3 Add `out_incidence` / `in_incidence` factory functions

After the existing `incidence()` factories (which remain unchanged and
serve as aliases), add:

```cpp
// --- out_incidence: explicit outgoing factories ---
template <adj_list::adjacency_list G>
[[nodiscard]] constexpr auto out_incidence(G& g, adj_list::vertex_t<G> u) noexcept {
  return incidence_view<G, void, out_edge_accessor>(g, u);
}
template <adj_list::adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::edge_t<G>>
[[nodiscard]] constexpr auto out_incidence(G& g, adj_list::vertex_t<G> u, EVF&& evf) {
  return incidence_view<G, std::decay_t<EVF>, out_edge_accessor>(
      g, u, std::forward<EVF>(evf));
}
// uid overloads ...

// --- in_incidence: incoming factories ---
template <adj_list::bidirectional_adjacency_list G>
[[nodiscard]] constexpr auto in_incidence(G& g, adj_list::vertex_t<G> u) noexcept {
  return incidence_view<G, void, in_edge_accessor>(g, u);
}
template <adj_list::bidirectional_adjacency_list G, class EVF>
requires edge_value_function<EVF, G, adj_list::in_edge_t<G>>
[[nodiscard]] constexpr auto in_incidence(G& g, adj_list::vertex_t<G> u, EVF&& evf) {
  return incidence_view<G, std::decay_t<EVF>, in_edge_accessor>(
      g, u, std::forward<EVF>(evf));
}
// uid overloads ...

// basic_out_incidence / basic_in_incidence — same pattern ...
```

Existing `incidence()` / `basic_incidence()` factories are **unchanged** —
they already create `out_edge_accessor` views by default.

#### 5.4 Same transformation for `neighbors.hpp`

- Add `Accessor` template param to `neighbors_view` and
  `basic_neighbors_view`.
- Switch from `adj_list::edges(...)` to `accessor_.edges(...)`.
- Switch from `adj_list::target(...)` / `adj_list::vertex_id(...)` to
  `accessor_.neighbor(...)` / `accessor_.neighbor_id(...)`.
- Add `out_neighbors()` / `in_neighbors()` factories; keep `neighbors()`
  as the alias (unchanged, creates `out_edge_accessor` by default).
- Same for `basic_out_neighbors()` / `basic_in_neighbors()`.

#### 5.5 Update `graph.hpp`

- Add `#include <graph/views/edge_accessor.hpp>`.
- Re-export `out_edge_accessor`, `in_edge_accessor` into `graph::views`.
- Re-export `out_incidence`, `in_incidence`, `out_neighbors`,
  `in_neighbors`, `basic_out_incidence`, `basic_in_incidence`,
  `basic_out_neighbors`, `basic_in_neighbors` factory functions.

#### 5.6 Create test files

**`test_in_incidence.cpp`** — mirror `test_incidence.cpp`:
- Use `undirected_adjacency_list` (from Phase 4) as the bidirectional graph.
- Verify `in_incidence(g, u)` yields `{source_id, edge}` tuples.
- Verify `in_incidence(g, u, evf)` yields `{source_id, edge, value}`.
- Verify `basic_in_incidence(g, uid)` yields `{source_id}`.
- Verify factory function overloads (descriptor and ID).
- Verify `incidence_view<G>` is the same type as
  `incidence_view<G, void, out_edge_accessor>` (compile-time).
- Verify `incidence_view<G, void, in_edge_accessor>` is a different type.

**`test_in_neighbors.cpp`** — mirror `test_neighbors.cpp`:
- Verify `in_neighbors(g, u)` yields `{source_id, vertex}` tuples.
- Verify `basic_in_neighbors(g, uid)` yields `{source_id}`.
- Verify alias equivalence.

#### 5.7 Register tests and build

### Merge gate

- [ ] Full test suite passes (zero regressions — default `Accessor` preserves
      all existing `incidence_view` / `incidence()` usage).
- [ ] `in_incidence(g, u)` iterates incoming edges, yields `source_id`.
- [ ] `in_neighbors(g, u)` iterates source vertices.
- [ ] All factory function overloads work (descriptor + uid, with/without EVF).
- [ ] `incidence_view<G>` defaults to `out_edge_accessor` (compile-time).
- [ ] Accessor is zero-cost (`sizeof(incidence_view<G>)` unchanged).

---

## Phase 6 — Pipe-syntax adaptors (rename + incoming)

**Goal:** Add `out_incidence` / `in_incidence` / `out_neighbors` /
`in_neighbors` adaptor instances with `incidence` / `neighbors` as aliases.
The adaptor closures now forward to the parameterized views from Phase 5.

**Why sixth:** Adaptors depend on the accessor-parameterized views from Phase 5.

### Files to modify

| File | Action |
|---|---|
| `include/graph/views/adaptors.hpp` | Add new adaptor closures using accessor-parameterized views |
| `tests/views/test_adaptors.cpp` | Add pipe-syntax tests for incoming views |

### Steps

#### 6.1 Add outgoing-primary and incoming adaptor closures (adaptors.hpp)

Rename existing adaptor classes and instances to `out_*` primary names.
Then add new `in_*` adaptor closures that construct the incoming accessor
variant of the views. Keep the short names as aliases:

```cpp
// Renamed adaptor classes:
//   incidence_adaptor_fn       → out_incidence_adaptor_fn
//   basic_incidence_adaptor_fn → basic_out_incidence_adaptor_fn
//   neighbors_adaptor_fn       → out_neighbors_adaptor_fn
//   basic_neighbors_adaptor_fn → basic_out_neighbors_adaptor_fn

// New incoming adaptor classes — same pattern but create
//   in_edge_accessor views:
//   in_incidence_adaptor_fn, basic_in_incidence_adaptor_fn
//   in_neighbors_adaptor_fn, basic_in_neighbors_adaptor_fn

namespace graph::views::adaptors {

// Primary outgoing
inline constexpr out_incidence_adaptor_fn       out_incidence{};
inline constexpr basic_out_incidence_adaptor_fn basic_out_incidence{};
inline constexpr out_neighbors_adaptor_fn       out_neighbors{};
inline constexpr basic_out_neighbors_adaptor_fn basic_out_neighbors{};

// Aliases (incidence = out_incidence, etc.)
inline constexpr auto& incidence       = out_incidence;
inline constexpr auto& basic_incidence = basic_out_incidence;
inline constexpr auto& neighbors       = out_neighbors;
inline constexpr auto& basic_neighbors = basic_out_neighbors;

// Incoming
inline constexpr in_incidence_adaptor_fn       in_incidence{};
inline constexpr basic_in_incidence_adaptor_fn basic_in_incidence{};
inline constexpr in_neighbors_adaptor_fn       in_neighbors{};
inline constexpr basic_in_neighbors_adaptor_fn basic_in_neighbors{};

} // namespace graph::views::adaptors
```

#### 6.2 Add tests to `test_adaptors.cpp`

```cpp
TEST_CASE("pipe: g | out_incidence(uid)", "[adaptors][out_incidence]") { ... }
TEST_CASE("pipe: g | in_incidence(uid)", "[adaptors][in_incidence]") { ... }
TEST_CASE("pipe: g | in_incidence(uid, evf)", "[adaptors][in_incidence]") { ... }
TEST_CASE("pipe: g | basic_in_incidence(uid)", "[adaptors][basic_in_incidence]") { ... }
TEST_CASE("pipe: g | in_neighbors(uid)", "[adaptors][in_neighbors]") { ... }
TEST_CASE("pipe: g | basic_in_neighbors(uid)", "[adaptors][in_neighbors]") { ... }
TEST_CASE("pipe: incidence/neighbors aliases", "[adaptors][aliases]") {
  // Verify &incidence == &out_incidence, &neighbors == &out_neighbors
}
```

#### 6.3 Build and run full test suite

### Merge gate

- [ ] Full test suite passes (zero regressions — alias forwarding ensures
      existing `incidence`/`neighbors` usage still compiles).
- [ ] `g | out_incidence(uid)` produces same results as `out_incidence(g, uid)`.
- [ ] `g | in_incidence(uid)` produces same results as `in_incidence(g, uid)`.
- [ ] `incidence` / `neighbors` aliases resolve to `out_incidence` / `out_neighbors`.
- [ ] All 8 pipe expressions from design doc §8.4 work.

---

## Phase 7 — BFS/DFS/topological-sort `Accessor` parameterization

**Goal:** Add an `Accessor` template parameter (defaulting to
`out_edge_accessor` from Phase 5) to all 6 traversal view classes and their
factory functions.  Replace every hardcoded `adj_list::edges()` call with
`accessor_.edges()`.  This reuses the same accessor types introduced in
Phase 5 — no new accessor headers are needed.

**Why seventh:** Traversal views already work with the accessor interface.
This is a straightforward mechanical change now that `out_edge_accessor` /
`in_edge_accessor` exist.

### Files to modify

| File | Action |
|---|---|
| `include/graph/views/bfs.hpp` | Add `Accessor` param to `vertices_bfs_view`, `edges_bfs_view`; replace 5 hardcoded `adj_list::edges()` calls with `accessor_.edges()` |
| `include/graph/views/dfs.hpp` | Same for `vertices_dfs_view`, `edges_dfs_view`; replace 5 calls |
| `include/graph/views/topological_sort.hpp` | Same for `vertices_topological_sort_view`, `edges_topological_sort_view`; replace 7 calls |
| `tests/views/CMakeLists.txt` | Register new test file |

### Files to create

| File | Content |
|---|---|
| `tests/views/test_reverse_traversal.cpp` | Reverse BFS/DFS tests |

### Steps

#### 7.1 Parameterize BFS views (bfs.hpp)

For each of `vertices_bfs_view` and `edges_bfs_view`:

1. Add `class Accessor = out_edge_accessor` as the last template parameter
   (before `Alloc` if present, or after the value function type).

2. Store `[[no_unique_address]] Accessor accessor_;` member (zero-cost for
   stateless accessors).

3. Replace every `adj_list::edges(g, v)` or `adj_list::edges(*g_, v)` with
   `accessor_.edges(g, v)` or `accessor_.edges(*g_, v)`.

4. Add factory function overloads accepting `Accessor`:
   ```cpp
   vertices_bfs(g, seed, vvf, accessor)
   edges_bfs(g, seed, evf, accessor)
   ```

5. Existing signatures remain unchanged (default `Accessor`).

#### 7.2 Parameterize DFS views (dfs.hpp)

Same transformation: 5 `adj_list::edges()` calls → `accessor_.edges()`.

#### 7.3 Parameterize topological sort views (topological_sort.hpp)

Same transformation: 7 `adj_list::edges()` calls → `accessor_.edges()`.

#### 7.4 Create `test_reverse_traversal.cpp`

Using an undirected_adjacency_list (where `in_edges == edges`):

1. **Forward BFS** — `vertices_bfs(g, seed)` produces expected order.
2. **Reverse BFS** — `vertices_bfs(g, seed, void_fn, in_edge_accessor{})`
   produces expected order (same as forward for undirected).
3. **Forward DFS** vs **Reverse DFS** — same pattern.
4. **Source-compatibility** — existing call sites `vertices_bfs(g, seed)` and
   `vertices_bfs(g, seed, vvf)` still compile unchanged.

#### 7.5 Build and run full test suite

### Merge gate

- [ ] Full test suite passes (zero regressions in existing BFS/DFS/topo tests).
- [ ] Reverse BFS/DFS with `in_edge_accessor` produces correct traversal.
- [ ] Existing factory signatures compile without changes.
- [ ] No new headers created (reuses `edge_accessor.hpp` from Phase 5).

---

## Phase 8 — `dynamic_graph` bidirectional support  ✅ Done

**Status:** Complete. All steps 8.1–8.10 done. 4391/4391 tests pass on both
GCC-15 and Clang, zero regressions. Direction tags (Option A) fully implemented
with `requires` constraints on tag-dependent member functions.

**Root cause of original failures (resolved):** The `edge_descriptor` was
exclusively out-edge oriented. For in-edges:
- `source_id()` returned the *owning* vertex's ID (which is the *target* for in-edges)
- `target_id(vertex_data)` navigated `.edges()` (the out-edge container) instead of
  `.in_edges()`
- `inner_value(vertex_data)` and `underlying_value(vertex_data)` did the same

All three CPOs (source_id, target_id, edge_value) produced wrong results when the
descriptor wrapped an in-edge for random-access containers (vov). Forward-iterator
containers (vol) work because Tier 1 (`(*uv.value()).source_id()`) bypasses the
descriptor entirely.

### Design Decision: Option A — Direction Tag on `edge_descriptor`

**Chosen approach:** Extend `edge_descriptor` and `edge_descriptor_view` with an
`EdgeDirection` template parameter (defaulting to `out_edge_tag`) rather than
creating separate in-edge descriptor classes.

**Rationale:**
- Users never see `edge_descriptor` directly — they use `out_edge_t<G>` / `edge_t<G>`
  and `in_edge_t<G>`. The descriptor is internal plumbing.
- The ~250 lines of `if constexpr` chains in `inner_value`/`target_id`/`underlying_value`
  are the densest code in the project. A single source of truth avoids maintaining two
  parallel 680-line files.
- Adding `if constexpr (Direction == in_edge_tag)` branches is one more axis in the
  existing `if constexpr` pattern (which already handles random-access vs forward,
  void vs non-void, sourced vs unsourced).
- The direction tag enables CPO dispatch via `is_in_edge_descriptor_v<E>`.

**Mirror principle (A1):** In-edge descriptor methods mirror out-edge descriptor
methods with source↔target roles swapped and `.edges()`↔`.in_edges()` container
access swapped:

| Method | Out-edge (`out_edge_tag`) | In-edge (`in_edge_tag`) |
|---|---|---|
| `source_id()` | Trivial: `source_.vertex_id()` | Navigates: `.in_edges()[idx].source_id()` |
| `target_id(vd)` | Navigates: `.edges()[idx].target_id()` | Trivial: `source_.vertex_id()` |
| `underlying_value(vd)` | Navigates `.edges()` container | Navigates `.in_edges()` container |
| `inner_value(vd)` | Navigates `.edges()` container | Navigates `.in_edges()` container |

**Goal:** Fix all three CPOs for in-edges by adding direction-aware branches to
`edge_descriptor`, `edge_descriptor_view`, descriptor traits, and the CPO
resolution tiers.

**Why eighth:** This is the most complex container change. All prerequisite
infrastructure (CPOs, concepts, views) is already proven by earlier phases.

### Files to modify

| File | Action |
|---|---|
| `include/graph/adj_list/edge_descriptor.hpp` | Add `EdgeDirection` tag param; add `source_id(vertex_data)` overload for in-edges; add `if constexpr` branches in `target_id`, `underlying_value`, `inner_value` to navigate `.in_edges()` |
| `include/graph/adj_list/edge_descriptor_view.hpp` | Add `EdgeDirection` tag param; propagate to `edge_descriptor` construction |
| `include/graph/adj_list/descriptor_traits.hpp` | Update `is_edge_descriptor` to match new signature; add `is_in_edge_descriptor` trait; update concepts |
| `include/graph/detail/edge_cpo.hpp` | Update `_adj_list_descriptor` tiers for source_id (add `source_id(vertex_data)` path for in-edges), target_id (use trivial `source_.vertex_id()` for in-edges), edge_value (use `.in_edges()` for in-edges) |
| `include/graph/container/dynamic_graph.hpp` | Add `in_edges(g,u)` ADL friend returning `edge_descriptor_view<..., in_edge_tag>` |
| `tests/container/CMakeLists.txt` | Already registered |

### Files already created

| File | Content |
|---|---|
| `tests/container/dynamic_graph/test_dynamic_graph_bidirectional.cpp` | Comprehensive tests (880+ lines, 27 test cases) |

### Steps

#### 8.1 Add `Bidirectional` template parameter ✅ Done

Propagated through `dynamic_graph`, `dynamic_edge`, `dynamic_vertex`, all
specializations, all 9 traits files, and 27 other traits headers.

#### 8.2 Add reverse edge storage (conditional) ✅ Done

`dynamic_vertex_bidir_base<..., true, Traits>` stores `edges_type in_edges_`
with `in_edges()` accessor. Empty base for `Bidirectional=false`.

#### 8.3 Update `load_edges` to populate both containers ✅ Done

`load_edges` inserts into both `edges()` and `in_edges()` when Bidirectional.

#### 8.4 Fix `_has_default_uid` in `graph_cpo.hpp` ✅ Done

Updated `_in_edges::_has_default_uid` concept to accept `_has_vertex_member_u`
in addition to `_has_adl_u`.

#### 8.5 Add direction tags and update edge_descriptor ✅ Done

1. **Define direction tags** in `descriptor.hpp` (root definitions):
   ```cpp
   struct out_edge_tag {};
   struct in_edge_tag {};
   ```

2. **Add `EdgeDirection` parameter** to `edge_descriptor<EdgeIter, VertexIter, EdgeDirection>`:
   - Default: `out_edge_tag`
   - Add `static constexpr bool is_in_edge = std::is_same_v<EdgeDirection, in_edge_tag>`
   - Existing `source_id()` (no-arg) behavior unchanged for `out_edge_tag`

3. **Add `source_id(vertex_data)` overload** for `in_edge_tag`:
   - Mirrors `target_id(vertex_data)` — navigates `.in_edges()` container
   - For random-access: `in_edges()[edge_storage_].source_id()`
   - For forward: `(*edge_storage_).source_id()`

4. **Add `if constexpr` branches** to `target_id(vertex_data)`:
   - `out_edge_tag`: existing logic (navigate `.edges()`)
   - `in_edge_tag`: trivial — return `source_.vertex_id()`

5. **Add `if constexpr` branches** to `underlying_value(vertex_data)` and
   `inner_value(vertex_data)`:
   - `out_edge_tag`: existing logic (navigate `.edges()`)
   - `in_edge_tag`: navigate `.in_edges()` instead

#### 8.6 Update `edge_descriptor_view` with direction tag ✅ Done

Added `EdgeDirection` parameter (default `out_edge_tag`), propagated to
`edge_descriptor` construction. Updated `enable_borrowed_range` for 3-param.

#### 8.7 Update `descriptor_traits.hpp` ✅ Done

- All 11 `is_edge_descriptor` specializations updated for 3-param
- Added `is_in_edge_descriptor` and `is_in_edge_descriptor_v` traits
- Updated `edge_descriptor_type` concept

#### 8.8 Update CPO tiers in `edge_cpo.hpp` ✅ Done

- **source_id** tier 4: `if constexpr (_E::is_in_edge)` with fallback safety
  for graphs without `.in_edges()` on vertex data
- **target_id** and **edge_value**: direction-aware via descriptor methods

#### 8.9 Add `in_edges(g,u)` ADL friend in `dynamic_graph.hpp` ✅ Done

Added non-const + const `in_edges(g, u)` ADL friends with `requires(Bidirectional)`
returning `edge_descriptor_view<..., in_edge_tag>`. Added `_wrap_in_edge` helper
in `graph_cpo.hpp` `_in_edges` namespace.

#### 8.10 Build on GCC and Clang, run all tests ✅ Done

4391/4391 tests pass on both GCC-15 and Clang. Also added `requires` constraints
on tag-dependent member functions (`source_id(vertex_data)` requires `is_in_edge`,
`target_id(vertex_data)` split into two constrained overloads with `requires(is_in_edge)`
and `requires(is_out_edge)`) and added `is_out_edge` complement to `is_in_edge`.

### Merge gate

- [x] Full test suite passes (4391/4391 on GCC and Clang).
- [x] All bidirectional test cases pass (69 bidir-related tests).
- [x] Non-bidirectional mode has identical behavior (no regressions).
- [x] `bidirectional_adjacency_list` concept satisfied.
- [x] vov and vol trait types both work correctly.

---

## Phase 9 — Algorithm updates (Kosaraju + transpose_view)

**Goal:** Optimize Kosaraju's SCC to use `in_edges()` when available;
add `transpose_view` as a zero-cost adaptor.

**Why ninth:** Algorithms depend on container support (Phase 8) and the
edge accessor infrastructure (Phase 5/7).

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

#### 9.1 Add `transpose_view` — Done

Created `include/graph/views/transpose.hpp` — zero-cost bidirectional graph
adaptor that swaps edges↔in_edges, target↔source via ADL friends.
Documented limitation: CPO tier-1 (`_native_edge_member`) bypasses ADL for
forward-iterator containers (vol, dol), so transpose_view works correctly
only for random-access containers (vov, vod, dov, dod, dofl).

#### 9.2 Optimize Kosaraju's SCC — Done

Added `kosaraju(G&& g, Component& component)` overload constrained on
`index_bidirectional_adjacency_list`. Uses manual iterative stack-based DFS
with `in_edges`/`source_id` for the second pass — works with ALL container
types (no transpose_view needed). Same O(V+E) complexity with lower constant
factor (no transpose construction).

#### 9.3 Create tests and build — Done

- `tests/views/test_transpose.cpp` — 9 test cases, 14 assertions
  (vertex forwarding, edge swap, degree swap, double-transpose, edge_value,
  empty graph, single vertex)
- `tests/algorithms/test_scc_bidirectional.cpp` — 14 test cases, 68 assertions
  (single vertex, cycle, two SCCs, DAG, complex, self-loops, weighted,
  disconnected, agreement with two-graph overload — tested on both vov and vol)
- Updated `graph.hpp` to include `transpose.hpp`
- 4405/4405 tests pass on GCC-15 and Clang

### Merge gate

- [x] Full test suite passes (4405/4405 on GCC-15 and Clang).
- [x] SCC produces correct results on bidirectional graph (vov + vol).
- [x] `transpose_view` swaps edge directions correctly (vov).

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

| Phase | Title | New files | Modified files | Key deliverable | Status |
|---|---|---|---|---|---|
| 1 | `in_edges`/`in_degree` CPOs + aliases | 1 test | 2 | Core CPOs + type aliases | **Done** |
| 2 | `find_in_edge`/`contains_in_edge` + traits | 3 tests | 3 | Complete CPO surface | Done |
| 3 | Concepts + re-exports | 1 test | 2 | `bidirectional_adjacency_list` concept | Done |
| 4 | Undirected container support | 1 test | 1 | First real bidirectional container | **Done** |
| 5 | Edge accessor + parameterized views | 1 header + 2 tests | 4 | `out/in_edge_accessor` + accessor-parameterized views | Not started |
| 6 | Pipe-syntax adaptors | — | 2 | `g \| in_incidence(uid)` | Not started |
| 7 | BFS/DFS/topo Accessor | 1 test | 3 | Reverse traversal (reuses Phase 5 accessor) | Not started |
| 8 | `dynamic_graph` bidirectional | 1 test | 1 | Directed bidirectional container | **Done** |
| 9 | Algorithms | 2 headers + 2 tests | 1 | Kosaraju + transpose | **Done** |
| 10 | Documentation | 1 guide | 12 | Complete docs | Not started |

**Total estimated effort:** 11-15 days (same as design doc estimate)

---

## Safety principles

1. **Each phase adds only; nothing is removed or renamed.**
   Existing tests pass between every phase.

2. **Default template arguments preserve source compatibility.**
   `Accessor = out_edge_accessor` means no existing call site changes.

3. **The edge accessor is introduced once (Phase 5) and reused everywhere.**
   Phases 5, 6, and 7 all share `out_edge_accessor` / `in_edge_accessor`
   from `edge_accessor.hpp`, eliminating class duplication in views and
   avoiding a separate accessor header in Phase 7.

4. **Stub graphs in early phases isolate CPO testing from container code.**
   Phases 1-3 don't touch any container — they use lightweight test stubs.

5. **The simplest container change comes first (Phase 4).**
   Undirected just forwards to the existing `edges()` function.

6. **The most complex change (Phase 8) comes last in the implementation
   sequence**, after all the infrastructure it depends on is proven.

7. **Each phase has an explicit merge gate** — a checklist of conditions
   that must pass before proceeding.
