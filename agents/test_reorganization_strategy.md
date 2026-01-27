# Test Reorganization Strategy

This document outlines a phased approach to reorganize the test suite for better maintainability,
reduced code duplication, and alignment with the new `adj_list/` directory structure.

## Current State Analysis

### Test File Statistics
- **Total test files**: 102 files
- **Dynamic graph tests**: 64 files (~63% of all tests)
- **CPO test files**: 51 files
- **Total lines in dynamic_graph_cpo_*.cpp**: ~57,000 lines

### Current Problems

1. **Massive Code Duplication**: 27 nearly-identical `test_dynamic_graph_cpo_*.cpp` files
   (one per trait combination), each ~2,000-3,500 lines with ~95% identical content.

2. **Flat Directory Structure**: All 102 test files in a single directory makes navigation difficult.

3. **Inconsistent Naming**: Mix of patterns:
   - `test_<cpo>_cpo.cpp` (e.g., `test_vertices_cpo.cpp`)
   - `test_<container>_cpo.cpp` (e.g., `test_dynamic_graph_cpo_vol.cpp`)
   - `test_<concept>.cpp` (e.g., `test_vertex_concepts.cpp`)

4. **Existing Good Pattern**: `test_dynamic_graph_common.cpp` demonstrates the correct approach
   using `TEMPLATE_TEST_CASE` to test across all trait combinations in a single file.

---

## Proposed Directory Structure

```
tests/
├── CMakeLists.txt
├── test_main.cpp
├── adj_list/                      # Tests for graph::adj_list abstractions
│   ├── concepts/
│   │   ├── test_edge_concepts.cpp
│   │   ├── test_vertex_concepts.cpp
│   │   ├── test_adjacency_list_edge_concepts.cpp
│   │   └── test_adjacency_list_vertex_concepts.cpp
│   ├── descriptors/
│   │   ├── test_vertex_descriptor.cpp
│   │   ├── test_edge_descriptor.cpp
│   │   └── test_descriptor_traits.cpp
│   ├── cpo/
│   │   ├── test_vertices_cpo.cpp
│   │   ├── test_edges_cpo.cpp
│   │   ├── test_vertex_id_cpo.cpp
│   │   ├── test_target_id_cpo.cpp
│   │   ├── test_source_id_cpo.cpp
│   │   ├── test_degree_cpo.cpp
│   │   ├── test_num_vertices_cpo.cpp
│   │   ├── test_num_edges_cpo.cpp
│   │   ├── test_find_vertex_cpo.cpp
│   │   ├── test_find_vertex_edge_cpo.cpp
│   │   ├── test_contains_edge_cpo.cpp
│   │   ├── test_has_edge_cpo.cpp
│   │   ├── test_vertex_value_cpo.cpp
│   │   ├── test_edge_value_cpo.cpp
│   │   ├── test_graph_value_cpo.cpp
│   │   ├── test_partition_id_cpo.cpp
│   │   ├── test_num_partitions_cpo.cpp
│   │   ├── test_target_cpo.cpp
│   │   └── test_source_cpo.cpp
│   └── traits/
│       ├── test_adjacency_list_traits.cpp
│       └── test_type_aliases.cpp
├── container/                      # Tests for graph::container implementations
│   ├── compressed_graph/
│   │   ├── test_compressed_graph.cpp
│   │   └── test_compressed_graph_cpo.cpp
│   ├── dynamic_graph/
│   │   ├── test_dynamic_graph_common.cpp      # Unified template tests
│   │   ├── test_dynamic_graph_cpo_common.cpp  # NEW: Unified CPO tests
│   │   ├── test_dynamic_graph_integration.cpp
│   │   ├── test_dynamic_graph_conversions.cpp
│   │   ├── test_dynamic_graph_validation.cpp
│   │   └── traits/                            # Trait-specific edge cases only
│   │       ├── test_vofl_specific.cpp
│   │       ├── test_vol_specific.cpp
│   │       └── ...
│   └── undirected_adjacency_list/
│       ├── test_undirected_adjacency_list.cpp
│       └── test_undirected_adjacency_list_cpo.cpp
└── edge_list/                      # Future: Tests for graph::edge_list
    ├── concepts/
    ├── cpo/
    └── container/
```

---

## Implementation Phases

### Phase 1: Create Directory Structure and Multiple Test Executables

**Goal**: Organize tests into logical subdirectories with separate test executables per major component.

#### Multiple Test Executables Architecture

**Recommendation**: Create three separate test executables aligned with the namespace structure:

1. **`graph3_adj_list_tests`** - Tests for `graph::adj_list` abstractions
   - CPOs, descriptors, concepts, traits
   - ~40 test files

2. **`graph3_container_tests`** - Tests for `graph::container` implementations
   - `dynamic_graph`, `compressed_graph`, `undirected_adjacency_list`
   - ~60 test files

3. **`graph3_edge_list_tests`** - Future tests for `graph::edge_list`
   - To be implemented in later phases

**Benefits:**
- **Faster incremental builds**: Only recompile/link affected test executable
- **Selective test execution**: Run only relevant tests during development
- **Parallel CI/CD**: Execute test suites concurrently
- **Smaller link times**: Each executable is smaller, faster to link
- **Clear boundaries**: Matches namespace and directory structure

**Trade-offs:**
- Need shared test utilities (recommend header-only helpers in `tests/common/`)
- Slightly more complex CMakeLists.txt setup

#### Tasks

1. **Create directory structure**
   ```bash
   mkdir -p tests/adj_list/{concepts,descriptors,cpo,traits}
   mkdir -p tests/container/{compressed_graph,dynamic_graph,undirected_adjacency_list}
   mkdir -p tests/edge_list/{concepts,cpo,container}
   mkdir -p tests/common  # Shared test utilities
   ```

2. **Move adj_list tests**
   - Move `test_*_concepts.cpp` → `tests/adj_list/concepts/`
   - Move `test_*_descriptor*.cpp` → `tests/adj_list/descriptors/`
   - Move `test_*_cpo.cpp` (pure CPO tests) → `tests/adj_list/cpo/`
   - Move `test_*_traits.cpp` → `tests/adj_list/traits/`

3. **Move container tests**
   - Move `test_compressed_graph*.cpp` → `tests/container/compressed_graph/`
   - Move `test_dynamic_graph*.cpp` → `tests/container/dynamic_graph/`
   - Move `test_undirected_adjacency_list*.cpp` → `tests/container/undirected_adjacency_list/`

4. **Restructure CMakeLists.txt**
   - Create `tests/adj_list/CMakeLists.txt` for `graph3_adj_list_tests`
   - Create `tests/container/CMakeLists.txt` for `graph3_container_tests`
   - Update root `tests/CMakeLists.txt` to add subdirectories
   - Extract common test utilities to `tests/common/` (header-only)

5. **Update include paths**
   - Fix any relative include paths broken by the move
   - Update paths to shared test utilities

---

### Phase 2: Consolidate Dynamic Graph CPO Tests (NEEDS EVALUATION)

**Goal**: Reduce 27 `test_dynamic_graph_cpo_*.cpp` files (~57,000 lines) into fewer files.

**⚠️ IMPORTANT: Consolidation Feasibility Analysis Required**

Analysis of actual file differences reveals significant behavioral differences between container types:

#### Container-Specific Behavioral Differences Found

| Difference Type | Containers Affected | Impact |
|-----------------|---------------------|--------|
| **Edge insertion order** | `forward_list` reverses order (push_front), others preserve order | ~20% of tests have different expected values |
| **num_edges(g, u) support** | Only `vector`/`deque` (random-access) support this CPO | Tests must be excluded for `list`/`forward_list` |
| **sized_range support** | `list`/`forward_list` are not sized_range | Affects CPO availability |
| **Iterator category** | forward vs bidirectional vs random-access | Affects which CPOs work |

#### Diff Analysis Results

- `vol` vs `vov` (list vs vector): **~1,159 diff lines** (~34% of file)
- `vol` vs `vofl` (list vs forward_list): **~1,091 diff lines** (~32% of file)

The differences are **not just naming** - they include:
1. Different expected edge orders in REQUIRE statements
2. Entire TEST_CASE blocks present in some files but not others
3. Different assertions for edge values due to insertion order

#### Revised Consolidation Strategy

Instead of full consolidation, consider a **hybrid approach**:

**Option A: Partial Consolidation (Recommended)**
- Consolidate tests that are truly identical across all traits (~60-70% of tests)
- Keep separate files for container-specific behavior tests
- Use compile-time traits to conditionally include tests

**Option B: Grouping by Container Category**
- Group 1: Random-access edge containers (`vov`, `vod`, `dov`, `dod`) - 4 files → 1 file
- Group 2: Bidirectional edge containers (`vol`, `dol`) - 2 files → 1 file  
- Group 3: Forward-only edge containers (`vofl`, `dofl`) - 2 files → 1 file
- Group 4: Map/unordered containers - keep separate

