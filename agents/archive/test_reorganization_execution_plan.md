# Test Reorganization Execution Plan

**Date**: January 26, 2026  
**Status**: Ready for execution  
**Safety Level**: HIGH (automated backups, rollback support)

## Overview

This document provides detailed, agent-executable instructions for reorganizing the test suite
with safety checkpoints and validation at each step.

---

## Pre-Execution Checklist

- [ ] All tests currently pass (run `ctest` in build directory)
- [ ] Working directory is clean (run `git status`)
- [ ] Create backup branch: `git checkout -b backup/test-reorg-$(date +%Y%m%d)`
- [ ] Return to main branch: `git checkout main` (or working branch)
- [ ] Create feature branch: `git checkout -b feature/test-reorganization`

---

## Phase 1: Create Directory Structure

**Goal**: Create subdirectories for organized tests.  
**Safety**: No file moves yet, only directory creation.  
**Rollback**: Simple `rm -rf` if needed.

### Steps

1. **Create base directories**
   ```bash
   mkdir -p tests/adj_list/concepts
   mkdir -p tests/adj_list/descriptors
   mkdir -p tests/adj_list/cpo
   mkdir -p tests/adj_list/traits
   mkdir -p tests/container/compressed_graph
   mkdir -p tests/container/dynamic_graph
   mkdir -p tests/container/undirected_adjacency_list
   mkdir -p tests/common
   ```

2. **Verify directory creation**
   ```bash
   ls -la tests/adj_list/
   ls -la tests/container/
   ```

3. **Checkpoint**: Directories created successfully ✓

---

## Phase 2: Move adj_list Tests

**Goal**: Move 23 adj_list-related test files to their subdirectories.  
**Safety**: Git tracks moves automatically, easy to revert.  
**Validation**: Compile and run tests after each group.

### Group 2a: Concept Tests (4 files)

```bash
git mv tests/test_edge_concepts.cpp tests/adj_list/concepts/
git mv tests/test_vertex_concepts.cpp tests/adj_list/concepts/
git mv tests/test_adjacency_list_edge_concepts.cpp tests/adj_list/concepts/
git mv tests/test_adjacency_list_vertex_concepts.cpp tests/adj_list/concepts/
```

**Validation**: Run `git status` to verify moves are tracked.

### Group 2b: Descriptor Tests (3 files)

```bash
git mv tests/test_vertex_descriptor.cpp tests/adj_list/descriptors/
git mv tests/test_edge_descriptor.cpp tests/adj_list/descriptors/
git mv tests/test_descriptor_traits.cpp tests/adj_list/descriptors/
```

### Group 2c: CPO Tests (13 files)

```bash
git mv tests/test_vertices_cpo.cpp tests/adj_list/cpo/
git mv tests/test_vertex_id_cpo.cpp tests/adj_list/cpo/
git mv tests/test_find_vertex_cpo.cpp tests/adj_list/cpo/
git mv tests/test_edges_cpo.cpp tests/adj_list/cpo/
git mv tests/test_edges_uid_cpo.cpp tests/adj_list/cpo/
git mv tests/test_target_id_cpo.cpp tests/adj_list/cpo/
git mv tests/test_target_cpo.cpp tests/adj_list/cpo/
git mv tests/test_source_id_cpo.cpp tests/adj_list/cpo/
git mv tests/test_source_cpo.cpp tests/adj_list/cpo/
git mv tests/test_num_vertices_cpo.cpp tests/adj_list/cpo/
git mv tests/test_num_edges_cpo.cpp tests/adj_list/cpo/
git mv tests/test_degree_cpo.cpp tests/adj_list/cpo/
git mv tests/test_find_vertex_edge_cpo.cpp tests/adj_list/cpo/
git mv tests/test_contains_edge_cpo.cpp tests/adj_list/cpo/
git mv tests/test_has_edge_cpo.cpp tests/adj_list/cpo/
git mv tests/test_vertex_value_cpo.cpp tests/adj_list/cpo/
git mv tests/test_edge_value_cpo.cpp tests/adj_list/cpo/
git mv tests/test_edge_value_cpo_value_method.cpp tests/adj_list/cpo/
git mv tests/test_graph_value_cpo.cpp tests/adj_list/cpo/
git mv tests/test_partition_id_cpo.cpp tests/adj_list/cpo/
git mv tests/test_num_partitions_cpo.cpp tests/adj_list/cpo/
git mv tests/test_vertices_pid_cpo.cpp tests/adj_list/cpo/
git mv tests/test_num_vertices_pid_cpo.cpp tests/adj_list/cpo/
```

