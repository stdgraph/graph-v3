# Documentation Refactor — Execution Plan

> **Purpose:** Agent-executable task list with exact file paths, commands, and verification
> criteria. Every step is self-contained — an agent can pick up at any checkbox.
>
> **Companion:** `agents/doc_refactor_strategy.md` (rationale, audience model, target structure)
>
> **Branch:** `newdoc`
>
> **Granularity convention:**
> - **One action per step** for code edits, file deletes, and anything that could break a build.
> - **Grouped steps** for reads, directory creation, and other non-destructive operations.
>
> **Status key:** `[ ]` not started · `[~]` in progress · `[x]` done · `[!]` blocked

---

## Notation

| Symbol | Meaning |
|--------|---------|
| `EDIT` | Modify an existing file in place |
| `CREATE` | Create a new file |
| `MOVE` | Move/rename a file (preserve git history with `git mv`) |
| `DELETE` | Remove a file |
| `RUN` | Execute a shell command |
| `READ` | Read files for information (no output artifact) |
| `VERIFY` | Run a command and check the exit code or output |

Each step specifies:
- **Depends:** prerequisite step IDs (empty = none)
- **Risk:** `low` (read/create) · `med` (edit existing) · `high` (delete/move/build-breaking)
- **Verify:** how to confirm the step succeeded

---

## Phase 0 — Truth Sync (Blocking Gate)

> No doc moves or rewrites until every step in Phase 0 is complete.

### 0.1 Fix `algorithms.hpp` umbrella header

The current header includes 6 non-existent paths. The mapping from broken → real:

| Broken include | Real header | Action |
|----------------|-------------|--------|
| `algorithm/bellman_ford.hpp` | `algorithm/bellman_ford_shortest_paths.hpp` | Fix path |
| `algorithm/bfs.hpp` | `algorithm/breadth_first_search.hpp` | Fix path |
| `algorithm/dfs.hpp` | `algorithm/depth_first_search.hpp` | Fix path |
| `algorithm/prim.hpp` | `algorithm/mst.hpp` | Fix path |
| `algorithm/kruskal.hpp` | *(merged into `mst.hpp`)* | Remove line |
| `algorithm/scc.hpp` | *(no such header)* | Remove line |
| *(missing)* | `algorithm/mis.hpp` | Add include |
| *(missing)* | `algorithm/tc.hpp` | Add include |

- [x] **0.1.1** `EDIT` `include/graph/algorithms.hpp`
  - **Depends:** —
  - **Risk:** high
  - **Action:** Replace all broken `#include` directives with correct paths per the table above.
    Specifically:
    ```cpp
    // Shortest Path Algorithms
    #include "algorithm/dijkstra_shortest_paths.hpp"
    #include "algorithm/bellman_ford_shortest_paths.hpp"
    #include "algorithm/breadth_first_search.hpp"

    // Community Detection
    #include "algorithm/label_propagation.hpp"

    // Search Algorithms
    #include "algorithm/depth_first_search.hpp"

    // Minimum Spanning Tree
    #include "algorithm/mst.hpp"

    // Connectivity
    #include "algorithm/connected_components.hpp"
    #include "algorithm/articulation_points.hpp"
    #include "algorithm/biconnected_components.hpp"

    // Link Analysis
    #include "algorithm/jaccard.hpp"

    // Topological Sort & DAG
    #include "algorithm/topological_sort.hpp"

    // Subgraph / Matching
    #include "algorithm/mis.hpp"

    // Triangle Counting
    #include "algorithm/tc.hpp"
    ```
    Also remove the trailing comment `// Note: Uncomment includes above as algorithms are implemented`
    and update the `@defgroup` block's `@see` line to point to `docs/user-guide/algorithms.md`
    (will exist after Phase 3).
  - **Verify:** step 0.1.2

- [x] **0.1.2** `VERIFY` Compile smoke test for umbrella headers
  - **Depends:** 0.1.1
  - **Risk:** low
  - **Action:** Build the existing test suite with GCC debug preset. All three umbrella headers
    (`graph.hpp`, `views.hpp`, `algorithms.hpp`) are already exercised by test compilations.
    ```bash
    cmake --preset linux-gcc-debug
    cmake --build --preset linux-gcc-debug 2>&1 | head -80
    ```
  - **Pass condition:** Build succeeds (exit code 0) with no errors referencing
    `algorithms.hpp`, `views.hpp`, or `graph.hpp`.

### 0.2 Fix `graph.hpp` stale comments

- [x] **0.2.1** `EDIT` `include/graph/graph.hpp`
  - **Depends:** —
  - **Risk:** med
  - **Action:** Updated stale comment blocks:
    1. Doc comment: replaced "when implemented" with accurate descriptions;
       containers listed as available via separate includes (NOT auto-included
       in `graph.hpp` per design decision).
    2. Removed `// Future:` commented-out container and algorithm includes;
       replaced with a comment listing available container headers for discoverability.
  - **Verify:** step 0.1.2 (rebuild covers this)
  - **Design decision:** Containers must NOT be `#include`d in `graph.hpp` to avoid
    heavyweight dependencies. Users include container headers explicitly.

### 0.3 Verify consumer CMake instructions

- [x] **0.3.1** `READ` + `VERIFY` Confirm CMake target name
  - **Depends:** —
  - **Risk:** low
  - **Action:** The install exports `graph::graph3` (confirmed from `cmake/InstallConfig.cmake`
    line 23: `NAMESPACE graph::` on target `graph3`). Consumers use:
    ```cmake
    find_package(graph3 REQUIRED)
    target_link_libraries(myapp PRIVATE graph::graph3)
    ```
    Read `README.md` and search for `find_package` or `target_link_libraries` to check
    whether the README matches. Record any drift.
  - **Pass condition:** Documented finding noted; actual fix deferred to Phase 2 README rewrite.

### 0.4 Generate implementation matrix