**Option C: Status Quo with Organization**
- Keep individual files but move to subdirectories
- Document why consolidation was not done
- Focus effort on other phases instead

#### Evaluation Tasks (Before Proceeding)

1. **Categorize all TEST_CASE blocks** by whether they are:
   - Container-agnostic (can be consolidated)
   - Order-dependent (need conditional expected values)
   - Container-specific (need separate tests)

2. **Prototype Option A** with a small subset of tests to validate the approach

3. **Measure compile time** impact of TEMPLATE_TEST_CASE with many trait types

#### Prototype Validation Results (Completed)

**Date:** Session continuation after Phase 1 completion

**Prototype Implemented:** `test_dynamic_graph_cpo_random_access.cpp`
- Consolidates random-access containers: vov, vod, dov, dod
- Uses `TEMPLATE_TEST_CASE` across all 4 trait types
- Covers 9 CPO test categories with 38 test cases and 166 assertions

**Key Findings:**

1. **Edge Creation Pattern for EV=void:**
   - `create_edge()` method doesn't exist for `EV=void` graphs
   - Must use `load_edges()` with `copyable_edge_t<uint64_t, void>` vector
   - Created helper function `add_edges(g, {{0,1}, {1,2}})` for convenience

2. **Vertex Access Pattern:**
   - `vertices(g).begin() + n` iterator arithmetic doesn't work
   - Must use `find_vertex(g, id)` CPO to access vertices by ID

3. **Build and Test Results:**
   - ✅ Builds successfully with gcc-15
   - ✅ All 166 assertions pass
   - ✅ No impact on existing tests (3,948 total tests pass)

4. **Code Reduction Potential:**
   - Each container-specific file is ~3,000 lines
   - 4 random-access files = ~12,000 lines → consolidated to ~400 lines
   - **~97% reduction** for the random-access category

**Recommendation:** Proceed with **Option B (Grouping by Container Category)**

| Category | Containers | Files | Estimated Lines |
|----------|------------|-------|-----------------|
| Random-access | vov, vod, dov, dod | 4 → 1 | ~12,000 → ~400 |
| Bidirectional | vol, dol | 2 → 1 | ~6,000 → ~400 |
| Forward-only | vofl, dofl | 2 → 1 | ~6,000 → ~400 |
| Map/unordered | mos, mous, mol, mofl, mod, moem | 6 | Keep separate or consolidate by behavior |

**Total estimated reduction:** ~40,000 lines → ~3,000 lines (~93% reduction)

#### Phase 2 Implementation Progress

**Completed Consolidated Files:**

1. **`test_dynamic_graph_cpo_random_access.cpp`** (vov, vod, dov, dod)
   - 38 test cases, 166 assertions
   - ~390 lines
   - Status: ✅ Complete and passing

2. **`test_dynamic_graph_cpo_bidirectional.cpp`** (vol, dol)
   - 22 test cases, 91 assertions
   - ~380 lines
   - Status: ✅ Complete and passing

3. **`test_dynamic_graph_cpo_forward.cpp`** (vofl, dofl)
   - 20 test cases, 74 assertions
   - ~380 lines
   - Status: ✅ Complete and passing

**Total Test Count:** 3,988 tests (up from 3,912 original)

**Remaining Work:**
- Map/unordered containers (mos, mous, mol, mofl, mod, moem) - to be evaluated
- Remove original per-container CPO files once fully migrated
- Currently the consolidated files add NEW tests; migration would remove ~24,000 lines

**Key Technical Patterns Established:**
```cpp
// Helper for EV=void graphs
template <typename Graph>
void add_edges(Graph& g, std::initializer_list<std::pair<uint64_t, uint64_t>> edges_list) {
    std::vector<copyable_edge_t<uint64_t, void>> edge_data;
    for (auto [src, tgt] : edges_list) {
        edge_data.push_back({src, tgt});
    }
    g.load_edges(edge_data);
}

// Use find_vertex instead of iterator arithmetic
auto v0 = *find_vertex(g, uint64_t{0});
```

---

### Phase 3: Consolidate Remaining Dynamic Graph Tests

**Goal**: Apply the same consolidation to `test_dynamic_graph_*.cpp` files (non-CPO).

#### Tasks

1. **Analyze remaining dynamic graph tests**
   - Identify which tests are already consolidated (`test_dynamic_graph_common.cpp`)
   - Identify which individual files can be merged

