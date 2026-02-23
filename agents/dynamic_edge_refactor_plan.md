# Implementation Plan: dynamic_edge → dynamic_out_edge / dynamic_in_edge Refactor

> **Strategy source:** [dynamic_edge_refactor_strategy.md](dynamic_edge_refactor_strategy.md)
>
> This plan is designed for safe, incremental, agent-driven implementation.
> Each phase is self-contained and has explicit build/test gates.

---

## Overview

**Goal:** Rename `dynamic_edge` → `dynamic_out_edge`, introduce `dynamic_in_edge`, remove
the `bool Sourced` template parameter from the entire class hierarchy, and support both
uniform and non-uniform bidirectional edge configurations.

**Key invariant:** Every phase must meet its defined verification gate before merge.
Where a phase temporarily allows test breakage (Phase 4), that exception is explicit and
the next phase must restore full green status.

**Files of interest (reference):**
- Core header: `include/graph/container/dynamic_graph.hpp` (2221 lines)
- Traits: 27 files in `include/graph/container/traits/`
- Test infra: `tests/common/graph_test_types.hpp` (460 lines)
- Container tests: ~48 files in `tests/container/dynamic_graph/`
- Edge comparison tests: `tests/container/dynamic_graph/test_dynamic_edge_comparison.cpp` (450 lines)
  — directly instantiates all 4 `dynamic_edge` specializations with explicit `Sourced=true/false`
- CPO tests: 26 files in `tests/adj_list/cpo/`
- Concept tests: `tests/adj_list/concepts/test_bidirectional_concepts.cpp`

---

## Phase 1 — Add `dynamic_in_edge` (additive only, no existing code changes)

**Goal:** Introduce `dynamic_in_edge` and the detection-idiom aliases in `dynamic_graph_base`.
No existing class, trait, or test is modified. The full test suite must remain green.

### Step 1.1 — Add `dynamic_in_edge` class to `dynamic_graph.hpp`

**Location:** After the last `dynamic_edge` specialization (after line ~627) and before
`dynamic_vertex_bidir_base` (line ~635).

**What to add:**

1. **`dynamic_edge_source` forward-compatible wrapper** — do NOT modify the existing
   `dynamic_edge_source` yet. Instead, create a *new* `dynamic_in_edge_source` base class
   that unconditionally stores `source_id_` (no `Sourced` parameter). This avoids touching
   live code.

   ```cpp
   // Temporary base — will replace dynamic_edge_source in Phase 4a.
   template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
   class dynamic_in_edge_source { ... };  // stores source_id_
   ```

   Members: `source_id()` const/non-const, `vertex_id_type`, constructors for `(source_id)`.

2. **`dynamic_in_edge` — 2 specializations:**

   a. Primary (`EV != void`):
   ```cpp
   template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
   class dynamic_in_edge : public dynamic_in_edge_source<...>,
                           public dynamic_edge_value<EV, VV, GV, VId, /*Sourced=*/true, Bidirectional, Traits>
   ```
   - Inherits `dynamic_edge_value` with `Sourced=true` as a pass-through (it ignores Sourced).
   - Constructors: `(source_id)`, `(source_id, value)`, `(source_id, value&&)`.
   - `operator<=>` compares `source_id()`.
   - `operator==` compares `source_id()` (and value if present).

   b. Specialization (`EV = void`):
   ```cpp
   template <class VV, class GV, class VId, bool Bidirectional, class Traits>
   class dynamic_in_edge<void, VV, GV, VId, Bidirectional, Traits>
        : public dynamic_in_edge_source<...>
   ```
   - Constructor: `(source_id)`.
   - `operator<=>` / `operator==` by `source_id()`.

3. **Forward declaration** — add near line 91:
   ```cpp
   template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
   class dynamic_in_edge;
   ```

**Safety:** This is purely additive — no existing template is modified.

### Step 1.2 — Add `std::hash<dynamic_in_edge>` specialization

**Location:** After the existing `std::hash<dynamic_edge<..., false, ...>>` (after line ~2221).

```cpp
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
struct hash<graph::container::dynamic_in_edge<EV, VV, GV, VId, Bidirectional, Traits>> {
  size_t operator()(const ... & edge) const noexcept {
    return std::hash<VId>{}(edge.source_id());
  }
};
```

### Step 1.3 — Add shared detection-idiom helpers and derived aliases

**Location:** Add helpers in a shared scope (e.g., `graph::container::detail` in
`dynamic_graph.hpp`), then consume them in both `dynamic_graph_base` and
`dynamic_vertex_bidir_base`.

Add shared helpers (preferred names are illustrative):

```cpp
template <class Default, template <class...> class Op, class... Args>
using detected_or_t = /* detection-idiom type */;

template <class T>
using detect_in_edge_type = typename T::in_edge_type;

template <class T>
using detect_in_edges_type = typename T::in_edges_type;

// Derived aliases
using edge_type     = typename Traits::edge_type;
using out_edge_type = edge_type;
using in_edge_type  = detail::detected_or_t<edge_type, detail::detect_in_edge_type, Traits>;
using in_edges_type = detail::detected_or_t<typename Traits::edges_type, detail::detect_in_edges_type, Traits>;
```

**Important:** `edge_type` is already used in `dynamic_graph_base` (it comes from `Traits::edge_type`). The new aliases (`out_edge_type`, `in_edge_type`, `in_edges_type`) are added alongside it. Existing code that uses `edge_type` is unchanged.

