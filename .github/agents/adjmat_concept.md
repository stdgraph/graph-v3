# Create adjacency matrix concept + container rename plan

---
description: Design and implement adjacency-matrix concept support with a safe, staged rename plan for the existing adjacency_matrix container.
model: GPT-5.3-Codex
tools: [execute, read, edit, search, web, agent, todo]
handoffs:
  - label: Start Implementation
    agent: agent
    prompt: Implement the plan exactly as specified, including compatibility aliases, tests, and docs updates.
    send: true
    model: GPT-5.3-Codex
---

## Objective

Introduce the `adjacency_matrix<G>` concept and free the name by renaming the existing dense-matrix container to `adjmat`.

**Why the rename is required:** `adjacency_matrix` is the planned concept name in the project API (see `docs/reference/adjacency-list-interface.md`). The current container occupies that name, causing a collision. Renaming the container to `adjmat` resolves the collision and lets the concept take the canonical name.

## Concept design

- `adjacency_matrix<G>` refines `index_adjacency_list<G>` — it adds O(1) edge-existence and element-access guarantees on top of the standard indexed adjacency-list requirements.
- **In-edge support is not required** by the base concept. Bidirectionality is an optional refinement, consistent with how `bidirectional_adjacency_list` refines `adjacency_list`.
- Required operations beyond `index_adjacency_list`:
    - `contains_out_edge(g, uid, vid)` — O(1) existence check.
    - `find_vertex_edge(g, u, vid)` (or equivalent `find_edge` CPO) — O(1) lookup returning an optional edge.

## Constraints

1. Do not break existing users in one step.
2. Preserve behavior and algorithm compatibility.
3. Keep concept and CPO responsibilities explicit.

## Required outputs

1. `adjacency_matrix<G>` concept defined in `include/graph/adj_list/adjacency_list_concepts.hpp`, refining `index_adjacency_list<G>` with the operations above.
2. Staged rename of the container:
    - rename `graph::container::adjacency_matrix` → `graph::container::adjmat` in `include/graph/container/adjacency_matrix.hpp` (file rename optional; class rename required).
    - keep a `using adjacency_matrix = adjmat<...>` compatibility alias with a deprecation notice.
    - update all internal call sites to the new name.
3. Tests updated or added to cover:
    - `static_assert(adjacency_matrix<adjmat<...>>)` concept conformance.
    - `contains_out_edge` and `find_vertex_edge` behavior on `adjmat`.
    - compatibility alias: code using `adjacency_matrix` still compiles.
4. Documentation updates:
    - `docs/user-guide/containers.md` reflects the `adjmat` container name.
    - `docs/reference/adjacency-list-interface.md` reflects the `adjacency_matrix<G>` concept entry.
    - migration note for existing `adjacency_matrix` users.
5. A short implementation checklist (todo) recorded in the change.

## Scope and file targets

1. Concept: `include/graph/adj_list/adjacency_list_concepts.hpp`.
2. Container: `include/graph/container/adjacency_matrix.hpp`.
3. Tests: `tests/container/adjacency_matrix/test_adjacency_matrix.cpp` and any concept static-assert tests.
4. Docs: `docs/user-guide/containers.md` and `docs/reference/adjacency-list-interface.md`.

## Acceptance criteria

1. Build succeeds for the active development preset.
2. Targeted adjacency-matrix tests pass.
3. Public docs compile/render without broken references.
4. Compatibility alias compiles without errors (deprecation warning is acceptable).

## Execution plan

1. Define `adjacency_matrix<G>` concept in `adjacency_list_concepts.hpp`.
2. Rename container class to `adjmat`; add compatibility alias.
3. Update all internal usages of the old class name.
4. Update tests and docs.
5. Run build/tests and summarize what changed plus any follow-up work.
