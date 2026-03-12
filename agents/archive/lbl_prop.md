# Implement a Label Propagation Algorithm

## Reference Material
- Spec: /mnt/d/dev_graph/P1709/D3128_Algorithms/tex/algorithms.tex §Label Propagation (line ~924)
- Reference signatures: /mnt/d/dev_graph/P1709/D3128_Algorithms/src/lp.hpp
- Study for implementation pattern: include/graph/algorithm/mis.hpp
- Study for test pattern: tests/algorithms/test_connected_components.cpp

## Algorithm Summary
Propagate vertex labels by setting each vertex's label to the most popular label of its neighboring vertices. Every vertex voting on its new label represents one iteration of label propagation. Vertex voting order is randomized every iteration. The algorithm will iterate until label convergence, or optionally for a user specified number of iterations. Convergence occurs when no vertex label changes from the previous iteration. O(M) complexity is based on the complexity of one iteration, with number of iterations required for convergence considered small relative to graph size.

Some label propagation implementations use vertex ids as an initial labeling. This is not supported here because the label type can be more generic than the vertex id type. User is responsible for meaningful initial labeling.

## Signatures to Implement
template <index_adjacency_list G, ranges::random_access_range Label, class Gen = default_random_engine, class T = size_t>
void label_propagation(G&& g, Label& label, Gen&& rng = default_random_engine {}, T max_iters = numeric_limits<T>::max());

template <index_adjacency_list G, ranges::random_access_range Label, class Gen = default_random_engine, class T = size_t>
void label_propagation(G&& g, Label& label, ranges::range_value_t<Label> empty_label, Gen&& rng = default_random_engine {}, T max_iters = numeric_limits<T>::max());

## Design Decisions
- **Namespace:** `graph::` — consistent with all other algorithms (mis, connected_components, etc.)
- **Return type:** `void` — matches D3128 spec. No convergence flag or iteration count is returned.
  If callers need to know whether convergence was reached they can re-check the label array.
- **`empty_label` parameter type:** Pass by value (`range_value_t<Label> empty_label`), not by reference.
  The `&` in the reference lp.hpp appears to be a formatting artefact. A sentinel value has no
  reason to be mutable from the caller's side.
- **`empty_label` semantics:** A vertex whose label equals `empty_label` is treated as unlabelled.
  It does not vote and is not counted in neighbor tallies. Once a labelled neighbour propagates to
  it, it acquires that label and participates normally from the next iteration onward.
- **Tie-breaking:** When two or more labels are tied for most-popular among a vertex's neighbours,
  randomly select one of the tied labels using the provided `rng`. This avoids deterministic bias.
- **`requires` clause:** Add
  `requires std::equality_comparable<std::ranges::range_value_t<Label>>`
  to both overloads. The algorithm compares labels for frequency counting and convergence checking.
- **Iteration order:** Shuffle vertex IDs at the start of each iteration using `std::shuffle` with
  the provided `rng`, then process vertices in that shuffled order.
- **Convergence check:** After each iteration compare the new label array against the previous one.
  Stop early as soon as no label changes. Still obey `max_iters`.
- **Label acquisition counts as a change (overload 2):** When an unlabelled vertex acquires a label
  from a neighbour, that counts as a change for convergence purposes. Otherwise the algorithm could
  converge prematurely on the first iteration when only pre-labelled vertices vote.
- **Label array ownership:** The caller pre-allocates `label` sized to `num_vertices(g)` and sets
  initial labels. The algorithm modifies it in-place. Document the precondition clearly.
- **Header guard:** Use `#ifndef GRAPH_LABEL_PROPAGATION_HPP` / `#define GRAPH_LABEL_PROPAGATION_HPP`
  (same pattern as mis.hpp uses `GRAPH_MIS_HPP`).