### Step 1.4 — Add compile-time safety checks

In `dynamic_graph_base`, add static assertions:

```cpp
// Verify alias consistency
static_assert(std::same_as<in_edge_type, edge_type> ||
              !std::same_as<in_edges_type, typename Traits::edges_type>,
              "Non-uniform traits must also define in_edges_type when in_edge_type differs from edge_type");
```

Also add positive assertions for default uniform behavior when aliases are absent.

### Step 1.5 — Verify

- Build the full project: `cmake --build build/linux-gcc-debug`
- Run the full test suite: `ctest --test-dir build/linux-gcc-debug`
- **Expected:** Zero failures, zero compile errors. New code is dead (unreferenced).

### Step 1.6 — Commit

Message: `feat: add dynamic_in_edge class and detection-idiom aliases (Phase 1)`

### Phase 1 Gate (must pass)

- Build: `cmake --build build/linux-gcc-debug`
- Tests: `ctest --test-dir build/linux-gcc-debug`
- Audit: no runtime behavior changes; additive-only diff in `dynamic_graph.hpp`

### Doc notes for Phase 1
- `dynamic_in_edge` structurally mirrors `dynamic_out_edge` will mirror later; at this point `dynamic_edge` is still the live out-edge class.
- `dynamic_in_edge_source` is a temporary class — Phase 4a will repurpose the existing `dynamic_edge_source` and remove this temporary.
- The detection-idiom aliases are in `dynamic_graph_base` but are not yet used by any live path.

---

## Phase 2 — Wire `dynamic_in_edge` into bidirectional support

**Goal:** `dynamic_vertex_bidir_base<..., true>` uses the graph-derived `in_edges_type`
(which falls back to `edges_type` for all existing traits). Bidirectional `load_edges`
uses `in_edge_type` construction. All existing tests pass unchanged.

### Step 2.1 — Update `dynamic_vertex_bidir_base<..., true>` to use `in_edges_type`

**Current** (line ~671):
```cpp
using edges_type = typename Traits::edges_type;   // used for in_edges storage
```

**Change to:**
```cpp
// Use graph-derived in_edges_type (falls back to edges_type for standard traits)
using in_edges_type_t = detail::detected_or_t<typename Traits::edges_type,
                        detail::detect_in_edges_type,
                        Traits>;
using edges_type = in_edges_type_t;  // rename kept for backward compat of internal member
```

**Key insight:** For all existing standard traits (27 files), `detect_in_edges_type` is absent,
so `in_edges_type_t` falls back to `Traits::edges_type`. Behavior is identical to current. The return
type of `in_edges()` is `edges_type&`, which still resolves to the same type.

### Step 2.2 — Update `load_edges` bidirectional branches

**Current pattern** (3 occurrences — associative, sequential, fallback paths):
```cpp
if constexpr (Sourced) {
  if constexpr (is_void_v<EV>) {
    if constexpr (Bidirectional) {
      emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id));
    }
    emplace_edge(vertex_edges, e.target_id, edge_type(e.source_id, e.target_id));
  } else {
    if constexpr (Bidirectional) {
      emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id, e.value));
    }
    emplace_edge(vertex_edges, e.target_id, edge_type(e.source_id, e.target_id, std::move(e.value)));
  }
}
```

**Change in-edge construction only** (the out-edge construction within the `Sourced` branch stays the same):

Use a temporary constructibility-based dispatch. In Phase 2, `Sourced` still exists,
so `edge_type` when `Sourced=true` takes `(source_id, target_id [, value])`. The new
`in_edge_type` (which defaults to `edge_type` for existing traits) must use the same
constructor. But we want to prepare for Phase 4 where `in_edge_type` will take only
`(source_id [, value])`.

**Approach — helper function in `dynamic_graph_base`:**

```cpp
private:
  // Temporary bridge for in-edge construction (removed in Phase 4d).
  // Existing edge_type takes (source_id, target_id [, value]).
  // Future in_edge_type takes (source_id [, value]).
  template <class InEdge = in_edge_type>
  static constexpr auto make_in_edge(vertex_id_type source_id, vertex_id_type target_id) {
    if constexpr (std::constructible_from<InEdge, vertex_id_type, vertex_id_type>) {
      return InEdge(source_id, target_id);              // legacy: (source, target)
    } else {
      return InEdge(source_id);                         // new: (source) only
    }
  }

  template <class InEdge = in_edge_type, class Val>
  static constexpr auto make_in_edge(vertex_id_type source_id, vertex_id_type target_id, Val&& val) {
    if constexpr (std::constructible_from<InEdge, vertex_id_type, vertex_id_type, Val&&>) {
      return InEdge(source_id, target_id, std::forward<Val>(val));  // legacy
    } else {
      return InEdge(source_id, std::forward<Val>(val));              // new
    }
  }
```

Then replace the 3 bidirectional in-edge construction sites:

```cpp
// Before (example — void EV):
emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id));
// After:
emplace_edge(rev, e.source_id, make_in_edge(e.source_id, e.target_id));

// Before (example — non-void EV, copy for in-edge):
emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id, e.value));
// After:
emplace_edge(rev, e.source_id, make_in_edge(e.source_id, e.target_id, e.value));
```

**Out-edge construction is unchanged.** The `make_in_edge` helper only affects in-edge emplacement.

### Step 2.3 — Verify

- Full build + test suite.
- **Expected:** All existing tests pass. `make_in_edge` dispatches to the legacy (source, target) constructor for all current traits because `in_edge_type == edge_type == dynamic_edge<..., Sourced=true, ...>`.