- [x] **0.4.1** `CREATE` `docs/status/implementation_matrix.md`
  - **Depends:** —
  - **Risk:** low
  - **Action:** Create directory `docs/status/` then create the file with tables built from the
    actual source tree. Contents:

    **Algorithms** (from `include/graph/algorithm/*.hpp`, excluding `traversal_common.hpp`):

    | Algorithm | Header | Test File | Status |
    |-----------|--------|-----------|--------|
    | Dijkstra shortest paths | `dijkstra_shortest_paths.hpp` | `test_dijkstra_shortest_paths.cpp` | Implemented |
    | Bellman-Ford shortest paths | `bellman_ford_shortest_paths.hpp` | `test_bellman_ford_shortest_paths.cpp` | Implemented |
    | Breadth-first search | `breadth_first_search.hpp` | `test_breadth_first_search.cpp` | Implemented |
    | Depth-first search | `depth_first_search.hpp` | `test_depth_first_search.cpp` | Implemented |
    | Topological sort | `topological_sort.hpp` | `test_topological_sort.cpp` | Implemented |
    | Connected components | `connected_components.hpp` | `test_connected_components.cpp` | Implemented |
    | Articulation points | `articulation_points.hpp` | `test_articulation_points.cpp` | Implemented |
    | Biconnected components | `biconnected_components.hpp` | `test_biconnected_components.cpp` | Implemented |
    | MST (Prim/Kruskal) | `mst.hpp` | `test_mst.cpp` | Implemented |
    | Triangle counting | `tc.hpp` | `test_triangle_count.cpp` | Implemented |
    | Maximal independent set | `mis.hpp` | `test_mis.cpp` | Implemented |
    | Label propagation | `label_propagation.hpp` | `test_label_propagation.cpp` | Implemented |
    | Jaccard coefficient | `jaccard.hpp` | `test_jaccard.cpp` | Implemented |

    **Views** (from `include/graph/views/*.hpp`, excluding infrastructure):

    | View | Header | Category |
    |------|--------|----------|
    | vertexlist | `vertexlist.hpp` | Basic |
    | edgelist | `edgelist.hpp` | Basic |
    | incidence | `incidence.hpp` | Basic |
    | neighbors | `neighbors.hpp` | Basic |
    | BFS (vertices + edges) | `bfs.hpp` | Search |
    | DFS (vertices + edges) | `dfs.hpp` | Search |
    | topological sort (vertices + edges) | `topological_sort.hpp` | Search |

    **Containers:**

    | Container | Header | Mutability |
    |-----------|--------|------------|
    | `dynamic_graph` | `container/dynamic_graph.hpp` | Mutable |
    | `compressed_graph` | `container/compressed_graph.hpp` | Immutable after construction |
    | `undirected_adjacency_list` | `container/undirected_adjacency_list.hpp` | Mutable |

    **`dynamic_graph` trait combinations** (26 — list all trait file names from
    `include/graph/container/traits/`).

  - **Verify:** File exists and has 4 tables with correct counts.

### 0.5 Pin canonical metrics

- [x] **0.5.1** `RUN` Run full test suite and capture counts (result: 4261 tests, 100% pass)
  - **Depends:** 0.1.2 (build must succeed)
  - **Risk:** low
  - **Action:**
    ```bash
    cd build/linux-gcc-debug
    ctest --test-dir . 2>&1 | tail -5
    ```
    Record the "X tests passed" line. Also count:
    ```bash
    # Algorithm count (exclude traversal_common.hpp)
    ls include/graph/algorithm/*.hpp | grep -v traversal_common | wc -l
    # View count (exclude infrastructure: view_concepts, adaptors, basic_views, search_base)
    echo "7 views (vertexlist, edgelist, incidence, neighbors, bfs, dfs, topological_sort)"
    # Container count
    echo "3 containers + 26 trait combinations"
    ```
  - **Pass condition:** Numbers recorded; will be used in status artifact (0.6.1).

### 0.6 Freeze status artifact

- [x] **0.6.1** `CREATE` `docs/status/metrics.md`
  - **Depends:** 0.4.1, 0.5.1
  - **Risk:** low
  - **Action:** Create the canonical metrics file. All user-facing docs will reference this.
    ```markdown
    # Canonical Metrics (auto-generated — do not edit without running ctest)
    
    | Metric | Value | Source |
    |--------|-------|--------|
    | Algorithms | 13 | `include/graph/algorithm/` (excl. traversal_common.hpp) |
    | Views | 7 | `include/graph/views/` (vertexlist, edgelist, incidence, neighbors, bfs, dfs, topological_sort) |
    | Containers | 3 | dynamic_graph, compressed_graph, undirected_adjacency_list |
    | Trait combinations | 26 | `include/graph/container/traits/` |
    | Test count | {PIN FROM 0.5.1} | `ctest` output on linux-gcc-debug |
    | C++ standard | C++20 | CMakeLists.txt |
    | License | BSL-1.0 | LICENSE |
    ```
  - **Verify:** File exists; `{PIN FROM 0.5.1}` replaced with actual number.

### 0.7 Commit Phase 0

- [x] **0.7.1** `RUN` Commit all Phase 0 changes
  - **Depends:** 0.1.2, 0.2.1, 0.4.1, 0.5.1, 0.6.1
  - **Risk:** low
  - **Action:**
    ```bash
    git add -A
    git commit -m "Phase 0: truth-sync — fix umbrella headers, pin metrics, create implementation matrix"
    ```
  - **Verify:** `git log --oneline -1` shows the commit.

---

## Phase 1 — Foundation

### 1.1 Create directory structure

- [x] **1.1.1** `RUN` Create doc directories
  - **Depends:** Phase 0 complete
  - **Risk:** low
  - **Action:**
    ```bash
    mkdir -p docs/user-guide docs/reference docs/contributing docs/archive
    ```
  - **Verify:** All four directories exist.

### 1.2 Write docs/index.md