2. **Consolidate trait-specific tests**
   - Files like `test_dynamic_graph_vol.cpp`, `test_dynamic_graph_vov.cpp`, etc.
   - Merge into template test files where possible
   - Keep only truly trait-specific edge cases in separate files

3. **Update documentation**
   - Document which tests cover which functionality
   - Add test coverage comments to consolidated files

---

### Phase 4: Standardize Naming Conventions

**Goal**: Consistent naming across all test files.

#### Naming Rules

| Category | Pattern | Example |
|----------|---------|---------|
| Concept tests | `test_<concept>_concepts.cpp` | `test_vertex_range_concepts.cpp` |
| CPO tests | `test_<cpo_name>_cpo.cpp` | `test_vertices_cpo.cpp` |
| Descriptor tests | `test_<type>_descriptor.cpp` | `test_vertex_descriptor.cpp` |
| Container tests | `test_<container>.cpp` | `test_dynamic_graph.cpp` |
| Container CPO tests | `test_<container>_cpo.cpp` | `test_dynamic_graph_cpo.cpp` |
| Trait tests | `test_<trait>_traits.cpp` | `test_adjacency_list_traits.cpp` |
| Integration tests | `test_<container>_integration.cpp` | `test_dynamic_graph_integration.cpp` |

#### Tasks

1. **Audit current names**
   - List all files not matching the conventions
   - Plan renames

2. **Rename files**
   - Use `git mv` to preserve history
   - Update `CMakeLists.txt`

3. **Update test tags**
   - Standardize Catch2 tags: `[adj_list]`, `[container]`, `[cpo]`, `[concepts]`, etc.

---

### Phase 5: Add Missing Test Coverage

**Goal**: Identify and fill gaps in test coverage.

#### Tasks

1. **Audit CPO coverage**
   - Verify each CPO has tests for: member, ADL, default paths
   - Add missing path tests

2. **Add container-specific CPO tests**
   - Ensure each container type has CPO integration tests
   - Focus on edge cases specific to container implementations

3. **Add edge_list tests** (when implemented)
   - Follow the established patterns from adj_list tests

---

## Phase Execution Order

| Phase | Priority | Effort | Impact | Dependencies |
|-------|----------|--------|--------|--------------|
| Phase 1 | HIGH | Low | MEDIUM | None |
| Phase 2 | EVALUATE | High | UNCERTAIN | Phase 1 |
| Phase 3 | MEDIUM | Medium | MEDIUM | Phase 1, Phase 2 |
| Phase 4 | LOW | Low | LOW | Phase 1 |
| Phase 5 | LOW | High | MEDIUM | Phase 1-4 |

**Recommended Order**: Phase 1 → (Phase 2 evaluation) → Phase 3 → Phase 4 → Phase 5

**Rationale**: Phase 1 (directory structure) provides immediate value with low risk.
Phase 2 consolidation needs careful evaluation first due to container-specific behaviors.

---

## Agent Implementation Notes

### Phase 2 Detailed Steps for Agent

1. **Read reference files**
   - Read `test_dynamic_graph_common.cpp` to understand the template pattern
   - Read `test_dynamic_graph_cpo_vov.cpp` as the source for CPO tests

2. **Create the consolidated file**
   - Create `test_dynamic_graph_cpo_common.cpp`
   - Define all trait types to test against
   - Convert each `TEST_CASE` to `TEMPLATE_TEST_CASE`

3. **Handle edge container differences**
   - Tests involving `num_edges(g, u)` only work with random-access containers
   - Use `if constexpr` or separate test sections for these cases

4. **Verify correctness**
   - Build and run tests
   - Compare assertion counts before/after

5. **Clean up**
   - Delete old files
   - Update CMakeLists.txt

### Build Verification Command

```bash
cd /home/phil/dev_graph/graph-v3
cmake --build build/linux-gcc-release
./build/linux-gcc-release/tests/graph3_tests --reporter compact | tail -5
```

### Expected Results

Before consolidation:
- ~3,912 test cases, ~36,131 assertions

After consolidation:
- Same assertion count (tests are equivalent, just organized differently)
- Significantly faster compile times
- Easier maintenance

---

## Success Criteria

1. **Phase 1 Complete**: Single `test_dynamic_graph_cpo_common.cpp` replaces 27 files
2. **Phase 2 Complete**: Tests organized into subdirectories, all tests pass
3. **Phase 3 Complete**: Dynamic graph tests reduced to <10 files
4. **Phase 4 Complete**: All files follow naming conventions
5. **Phase 5 Complete**: 100% CPO path coverage documented

---

## Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-26 | 1.0 | Initial strategy document |