### Step 2.4 — Commit

Message: `feat: wire in_edge_type into bidir vertex base and load_edges (Phase 2)`

### Phase 2 Gate (must pass)

- Build: `cmake --build build/linux-gcc-debug`
- Tests: `ctest --test-dir build/linux-gcc-debug`
- Audit: `load_edges` reverse insertion uses `make_in_edge(...)` in all 3 insertion paths

### Doc notes for Phase 2
- The `make_in_edge` helper is a temporary bridge. It will be removed in Phase 4d when
  `Sourced` is eliminated and all constructors are standardized.
- `in_edges()` return type is still `Traits::edges_type&` for all existing traits.

---

## Phase 3 — Rename `dynamic_edge` → `dynamic_out_edge` (mechanical rename)

**Goal:** The out-edge class has the correct name. `Sourced` still exists. All tests pass.

### Step 3.1 — Rename the class in `dynamic_graph.hpp`

This is a mechanical find-and-replace within `dynamic_graph.hpp`. The renaming targets:

| What | Location (approx lines) |
|------|------------------------|
| Forward declaration | L91: `class dynamic_edge` → `class dynamic_out_edge` |
| `dynamic_edge_target::edge_type` alias | L175: `using edge_type = dynamic_edge<...>` → `dynamic_out_edge<...>` |
| `dynamic_edge_source::edge_type` alias | L239: same |
| `dynamic_edge_value::edge_type` alias | L322: same |
| Primary class declaration | L405: `class dynamic_edge` → `class dynamic_out_edge` |
| All 4 specialization declarations | L488, L534, L600: same |
| Doxygen comments | `@c dynamic_edge` → `@c dynamic_out_edge` throughout |
| `std::hash` specializations | L2196, L2210: `hash<...dynamic_edge<...>>` → `hash<...dynamic_out_edge<...>>` |

**Pattern:** `\bdynamic_edge\b` → `dynamic_out_edge` (word-boundary match avoids
`dynamic_edge_target`, `dynamic_edge_source`, `dynamic_edge_value`).

**Exclusions:** Do NOT rename `dynamic_edge_target`, `dynamic_edge_source`, `dynamic_edge_value`
— these are base classes that will be refactored in Phase 4.

### Step 3.2 — Add deprecated alias

After the last `dynamic_out_edge` specialization (after line ~627):

```cpp
/// @deprecated Use dynamic_out_edge instead.
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
using dynamic_edge = dynamic_out_edge<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
```

This keeps all 27 traits files and all test files compiling without changes yet.

### Step 3.3 — Rename forward declarations in all 27 traits files

Each traits file has a forward declaration block like:
```cpp
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct xxx_graph_traits;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge;
```

Change `class dynamic_edge` → `class dynamic_out_edge` in all 27 files.

**Because the deprecated alias exists**, the `using edge_type = dynamic_edge<...>` in
each traits file still compiles. This will be updated in Phase 4c.

### Step 3.4 — Update any direct `dynamic_edge` references in test files

Search: `grep -rn "\bdynamic_edge\b" tests/` (excluding `dynamic_edge_target|source|value`).

Most test files use `graph_test_types<Tag>::...` aliases and never name `dynamic_edge`
directly. If any do, update them. The deprecated alias provides a safety net.

### Step 3.5 — Verify