- [x] **1.2.1** `CREATE` `docs/index.md`
  - **Depends:** 1.1.1
  - **Risk:** low
  - **Action:** Create navigation hub. Skeleton:
    ```markdown
    # graph-v3 Documentation
    
    ## For Users
    - [Getting Started](getting-started.md)
    - [Adjacency Lists](user-guide/adjacency-lists.md)
    - [Edge Lists](user-guide/edge-lists.md)
    - [Containers](user-guide/containers.md)
    - [Views](user-guide/views.md)
    - [Algorithms](user-guide/algorithms.md)
    - [Examples](user-guide/examples.md)
    
    ## Reference
    - [Adjacency List Interface](reference/adjacency-list-interface.md)
    - [Edge List Interface](reference/edge-list-interface.md)
    - [Concepts](reference/concepts.md)
    - [CPO Reference](reference/cpo-reference.md)
    - [Algorithm Complexity](reference/algorithm-complexity.md)
    - [Edge Value Concepts](reference/edge-value-concepts.md)
    - [Vertex Patterns](reference/vertex-patterns.md)
    - [Type Aliases](reference/type-aliases.md)
    
    ## For Contributors
    - [Architecture](contributing/architecture.md)
    - [Coding Guidelines](contributing/coding-guidelines.md)
    - [CPO Implementation Guide](contributing/cpo-implementation.md)
    - [CPO Order](contributing/cpo-order.md)
    - [Algorithm Template](contributing/algorithm-template.md)
    - [View Template](contributing/view-template.md)
    - [View Chaining](contributing/view-chaining.md)
    
    ## Other
    - [FAQ](FAQ.md)
    - [Migration from v2](migration-from-v2.md)
    - [Status & Metrics](status/metrics.md)
    ```
    All links are forward references — target files will be created in later phases. This is
    intentional: the index is the skeleton that later phases fill in.
  - **Verify:** File exists; all link targets listed match the target structure in
    `doc_refactor_strategy.md`.

### 1.3 Write migration-from-v2.md

- [x] **1.3.1** `READ` Extract v2 content from README.md
  - **Depends:** —
  - **Risk:** low
  - **Action:** Read `README.md` and identify all v2 comparison content. Key sections:
    - "Enhancements and Differences from graph-v2" (starts around line 100+)
    - Subsections: descriptor changes, container changes, view changes, algorithm changes,
      namespace changes
    Record line ranges for extraction.

- [x] **1.3.2** `CREATE` `docs/migration-from-v2.md`
  - **Depends:** 1.3.1, 1.1.1
  - **Risk:** low
  - **Action:** Create migration guide with content extracted from README.md. Structure:
    ```markdown
    # Migrating from graph-v2 to graph-v3
    
    ## Overview
    [Brief rationale for the v3 rewrite]
    
    ## Key Changes
    ### Descriptors
    [Extracted from README]
    
    ### Containers
    [Extracted from README]
    
    ### Views
    [Extracted from README]
    
    ### Algorithms
    [Extracted from README]
    
    ### Namespaces
    [Extracted from README]
    
    ## Migration Checklist
    [Actionable list for users updating code]
    ```
  - **Verify:** File exists; content matches README extraction; no v2 content *created* — only
    moved and reorganized.

### 1.4 Archive stale design docs

- [x] **1.4.1** `MOVE` `docs/edge_map_analysis.md` → `docs/archive/edge_map_analysis.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv docs/edge_map_analysis.md docs/archive/edge_map_analysis.md`
  - **Verify:** File absent from `docs/`, present in `docs/archive/`.

- [x] **1.4.2** `MOVE` `descriptor.md` → `docs/archive/descriptor.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv descriptor.md docs/archive/descriptor.md`
  - **Verify:** File absent from root, present in `docs/archive/`.

- [x] **1.4.3** `MOVE` `include/graph/README.md` → `docs/archive/include_graph_README.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv include/graph/README.md docs/archive/include_graph_README.md`
  - **Verify:** File absent from `include/graph/`, present in `docs/archive/`.

- [x] **1.4.4** `MOVE` `include/graph/algorithm/README.md` → `docs/archive/include_graph_algorithm_README.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv include/graph/algorithm/README.md docs/archive/include_graph_algorithm_README.md`
  - **Verify:** File absent from `include/graph/algorithm/`, present in `docs/archive/`.

### 1.5 Write FAQ

- [x] **1.5.1** `CREATE` `docs/FAQ.md`
  - **Depends:** 1.1.1
  - **Risk:** low
  - **Action:** Create FAQ with answers sourced from existing docs and code. Questions:
    1. **Can I use my own graph type?** — Yes, via CPOs. Explain briefly, link to
       `reference/cpo-reference.md`.
    2. **How do I add edge weights?** — Via edge values, link to
       `reference/edge-value-concepts.md`.
    3. **What's the difference between views and algorithms?** — Views are lazy ranges,
       algorithms produce results. Link to `user-guide/views.md` and `user-guide/algorithms.md`.
    4. **Why descriptors instead of iterators?** — Stability across mutations, smaller size,
       composability. Link to `contributing/architecture.md`.
    5. **How does this compare to Boost.Graph?** — Brief table, link to Boost.Graph comparison
       in README.
    6. **Which container should I use?** — Decision flowchart pointing to
       `user-guide/containers.md`.
    7. **Does this work on Windows/macOS?** — Yes, supported compilers table from README.
  - **Verify:** File exists with 7 Q&A entries.

### 1.6 Commit Phase 1

- [x] **1.6.1** `RUN` Commit Phase 1
  - **Depends:** 1.2.1, 1.3.2, 1.4.1–1.4.4, 1.5.1
  - **Risk:** low
  - **Action:**
    ```bash
    git add -A
    git commit -m "Phase 1: foundation — docs index, migration guide, FAQ, archive stale docs"
    ```
  - **Verify:** `git log --oneline -1` shows the commit.

---

## Phase 2 — README Rewrite

### 2.1 Draft new README

- [x] **2.1.1** `READ` Inventory current README
  - **Depends:** Phase 1 complete
  - **Risk:** low
  - **Action:** Read `README.md` end to end. Note:
    - Content to **keep** (rewrite): overview, features, getting started
    - Content to **remove** (already moved): v2 comparison (→ migration-from-v2.md)
    - Content to **move**: design principles (→ contributing/architecture.md),
      test commands (→ CONTRIBUTING.md), directory tree (→ contributing/architecture.md)
    - **Numbers to pin**: test count, algorithm count, view count, container count
      (from `docs/status/metrics.md`)