### Group 2d: Trait Tests (2 files)

```bash
git mv tests/test_type_aliases.cpp tests/adj_list/traits/
git mv tests/test_adjacency_list_traits.cpp tests/adj_list/traits/
```

**Checkpoint**: 23 adj_list files moved ✓

---

## Phase 3: Move container Tests

**Goal**: Move 72 container test files to their subdirectories.  
**Safety**: Large move, but git tracks everything.  
**Validation**: Test compilation after completion.

### Group 3a: compressed_graph Tests (2 files)

```bash
git mv tests/test_compressed_graph.cpp tests/container/compressed_graph/
git mv tests/test_compressed_graph_cpo.cpp tests/container/compressed_graph/
```

### Group 3b: dynamic_graph Tests (68 files)

```bash
# Non-CPO dynamic_graph tests (32 files)
git mv tests/test_dynamic_graph_vofl.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_vol.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_vov.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_vod.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_dofl.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_dol.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_dov.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_dod.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_mofl.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_mol.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_mov.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_mod.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_uofl.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_uol.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_uov.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_uod.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_vos.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_vom.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_mom.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_dos.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_mos.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_uos.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_vous.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_dous.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_mous.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_uous.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_common.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_edge_comparison.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_nonintegral_ids.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_integration.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_stl_algorithms.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_generic_queries.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_traversal_helpers.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_transformations.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_validation.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_type_erasure.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_mixed_types.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_conversions.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_heterogeneous.cpp tests/container/dynamic_graph/

# CPO dynamic_graph tests (27 files)
git mv tests/test_dynamic_graph_cpo_vofl.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_vol.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_vov.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_vod.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_dofl.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_dol.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_dov.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_dod.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_mofl.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_mol.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_mov.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_mod.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_uofl.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_uol.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_uov.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_uod.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_vos.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_vom.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_mom.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_dos.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_mos.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_uos.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_vous.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_dous.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_mous.cpp tests/container/dynamic_graph/
git mv tests/test_dynamic_graph_cpo_uous.cpp tests/container/dynamic_graph/
```

### Group 3c: undirected_adjacency_list Tests (2 files)

```bash
git mv tests/test_undirected_adjacency_list.cpp tests/container/undirected_adjacency_list/
git mv tests/test_undirected_adjacency_list_cpo.cpp tests/container/undirected_adjacency_list/
```

**Checkpoint**: 72 container files moved ✓

---

## Phase 4: Create Subdirectory CMakeLists.txt Files

**Goal**: Create separate CMakeLists.txt for each subdirectory with its own test executable.  
**Safety**: New files only, no existing files modified yet.

### Step 4a: Create tests/adj_list/CMakeLists.txt

Create file with content:

```cmake
# adj_list tests executable
add_executable(graph3_adj_list_tests
    # Concepts
    concepts/test_edge_concepts.cpp
    concepts/test_vertex_concepts.cpp
    concepts/test_adjacency_list_edge_concepts.cpp
    concepts/test_adjacency_list_vertex_concepts.cpp
    
    # Descriptors
    descriptors/test_vertex_descriptor.cpp
    descriptors/test_edge_descriptor.cpp
    descriptors/test_descriptor_traits.cpp
    
    # CPOs
    cpo/test_vertices_cpo.cpp
    cpo/test_vertex_id_cpo.cpp
    cpo/test_find_vertex_cpo.cpp
    cpo/test_edges_cpo.cpp
    cpo/test_edges_uid_cpo.cpp
    cpo/test_target_id_cpo.cpp
    cpo/test_target_cpo.cpp
    cpo/test_source_id_cpo.cpp
    cpo/test_source_cpo.cpp
    cpo/test_num_vertices_cpo.cpp
    cpo/test_num_edges_cpo.cpp
    cpo/test_degree_cpo.cpp
    cpo/test_find_vertex_edge_cpo.cpp
    cpo/test_contains_edge_cpo.cpp
    cpo/test_has_edge_cpo.cpp
    cpo/test_vertex_value_cpo.cpp
    cpo/test_edge_value_cpo.cpp
    cpo/test_edge_value_cpo_value_method.cpp
    cpo/test_graph_value_cpo.cpp
    cpo/test_partition_id_cpo.cpp
    cpo/test_num_partitions_cpo.cpp
    cpo/test_vertices_pid_cpo.cpp
    cpo/test_num_vertices_pid_cpo.cpp
    
    # Traits
    traits/test_type_aliases.cpp
    traits/test_adjacency_list_traits.cpp
)

target_link_libraries(graph3_adj_list_tests
    PRIVATE
        graph3
        Catch2::Catch2WithMain
)

# MSVC requires /bigobj for files with many template instantiations
if(MSVC)
    target_compile_options(graph3_adj_list_tests PRIVATE /bigobj)
endif()

# Register tests with CTest
catch_discover_tests(graph3_adj_list_tests)
```

### Step 4b: Create tests/container/CMakeLists.txt

Create file with content:

```cmake
# container tests executable
add_executable(graph3_container_tests
    # compressed_graph
    compressed_graph/test_compressed_graph.cpp
    compressed_graph/test_compressed_graph_cpo.cpp
    
    # dynamic_graph - non-CPO tests
    dynamic_graph/test_dynamic_graph_vofl.cpp
    dynamic_graph/test_dynamic_graph_vol.cpp
    dynamic_graph/test_dynamic_graph_vov.cpp
    dynamic_graph/test_dynamic_graph_vod.cpp
    dynamic_graph/test_dynamic_graph_dofl.cpp
    dynamic_graph/test_dynamic_graph_dol.cpp
    dynamic_graph/test_dynamic_graph_dov.cpp
    dynamic_graph/test_dynamic_graph_dod.cpp
    dynamic_graph/test_dynamic_graph_mofl.cpp
    dynamic_graph/test_dynamic_graph_mol.cpp
    dynamic_graph/test_dynamic_graph_mov.cpp
    dynamic_graph/test_dynamic_graph_mod.cpp
    dynamic_graph/test_dynamic_graph_uofl.cpp
    dynamic_graph/test_dynamic_graph_uol.cpp
    dynamic_graph/test_dynamic_graph_uov.cpp
    dynamic_graph/test_dynamic_graph_uod.cpp
    dynamic_graph/test_dynamic_graph_vos.cpp
    dynamic_graph/test_dynamic_graph_vom.cpp
    dynamic_graph/test_dynamic_graph_mom.cpp
    dynamic_graph/test_dynamic_graph_dos.cpp
    dynamic_graph/test_dynamic_graph_mos.cpp
    dynamic_graph/test_dynamic_graph_uos.cpp
    dynamic_graph/test_dynamic_graph_vous.cpp
    dynamic_graph/test_dynamic_graph_dous.cpp
    dynamic_graph/test_dynamic_graph_mous.cpp
    dynamic_graph/test_dynamic_graph_uous.cpp
    dynamic_graph/test_dynamic_graph_common.cpp
    dynamic_graph/test_dynamic_edge_comparison.cpp
    dynamic_graph/test_dynamic_graph_nonintegral_ids.cpp
    dynamic_graph/test_dynamic_graph_integration.cpp
    dynamic_graph/test_dynamic_graph_stl_algorithms.cpp
    dynamic_graph/test_dynamic_graph_generic_queries.cpp
    dynamic_graph/test_dynamic_graph_traversal_helpers.cpp
    dynamic_graph/test_dynamic_graph_transformations.cpp
    dynamic_graph/test_dynamic_graph_validation.cpp
    dynamic_graph/test_dynamic_graph_type_erasure.cpp
    dynamic_graph/test_dynamic_graph_mixed_types.cpp
    dynamic_graph/test_dynamic_graph_conversions.cpp
    dynamic_graph/test_dynamic_graph_heterogeneous.cpp
    
    # dynamic_graph - CPO tests
    dynamic_graph/test_dynamic_graph_cpo_vofl.cpp
    dynamic_graph/test_dynamic_graph_cpo_vol.cpp
    dynamic_graph/test_dynamic_graph_cpo_vov.cpp
    dynamic_graph/test_dynamic_graph_cpo_vod.cpp
    dynamic_graph/test_dynamic_graph_cpo_dofl.cpp
    dynamic_graph/test_dynamic_graph_cpo_dol.cpp
    dynamic_graph/test_dynamic_graph_cpo_dov.cpp
    dynamic_graph/test_dynamic_graph_cpo_dod.cpp
    dynamic_graph/test_dynamic_graph_cpo_mofl.cpp
    dynamic_graph/test_dynamic_graph_cpo_mol.cpp
    dynamic_graph/test_dynamic_graph_cpo_mov.cpp
    dynamic_graph/test_dynamic_graph_cpo_mod.cpp
    dynamic_graph/test_dynamic_graph_cpo_uofl.cpp
    dynamic_graph/test_dynamic_graph_cpo_uol.cpp
    dynamic_graph/test_dynamic_graph_cpo_uov.cpp
    dynamic_graph/test_dynamic_graph_cpo_uod.cpp
    dynamic_graph/test_dynamic_graph_cpo_vos.cpp
    dynamic_graph/test_dynamic_graph_cpo_vom.cpp
    dynamic_graph/test_dynamic_graph_cpo_mom.cpp
    dynamic_graph/test_dynamic_graph_cpo_dos.cpp
    dynamic_graph/test_dynamic_graph_cpo_mos.cpp
    dynamic_graph/test_dynamic_graph_cpo_uos.cpp
    dynamic_graph/test_dynamic_graph_cpo_vous.cpp
    dynamic_graph/test_dynamic_graph_cpo_dous.cpp
    dynamic_graph/test_dynamic_graph_cpo_mous.cpp
    dynamic_graph/test_dynamic_graph_cpo_uous.cpp
    
    # undirected_adjacency_list
    undirected_adjacency_list/test_undirected_adjacency_list.cpp
    undirected_adjacency_list/test_undirected_adjacency_list_cpo.cpp
)

target_link_libraries(graph3_container_tests
    PRIVATE
        graph3
        Catch2::Catch2WithMain
)

# MSVC requires /bigobj for files with many template instantiations
if(MSVC)
    target_compile_options(graph3_container_tests PRIVATE /bigobj)
endif()

# Register tests with CTest
catch_discover_tests(graph3_container_tests)
```

**Checkpoint**: Subdirectory CMakeLists.txt files created ✓

---

## Phase 5: Update Root tests/CMakeLists.txt

**Goal**: Replace monolithic test executable with subdirectory includes.  
**Safety**: Keep original as `CMakeLists.txt.backup` before modifying.

### Steps

1. **Backup existing CMakeLists.txt**
   ```bash
   cp tests/CMakeLists.txt tests/CMakeLists.txt.backup
   ```

2. **Replace tests/CMakeLists.txt content**

Replace entire file with:

```cmake
cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# Fetch Catch2 v3
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.0
)
FetchContent_MakeAvailable(Catch2)

# Include CTest and Catch2 integration
include(CTest)
include(Catch)

# Add subdirectories with their own test executables
add_subdirectory(adj_list)
add_subdirectory(container)

# Keep test_main.cpp in root for now (can be moved to common/ later)
# No main executable needed - each subdirectory creates its own
```

**Checkpoint**: Root CMakeLists.txt updated ✓

---

## Phase 6: Build and Test Validation

**Goal**: Verify all tests compile and pass.  
**Safety**: This is the critical validation step.