- Full build + test suite.
- Run: `grep -rn "\bdynamic_edge\b" include/ tests/ --include="*.hpp" --include="*.cpp" | grep -v "dynamic_edge_target\|dynamic_edge_source\|dynamic_edge_value\|dynamic_edge ="` to audit remaining references (should only be the deprecated alias and traits' `using edge_type`).

### Step 3.6 — Commit

Message: `refactor: rename dynamic_edge → dynamic_out_edge with deprecated alias (Phase 3)`

### Phase 3 Gate (must pass)

- Build: `cmake --build build/linux-gcc-debug`
- Tests: `ctest --test-dir build/linux-gcc-debug`
- Audit: `grep -rn "\\bdynamic_edge\\b" include/ tests/ --include="*.hpp" --include="*.cpp"` only shows allowed transition references (deprecated alias + intended transitional uses)

---

## Phase 4 — Remove `Sourced` template parameter

**Goal:** Eliminate `bool Sourced` from every template parameter list. This is the highest-risk phase
— work sub-step by sub-step, compiling frequently.

### Step 4a — Remove `Sourced` from edge base classes and `dynamic_out_edge`

**4a.1 — `dynamic_edge_target`** (line ~169):
- Remove `bool Sourced` from template params (7→6 params).
- Update `edge_type` alias to use `dynamic_out_edge<EV, VV, GV, VId, Bidirectional, Traits>` (6 params).

**4a.2 — `dynamic_edge_source`** (line ~234):
- Remove `bool Sourced` template parameter from the primary class.
- **Delete** the `Sourced=false` empty specialization (line ~298). The primary class now
  unconditionally stores `source_id_`.
- Update `edge_type` alias to match new params.
- This class now serves as the base for `dynamic_in_edge` (replacing the temporary
  `dynamic_in_edge_source` from Phase 1).

**4a.3 — `dynamic_edge_value`** (line ~327):
- Remove `bool Sourced` from both specializations (primary and `EV=void`).

**4a.4 — `dynamic_out_edge`** — collapse from 4 to 2 specializations:
- Remove `bool Sourced` from all template params.
- **Delete** the 2 `Sourced=true` specializations (lines ~421–466 and ~488–509) which took
  `(source_id, target_id)` constructors.
- **Promote** the 2 `Sourced=false` specializations (lines ~534–575 and ~600–627) to become
  the new primary template and `EV=void` specialization. Remove `Sourced` from their
  parameter lists:
  - `dynamic_out_edge<EV, VV, GV, VId, Bidirectional, Traits>` — `(target_id)`, `(target_id, val)`.
  - `dynamic_out_edge<void, VV, GV, VId, Bidirectional, Traits>` — `(target_id)`.
- **CRITICAL: Remove `dynamic_edge_source` from `dynamic_out_edge`'s base class list.**
  After 4a.2 removes the empty `Sourced=false` specialization, `dynamic_edge_source`
  unconditionally stores `source_id_`. Out-edges must NOT inherit this — they only need
  `dynamic_edge_target`. Replace the inheritance chain:
  - Before: `dynamic_out_edge : dynamic_edge_source<..., false, ...>, dynamic_edge_value<...>`
  - After:  `dynamic_out_edge : dynamic_edge_target<...>, dynamic_edge_value<...>`
  (This mirrors how `dynamic_in_edge` inherits `dynamic_edge_source` + `dynamic_edge_value`
  but not `dynamic_edge_target`.)

**4a.5 — Update `dynamic_in_edge` to use repurposed `dynamic_edge_source`:**
- Change `dynamic_in_edge`'s base from `dynamic_in_edge_source` to the now-Sourced-free
  `dynamic_edge_source`.
- Also update `dynamic_in_edge`'s `dynamic_edge_value` base class reference — remove
  `Sourced` from its template arguments (it was passed as `Sourced=true` as a pass-through).
- **Delete** the temporary `dynamic_in_edge_source` class from Phase 1.

**4a.6 — Update forward declarations** (line ~91):
- `class dynamic_out_edge` — remove `bool Sourced` (now 6 params).
- Remove `class dynamic_edge` forward declaration (or the deprecated alias will need updating).

**4a.7 — Update deprecated alias:**
```cpp
// The deprecated alias can no longer accept Sourced — remove it or update to a 6-param version.
// Since external code is unlikely to use the 7-param form, REMOVE the alias entirely.
```

**Do NOT compile yet — vertex/graph classes still reference Sourced. Continue to 4b.**

### Step 4b — Remove `Sourced` from vertex and graph classes

**4b.1 — `dynamic_vertex_bidir_base`** (lines ~647, ~665):
- Remove `bool Sourced` from both specializations.
- **Delete** the `static_assert(Sourced, ...)` in the `Bidirectional=true` specialization.
- Update `edges_type` in the bidir specialization: it already uses the detection idiom
  from Phase 2 — just remove `Sourced` from the param list.

**4b.2 — `dynamic_vertex_base`** (line ~701):
- Remove `bool Sourced` from template params.
- Update base class reference to `dynamic_vertex_bidir_base<..., Bidirectional, Traits>` (6 params).
- Update the `edge_type` and `edges_type` aliases defined inside `dynamic_vertex_base`
  (lines ~721–722) — these reference `dynamic_out_edge` and must have `Sourced` removed
  from their template arguments.

**4b.3 — `dynamic_vertex`** — both specializations (lines ~836, ~906):
- Remove `bool Sourced`.
- Update base class reference.

**4b.4 — `dynamic_graph_base`** (line ~956):
- Remove `bool Sourced` from template params.
- Remove `static constexpr bool sourced = Sourced;`.
- Update all internal `edge_type`, `vertex_type` references to 6-param forms.
- **Audit `edge_allocator_type`** (line ~974): currently derived from `edges_type`. In
  non-uniform bidirectional mode, `in_edges_type` may use a different element type
  (`dynamic_in_edge` vs `dynamic_out_edge`). Add an `in_edge_allocator_type` alias
  derived from `in_edges_type` for in-edge container allocation. If both types have
  identical layout (same size/alignment), a single allocator suffices, but add a
  `static_assert` verifying this.

**4b.5 — `dynamic_graph`** — both specializations (lines ~1930, ~2088):
- Remove `bool Sourced` from template params.
- Update base class references.
- Update forward declaration defaults (line ~97): remove `bool Sourced = false`.

**4b.6 — Update `edge_value` friend function** (lines ~1809–1830 in `dynamic_graph_base`):
- This direction-aware friend function dispatches differently for out-edges vs in-edges.
- Remove `Sourced` from template params in the function signature.
- Verify it works for both `dynamic_out_edge` and `dynamic_in_edge`:
  - For out-edges: accesses value via `dynamic_edge_value` base (unchanged).
  - For in-edges: same mechanism — `dynamic_in_edge` also inherits `dynamic_edge_value`.
- If the function uses `Sourced` in any `if constexpr` branch, replace with a
  type-based check (e.g., `is_same_v<Edge, in_edge_type>`).

**Do NOT compile yet — traits still pass Sourced. Continue to 4c.**

### Step 4c — Remove `Sourced` from all 27 traits files

For every file in `include/graph/container/traits/`:

1. Remove `bool Sourced = false` from the template parameter list (6→5 params).
2. Remove `static constexpr bool sourced = Sourced;`.
3. Update `using edge_type = dynamic_out_edge<EV, VV, GV, VId, Bidirectional, xxx_graph_traits>;` (6 params, no `Sourced`).
4. Update `using vertex_type = dynamic_vertex<EV, VV, GV, VId, Bidirectional, xxx_graph_traits>;` (6 params).
5. Update `using graph_type = dynamic_graph<EV, VV, GV, VId, Bidirectional, xxx_graph_traits>;` (6 params).
6. Update the forward declarations at the top (remove `bool Sourced`).

**Pattern is identical across all 27 files.** Script this.

**Standard traits do NOT need `in_edge_type` or `in_edges_type`** — the detection-idiom
fallback in `dynamic_graph_base` (Phase 1) handles it.

**Complete list of 27 traits files:**
```
vov, vod, vol, vofl, vos, vous, vom, voum,
dov, dod, dol, dofl, dos, dous,
mov, mod, mol, mofl, mos, mous, mom,
uov, uod, uol, uofl, uos, uous
```

### Step 4d — Update `load_edges` and remove `make_in_edge`

**4d.1 — Simplify out-edge construction.** Each of the 3 insertion paths currently has an
`if constexpr (Sourced)` guard. With `Sourced` removed:
- The `Sourced=true` branch (which passed `(source_id, target_id)` to out-edge constructors)
  is deleted — `dynamic_out_edge` no longer accepts `source_id`.
- The `Sourced=false` branch (which passed `(target_id)` only) is **kept** and promoted to
  the unconditional path — it was already correct for the new `dynamic_out_edge` API.

**There are 18 `emplace_edge` calls across the 3 paths** (associative, sequential-forward,
fallback). Each path has both `Sourced=true` and `Sourced=false` branches. Audit all 18.

Out-edge construction is now always:
```cpp
if constexpr (is_void_v<EV>) {
  emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id));
} else {
  emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id, std::move(e.value)));
}
```

**4d.2 — Simplify in-edge construction.** Replace `make_in_edge` with direct construction:
```cpp
if constexpr (Bidirectional) {
  auto& rev = vertices_[...].in_edges();
  if constexpr (is_void_v<EV>) {
    emplace_edge(rev, e.source_id, in_edge_type(e.source_id));
  } else {
    emplace_edge(rev, e.source_id, in_edge_type(e.source_id, e.value));  // copy for in-edge
  }
}
```

**4d.3 — Delete `make_in_edge` helper** (added in Phase 2).

**Repeat for all 3 paths:** associative, sequential-forward, fallback.

### Step 4e — Update `dynamic_adjacency_graph` alias

**Current** (line ~139):
```cpp
template <typename Traits>
using dynamic_adjacency_graph = dynamic_graph<..., Traits::sourced, Traits::bidirectional, Traits>;
```

**Change to:**
```cpp
template <typename Traits>
using dynamic_adjacency_graph = dynamic_graph<..., Traits::bidirectional, Traits>;
```
Remove `Traits::sourced`.

### Step 4f — Update `std::hash` specializations

- Update `hash<dynamic_out_edge<...>>` — remove `Sourced` from template params.
  Only one specialization remains (hashes `target_id()` only). The old `Sourced=true`
  specialization is deleted (it hashed `source_id() ^ target_id()`).
- `hash<dynamic_in_edge<...>>` already has no `Sourced` parameter (added in Phase 1).

### Step 4g — Compile (library code only — exclude tests)

There is no library-only CMake target, so use target-level builds to verify headers compile:
```bash
# Build only the example targets (they exercise the full header but not the test code):
cmake --build build/linux-gcc-debug --target basic_usage dijkstra_clrs_example mst_usage_example
```

If examples also reference `Sourced`, create a minimal `phase4_smoke.cpp` translation unit
that includes `<graph/container/dynamic_graph.hpp>` and instantiates one `dynamic_graph`
with default traits. Build that single target to verify header correctness.

**Expected:** Compilation succeeds for library header consumers. Tests will NOT compile yet
(they still reference `Sourced`). That's Phase 5.

### Step 4h — Commit

Message: `refactor: remove Sourced template parameter from entire class hierarchy (Phase 4)`

### Phase 4 Gate (explicit exception)

- Build: example targets (or Phase 4g smoke-test TU) must compile
- Tests: may fail due to expected test API drift — this is expected and resolved in Phase 5
- Audit: `grep -rn "Sourced" include/ --include="*.hpp" --include="*.cpp"` returns zero code references

### Doc notes for Phase 4
- `dynamic_edge_source` is now the unconditional source-id base (no empty specialization).
- `dynamic_out_edge` has 2 specializations (keyed on `EV`), not 4.
- `dynamic_out_edge` no longer inherits `dynamic_edge_source` — only `dynamic_edge_target` + `dynamic_edge_value`.
- `dynamic_in_edge` inherits `dynamic_edge_source` + `dynamic_edge_value` (no `dynamic_edge_target`).
- `make_in_edge` bridge from Phase 2 is removed.
- Deprecated `dynamic_edge` alias is removed.
- All traits are now 5-param: `<EV, VV, GV, VId, Bidirectional>`.
- `edge_allocator_type` verified compatible with both `edges_type` and `in_edges_type`.

---

## Phase 5 — Update tests

**Goal:** All test files compile and pass with the new API.

### Step 5.1 — Update `graph_test_types.hpp`

**5.1.1 — Update tag `traits` template aliases** (all ~27 tags):

Current pattern:
```cpp
struct vov_tag {
  static constexpr const char* name = "vov";
  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::vov_graph_traits<EV, VV, GV, VId, Sourced>;
};
```

Change to (replace `Sourced` with `Bidirectional`):
```cpp
struct vov_tag {
  static constexpr const char* name = "vov";
  template <typename EV, typename VV, typename GV, typename VId, bool Bidirectional = false>
  using traits = graph::container::vov_graph_traits<EV, VV, GV, VId, Bidirectional>;
};
```

**Note:** `Sourced` occupied position 5 in the old signature; `Bidirectional` takes the
same position. Callers passing `true`/`false` for `Sourced` must be audited — the boolean
now means `Bidirectional`, which has different semantics. The `sourced_*` aliases in 5.1.2
below are the primary callers and are deleted. Any other callers passing a 5th argument
must be updated to pass the correct `Bidirectional` value.

**5.1.2 — Replace `sourced_*` aliases with `bidir_*` aliases:**

Remove:
```cpp
using sourced_void = dynamic_graph<void, ..., true, false, Tag::traits<..., true>>;
using sourced_int  = dynamic_graph<int, ..., true, false, Tag::traits<..., true>>;
using sourced_all  = dynamic_graph<int, int, int, ..., true, false, Tag::traits<..., true>>;
```

Add:
```cpp
// Uniform bidirectional: both containers use dynamic_out_edge (edge_type)
using bidir_void = dynamic_graph<void, void, void, VId, true,
                                 typename Tag::template traits<void, void, void, VId, true>>;
using bidir_int  = dynamic_graph<int, void, void, VId, true,
                                 typename Tag::template traits<int, void, void, VId, true>>;
using bidir_all  = dynamic_graph<int, int, int, VId, true,
                                 typename Tag::template traits<int, int, int, VId, true>>;
```

**5.1.3 — Update non-sourced aliases** (remove `Sourced` parameter):

Current: `dynamic_graph<void, void, void, VId, false, false, Tag::traits<..., false>>`
Change: `dynamic_graph<void, void, void, VId, false, Tag::traits<..., false>>`

(Remove the `Sourced` argument; `Bidirectional` takes its position.)

### Step 5.2 — Update individual container test files (~48 files)

For each test file in `tests/container/dynamic_graph/`:

1. **Remove `xxx_sourced` type aliases** (e.g., `using vov_sourced_void = ...`, etc.).
2. **Remove tests that assert `G::sourced == true/false`** (search for `sourced`).
3. **Update remaining type aliases** — remove `Sourced` from any explicit `dynamic_graph<...>` instantiations.
4. **Keep all non-sourced test logic unchanged** — it should compile with the updated `graph_test_types.hpp`.

**Special attention: `test_dynamic_edge_comparison.cpp`** (~450 lines):
- This file directly instantiates all 4 `dynamic_edge` specializations with explicit
  `Sourced=true` and `Sourced=false` template arguments.
- **Rename** `dynamic_edge` → `dynamic_out_edge` in all type aliases.
- **Remove** the `Sourced` template argument from all instantiations.
- **Delete** tests for `Sourced=true` specializations (they test `source_id()` on out-edges,
  which no longer exists).
- **Add** `dynamic_in_edge` test cases for `operator<=>`, `operator==`, and `std::hash`
  that mirror the deleted `Sourced=true` tests but use `source_id()` on in-edges.

**Approach:** For each file:
```bash
grep -n "Sourced\|sourced" <file>
```
If no matches, the file needs no changes beyond what `graph_test_types.hpp` provides.

**Estimated impact per file:**
- Type alias declaration changes: ~2–6 lines
- Sourced-specific test section deletions: ~20–40 lines per file (where present)
- Logic is unchanged for non-sourced tests

### Step 5.3 — Update bidirectional container-native tests

**File:** `tests/container/dynamic_graph/test_dynamic_graph_bidirectional.cpp`

**5.3.1 — Update type aliases** (lines 33–68):

Current:
```cpp
using bidir_vov_int = dynamic_graph<int, void, void, uint32_t, true, true,
                                    vov_graph_traits<int, void, void, uint32_t, true, true>>;
```

Change (remove `Sourced`):
```cpp
using bidir_vov_int = dynamic_graph<int, void, void, uint32_t, true,
                                    vov_graph_traits<int, void, void, uint32_t, true>>;
```

Apply to all 7 type aliases in this file.

**5.3.2 — Add non-uniform bidirectional type aliases and tests:**

Create a new non-uniform traits struct (either in this file or in a shared header):
```cpp
// Non-uniform traits: out-edges use dynamic_out_edge, in-edges use dynamic_in_edge
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t>
struct vov_nonuniform_graph_traits {
  static constexpr bool bidirectional = true;
  using edge_type    = dynamic_out_edge<EV, VV, GV, VId, true, vov_nonuniform_graph_traits>;
  using in_edge_type = dynamic_in_edge<EV, VV, GV, VId, true, vov_nonuniform_graph_traits>;
  using vertex_type  = dynamic_vertex<EV, VV, GV, VId, true, vov_nonuniform_graph_traits>;
  using graph_type   = dynamic_graph<EV, VV, GV, VId, true, vov_nonuniform_graph_traits>;
  using edges_type    = std::vector<edge_type>;
  using in_edges_type = std::vector<in_edge_type>;
  using vertices_type = std::vector<vertex_type>;
};
```

Add non-uniform type aliases and parallel tests:
- `using bidir_nu_vov_int = dynamic_graph<..., vov_nonuniform_graph_traits<int>>;`
- `using bidir_nu_vov_void = dynamic_graph<..., vov_nonuniform_graph_traits<void>>;`

Add test sections that verify uniform and non-uniform produce identical CPO results.

### Step 5.4 — Update CPO bidirectional tests

**5.4.1 — `tests/adj_list/cpo/test_in_edges_cpo.cpp`:**
- Add `dynamic_graph` sections (uniform + non-uniform) alongside existing stub tests.

**5.4.2 — `tests/adj_list/cpo/test_source_id_cpo.cpp`:**
- Add sections verifying `source_id(g, uv)` on out-edges (Tier 4) and in-edges (Tier 1 non-uniform, Tier 4 uniform).

**5.4.3 — `tests/adj_list/cpo/test_find_in_edge_cpo.cpp`, `test_contains_in_edge_cpo.cpp`:**
- Add `dynamic_graph` sections.

**5.4.4 — New file: `tests/adj_list/cpo/test_bidir_cpo_consistency.cpp`:**
- Build the same graph with uniform and non-uniform traits.
- Verify `source_id`, `target_id`, `edge_value`, `in_edges`, `in_degree` produce identical results.
- Add to `tests/adj_list/cpo/CMakeLists.txt`.

**5.4.5 — Centralize non-uniform test traits:**
- Put reusable non-uniform traits in common test infra (`tests/common/graph_test_types.hpp` or
  `tests/common/graph_fixtures.hpp`) instead of file-local duplication.
- Reuse the same type in container-native tests, CPO tests, and concept tests.

### Step 5.5 — Update concept tests

**File:** `tests/adj_list/concepts/test_bidirectional_concepts.cpp`

- Add `STATIC_REQUIRE` checks for both uniform and non-uniform `dynamic_graph` types.
- Verify non-bidirectional `dynamic_graph` does NOT satisfy `bidirectional_adjacency_list`.

### Step 5.6 — Update algorithm test types

**File:** `tests/common/algorithm_test_types.hpp`

Current file does NOT reference `Sourced` — but its types use `graph_test_types<tag>::int_ev`
which internal type alias will have changed (removed `Sourced` param). Verify it still
compiles. Likely no changes needed.

### Step 5.7 — Update examples and benchmarks

- Search for `Sourced` or `dynamic_edge` in `examples/` and `benchmark/`.
- Update any explicit type instantiations.
- Most likely minimal — they tend to use defaults (`Sourced=false`).

### Step 5.8 — Verify

```bash
cmake --build build/linux-gcc-debug
ctest --test-dir build/linux-gcc-debug
```

**Post-verification audit:**
```bash
grep -rn "Sourced" include/ tests/ --include="*.hpp" --include="*.cpp"
grep -rn "dynamic_edge[^_]" include/ tests/ --include="*.hpp" --include="*.cpp"
```

Both should return zero matches (excluding comments/documentation).

### Step 5.9 — Commit

Message: `test: update all tests for Sourced removal and add bidir uniform/non-uniform coverage (Phase 5)`

### Phase 5 Gate (must pass, full green restoration)

- Build: `cmake --build build/linux-gcc-debug`
- Tests: `ctest --test-dir build/linux-gcc-debug` (no failures)
- Audit: zero `Sourced` code references under `include/` and `tests/`

### Doc notes for Phase 5
- `graph_test_types.hpp` tag templates now take `Bidirectional` instead of `Sourced`.
- `sourced_void/int/all` replaced with `bidir_void/int/all` (uniform mode).
- Non-uniform bidirectional tests use a custom `vov_nonuniform_graph_traits` struct.
- CPO consistency test validates that uniform and non-uniform produce identical results.

---

## Phase 6 — Cleanup and documentation

**Goal:** Remove temporary artifacts, update documentation, verify clean state.

### Step 6.1 — Remove deprecated alias (if not already removed in Phase 4)

Verify no `dynamic_edge` alias remains in `dynamic_graph.hpp`.

### Step 6.2 — Update Doxygen comments

Audit `dynamic_graph.hpp` for references to:
- `Sourced` parameter (in comments)
- `dynamic_edge` class name (in `@brief`, `@c`, etc.)
- Outdated constructor documentation

Update all to reference `dynamic_out_edge` / `dynamic_in_edge`.

### Step 6.3 — Update user-facing documentation

- `docs/getting-started.md` — if it references edge types
- `docs/reference/` — edge type documentation
- `docs/migration-from-v2.md` — add migration note for `Sourced` removal
- `README.md` — if it references edge construction

### Step 6.4 — Archive strategy documents

- Move `agents/incoming_edges_design.md` to `agents/archive/` if superseded.
- Move `agents/dynamic_edge_refactor_strategy.md` to `agents/archive/` after implementation.

### Step 6.5 — Final verification

```bash
# Full build on all presets
cmake --build build/linux-gcc-debug && ctest --test-dir build/linux-gcc-debug
cmake --build build/linux-clang-debug && ctest --test-dir build/linux-clang-debug

# Audit for stale references
grep -rn "Sourced" include/ tests/ --include="*.hpp" --include="*.cpp"
grep -rn "\bdynamic_edge\b" include/ tests/ --include="*.hpp" --include="*.cpp"
grep -rn "dynamic_in_edge_source" include/ --include="*.hpp"  # temp class should be gone

# Memory validation
# (Add a static_assert in a test: sizeof(dynamic_out_edge<void,...>) == sizeof(dynamic_in_edge<void,...>))
```

### Step 6.6 — Commit

Message: `docs: cleanup Doxygen, update user docs, archive strategy (Phase 6)`

### Documentation Deliverables Checklist

- API signatures updated in docs for `dynamic_graph` and traits (no `Sourced` parameter).
- Migration note: map old `dynamic_edge<..., Sourced=...>` usage to `dynamic_out_edge` / `dynamic_in_edge`.
- Clarify uniform bidirectional semantics: in-edge container stores `dynamic_out_edge` where raw `target_id_` encodes source.
- Clarify CPO dispatch expectations for `source_id` (Tier 4 for out-edges; Tier 1 or Tier 4 for in-edges depending on mode).
- Update examples showing both default uniform mode and custom non-uniform traits.

### Phase 6 Gate (must pass)

- Build: `cmake --build build/linux-gcc-debug` and `cmake --build build/linux-clang-debug`
- Tests: `ctest --test-dir build/linux-gcc-debug` and `ctest --test-dir build/linux-clang-debug`
- Audit: no transitional symbols (`dynamic_in_edge_source`, deprecated aliases)

---

## Risk Mitigations Summary

| Risk | Mitigation | Phase |
|------|-----------|-------|
| Breaking existing non-bidir tests | Deprecated alias in Phase 3; test updates deferred to Phase 5 | 3, 5 |
| `source_id` CPO breaks for out-edges | Out-edge `source_id` always resolves via descriptor Tier 4 — no code change needed | 4 |
| 27 traits files: mechanical errors | Script the changes; identical pattern in all files | 4c |
| `load_edges` constructor mismatch during transition | `make_in_edge` bridge (Phase 2) removed in Phase 4d | 2, 4d |
| Detection-idiom fallback incorrect | Compile-time `static_assert` in Phase 1.4 | 1 |
| Non-uniform traits missing `in_edges_type` | `static_assert` catches inconsistent traits | 1 |
| Phase 4 is large and risky | Sub-steps 4a–4g; compile at 4g before touching tests | 4 |
| No library-only CMake target | Phase 4g uses example targets or a smoke-test TU as proxy for header-only compilation check | 4g |
| Two bidir modes increase test matrix | Non-uniform tests limited to vov/vol representatives | 5 |

---

## Commit Sequence

| # | Phase | Commit message | Risk |
|---|-------|---------------|------|
| 1 | 1 | `feat: add dynamic_in_edge class and detection-idiom aliases` | Low (additive only) |
| 2 | 2 | `feat: wire in_edge_type into bidir vertex base and load_edges` | Low (behavioral no-op) |
| 3 | 3 | `refactor: rename dynamic_edge → dynamic_out_edge with deprecated alias` | Low (mechanical + alias safety net) |
| 4 | 4 | `refactor: remove Sourced template parameter from entire class hierarchy` | **High** (breaks tests until Phase 5) |
| 5 | 5 | `test: update all tests for Sourced removal and add bidir coverage` | Medium (many files) |
| 6 | 6 | `docs: cleanup Doxygen, update user docs, archive strategy` | Low (non-functional) |

> **Note:** Phases 4 and 5 can be combined into one commit if desired — this keeps the
> repo green at every commit. The trade-off is a larger commit.

---

## Status Tracker

| Phase | Description | Status | Notes |
|-------|-------------|--------|-------|
| 1 | Add `dynamic_in_edge` + detection aliases | COMPLETE | |
| 2 | Wire `in_edge_type` into bidir support | COMPLETE | |
| 3 | Rename `dynamic_edge` → `dynamic_out_edge` | COMPLETE | f560d86 |
| 4a | Remove `Sourced` from edge classes | COMPLETE | 89e1018 |
| 4b | Remove `Sourced` from vertex/graph classes | COMPLETE | 08e6ef6 |
| 4c | Remove `Sourced` from 27 traits files | COMPLETE | 67a0e1f |
| 4d | Update `load_edges`, remove `make_in_edge` | COMPLETE | 7b06988 |
| 4e | Update `dynamic_adjacency_graph` alias | COMPLETE | done in 4b (08e6ef6) |
| 4f | Update `std::hash` specializations | COMPLETE | done in 4a (89e1018) |
| 4g | Compile non-test code | COMPLETE | 01a980a |
| 5.1 | Update `graph_test_types.hpp` | COMPLETE | 9fd9c1f; tags 6-param; sourced_* → bidir_* |
| 5.2 | Update ~48 container test files | COMPLETE | bulk script + manual CPO fixes; `Types::all` → `all_int` |
| 5.3 | Update bidir container-native tests | COMPLETE | non-uniform traits added to bidir/scc/transpose/reverse_traversal |
| 5.4 | Update/add CPO bidir tests | COMPLETE | 5.4.1–5.4.5; 20 new tests; 4329/4329 pass (2026-02-23) |
| 5.5 | Update concept tests | COMPLETE | 8 STATIC_REQUIREs for dynamic_graph; 4337/4337 pass (2026-02-23) |
| 5.6 | Update algorithm test types | COMPLETE | no changes needed; file does not reference `Sourced` |
| 5.7 | Update examples/benchmarks | COMPLETE | no changes needed in examples/; transpose + reverse_traversal fixed |
| 5.8 | Full test suite verification | COMPLETE | 4337/4337 pass (2026-02-23) |
| 5.9 | Commit Phase 5 | COMPLETE | 880ae00 |
| 6.1 | Remove deprecated `dynamic_edge` alias | COMPLETE | alias absent; stale comment in edge_descriptor.hpp fixed |
| 6 | Cleanup + documentation | IN PROGRESS | |