- [x] **2.1.2** `EDIT` `README.md` — full rewrite
  - **Depends:** 2.1.1, Phase 0 (metrics pinned)
  - **Risk:** high
  - **Action:** Replace entire README with new structure per strategy §"New README.md Design".
    Sections in order:
    1. Title + tagline
    2. Shields.io badges (C++20, BSL-1.0, test count from metrics.md, build status placeholder)
    3. Highlights (7 bullets — header-only, works with your containers, 13 algorithms,
       7 lazy views, CPOs, 3 containers + 26 combinations, test count)
    4. Quick example (Dijkstra on vector-of-vectors — extract & modernize from current README
       or `examples/dijkstra_clrs_example.cpp`)
    5. Two ADTs: brief equal-weight description of adjacency lists and edge lists
    6. Feature overview with links to docs/user-guide/*
    7. Supported compilers table:

       | Compiler | Version | Platform | Status |
       |----------|---------|----------|--------|
       | GCC | 10+ | Linux | Tested |
       | Clang | 10+ | Linux, macOS | Tested |
       | MSVC | 2019+ | Windows | Tested |

    8. Installation (CMake `find_package(graph3)`, `target_link_libraries(... graph::graph3)`)
    9. Boost.Graph comparison table (4 rows: C++20 vs C++98, CPOs vs property maps,
       standard containers vs custom types, header-only vs compiled)
    10. Documentation links → `docs/index.md`
    11. Contributing → `CONTRIBUTING.md`
    12. License
    13. Status footer (pinned counts from metrics.md)

    **Source files to consult while writing:**
    - `docs/status/metrics.md` — canonical numbers
    - `examples/dijkstra_clrs_example.cpp` — working Dijkstra example
    - `include/graph/graph.hpp` — namespace doc comments
    - Current `README.md` — prose to salvage

  - **Verify:** New README has all 13 sections listed above; no v2 comparison inline;
    test count matches `docs/status/metrics.md`; `find_package(graph3)` and
    `target_link_libraries(... graph::graph3)` match `cmake/InstallConfig.cmake`.

### 2.2 Create CONTRIBUTING.md

- [x] **2.2.1** `CREATE` `CONTRIBUTING.md`
  - **Depends:** 2.1.1 (know what to extract from README)
  - **Risk:** low
  - **Action:** Extract from current README:
    - Build instructions (cmake presets, available presets table)
    - Test commands (`ctest --preset ...`)
    - Code style conventions
    - PR process (if any)
    Add new sections:
    - Link to `docs/contributing/` for architecture, CPO implementation
    - How to run sanitizers (`linux-gcc-asan` preset)
    - How to run coverage (`linux-gcc-coverage` preset)
  - **Verify:** File exists; contains build/test/PR sections.

### 2.3 Verify all README examples compile

- [x] **2.3.1** `VERIFY` Check README code blocks
  - **Depends:** 2.1.2
  - **Risk:** low
  - **Action:** Extract each C++ code block from the new README. For each:
    - If it's a full compilable example, verify it matches an existing example in `examples/`
      or create a minimal test.
    - If it's a snippet, mark with `// (simplified)` comment.
  - **Pass condition:** Every code block is either verified compilable or explicitly marked as
    a simplified snippet.

### 2.4 Commit Phase 2

- [x] **2.4.1** `RUN` Commit Phase 2
  - **Depends:** 2.1.2, 2.2.1, 2.3.1
  - **Risk:** low
  - **Action:**
    ```bash
    git add -A
    git commit -m "Phase 2: README rewrite, CONTRIBUTING.md"
    ```

---

## Phase 3 — User Guide

### 3.1 Getting started

- [x] **3.1.1** `CREATE` `docs/getting-started.md`
  - **Depends:** Phase 2 complete
  - **Risk:** low
  - **Action:** Write installation + first-graph + first-algorithm guide. Structure:
    1. **Requirements** (C++20 compiler, CMake 3.20+)
    2. **Installation** (CMake FetchContent, CPM, git submodule, system install)
    3. **Your first graph** — create a `dynamic_graph`, add vertices/edges, iterate
    4. **Your first algorithm** — run Dijkstra on the graph
    5. **Using edge lists** — show the same data as an edge list
    6. **Next steps** — links to user-guide pages

    **Source files to consult:**
    - `examples/basic_usage.cpp` — existing usage example
    - `examples/dijkstra_clrs_example.cpp` — Dijkstra example
    - `include/graph/container/dynamic_graph.hpp` — construction API
    - `tests/edge_list/test_edge_list_integration.cpp` — edge list examples
  - **Verify:** File exists; has all 6 sections; example code is consistent with actual API.

### 3.2 Adjacency lists user guide

- [x] **3.2.1** `READ` Gather adjacency list surface area
  - **Depends:** —
  - **Risk:** low
  - **Action:** Read these files and note concepts, CPOs, and types:
    - `include/graph/adj_list/adjacency_list_concepts.hpp` — concept definitions
    - `include/graph/adj_list/adjacency_list_traits.hpp` — trait definitions
    - `include/graph/adj_list/descriptor.hpp` — descriptor types
    - `include/graph/graph.hpp` lines 89-161 — re-exported symbols
    - `docs/container_interface.md` — existing adjacency list documentation
  - **Output:** Notes on: concepts (9), CPOs (19), descriptor types (4), type aliases (8).

- [x] **3.2.2** `CREATE` `docs/user-guide/adjacency-lists.md`
  - **Depends:** 3.2.1
  - **Risk:** low
  - **Action:** Write user-facing guide. Structure:
    1. **What is an adjacency list?** — range-of-ranges mental model
    2. **Core concepts** — `adjacency_list`, `index_adjacency_list`, `vertex`, `edge`, etc.
       with one-line descriptions
    3. **Accessing graph structure** — CPOs: `vertices`, `edges`, `target_id`, `vertex_id`,
       `source_id`, `num_vertices`, `num_edges`, `degree`, etc.
    4. **Vertex and edge values** — `vertex_value`, `edge_value`, `graph_value`
    5. **Descriptors** — what they are, why not iterators, descriptor types
    6. **Working with views** — link to views.md
    7. **See also** — link to `reference/adjacency-list-interface.md`
  - **Verify:** File exists; concept names match code; CPO names match code.

### 3.3 Edge lists user guide 

- [x] **3.3.1** `READ` Gather edge list surface area
  - **Depends:** —
  - **Risk:** low
  - **Action:** Read these files:
    - `include/graph/edge_list/edge_list.hpp` — concepts and CPOs
    - `include/graph/edge_list/edge_list_traits.hpp` — traits
    - `include/graph/edge_list/edge_list_descriptor.hpp` — descriptors
    - `tests/edge_list/test_edge_list_concepts.cpp` — usage patterns
    - `tests/edge_list/test_edge_list_integration.cpp` — integration examples
    - `docs/container_interface.md` — existing edge list section (note: terminology drift!)

- [x] **3.3.2** `CREATE` `docs/user-guide/edge-lists.md`
  - **Depends:** 3.3.1
  - **Risk:** low
  - **Action:** Write user-facing guide. Structure:
    1. **What is an edge list?** — flat range of sourced edges
    2. **Core concepts** — `basic_sourced_edgelist`, `basic_sourced_index_edgelist`,
       `has_edge_value` (use actual code names, NOT doc drift names)
    3. **Edge patterns** — pair, tuple, struct, edge_descriptor
    4. **Vertex ID types** — integral vs non-integral, index vs sourced
    5. **Working with algorithms that accept edge lists**
    6. **See also** — link to `reference/edge-list-interface.md`
  - **Verify:** File exists; concept names match `include/graph/edge_list/edge_list.hpp`;
    namespace is `graph::edge_list` (NOT `graph::container::edgelist`).

### 3.4 Containers user guide

- [x] **3.4.1** `READ` Gather container details
  - **Depends:** —
  - **Risk:** low
  - **Action:** Read header files:
    - `include/graph/container/dynamic_graph.hpp` — template params, construction
    - `include/graph/container/compressed_graph.hpp` — CSR layout, construction
    - `include/graph/container/undirected_adjacency_list.hpp` — dual-linked construction using intrusive lists
    - `include/graph/container/container_utility.hpp` — shared utilities
    - `/mnt/d/dev_graph/P1709/D3131_Containers/` - source and latex files in subdirectories for standard library proposal on containers; use container classification table (add more items if applicable)
    - List all 26 trait files in `include/graph/container/traits/`

- [x] **3.4.2** `CREATE` `docs/user-guide/containers.md`
  - **Depends:** 3.4.1
  - **Risk:** low
  - **Action:** Write container guide. Structure per strategy §"Container Documentation
    Requirements" — includes all 3 containers, selection guide, full 26-combination matrix,
    abbreviation conventions, and example usage of `dynamic_graph` with non-default traits.
  - **Verify:** File mentions all 3 containers with equal depth; 26 trait combinations listed;
    trait naming convention (`{vertex}o{edge}_graph_traits`) documented.

### 3.5 Move and update views

- [x] **3.5.1** `MOVE` `docs/views.md` → `docs/user-guide/views.md`
  - **Depends:** 1.1.1 (directory exists)
  - **Risk:** med
  - **Action:** `git mv docs/views.md docs/user-guide/views.md`
  - **Verify:** File moved.

- [x] **3.5.2** `EDIT` `docs/user-guide/views.md`
  - **Depends:** 3.5.1
  - **Risk:** med
  - **Action:** Add navigation header linking to `../index.md` and cross-links to
    `adjacency-lists.md`, `edge-lists.md`. Update any stale content (check against
    `include/graph/views/` headers). Ensure all 7 views listed.
  - **Verify:** All 7 views documented; links to index and adjacent pages present.

### 3.6 Algorithms user guide

- [x] **3.6.1** `CREATE` `docs/user-guide/algorithms.md`
  - **Depends:** 0.4.1 (implementation matrix)
  - **Risk:** low
  - **Action:** Write algorithms guide using implementation matrix as source of truth.
    Only list the 13 implemented algorithms. Structure:
    1. **Overview** — how algorithms work with CPOs and concepts
    2. **Algorithm catalog** — table with: name, header, brief description, required concepts
    3. **Per-algorithm sections** — for each of the 13 algorithms:
       - What it does (1-2 sentences)
       - Include header
       - Basic usage code snippet
       - Link to test file for more examples
    4. **Roadmap** — clearly labeled "planned" section for unimplemented algorithms

    **Source files to consult per algorithm:**
    - Header: `include/graph/algorithm/{name}.hpp`
    - Test: `tests/algorithms/test_{name}.cpp`
  - **Verify:** Exactly 13 algorithms in catalog; every listed header exists in file system;
    no header listed that doesn't exist.

### 3.7 Examples page

- [ ] **3.7.1** `CREATE` `docs/user-guide/examples.md`
  - **Depends:** Phase 2 complete
  - **Risk:** low
  - **Action:** Create curated examples page. Include annotated versions of:
    - `examples/basic_usage.cpp`
    - `examples/dijkstra_clrs_example.cpp`
    - `examples/mst_usage_example.cpp`
    Each with: goal, full code, line-by-line annotations, expected output.
  - **Verify:** All 3 examples from `examples/` directory represented; code matches source files.

### 3.8 Commit Phase 3

- [ ] **3.8.1** `RUN` Commit Phase 3
  - **Depends:** 3.1.1, 3.2.2, 3.3.2, 3.4.2, 3.5.2, 3.6.1, 3.7.1
  - **Risk:** low
  - **Action:**
    ```bash
    git add -A
    git commit -m "Phase 3: user guide — getting started, ADTs, containers, views, algorithms, examples"
    ```

---

## Phase 4 — Reference Documentation

### 4.1 Split container interface

- [ ] **4.1.1** `READ` Analyze `docs/container_interface.md` sections
  - **Depends:** —
  - **Risk:** low
  - **Action:** Read `docs/container_interface.md` (387 lines). Identify:
    - Line ranges for adjacency list section
    - Line ranges for edge list section
    - Any shared content (overview, design principles)

- [ ] **4.1.2** `CREATE` `docs/reference/adjacency-list-interface.md`
  - **Depends:** 4.1.1, 1.1.1
  - **Risk:** low
  - **Action:** Extract adjacency list section from container_interface.md. Add:
    - Navigation header
    - Full concept table (with concept names matching code)
    - CPO signatures
    - Type alias reference
  - **Verify:** Concept names match `include/graph/adj_list/adjacency_list_concepts.hpp`.

- [ ] **4.1.3** `CREATE` `docs/reference/edge-list-interface.md`
  - **Depends:** 4.1.1, 1.1.1
  - **Risk:** low
  - **Action:** Extract edge list section from container_interface.md. **Critical fix:**
    - Use `graph::edge_list` namespace (NOT `graph::container::edgelist`)
    - Use `basic_sourced_edgelist` (NOT `sourced_edgelist`)
    - Use `basic_sourced_index_edgelist` (NOT `index_sourced_edgelist`)
    - Use `has_edge_value` concept name
  - **Verify:** All concept/namespace names match `include/graph/edge_list/edge_list.hpp`.

- [ ] **4.1.4** `DELETE` `docs/container_interface.md` (after split verified)
  - **Depends:** 4.1.2, 4.1.3
  - **Risk:** high
  - **Action:** Remove the original file. Content has been split into two reference docs.
    `git rm docs/container_interface.md`
  - **Verify:** File gone; no remaining references in other docs (grep for
    `container_interface`).

### 4.2 Merge vertex concept docs

- [ ] **4.2.1** `CREATE` `docs/reference/vertex-patterns.md`
  - **Depends:** 1.1.1
  - **Risk:** low
  - **Action:** Merge content from:
    - `docs/vertex_inner_value_patterns.md` (~210 lines)
    - `docs/vertex_storage_concepts.md` (~260 lines)
    Structure as: inner value patterns section + storage concepts section + unified terminology.
  - **Verify:** File exists; covers both inner value patterns and storage concepts.

- [ ] **4.2.2** `DELETE` source files after merge
  - **Depends:** 4.2.1
  - **Risk:** high
  - **Action:**
    ```bash
    git rm docs/vertex_inner_value_patterns.md docs/vertex_storage_concepts.md
    ```
  - **Verify:** Both files removed.

### 4.3 Move edge value concepts

- [ ] **4.3.1** `MOVE` `docs/edge_value_concepts.md` → `docs/reference/edge-value-concepts.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv docs/edge_value_concepts.md docs/reference/edge-value-concepts.md`
  - **Verify:** File moved; update any internal links.

### 4.4 Write CPO reference

- [ ] **4.4.1** `READ` Extract CPO signatures
  - **Depends:** —
  - **Risk:** low
  - **Action:** Read these files to build a CPO reference table:
    - `include/graph/adj_list/detail/graph_cpo.hpp` — CPO implementations
    - `include/graph/graph.hpp` — re-exported CPO list (lines 121-160)
    - `docs/cpo.md` — existing (verbose) CPO documentation
    For each CPO, record: name, namespace, signature, return type, required concepts.

- [ ] **4.4.2** `CREATE` `docs/reference/cpo-reference.md`
  - **Depends:** 4.4.1, 1.1.1
  - **Risk:** low
  - **Action:** Write pure reference page. Structure:
    1. CPO overview (what they are, how they work — 1 paragraph)
    2. Table: all CPOs with signatures, parameter types, return types
    3. Per-CPO sections: name, synopsis, semantics, complexity, example
  - **Verify:** Every CPO re-exported in `graph.hpp` is documented.

### 4.5 Write concepts reference

- [ ] **4.5.1** `CREATE` `docs/reference/concepts.md`
  - **Depends:** 3.2.1, 3.3.1 (from READ steps)
  - **Risk:** low
  - **Action:** Consolidated concept reference. Two sections:
    1. **Adjacency list concepts** (from `adjacency_list_concepts.hpp`): ~9 concepts
    2. **Edge list concepts** (from `edge_list.hpp`): ~3 concepts
    For each: name, namespace, definition, semantic requirements, see-also.
  - **Verify:** All concepts from both headers listed; names match code exactly.

### 4.6 Algorithm complexity cheat sheet

- [ ] **4.6.1** `CREATE` `docs/reference/algorithm-complexity.md`
  - **Depends:** 0.4.1 (implementation matrix)
  - **Risk:** low
  - **Action:** Single-page cheat sheet. One table:

    | Algorithm | Time | Space | Required Concepts | Header |
    |-----------|------|-------|-------------------|--------|
    | Dijkstra | O((V+E) log V) | O(V) | `index_adjacency_list` | `dijkstra_shortest_paths.hpp` |
    | Bellman-Ford | O(V·E) | O(V) | `index_adjacency_list` | `bellman_ford_shortest_paths.hpp` |
    | ... | ... | ... | ... | ... |

    **Source:** Read each algorithm header's doc comment for complexity info. If missing,
    derive from standard algorithmic complexity.
  - **Verify:** All 13 algorithms listed; complexities present for all.

### 4.7 Write type aliases reference

- [ ] **4.7.1** `CREATE` `docs/reference/type-aliases.md`
  - **Depends:** —
  - **Risk:** low
  - **Action:** Extract all type aliases from `include/graph/graph.hpp` (lines 149-161) and
    document: alias name, underlying type, when to use.
  - **Verify:** All 8 type aliases documented.

### 4.8 Commit Phase 4

- [ ] **4.8.1** `RUN` Commit Phase 4
  - **Depends:** all 4.x steps
  - **Risk:** low
  - **Action:**
    ```bash
    git add -A
    git commit -m "Phase 4: reference docs — split interfaces, concepts, CPOs, complexity, type aliases"
    ```

---

## Phase 5 — Contributor Documentation

### 5.1 Write CONTRIBUTING.md (if not done in Phase 2)

This was created in step 2.2.1. Skip if already done.

### 5.2 Architecture guide

- [ ] **5.2.1** `CREATE` `docs/contributing/architecture.md`
  - **Depends:** 1.1.1
  - **Risk:** low
  - **Action:** Write architecture guide. Structure:
    1. **Directory structure** — accurate tree of `include/graph/`, `tests/`, `examples/`,
       `benchmark/`, `docs/`
    2. **Design principles** — extract from current README's "Design Principles" section
    3. **Range-of-ranges model** — how adjacency lists work under the hood
    4. **CPO-based interface** — why CPOs, priority dispatch, MSVC workaround
    5. **Namespace organization** — `graph::`, `graph::adj_list::`, etc.
    6. **Build system** — CMake presets, test organization

    **Source files:**
    - Current README.md Design Principles section
    - `include/graph/graph.hpp` namespace documentation
    - `CMakePresets.json` for preset inventory
  - **Verify:** Directory tree matches actual `ls` output.

### 5.3 Consolidate CPO implementation guides

- [ ] **5.3.1** `READ` Analyze duplication between `docs/cpo.md` and `docs/graph_cpo_implementation.md`
  - **Depends:** —
  - **Risk:** low
  - **Action:** Read both files. Identify:
    - Unique content in `cpo.md` (1706 lines)
    - Unique content in `graph_cpo_implementation.md` (1434 lines)
    - Overlapping sections (expected: the MSVC CPO pattern, priority dispatch, worked examples)
    Record a merge plan: which sections from each file, target size ~1500 lines.

- [ ] **5.3.2** `CREATE` `docs/contributing/cpo-implementation.md`
  - **Depends:** 5.3.1, 1.1.1
  - **Risk:** low
  - **Action:** Consolidated guide with two parts:
    - **Part 1:** How CPOs work (concepts, MSVC pattern, priority tiers) — from `cpo.md`
    - **Part 2:** How to implement a CPO (step-by-step, worked `vertex_id` example) —
      from `graph_cpo_implementation.md`
    Deduplicate: the CPO pattern shown 3× across the two files → show 1× with cross-references.
    Target: ~1500 lines total (down from ~3140 combined).
  - **Verify:** File exists; < 2000 lines; covers both "understanding" and "implementing" use cases.

- [ ] **5.3.3** `DELETE` source files after consolidation
  - **Depends:** 5.3.2
  - **Risk:** high
  - **Action:**
    ```bash
    git rm docs/cpo.md docs/graph_cpo_implementation.md
    ```
  - **Verify:** Both removed; no broken references (grep for `cpo.md` and
    `graph_cpo_implementation.md` in all `.md` files).

### 5.4 Move remaining contributor docs

- [ ] **5.4.1** `MOVE` `docs/algorithm_template.md` → `docs/contributing/algorithm-template.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv docs/algorithm_template.md docs/contributing/algorithm-template.md`

- [ ] **5.4.2** `MOVE` `docs/view_template.md` → `docs/contributing/view-template.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv docs/view_template.md docs/contributing/view-template.md`

- [ ] **5.4.3** `MOVE` `docs/view_chaining_limitations.md` → `docs/contributing/view-chaining.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv docs/view_chaining_limitations.md docs/contributing/view-chaining.md`
    Also update content: the file describes the *solution* to view chaining, not limitations.
    Rename the title inside the file accordingly.

- [ ] **5.4.4** `MOVE` `docs/common_graph_guidelines.md` → `docs/contributing/coding-guidelines.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv docs/common_graph_guidelines.md docs/contributing/coding-guidelines.md`
    Then update content: fix outdated directory tree, remove v2 references.

- [ ] **5.4.5** `MOVE` `docs/graph_cpo_order.md` → `docs/contributing/cpo-order.md`
  - **Depends:** 1.1.1
  - **Risk:** med
  - **Action:** `git mv docs/graph_cpo_order.md docs/contributing/cpo-order.md`
    Update: remove v2 path references.

### 5.5 Commit Phase 5

- [ ] **5.5.1** `RUN` Commit Phase 5
  - **Depends:** all 5.x steps
  - **Risk:** low
  - **Action:**
    ```bash
    git add -A
    git commit -m "Phase 5: contributor docs — architecture, consolidated CPO guide, moved templates/guidelines"
    ```

---

## Phase 6 — Cleanup & Final Validation

### 6.1 Archive remaining orphans

- [ ] **6.1.1** `VERIFY` Check for orphaned docs
  - **Depends:** Phase 5 complete
  - **Risk:** low
  - **Action:** List all `.md` files under `docs/` that are NOT in `user-guide/`, `reference/`,
    `contributing/`, `archive/`, or `status/`:
    ```bash
    find docs/ -name '*.md' -not -path 'docs/user-guide/*' \
      -not -path 'docs/reference/*' -not -path 'docs/contributing/*' \
      -not -path 'docs/archive/*' -not -path 'docs/status/*' \
      | sort
    ```
    Expected remaining: `docs/index.md`, `docs/getting-started.md`, `docs/FAQ.md`,
    `docs/migration-from-v2.md`. Anything else needs to be archived or investigated.
  - **Pass condition:** Only expected files remain outside subdirectories.

- [ ] **6.1.2** `MOVE` Any unexpected orphans → `docs/archive/`
  - **Depends:** 6.1.1
  - **Risk:** med
  - **Action:** For each unexpected file found in 6.1.1, move to archive.

### 6.2 Fix all cross-references

- [ ] **6.2.1** `VERIFY` Check for broken markdown links
  - **Depends:** Phase 5 complete
  - **Risk:** low
  - **Action:**
    ```bash
    # Find all markdown link targets
    grep -roh '\]([^)]*\.md[^)]*)' docs/ README.md CONTRIBUTING.md | \
      sed 's/.*](\(.*\))/\1/' | sed 's/#.*//' | sort -u | \
      while read f; do
        [ -f "$f" ] || echo "BROKEN: $f"
      done
    ```
    Note: relative links need to be resolved from the file's directory. A more sophisticated
    check may be needed if the simple grep misses context.
  - **Pass condition:** No "BROKEN:" lines in output.

- [ ] **6.2.2** `EDIT` Fix any broken links found
  - **Depends:** 6.2.1
  - **Risk:** med
  - **Action:** For each broken link, determine correct target and fix.

### 6.3 Verify ADT parity

- [ ] **6.3.1** `VERIFY` Check adjacency list / edge list balance
  - **Depends:** Phase 5 complete
  - **Risk:** low
  - **Action:** Verify both ADTs appear in:
    ```bash
    for f in docs/index.md docs/getting-started.md README.md; do
      echo "=== $f ==="
      grep -ci 'adjacency.list' "$f"
      grep -ci 'edge.list' "$f"
    done
    ```
    Also verify:
    - `docs/user-guide/adjacency-lists.md` exists
    - `docs/user-guide/edge-lists.md` exists
    - `docs/reference/adjacency-list-interface.md` exists
    - `docs/reference/edge-list-interface.md` exists
  - **Pass condition:** Both ADTs mentioned in all 3 checked files; all 4 dedicated
    docs exist.

### 6.4 Verify container coverage

- [ ] **6.4.1** `VERIFY` All 3 containers documented
  - **Depends:** Phase 3 complete
  - **Risk:** low
  - **Action:**
    ```bash
    grep -c 'dynamic_graph\|compressed_graph\|undirected_adjacency_list' \
      docs/user-guide/containers.md
    ```
  - **Pass condition:** All 3 container names appear in the file.

### 6.5 Verify `docs/algorithms/README.md` handled

- [ ] **6.5.1** `VERIFY` Old algorithm README archived or removed
  - **Depends:** 1.4.4 (archived)
  - **Risk:** low
  - **Action:** Confirm `include/graph/algorithm/README.md` no longer exists (was archived
    in step 1.4.4) and `docs/algorithms/README.md` is either archived, deleted, or replaced.
    ```bash
    ls -la include/graph/algorithm/README.md 2>&1
    ls -la docs/algorithms/README.md 2>&1
    ```
  - **Pass condition:** Neither file exists (both archived or replaced).

- [ ] **6.5.2** `MOVE` or `DELETE` `docs/algorithms/README.md`
  - **Depends:** 6.5.1
  - **Risk:** med
  - **Action:** If the file still exists, archive it:
    ```bash
    git mv docs/algorithms/README.md docs/archive/algorithms_README.md
    rmdir docs/algorithms 2>/dev/null || true
    ```

### 6.6 Write CHANGELOG.md

- [ ] **6.6.1** `CREATE` `CHANGELOG.md`
  - **Depends:** Phase 5 complete
  - **Risk:** low
  - **Action:** Create initial changelog noting the documentation reorganization:
    ```markdown
    # Changelog
    
    ## [Unreleased]
    
    ### Documentation
    - Complete documentation reorganization: user guide, reference, contributor docs
    - New README with badges, compiler table, feature highlights
    - Separated adjacency list and edge list documentation
    - Full container documentation including 26 dynamic_graph trait combinations
    - Consolidated CPO implementation guide (from 3140 lines → ~1500)
    - Created FAQ, migration guide, getting started, and examples pages
    - Fixed algorithms.hpp umbrella header (6 broken includes)
    - Archived stale design documents
    - Added canonical metrics tracking (docs/status/metrics.md)
    ```

### 6.7 Full build verification

- [ ] **6.7.1** `VERIFY` Full build + test pass
  - **Depends:** all Phase 6 edits complete
  - **Risk:** low
  - **Action:**
    ```bash
    cmake --preset linux-gcc-debug
    cmake --build --preset linux-gcc-debug
    ctest --preset linux-gcc-debug
    ```
  - **Pass condition:** Build succeeds; all tests pass; test count matches
    `docs/status/metrics.md`.

### 6.8 Final commit

- [ ] **6.8.1** `RUN` Commit Phase 6
  - **Depends:** 6.7.1
  - **Risk:** low
  - **Action:**
    ```bash
    git add -A
    git commit -m "Phase 6: cleanup — cross-references, CHANGELOG, final validation"
    ```

---

## Summary Statistics

| Phase | Steps | Done | High-risk | Status | Deliverables |
|-------|-------|------|-----------|--------|--------------|
| 0 | 8 | 8 | 1 | **Complete** | Fixed `algorithms.hpp`, fixed `graph.hpp`, implementation matrix, canonical metrics |
| 1 | 10 | 10 | 0 | **Complete** | `docs/index.md`, migration guide, FAQ, 4 archived files |
| 2 | 5 | 5 | 1 | Complete | New README, CONTRIBUTING.md |
| 3 | 11 | 0 | 0 | Not started | 6 user-guide pages + views move |
| 4 | 12 | 0 | 2 | Not started | 7 reference pages + 3 deleted sources |
| 5 | 10 | 0 | 1 | Not started | Architecture guide, consolidated CPO doc, 5 moved docs |
| 6 | 13 | 0 | 0 | Not started | CHANGELOG, cross-ref fixes, validation |
| **Total** | **69** | **23** | **6** | **33%** | |

> Last updated: 2026-02-19 · Branch: `newdoc`

### Dependency graph (phases)

```
Phase 0 (truth sync)
  ↓
Phase 1 (foundation) ─── directory structure needed by all later phases
  ↓
Phase 2 (README) ──────── needs metrics from Phase 0, migration doc from Phase 1
  ↓
Phase 3 (user guide) ──── needs directory from Phase 1, algorithms matrix from Phase 0
  ↓
Phase 4 (reference) ────── needs READ results from Phase 3 steps, directory from Phase 1
  ↓
Phase 5 (contributor) ──── needs directory from Phase 1
  ↓
Phase 6 (cleanup) ──────── needs all content from Phases 1-5
```

> Phases 3, 4, and 5 can run in parallel after Phase 2 if independent steps are tracked carefully.
> Within each phase, steps with no dependencies on each other can also run in parallel.