### Steps

1. **Reconfigure CMake**
   ```bash
   cd build/linux-gcc-debug
   cmake ../..
   ```

2. **Build all test executables**
   ```bash
   cmake --build . --target graph3_adj_list_tests
   cmake --build . --target graph3_container_tests
   ```
   
   Expected output:
   - Both executables build successfully
   - No compilation errors
   - Linking completes

3. **Run all tests**
   ```bash
   ctest --output-on-failure
   ```
   
   Expected output:
   - All tests pass (36,131+ assertions)
   - Both `graph3_adj_list_tests` and `graph3_container_tests` execute

4. **Run individual test executables**
   ```bash
   ./tests/adj_list/graph3_adj_list_tests
   ./tests/container/graph3_container_tests
   ```

**Checkpoint**: All tests build and pass ✓

---

## Phase 7: Verify Git Status and Commit

**Goal**: Review changes and commit the reorganization.  
**Safety**: Git allows easy revert if needed.

### Steps

1. **Check git status**
   ```bash
   git status
   ```
   
   Expected:
   - 97 renamed files (git tracked moves)
   - 3 new files (subdirectory CMakeLists.txt)
   - 1 modified file (root CMakeLists.txt)
   - 1 backup file (CMakeLists.txt.backup - can ignore or commit)

2. **Review specific changes**
   ```bash
   git diff --staged tests/CMakeLists.txt
   ```

3. **Add all changes**
   ```bash
   git add -A
   ```

4. **Commit with descriptive message**
   ```bash
   git commit -m "Reorganize test suite into subdirectories with multiple executables

   - Created tests/adj_list/ for adjacency list abstraction tests
   - Created tests/container/ for container implementation tests
   - Split monolithic graph3_tests into:
     * graph3_adj_list_tests (23 test files)
     * graph3_container_tests (72 test files)
   - Benefits: faster incremental builds, selective test execution
   - All 36,131+ test assertions pass"
   ```

**Checkpoint**: Changes committed ✓

---

## Phase 8: Optional Cleanup

**Goal**: Remove backup file and verify clean state.

### Steps

1. **Remove backup (if committed)**
   ```bash
   rm tests/CMakeLists.txt.backup
   git add tests/CMakeLists.txt.backup
   git commit -m "Remove CMakeLists.txt backup file"
   ```

2. **Verify clean working directory**
   ```bash
   git status
   ```
   
   Expected: "nothing to commit, working tree clean"

**Checkpoint**: Cleanup complete ✓

---

## Rollback Procedures

If anything goes wrong during execution:

### Rollback During File Moves (Phase 2-3)

```bash
git reset --hard HEAD
```

This reverts all uncommitted changes.

### Rollback After Commit

```bash
git reset --hard HEAD~1
```

This removes the last commit and reverts files.

### Complete Rollback to Backup Branch

```bash
git checkout backup/test-reorg-$(date +%Y%m%d)
```

---

## Success Criteria

- ✅ All 97 test files moved to appropriate subdirectories
- ✅ Two separate test executables created
- ✅ All tests compile without errors
- ✅ All test assertions pass (36,131+)
- ✅ CMake configuration succeeds
- ✅ CTest discovers and runs all tests
- ✅ Changes committed to git with clear history

---

## Agent Execution Notes

**For automated execution:**

1. Execute phases sequentially - do not skip validation steps
2. After each checkpoint, verify success before proceeding
3. If any `git mv` fails, stop and report the error
4. If compilation fails in Phase 6, report errors and stop
5. If tests fail in Phase 6, report failures and stop
6. Keep user informed at each checkpoint

**Parallel execution is safe for:**
- Phase 1 (directory creation)
- Phase 2 groups (2a, 2b, 2c, 2d can be parallelized)
- Phase 3 groups (3a, 3b, 3c can be parallelized)

**Must be sequential:**
- Phases must execute in order (1 → 2 → 3 → 4 → 5 → 6 → 7 → 8)
- Phase 6 validation must complete before Phase 7 commit
