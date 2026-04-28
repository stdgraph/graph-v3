# Index-Only Vertex Descriptor — Implementation Plan

This plan introduces canonical support for **index-only vertex descriptors** —
vertex descriptors parameterized on an iota iterator rather than a physical
container iterator. This eliminates the current workaround where `compressed_graph`
borrows a `row_index_vector::iterator` solely to satisfy the `vertex_descriptor`
template, even though it never uses the iterator-based access methods.

Reference: discussion on `vertex_descriptor` design (April 2026).

**Branch:** `vert_patterns`

**Invariant:** After every sub-phase, `ctest` passes all existing tests. No
sub-phase may break backward compatibility.

---

## Conventions

| Symbol | Meaning |
|--------|---------|
| **File** | Path relative to repo root |
| **Read** | Files the agent must read for context before editing |
| **Create** | New files to create |
| **Modify** | Existing files to edit |
| **Verify** | Commands to run and expected outcomes |
| **Commit** | Git commit message (conventional-commit style) |

---

## Background

### Problem

`vertex_descriptor<VertexIter>` uses `VertexIter` for two purposes:

1. **Selecting the storage strategy** — random-access → `size_t` index;
   forward → stored iterator.
2. **Binding to a physical container** — `inner_value(container)` and
   `underlying_value(container)` index into a container using the stored value.

For graphs without a physical vertex container (e.g., `compressed_graph`, implicit
graphs, generated graphs), purpose #2 is invalid. Currently `compressed_graph`
works around this by using `row_index_vector::iterator` as the template argument,
which gives it the `size_t` storage path. But:

- It exposes `inner_value()` and `underlying_value()` that are semantically
  meaningless (they'd index into the CSR row_index array, not vertex data).
- The `vertex_descriptor_view` deduction guides require a container, so
  `compressed_graph` must manually specify the iterator type.
- Any future implicit/generated graph would need a similar workaround.

### Solution

1. Define `index_iterator` as the canonical iterator type for index-only vertices.
2. Provide convenience aliases `index_vertex_descriptor` and
   `index_vertex_descriptor_view`.
3. Constrain `inner_value()` and `underlying_value()` so they are unavailable
   on index-only descriptors (compile error instead of silent misuse).
4. Migrate `compressed_graph` to use the new aliases.
5. Add tests and update documentation.

---

## Phase 0 — Verify Baseline

### 0.1 Run Full Test Suite

| Item | Detail |
|------|--------|
| **Action** | Confirm the starting baseline is green. |
| **Verify** | `cd build/linux-gcc-debug && cmake --build . 2>&1 \| tail -5` — build succeeds |
| **Verify** | `cd build/linux-gcc-debug && ctest --output-on-failure` — all tests pass |

---

## Phase 1 — Define Index Iterator and Aliases

### 1.1 Add `index_iterator` and Type Aliases

| Item | Detail |
|------|--------|
| **Read** | `include/graph/adj_list/vertex_descriptor.hpp`, `include/graph/adj_list/vertex_descriptor_view.hpp`, `include/graph/adj_list/descriptor.hpp` |
| **Modify** | `include/graph/adj_list/vertex_descriptor.hpp` |
| **Changes** | After the `vertex_descriptor` class definition (before the closing `} // namespace graph::adj_list`), add: |

```cpp
/// Canonical iterator type for index-only vertices (no physical container).
/// Satisfies random_access_iterator, so vertex_descriptor stores size_t.
using index_iterator = std::ranges::iterator_t<std::ranges::iota_view<std::size_t, std::size_t>>;

/// Vertex descriptor for index-only graphs (no physical vertex container).
using index_vertex_descriptor = vertex_descriptor<index_iterator>;
```

| Item | Detail |
|------|--------|
| **Modify** | `include/graph/adj_list/vertex_descriptor_view.hpp` |
| **Changes** | After the `vertex_descriptor_view` class definition (before the closing `} // namespace graph::adj_list`), add: |

```cpp
/// Vertex descriptor view for index-only graphs (no physical vertex container).
using index_vertex_descriptor_view = vertex_descriptor_view<index_iterator>;
```

| **Verify** | Build succeeds; all existing tests pass |

### 1.2 Verify `index_iterator` Satisfies Existing Concepts

| Item | Detail |
|------|--------|
| **Modify** | `tests/adj_list/descriptors/test_vertex_descriptor.cpp` |
| **Changes** | Add a new test section that verifies the iota iterator works with existing concept machinery: |

```cpp
TEST_CASE("index_vertex_descriptor type properties", "[vertex_descriptor][index]") {
  using graph::adj_list::index_iterator;
  using graph::adj_list::index_vertex_descriptor;
  using graph::adj_list::index_vertex_descriptor_view;

  // index_iterator must satisfy the vertex_iterator concept
  static_assert(graph::adj_list::vertex_iterator<index_iterator>);
  static_assert(graph::adj_list::direct_vertex_type<index_iterator>);
  static_assert(std::random_access_iterator<index_iterator>);

  // Storage type must be size_t
  static_assert(std::same_as<index_vertex_descriptor::storage_type, std::size_t>);

  // vertex_id() returns size_t by value
  index_vertex_descriptor vd{42uz};
  CHECK(vd.vertex_id() == 42);
  CHECK(vd.value() == 42);

  // Increment
  auto vd2 = vd;
  ++vd2;
  CHECK(vd2.vertex_id() == 43);

  // Comparison
  CHECK(vd < vd2);
  CHECK(vd != vd2);
}

TEST_CASE("index_vertex_descriptor_view iteration", "[vertex_descriptor_view][index]") {
  using graph::adj_list::index_vertex_descriptor_view;

  index_vertex_descriptor_view view(std::size_t{0}, std::size_t{5});
  CHECK(view.size() == 5);

  std::vector<std::size_t> ids;
  for (auto desc : view) {
    ids.push_back(desc.vertex_id());
  }
  CHECK(ids == std::vector<std::size_t>{0, 1, 2, 3, 4});
}
```

| **Verify** | Build succeeds; new tests pass; all existing tests pass |
| **Commit** | `feat: add index_iterator alias and index_vertex_descriptor types` |

---

## Phase 2 — Constrain Container-Access Methods

### 2.1 Add `index_only_vertex` Detection Trait

| Item | Detail |
|------|--------|
| **Read** | `include/graph/adj_list/descriptor.hpp` (concept definitions), `include/graph/adj_list/vertex_descriptor.hpp` |
| **Modify** | `include/graph/adj_list/descriptor.hpp` |
| **Changes** | After the `vertex_id_type_t` alias (end of Vertex ID Type Extraction section), add: |

```cpp
// =============================================================================
// Index-Only Vertex Detection
// =============================================================================

/**
 * @brief Concept for index-only vertex iterators (no physical container backing)
 *
 * An index-only vertex iterator is a random-access iterator whose value_type
 * is an integral type (e.g., iota_view iterator). For these iterators,
 * inner_value() and underlying_value() are not meaningful because there is
 * no physical container to index into.
 */
template <typename Iter>
concept index_only_vertex = std::random_access_iterator<Iter> &&
                            std::integral<typename std::iterator_traits<Iter>::value_type>;

/**
 * @brief Concept for container-backed vertex iterators
 *
 * The inverse of index_only_vertex. These iterators reference elements in
 * a physical container, so inner_value() and underlying_value() are valid.
 */
template <typename Iter>
concept container_backed_vertex = vertex_iterator<Iter> && !index_only_vertex<Iter>;
```

| **Verify** | Build succeeds; all existing tests pass |

### 2.2 Add `requires` Clauses to `inner_value()` and `underlying_value()`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/adj_list/vertex_descriptor.hpp` |
| **Modify** | `include/graph/adj_list/vertex_descriptor.hpp` |
| **Changes** | Add a `requires container_backed_vertex<VertexIter>` clause to each of the four methods: `underlying_value(Container&)`, `underlying_value(const Container&)`, `inner_value(Container&)`, `inner_value(const Container&)`. For example: |

```cpp
  template <typename Container>
  [[nodiscard]] constexpr decltype(auto) underlying_value(Container& container) const noexcept
  requires container_backed_vertex<VertexIter>
  {
    // ... existing body unchanged ...
  }
```

Apply identically to all four overloads.

| **Verify** | Build succeeds; all existing tests pass |

### 2.3 Add Compile-Time Tests for Constraint

| Item | Detail |
|------|--------|
| **Modify** | `tests/adj_list/descriptors/test_vertex_descriptor.cpp` |
| **Changes** | Add tests verifying that `inner_value` and `underlying_value` are not callable on `index_vertex_descriptor`: |

```cpp
TEST_CASE("index_vertex_descriptor does not expose container methods", "[vertex_descriptor][index]") {
  using graph::adj_list::index_vertex_descriptor;

  // inner_value() and underlying_value() should NOT be available
  static_assert(!requires(index_vertex_descriptor vd, std::vector<int>& c) {
    vd.inner_value(c);
  });
  static_assert(!requires(index_vertex_descriptor vd, std::vector<int>& c) {
    vd.underlying_value(c);
  });

  // But vertex_id() and value() should still work
  static_assert(requires(index_vertex_descriptor vd) {
    { vd.vertex_id() } -> std::same_as<std::size_t>;
    { vd.value() } -> std::same_as<std::size_t>;
  });
}
```

| **Verify** | Build succeeds; new tests pass; all existing tests pass |
| **Commit** | `feat: constrain inner_value/underlying_value to container-backed descriptors` |

---

## Phase 3 — Migrate `compressed_graph`

### 3.1 Switch `compressed_graph` to `index_iterator`

| Item | Detail |
|------|--------|
| **Read** | `include/graph/container/compressed_graph.hpp` (lines 1070–1220, the `vertices()`, `find_vertex()`, and `vertex_id()` friend functions) |
| **Modify** | `include/graph/container/compressed_graph.hpp` |
| **Changes** | |

1. Add `using adj_list::index_iterator;` to the existing `using` declarations near
   line 36–40.

2. In the `vertices(G&& g)` friend function (~line 1085), replace:
   ```cpp
   using vertex_iter_type =
         std::conditional_t<std::is_const_v<std::remove_reference_t<G>>,
                            typename row_index_vector::const_iterator,
                            typename row_index_vector::iterator>;
   ```
   with:
   ```cpp
   using vertex_iter_type = index_iterator;
   ```
   The const/non-const distinction is irrelevant for index-only descriptors
   (they only store `size_t`).

3. Apply the same change to `vertices(G&& g, const PId& pid)` (~line 1115)
   and `find_vertex(G&& g, const VId2& uid)` (~line 1183).

4. Keep the `vertex_id(const G& g, const VertexDesc& u)` friend function
   unchanged — it only calls `u.vertex_id()` which remains valid.

| **Verify** | Build succeeds; all existing `compressed_graph` tests pass; all other tests pass |
| **Commit** | `refactor: migrate compressed_graph to index_iterator` |

---

## Phase 4 — Export from `graph.hpp`

### 4.1 Add Aliases to Public Header

| Item | Detail |
|------|--------|
| **Read** | `include/graph/graph.hpp` — find the existing `using adj_list::...` block |
| **Modify** | `include/graph/graph.hpp` |
| **Changes** | In the block of `using adj_list::...` declarations, add: |

```cpp
using adj_list::index_iterator;
using adj_list::index_vertex_descriptor;
using adj_list::index_vertex_descriptor_view;
using adj_list::index_only_vertex;
using adj_list::container_backed_vertex;
```

| **Verify** | Build succeeds; all tests pass |
| **Commit** | `feat: export index vertex types from graph.hpp` |

---

## Phase 5 — Update Documentation

### 5.1 Update `vertex-patterns.md`

| Item | Detail |
|------|--------|
| **Read** | `docs/reference/vertex-patterns.md` |
| **Modify** | `docs/reference/vertex-patterns.md` |
| **Changes** | |

1. In the **Foundation Concepts** section, add `index_only_vertex` and
   `container_backed_vertex` to the table.

2. In the **Vertex Storage Patterns → Storage Concepts** table, add a note
   that `direct_vertex_type` includes both container-backed iterators
   (e.g., `vector::iterator`) and index-only iterators (`index_iterator`).

3. In the **`vertex_descriptor<VertexIter>` Class Reference → Member Functions**
   table, add a note that `inner_value()` and `underlying_value()` require
   `container_backed_vertex<VertexIter>`.

4. Add a new subsection **Index-Only Aliases** after the `vertex_descriptor_view`
   class reference:

   ```markdown
   ## Index-Only Aliases

   For graphs without a physical vertex container (e.g., `compressed_graph`,
   implicit graphs), the library provides canonical aliases:

   | Alias | Definition |
   |-------|------------|
   | `index_iterator` | `std::ranges::iterator_t<std::ranges::iota_view<size_t, size_t>>` |
   | `index_vertex_descriptor` | `vertex_descriptor<index_iterator>` |
   | `index_vertex_descriptor_view` | `vertex_descriptor_view<index_iterator>` |

   These descriptors store only a `size_t` index. `vertex_id()` and `value()`
   work normally. `inner_value()` and `underlying_value()` are not available
   (constrained out by `container_backed_vertex`).
   ```

### 5.2 Update `archive/descriptor.md`

| Item | Detail |
|------|--------|
| **Read** | `docs/archive/descriptor.md` |
| **Modify** | `docs/archive/descriptor.md` |
| **Changes** | In the vertex descriptor spec section, add a third iterator category: |

   - **Index-only iterator** (e.g., `iota_view<size_t>::iterator`): Used for
     graphs with no physical vertex container. `inner_value()` and
     `underlying_value()` are not available.

| **Verify** | Review documentation renders correctly (no broken tables or links) |
| **Commit** | `docs: document index-only vertex descriptor aliases and constraints` |

---

## Phase 6 — Final Verification

### 6.1 Full Test Suite

| Item | Detail |
|------|--------|
| **Action** | Run the full test suite across available build configurations. |
| **Verify** | `cd build/linux-gcc-debug && cmake --build . && ctest --output-on-failure` — all pass |
| **Verify** | `cd build/linux-clang-debug && cmake --build . && ctest --output-on-failure` — all pass |

### 6.2 Verify No Regressions in `compressed_graph` Behavior

| Item | Detail |
|------|--------|
| **Action** | Run compressed_graph tests specifically, confirming vertex iteration, find_vertex, vertex_id, edges, and partition queries all work identically. |
| **Verify** | `cd build/linux-gcc-debug && ctest -R compressed --output-on-failure` — all pass |

---

## Summary

| Phase | Description | Files Modified | New Files |
|-------|-------------|----------------|-----------|
| 0 | Verify baseline | — | — |
| 1 | Define `index_iterator`, aliases, basic tests | `vertex_descriptor.hpp`, `vertex_descriptor_view.hpp`, `test_vertex_descriptor.cpp` | — |
| 2 | Constrain `inner_value`/`underlying_value`, add concept | `descriptor.hpp`, `vertex_descriptor.hpp`, `test_vertex_descriptor.cpp` | — |
| 3 | Migrate `compressed_graph` | `compressed_graph.hpp` | — |
| 4 | Export from `graph.hpp` | `graph.hpp` | — |
| 5 | Update documentation | `vertex-patterns.md`, `archive/descriptor.md` | — |
| 6 | Final verification | — | — |