## Files to Create/Modify
| Action   | File                                                   | What to do                                  |
|----------|--------------------------------------------------------|---------------------------------------------|
| Create   | `include/graph/algorithm/label_propagation.hpp`        | Full implementation + Doxygen doc           |
| Modify   | `include/graph/algorithms.hpp`                         | Add `#include "graph/algorithm/label_propagation.hpp"` |
| Create   | `tests/algorithms/test_label_propagation.cpp`          | Catch2 tests (see Test Plan)                |
| Modify   | `tests/algorithms/CMakeLists.txt`                      | Add `test_label_propagation.cpp` to the test target |

## Implementation Steps
1. Read `tex/algorithms.tex` §Label Propagation (~line 924) and `src/lp.hpp` to confirm spec details.
2. Study `include/graph/algorithm/mis.hpp` for file layout: header guard, namespace, using-declarations,
   Doxygen style (complexity table, supported graph properties, container requirements, @pre/@post,
   exception safety, example usage).
3. Study `tests/algorithms/test_mis.cpp` for test file structure and helper patterns.
4. Implement `include/graph/algorithm/label_propagation.hpp`:
   - File-level Doxygen comment (description, authors, SPDX)
   - Header guard and includes (`graph/graph.hpp`, `<algorithm>`, `<random>`, `<vector>`, `<ranges>`)
   - `using` declarations matching the other algorithm headers
   - Overload 1 (no `empty_label`): implement shuffle → vote → convergence loop
   - Overload 2 (`empty_label` sentinel): same loop, skip unlabelled vertices in tallies
   - Full Doxygen doc block for each overload
5. Add `#include "graph/algorithm/label_propagation.hpp"` to `include/graph/algorithms.hpp`.
6. Write `tests/algorithms/test_label_propagation.cpp` (see Test Plan).
7. Add the new test file to `tests/algorithms/CMakeLists.txt`.
8. Build: `cmake --build build/linux-gcc-debug`
9. Run targeted tests: `ctest --test-dir build/linux-gcc-debug -R label_propagation`
10. Run full suite: `ctest --test-dir build/linux-gcc-debug` — confirm 0 failures.

## Test Plan
All tests use `std::mt19937{42}` as the RNG for reproducibility unless the test specifically
exercises RNG-sensitive behaviour.

### Overload 1 — no `empty_label`
| Test case | What to verify |
|-----------|----------------|
| Empty graph (0 vertices) | Returns without crash; label array untouched |
| Single vertex, no edges | Label unchanged after any number of iterations |
| Single edge, two different labels | After convergence both vertices share one label |
| Path graph (0-1-2-3), all same label | Already converged; 0 iterations actually execute |
| Path graph, alternating labels | Verify stable result after convergence |
| Cycle graph (5 vertices) | Verify all vertices converge to one label |
| Complete graph K4 | Majority label wins; verify all labels equal |
| Disconnected graph (two components, different labels) | Each component converges independently; components may differ |
| `max_iters = 0` | Label array unchanged |
| `max_iters = 1` | Exactly one round of voting; verify intermediate state |
| All vertices same label | 0 changes detected; loop exits after first iteration |
| Tie-breaking | Graph where exactly two labels tie; result is one of the tied labels (not a third) |

### Overload 2 — with `empty_label` sentinel
| Test case | What to verify |
|-----------|----------------|
| All vertices unlabelled | All remain `empty_label`; no crash |
| One labelled vertex, rest `empty_label` | Label propagates outward from seed |
| Disconnected: labelled component + fully unlabelled component | Unlabelled component stays `empty_label` |
| Mixed: some pre-labelled, some `empty_label` | Unlabelled vertices acquire labels from neighbours |
| `empty_label` never appears in neighbours | Behaves identically to overload 1 |

### Parameterisation
Run the core correctness cases (single edge, path, cycle, disconnected) as
`TEMPLATE_TEST_CASE` over `vov_void` and `dov_void` from `algorithm_test_types.hpp`
to confirm the algorithm is not container-specific.

### Helper functions to define (model on test_mis.cpp)
```cpp
// Returns true if every vertex has one of the expected labels
bool all_labelled(const auto& label, auto expected_labels);

// Returns true if all vertices share the same label
bool fully_converged(const auto& label);
```
