# adjacency_matrix Concept — Implementation Plan

Companion to [adjmat_concept.md](adjmat_concept.md).

## Recommended execution order

Perform phases in this order to keep intermediate builds stable:

1. Phase 2 (rename container classes to adjmat/md_adjmat)
2. Phase 1 (define adjacency_matrix<G> concept)
3. Phase 3 (docs and call-site cleanup)
4. Phase 4 (tests)
5. Phase 5 (build and verify)

Reason: adding the adjacency_matrix concept before renaming the container can create
temporary identifier ambiguity in translation units that import both graph::adj_list
and graph::container names.

---

## Phase 1 — Define `adjacency_matrix<G>` concept

**File:** `include/graph/adj_list/adjacency_list_concepts.hpp`

Add the concept after `index_bidirectional_adjacency_list` (currently the last concept in the
Adjacency List section, around line 322). Follow the established doc-comment style of the
surrounding concepts.

**Concept body:**

```cpp
/**
 * @brief Concept for dense adjacency-matrix graphs.
 *
 * An adjacency_matrix refines index_adjacency_list by additionally requiring O(1)
 * edge-existence queries and O(1) edge lookup by source/target vertex ID.
 *
 * Requirements:
 * - Must satisfy index_adjacency_list<G>
 * - contains_out_edge(g, uid, vid) must be valid and return bool (O(1) existence)
 * - find_vertex_edge(g, u, vid) must be valid (O(1) edge lookup)
 *
 * In-edge support is NOT required by this base concept; bidirectionality remains
 * an optional, separate refinement (bidirectional_adjacency_list).
 *
 * @tparam G Graph type
 */
template <class G>
concept adjacency_matrix =
    index_adjacency_list<G> &&
    requires(G& g, vertex_t<G> u, vertex_id_t<G> uid, vertex_id_t<G> vid) {
      { contains_out_edge(g, uid, vid) } -> std::convertible_to<bool>;
      find_vertex_edge(g, u, vid);
    };
```

**Verification:** `graph::container::adjacency_matrix` (old name) already provides both
`contains_out_edge` (via the CPO in `adj_list/detail/graph_cpo.hpp` line 2874 using
`exists()`) and `find_vertex_edge` (via `has_find_vertex_edge`). The static asserts in
Phase 4 will confirm this.

---

## Phase 2 — Rename container class to `adjmat`

**File:** `include/graph/container/adjacency_matrix.hpp`

### 2a. Rename the primary class

Locate the class definition (line ~279):
```cpp
class adjacency_matrix {
```
Change to:
```cpp
class adjmat {
```

Update the delegating constructor call inside the range constructor (line ~317) from
`adjacency_matrix(order)` to `adjmat(order)`.

Update the two `friend` function `requires` clauses (lines ~387 and ~396) from
`std::same_as<std::remove_cvref_t<G>, adjacency_matrix>` to
`std::same_as<std::remove_cvref_t<G>, adjmat>`.

### 2b. Rename mdspan variant to `md_adjmat`

Rename class `md_adjacency_matrix` to `md_adjmat` and update inheritance from
`adjacency_matrix<EV, VId, Directed>` to `adjmat<EV, VId, Directed>`.

Also update:
- constructor names,
- any `using base = ...` aliases,
- doc comments referencing `md_adjacency_matrix`.

### 2c. Remove legacy aliases entirely

Do not add compatibility aliases for `adjacency_matrix` or `md_adjacency_matrix`.

After renaming the classes, remove old-name declarations/usages in this header so only
`adjmat` and `md_adjmat` remain as the supported public type names.

---

## Phase 3 — Update internal call sites

### 3a. File-level `@file` comment and class doc

In `adjacency_matrix.hpp` update the `@brief` and any `@ref adjacency_matrix` occurrences in
doc comments to reference `adjmat` / `md_adjmat`.

### 3b. `graph.hpp` / public include umbrella

**File:** `include/graph/graph.hpp`

Check whether `adjacency_matrix` or related names appear in `using` declarations (line ~159
already has `using adj_list::contains_out_edge`). No graph-container `using` declarations are
expected here, but confirm with a search.

### 3c. Docs cross-references

**File:** `docs/user-guide/containers.md`

- Section heading `## 4. \`adjacency_matrix\`` → `## 4. \`adjmat\``
- All code samples using `adjacency_matrix<>` → `adjmat<>`
- Table entry (line ~21) updated accordingly.
- Replace `md_adjacency_matrix` references/examples with `md_adjmat`.
- Remove references to compatibility aliases.

**File:** `docs/reference/adjacency-list-interface.md`

Confirm the `adjacency_matrix<G>` concept row is current and there are no residual
references to removed legacy traits or old container type names.

---

## Phase 4 — Update tests

**File:** `tests/container/adjacency_matrix/test_adjacency_matrix.cpp`

### 4a. Update type usage

Replace every `adjacency_matrix<...>` used as a type in the test body with `adjmat<...>`.
The `using namespace graph::container;` already at the top means only unqualified names need
updating.

Also replace `md_adjacency_matrix<...>` test usage (if present) with `md_adjmat<...>`.

### 4b. Add concept conformance static asserts

After the existing `static_assert` block (lines 24–29), add:

```cpp
// New concept: adjacency_matrix<G> (concept, not the old type name)
static_assert(graph::adj_list::adjacency_matrix<adjmat<>>,
              "unweighted adjmat must model adjacency_matrix concept");
static_assert(graph::adj_list::adjacency_matrix<adjmat<double>>,
              "weighted adjmat must model adjacency_matrix concept");
```

### 4c. Update test case tags

Replace `[adjacency_matrix]` tag with `[adjmat]` in all `TEST_CASE` macros. Optionally keep
`[adjacency_matrix]` as a second tag during the transition window.

**Decision for test selection:** use CTest **name-based** filtering, not Catch2 tag filtering.
`ctest -R` matches discovered test names, so keep `adjacency_matrix` or `adjmat` in
test-case names during transition to ensure stable selection.

---

## Phase 5 — Build and verify

Run the active development preset build and targeted tests:

```bash
cmake --build build/linux-clang-debug --target graph3-tests
ctest --test-dir build/linux-clang-debug -R "adjmat|adjacency_matrix" -V
```

If this preset has no discovered matrix tests, run the full test executable directly from
the build tree once it is identified (do not rely on tag-only selection).

Expected outcomes:
- Zero compilation errors.
- All adjacency-matrix tests pass.
- No deprecation warnings introduced by aliasing (aliases are removed).

---

## Implementation Status

| Phase | Description | Status |
|-------|-------------|--------|
| 1 | Define `adjacency_matrix<G>` concept in `adjacency_list_concepts.hpp` | done |
| 2a | Rename class `adjacency_matrix` → `adjmat` | done |
| 2b | Rename class `md_adjacency_matrix` → `md_adjmat` and update base type | done |
| 2c | Remove old-name aliases and old-name type references in header | done |
| 3a | Update doc comments inside `adjacency_matrix.hpp` | done |
| 3b | Verify `graph.hpp` umbrella — no changes expected | done |
| 3c | Update `docs/user-guide/containers.md` (`adjmat`/`md_adjmat`) | done |
| 3d | Verify `docs/reference/adjacency-list-interface.md` consistency | done |
| 4a | Update type usage in test file to `adjmat<>` | done |
| 4b | Add concept conformance `static_assert` blocks | done |
| 4c | Update `TEST_CASE` tags | done |
| 5  | Build + `ctest` pass, review diagnostics | done |
