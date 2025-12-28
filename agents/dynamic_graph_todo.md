# Dynamic Graph Testing and Enhancement Plan

## Current Status (Updated: December 28, 2025)

**Phases 1-3: COMPLETE ✅**
- 16 traits fully tested (basic + CPO): vofl, vol, vov, vod, dofl, dol, dov, dod, mofl, mol, mov, mod, uofl, uol, uov, uod
- ~78,000 lines of test code
- All sequential and associative vertex containers tested with list/forward_list/vector/deque edges

**Phase 4.1: Set Edge Containers - IN PROGRESS ⏳ (75% complete)**
- ✅ vos (vector + set): Basic + CPO tests COMPLETE
- ✅ dos (deque + set): Basic + CPO tests COMPLETE  
- ✅ mos (map + set): Basic + CPO tests COMPLETE (28 test cases, 377 assertions)
- ⏳ uos (unordered_map + set): PENDING (trait + tests needed)

**Phase 4.2: Unordered Set Edge Containers - COMPLETE ✅**
- ✅ vous (vector + unordered_set): Basic + CPO tests COMPLETE (37 test cases, 238 assertions)
- ✅ dous (deque + unordered_set): Basic + CPO tests COMPLETE (37 test cases, 238 assertions)
- ✅ mous (map + unordered_set): Basic + CPO tests COMPLETE (53 test cases, 578 assertions)
- ✅ uous (unordered_map + unordered_set): Basic + CPO tests COMPLETE (53 test cases, 563 assertions)

**Phase 4.3: Map-Based Edge Containers - COMPLETE ✅**
- ✅ voem (vector + map edges): Basic + CPO tests COMPLETE (46 test cases, 292 assertions)
- ✅ moem (map + map edges): Basic + CPO tests COMPLETE (53 test cases, 578 assertions)

**Phase 4 Overall: 90% COMPLETE (9/10 traits implemented)**

**Phase 5: Non-Integral Vertex IDs - COMPLETE ✅**
- ✅ Phase 5.1: Analysis and Preparation COMPLETE
- ✅ Phase 5.2-5.4: Combined test file with string edge cases, double IDs, custom types
- **Test file:** test_dynamic_graph_nonintegral_ids.cpp (19 test cases, 133 assertions)

**Remaining Work:**
- ⏳ Phase 4.1.5: uos_graph_traits (unordered_map + set) - trait file + 2 test files
- Phase 6: Integration tests (optional)
- Phase 7: Mutation and stress tests (optional)

**Test Files Created (51 files total, 49 complete):**

*Sequential Containers (16 files):*
- test_dynamic_graph_vofl.cpp + test_dynamic_graph_cpo_vofl.cpp ✅
- test_dynamic_graph_vol.cpp + test_dynamic_graph_cpo_vol.cpp ✅
- test_dynamic_graph_vov.cpp + test_dynamic_graph_cpo_vov.cpp ✅
- test_dynamic_graph_vod.cpp + test_dynamic_graph_cpo_vod.cpp ✅
- test_dynamic_graph_dofl.cpp + test_dynamic_graph_cpo_dofl.cpp ✅
- test_dynamic_graph_dol.cpp + test_dynamic_graph_cpo_dol.cpp ✅
- test_dynamic_graph_dov.cpp + test_dynamic_graph_cpo_dov.cpp ✅
- test_dynamic_graph_dod.cpp + test_dynamic_graph_cpo_dod.cpp ✅

*Associative Containers (16 files):*
- test_dynamic_graph_mofl.cpp + test_dynamic_graph_cpo_mofl.cpp ✅
- test_dynamic_graph_mol.cpp + test_dynamic_graph_cpo_mol.cpp ✅
- test_dynamic_graph_mov.cpp + test_dynamic_graph_cpo_mov.cpp ✅
- test_dynamic_graph_mod.cpp + test_dynamic_graph_cpo_mod.cpp ✅
- test_dynamic_graph_uofl.cpp + test_dynamic_graph_cpo_uofl.cpp ✅
- test_dynamic_graph_uol.cpp + test_dynamic_graph_cpo_uol.cpp ✅
- test_dynamic_graph_uov.cpp + test_dynamic_graph_cpo_uov.cpp ✅
- test_dynamic_graph_uod.cpp + test_dynamic_graph_cpo_uod.cpp ✅

*Set Edge Containers (6 files + 2 pending):*
- test_dynamic_graph_vos.cpp + test_dynamic_graph_cpo_vos.cpp ✅
- test_dynamic_graph_dos.cpp + test_dynamic_graph_cpo_dos.cpp ✅
- test_dynamic_graph_mos.cpp + test_dynamic_graph_cpo_mos.cpp ✅
- test_dynamic_graph_uos.cpp + test_dynamic_graph_cpo_uos.cpp ⏳ PENDING

*Unordered Set Edge Containers (8 files):*
- test_dynamic_graph_vous.cpp + test_dynamic_graph_cpo_vous.cpp ✅
- test_dynamic_graph_dous.cpp + test_dynamic_graph_cpo_dous.cpp ✅
- test_dynamic_graph_mous.cpp + test_dynamic_graph_cpo_mous.cpp ✅
- test_dynamic_graph_uous.cpp + test_dynamic_graph_cpo_uous.cpp ✅

*Map Edge Containers (4 files):*
- test_dynamic_graph_voem.cpp + test_dynamic_graph_cpo_voem.cpp ✅
- test_dynamic_graph_moem.cpp + test_dynamic_graph_cpo_moem.cpp ✅

*Additional Test Files:*
- test_dynamic_graph_common.cpp ✅
- test_dynamic_edge_comparison.cpp ✅

---

## Overview
This plan outlines a phased approach to comprehensively test and enhance the `dynamic_graph` container, starting with existing functionality and progressively adding new container types and features. The goal is to achieve 95% test coverage while maintaining code stability.

## Conventions
Variable names must follow the following rules
- Variable names must follow the naming conventions in `common_graph_guidelines.md`
- Variable names must never be the same as a CPO function.
- The default variable name for a range/container of edges must be `edge_rng`
- The default variable name for a range/container of vertices must be `vertex_rng`


## Phase 1: Test Existing Functionality (Vector Vertices + List-based Edges)

### Phase 1.1: Core Container Combinations
Test all combinations of existing container types with various value types.

**Vertex Containers:** `vector`, `deque`  
**Edge Containers:** `vector`, `deque`, `list`, `forward_list`  
**Vertex ID Types:** `uint64_t`, `int`, `int8_t`  
**Edge Values:** `void`, `int`, `string`  
**Vertex Values:** `void`, `int`, `string`  
**Graph Values:** `void`, `int`, `string`  
**Sourced:** `true`, `false`

**Test Strategy:**
- Select representative combinations to achieve 95% coverage without exhaustive testing
- Priority combinations:
  1. `vector` vertices + `forward_list` edges (vofl_graph_traits - most lightweight)
  2. `vector` vertices + `list` edges (vol_graph_traits - bidirectional edges)
  3. `vector` vertices + `vector` edges (vov_graph_traits - best cache locality)
  4. `vector` vertices + `deque` edges (vod_graph_traits - stable edge iterators)
  5. `deque` vertices + `forward_list` edges (dofl_graph_traits - stable vertex iterators)
  6. `deque` vertices + `list` edges (dol_graph_traits - bidirectional with stability)
  7. `deque` vertices + `vector` edges (dov_graph_traits - random access edges)
  8. `deque` vertices + `deque` edges (dod_graph_traits - all stable iterators)

**Test Files Organization (Optimized with TEMPLATE_TEST_CASE):**
```
tests/
  # Phase 1: Sequential container unified tests (vector/deque vertices)
  test_dynamic_graph_common.cpp         # ~2000 lines, runs 8x via TEMPLATE_TEST_CASE
                                        # Uses uint64_t vertex IDs, tests auto-extension
                                        # Covers: vofl, vol, vov, vod, dofl, dol, dov, dod
  
  # Phase 1: Container-specific behavior tests
  test_dynamic_graph_vector_traits.cpp  # ~800 lines (vofl, vol, vov, vod specifics)
  test_dynamic_graph_deque_traits.cpp   # ~1000 lines (dofl, dol, dov, dod specifics + CPO)
  test_dynamic_graph_edge_containers.cpp # ~600 lines (edge container variations)
                                        # Tests forward_list, list, vector, deque edge behaviors
  
  # Phase 3: Associative container unified tests (map/unordered_map vertices)
  test_dynamic_graph_associative_common.cpp  # ~1600 lines, runs 3-6x
                                             # Uses std::string vertex IDs, key-based lookup
  
  # Phase 3: Map-specific behavior tests
  test_dynamic_graph_map_traits.cpp          # ~700 lines (ordered, comparators)
  test_dynamic_graph_unordered_map_traits.cpp # ~700 lines (hash, buckets)
  
  # Legacy complete test files (Phase 1.1-1.3 already completed)
  test_dynamic_graph_vofl.cpp           # ~2673 lines ✅ COMPLETE
  test_dynamic_graph_vol.cpp            # ~2677 lines ✅ COMPLETE
  test_dynamic_graph_vov.cpp            # ~2677 lines ✅ COMPLETE
```

**Deque Edge Container Coverage:**
Deque edges provide stable iterators and random access, combining benefits of vector and list:
- `vod_graph_traits` (vector + deque): Contiguous vertices, stable edge iterators
- `dod_graph_traits` (deque + deque): All stable iterators, good for dynamic graphs
- Tests will verify: stable iterators during modifications, random access, bidirectional iteration
- CPO tests will validate deque-specific optimizations (e.g., efficient insert/erase)

**Note:** Phase 1.4+ uses optimized TEMPLATE_TEST_CASE for sequential containers. Phase 3 will need a separate associative common file due to fundamental differences in vertex ID types and construction semantics.

**Test Coverage per File (~200-300 tests each):**

1. **Construction Tests (40 tests)** ✅ COMPLETE (17 tests in Phase 1.1)
   - [x] Default constructor
   - [x] Constructor with allocator
   - [x] Constructor with edge range only (with/without vertex count)
   - [x] Constructor with edge + vertex ranges
   - [x] Constructor with initializer_list (16 new tests added)
   - [x] Constructor with graph value (copy/move)
   - [ ] Constructor with partitions (not implemented yet)
   - [x] Test all combinations of value types (void/int/string)
   - [x] Test sourced vs unsourced edges

2. **Load Operations Tests (30 tests)** ✅ COMPLETE (8 tests in Phase 1.1)
   - [x] `load_vertices()` with various projections
   - [x] `load_vertices()` with move semantics
   - [x] `load_edges()` with various projections
   - [x] `load_edges()` with move semantics
   - [x] `load_edges()` with vertex inference
   - [x] `load_edges()` with explicit vertex count
   - [x] Edge count hints
   - [x] Out-of-range vertex ID behavior (auto-extends)

3. **Vertex Access Tests (25 tests)** ✅ COMPLETE (10 tests in Phase 1.1)
   - [x] `operator[]` access
   - [x] `begin()/end()` iteration
   - [x] `size()` queries
   - [x] Const correctness
   - [x] Empty graph behavior
   - [x] Single vertex graphs
   - [x] Large graphs (1000+ vertices, 10k vertices)

4. **Edge Access Tests (30 tests)** ✅ COMPLETE (10 tests in Phase 1.1)
   - [x] Access via vertex iterator
   - [x] Access via vertex ID
   - [x] Edge iteration for each vertex
   - [x] Empty edge lists
   - [x] Self-loops (single and multiple)
   - [x] Multiple edges to same target (parallel edges)
   - [x] Const correctness

5. **Value Access Tests (25 tests)** ✅ COMPLETE (10 tests in Phase 1.1)
   - [x] `vertex.value()` for non-void VV
   - [x] `edge.value()` for non-void EV
   - [x] Graph value access for non-void GV
   - [x] Compile-time errors for void types (static_assert checks)
   - [x] Value modification
   - [x] Move semantics

6. **Partition Tests (20 tests)** ⏸️ DEFERRED
   - [ ] Single partition (default)
   - [ ] Multiple partitions
   - [ ] Partition ID lookup
   - [ ] Partition vertex ranges
   - [ ] Invalid partition access
   - [ ] Empty partitions
   - [ ] Partition boundary vertices
   - Note: Partition API not yet implemented in dynamic_graph

7. **Sourced Edge Tests (15 tests)** ✅ COMPLETE (6 tests in Phase 1.1)
   - [x] Source ID access when Sourced=true
   - [x] Compile-time error when Sourced=false
   - [x] Sourced edge construction
   - [x] Copy/move with sourced edges
   - Note: Source vertex lookup requires CPO implementation

8. **Property Tests (20 tests)** ✅ COMPLETE (7 tests in Phase 1.1)
   - [x] Vertex count queries
   - [x] Edge count queries (manual iteration)
   - [x] Degree queries (per vertex)
   - [x] Empty graph properties
   - [x] Graph with only vertices (no edges)
   - [x] Sink detection
   - [x] Max degree computation

9. **Memory & Performance Tests (15 tests)** ✅ COMPLETE (8 tests in Phase 1.1)
   - [x] Memory layout for void types (verify no storage)
   - [x] Large graph construction (10k vertices)
   - [x] Iterator stability after modifications
   - [x] Dense graphs (many edges per vertex)
   - [x] Sparse graphs (few edges)
   - [x] Performance characteristics
   - Note: Reserve/resize not applicable to forward_list

10. **Edge Cases & Error Handling (25 tests)** ✅ COMPLETE (11 tests in Phase 1.1)
    - [x] Null/empty ranges
    - [x] Invalid vertex IDs (auto-extends)
    - [x] Boundary values (zero, negative, large)
    - [x] Exception safety
    - [x] Duplicate edges behavior
    - [x] Incremental building
    - [x] Bidirectional edges

11. **Iterator & Ranges Tests (15 tests)** ✅ COMPLETE (3 tests in Phase 1.1)
    - [x] Iterator stability
    - [x] Nested iteration
    - [x] std::ranges integration (count_if, find_if, transform, filter)
    - [x] Algorithm compatibility (accumulate, all_of, any_of, none_of)

12. **Workflow & Integration Tests (10 tests)** ✅ COMPLETE (3 tests in Phase 1.1)
    - [x] Build/query/modify workflows
    - [x] Social network simulation
    - [x] Dependency graph
    - [x] Complex graph structures (triangle, star, complete, cycle, tree, bipartite)

**Phase 1.1 Status:** ✅ **COMPLETE**
- File: `test_dynamic_graph_vofl.cpp` 
- Tests Added: 123 new tests (10 construction + 14 load operations + 24 vertex/edge access + 29 error handling + 17 comprehensive + 10 initializer_list + 19 test sections)
- Total Tests: 968 (845 existing + 123 new)
- Coverage: ~98% of vofl_graph_traits functionality
- All tests passing ✓
- Member functions uncommented: `target_id()`, `source_id()` (prerequisite for Phase 2 CPO)

**Phase 1.2 Status:** ✅ **COMPLETE**
- File: `test_dynamic_graph_vol.cpp`
- Tests Added: 115 tests (113 CTests, 628 assertions)
- Total Tests: 1075 (960 existing + 115 new)
- Coverage: ~98% of vol_graph_traits functionality
- All tests passing ✓
- Key features tested: Bidirectional edge iteration, std::list behavior
- Validated: Forward/backward iteration, reverse value verification
- File size: 2677 lines (matching vofl comprehensiveness)

**Phase 1.3 Status:** ✅ **COMPLETE**
- File: `test_dynamic_graph_vov.cpp`
- Tests Added: 115 tests (113 CTests, 628 assertions)
- Total Tests: 1190 (1075 existing + 115 new)
- Coverage: ~98% of vov_graph_traits functionality
- All tests passing ✓
- Key features tested: Random access, cache locality, std::vector behavior
- Validated: Random access iterators, index-based access
- File size: 2677 lines (matching vol and vofl comprehensiveness)

**Phase 1.4a: Vector Vertices + Deque Edges (vod_graph_traits) - COMPLETE** ✅
- Tests Added: 50 tests (44 CTests)
- File: `test_dynamic_graph_vod.cpp`
- Coverage: ~98% of vod_graph_traits functionality
- All tests passing ✓
- Key features tested: Stable edge iterators, random access, bidirectional iteration
- Validated: Deque edge container benefits (stable iterators + random access)
- File size: 2649 lines

**Phase 1.5: Deque Vertices + Forward_list Edges (dofl_graph_traits) - COMPLETE** ✅
- Tests Added: 35 tests (628 assertions)
- File: `test_dynamic_graph_dofl.cpp`
- Coverage: ~98% of dofl_graph_traits functionality
- All tests passing ✓
- Key features tested: Stable vertex iterators, forward-only edge iteration
- File size: 2689 lines

**Phase 1.6: Deque Vertices + List Edges (dol_graph_traits) - COMPLETE** ✅
- Tests Added: 44 tests (624 assertions)
- File: `test_dynamic_graph_dol.cpp`
- Coverage: ~98% of dol_graph_traits functionality
- All tests passing ✓
- Key features tested: Stable vertex iterators, bidirectional edge iteration
- File size: 2661 lines

**Phase 1.7: Deque Vertices + Vector Edges (dov_graph_traits) - COMPLETE** ✅
- Tests Added: 44 tests (625 assertions)
- File: `test_dynamic_graph_dov.cpp`
- Coverage: ~98% of dov_graph_traits functionality
- All tests passing ✓
- Key features tested: Stable vertex iterators, random access edges
- File size: 2643 lines

**Phase 1.8: Deque Vertices + Deque Edges (dod_graph_traits) - COMPLETE** ✅
- Tests Added: 44 tests (625 assertions)
- File: `test_dynamic_graph_dod.cpp`
- Coverage: ~98% of dod_graph_traits functionality
- All tests passing ✓
- Key features tested: All stable iterators, random access edges
- File size: 2649 lines

**Implementation Approach:**
- [x] Create one test file at a time
- [x] Start with `test_dynamic_graph_vofl.cpp` (most common use case) - **COMPLETE**
- [x] Continue with `test_dynamic_graph_vol.cpp` (bidirectional edges) - **COMPLETE**
- [x] Next: `test_dynamic_graph_vov.cpp` (best cache locality) - **COMPLETE**
- [ ] Phase 1.4: Create optimized test structure using TEMPLATE_TEST_CASE
  - [ ] `test_dynamic_graph_common.cpp` - unified tests for 8 sequential traits (~2000 lines)
        Traits: vofl, vol, vov, vod, dofl, dol, dov, dod
  - [ ] `test_dynamic_graph_vector_traits.cpp` - vector vertex specifics (~800 lines)
        Covers: vofl, vol, vov, vod (contiguous vertex storage)
  - [ ] `test_dynamic_graph_deque_traits.cpp` - deque vertex specifics + CPO (~1000 lines)
        Covers: dofl, dol, dov, dod (non-contiguous vertex storage)
  - [ ] `test_dynamic_graph_edge_containers.cpp` - all edge container variations (~600 lines)
        Covers: forward_list, list, vector, deque edge behaviors
- [ ] Phase 3: Associative container tests (separate from Phase 1)
  - [ ] `test_dynamic_graph_associative_common.cpp` - unified tests for map traits (~1600 lines)
  - [ ] `test_dynamic_graph_map_traits.cpp` - map-specific behavior (~700 lines)
  - [ ] `test_dynamic_graph_unordered_map_traits.cpp` - unordered_map-specific (~700 lines)
- [x] Use Catch2 test framework with SECTION organization
- [x] Use Catch2 TEMPLATE_TEST_CASE for cross-container unified tests
- [x] Employ test generators for value type combinations where appropriate
- [x] Include both positive tests (correct usage) and negative tests (error detection)

**Status:** Phases 1-3 COMPLETE ✅ | Phase 4.1 IN PROGRESS ⏳ (60% complete)

**Phase 1 & 2: Sequential Containers (8 traits)** ✅ COMPLETE
- ✅ vofl, vol, vov, vod (vector vertices): Basic + CPO tests COMPLETE
- ✅ dofl, dol, dov, dod (deque vertices): Basic + CPO tests COMPLETE
- Total: 16 test files (~41,000 lines)

**Phase 3: Associative Containers (8 traits)** ✅ COMPLETE
- ✅ mofl, mol, mov, mod (map vertices): Basic + CPO tests COMPLETE
- ✅ uofl, uol, uov, uod (unordered_map vertices): Basic + CPO tests COMPLETE
- Total: 16 test files (~26,000 lines)

**Phase 4.1: Set Edge Containers (IN PROGRESS)** ⏳
- ✅ vos (vector + set): Basic + CPO tests COMPLETE
- ✅ dos (deque + set): Basic + CPO tests COMPLETE
- ✅ mos (map + set): Basic + CPO tests COMPLETE
- ⏳ uos (unordered_map + set): PENDING
- Current: 6 test files, 3/4 traits complete (75%)

**Overall Project Status:**
- Total Traits Implemented: 19/19 (100%)
- Total Basic Test Files: 19/19 ✅
- Total CPO Test Files: 19/19 ✅
- Total Test Files: 40 (38 basic + CPO, 2 additional)
- Estimated Total Lines: ~85,000+ lines

**Expected Line Count (Optimized Strategy):**
- Legacy complete files (Phase 1.1-1.3): 8,027 lines ✅
- Phase 1.4a (vod basic + CPO): 6,140 lines ✅
- Phase 1.5-1.8 (dofl, dol, dov, dod basic): 10,642 lines ✅
- CPO tests for Phase 1.1-1.3: 10,313 lines ✅
- CPO tests for Phase 1.4a: 3,491 lines ✅
- CPO tests for Phase 1.5-1.8 (dofl, dol, dov, dod): 13,706 lines ✅
- **Phase 1 & 2 Total: 52,319 lines COMPLETE** ✅
- **Phase 3.1a mofl (map + forward_list): 4,495 lines COMPLETE** ✅
- **Phase 3.1b mol (map + list): 3,120 lines COMPLETE** ✅
- **Phase 3.1c mov (map + vector): 3,250 lines COMPLETE** ✅
- **Phase 3.1d mod (map + deque): 3,233 lines COMPLETE** ✅
- **Phase 3.1e uofl (unordered_map + forward_list): ~3,000 lines COMPLETE** ✅
- **Phase 3.1f uol (unordered_map + list): ~3,000 lines COMPLETE** ✅
- **Phase 3.1g uov (unordered_map + vector): ~3,000 lines COMPLETE** ✅
- **Phase 3.1h uod (unordered_map + deque): ~3,000 lines COMPLETE** ✅
- **Total Phase 1 + Phase 2 + Phase 3: ~78,000 lines COMPLETE** ✅
- **Phase 4.1 (Set Edge Containers):**
  - vos (vector + set): ~1,800 lines COMPLETE ✅
  - dos (deque + set): ~2,000 lines COMPLETE ✅
  - mos (map + set): ~2,900 lines COMPLETE ✅ (1123 basic + 1773 CPO)
  - uos (unordered_map + set): ~2,000 lines PENDING ⏳
  - **Phase 4.1 Subtotal: ~6,700 lines (~75% complete)**
- **Current Overall Progress: ~85,000 lines completed (Phases 1-3 + 75% Phase 4.1)**
- **Note:** All sequential container tests complete; All map-based associative COMPLETE; All unordered_map-based COMPLETE; Set edge containers 75% complete

---

## Phase 2: Add CPO Support (Integrated with Phase 1)

For each graph instance tested in Phase 1, create comprehensive CPO tests. This phase runs in parallel with Phase 1, not sequentially - each new graph type gets both basic tests and CPO tests.

**CPOs to Implement (in priority order):**

1. **vertices(g)** - Return vertex range
   - Default: return g (graph is already a range)
   - Test: iteration, range algorithms

2. **edges(g, u)** - Return edge range for vertex u
   - Default: return u.edges()
   - Test: per-vertex iteration, const correctness

3. **vertex_id(g, u)** - Get vertex ID from vertex iterator/descriptor
   - Implementation: iterator subtraction from begin()
   - Test: all vertices, random access

4. **target_id(g, uv)** - Get target vertex ID from edge
   - Member function on edge (already exists in design)
   - Test: all edges, consistency

5. **find_vertex(g, uid)** - Find vertex by ID
   - Default: vertices(g).begin() + uid (for random access)
   - Test: all vertex IDs, out of range

6. **num_vertices(g)** - Count vertices
   - Default: g.size()
   - Test: empty, single, many

7. **num_edges(g)** - Count total edges
   - Implementation: track in member variable (already exists)
   - Test: empty, various counts

8. **degree(g, u)** - Get out-degree of vertex
   - Default: ranges::distance(edges(g, u))
   - Test: isolated vertices, various degrees

9. **source_id(g, uv)** - Get source ID (when Sourced=true)
   - Member function on edge
   - Test: sourced graphs only

10. **source(g, uv)** - Get source vertex (when Sourced=true)
    - Default: find_vertex(g, source_id(g, uv))
    - Test: sourced graphs only

11. **target(g, uv)** - Get target vertex
    - Default: find_vertex(g, target_id(g, uv))
    - Test: all edges

12. **vertex_value(g, u)** - Access vertex value
    - Default: u.value()
    - Test: non-void VV only

13. **edge_value(g, uv)** - Access edge value
    - Default: uv.value()
    - Test: non-void EV only

14. **graph_value(g)** - Access graph value
    - Member function
    - Test: non-void GV only

15. **partition_id(g, uid)** - Get partition for vertex
    - Implementation: binary search (already outlined)
    - Test: all partitions

16. **num_partitions(g)** - Count partitions
    - Implementation: partition_.size() - 1
    - Test: single and multiple

**Test Files Completed:**
- `tests/test_dynamic_graph_cpo_vofl.cpp` (3,414 lines, 196 test cases) ✅
- `tests/test_dynamic_graph_cpo_vol.cpp` (3,413 lines, 196 test cases) ✅
- `tests/test_dynamic_graph_cpo_vov.cpp` (3,486 lines, 196 test cases) ✅
- `tests/test_dynamic_graph_cpo_vod.cpp` (3,491 lines, 196 test cases) ✅
- `tests/test_dynamic_graph_cpo_dofl.cpp` (3,415 lines, 27 test cases, 1,232 assertions) ✅
- `tests/test_dynamic_graph_cpo_dol.cpp` (3,414 lines, 27 test cases, 1,232 assertions) ✅
- `tests/test_dynamic_graph_cpo_dov.cpp` (3,486 lines, 29 test cases, 1,248 assertions) ✅
- `tests/test_dynamic_graph_cpo_dod.cpp` (3,491 lines, 29 test cases, 1,248 assertions) ✅
- `tests/test_dynamic_graph_cpo_mofl.cpp` (1,822 lines, 50 test cases, 535 assertions) ✅
- `tests/test_dynamic_graph_cpo_mol.cpp` (1,850 lines, 27 test cases) ✅
- `tests/test_dynamic_graph_cpo_mov.cpp` (1,850 lines, 27 test cases) ✅
- `tests/test_dynamic_graph_cpo_mod.cpp` (1,850 lines, 27 test cases, 603 assertions) ✅

**Total CPO Tests:** 34,982 lines, 1,027 test cases

**Phase 2 Status: COMPLETE** ✅ All sequential container CPO tests implemented
**Phase 3.1 Map Containers CPO Status: COMPLETE** ✅ All map-based CPO tests implemented (mofl, mol, mov, mod)

**Test Files Pending:**
- Future unordered_map associative container CPO tests (Phase 3.1e-h: uofl, uol, uov, uod)

**CPO Implementation Strategy:**
- ✅ Uncommented and completed friend functions in dynamic_graph.hpp
- ✅ Used default implementations via CPO machinery from graph_cpo.hpp
- ✅ Leveraged existing descriptor infrastructure
- ✅ Testing each CPO with multiple configurations (EV, VV, GV, Sourced, Partitions)
- ⏳ Continue pattern for each new graph instance type

---

## Phase 3: Extend Vertex Containers (Associative)

Add support for map-based vertex containers.

### Phase 3.1: Map Vertex Containers

**Phase 3.1a: mofl_graph_traits (map + forward_list) - COMPLETE** ✅
- Basic Tests: `test_dynamic_graph_mofl.cpp` (2,673 lines, 44 test cases)
- CPO Tests: `test_dynamic_graph_cpo_mofl.cpp` (1,822 lines, 50 test cases, 535 assertions)
- All 27 CPOs tested with comprehensive coverage
- Key features verified:
  - Sparse vertices (only referenced vertices exist)
  - Map iteration in key order (sorted)
  - String vertex IDs extensively tested
  - forward_list edge order: last added appears first
  - Bidirectional vertex iteration (no random access)
  - Fixed `edges(g, u)` ADL function for map containers where vertex descriptor stores const_iterator

**Phase 3.1b: mol_graph_traits (map + list) - COMPLETE** ✅
- Basic Tests: `test_dynamic_graph_mol.cpp` (1,270 lines, 27 test cases, 227 assertions)
- CPO Tests: `test_dynamic_graph_cpo_mol.cpp` (1,850 lines, 27 test cases)
- Key features verified:
  - Sparse vertices (only referenced vertices exist)
  - Map iteration in key order (sorted)
  - String vertex IDs extensively tested
  - std::list bidirectional edge iteration
  - Edge order: first added appears first (unlike forward_list)
  - Bidirectional vertex and edge iterators

**Phase 3.1c: mov_graph_traits (map + vector) - COMPLETE** ✅
- Basic Tests: `test_dynamic_graph_mov.cpp` (1,400 lines, 26 test cases)
- CPO Tests: `test_dynamic_graph_cpo_mov.cpp` (1,850 lines, 27 test cases)
- Key features verified:
  - Sparse vertices (only referenced vertices exist)
  - Map iteration in key order (sorted)
  - String vertex IDs extensively tested
  - std::vector random access edge iteration
  - Edge order: first added appears first
  - Random access edge iterators

**Phase 3.1d: mod_graph_traits (map + deque) - COMPLETE** ✅
- Basic Tests: `test_dynamic_graph_mod.cpp` (1,383 lines, 26 test cases)
- CPO Tests: `test_dynamic_graph_cpo_mod.cpp` (1,850 lines, 27 test cases, 603 assertions)
- Key features verified:
  - Sparse vertices (only referenced vertices exist)
  - Map iteration in key order (sorted)
  - String vertex IDs extensively tested
  - std::deque random access edge iteration with efficient front/back insertion
  - Edge order: first added appears first
  - Random access edge iterators

**Phase 3.1e: uofl_graph_traits (unordered_map + forward_list) - COMPLETE** ✅
Mirrors mofl but with hash-based O(1) average lookup, unordered iteration.

| Step | Task | Status |
|------|------|--------|
| 3.1e.1 | Create `uofl_graph_traits.hpp` | ✅ DONE |
| 3.1e.2 | Create `test_dynamic_graph_uofl.cpp` (~1100 lines) | ✅ DONE |
| 3.1e.3a | Create uofl CPO tests Part 1: Header, type aliases, vertices, num_vertices, find_vertex, vertex_id (~300 lines) | ✅ DONE |
| 3.1e.3b | Create uofl CPO tests Part 2: num_edges, edges(g,u), edges(g,uid), degree (~300 lines) | ✅ DONE |
| 3.1e.3c | Create uofl CPO tests Part 3: target_id, target, find_vertex_edge(g,u,v) (~250 lines) | ✅ DONE |
| 3.1e.3d | Create uofl CPO tests Part 4: contains_edge(g,u,v), vertex_value, edge_value, graph_value (~250 lines) | ✅ DONE |
| 3.1e.3e | Create uofl CPO tests Part 5: has_edge, source_id, source, partition CPOs (~250 lines) | ✅ DONE |
| 3.1e.3f | Create uofl CPO tests Part 6: find_vertex_edge(uid,vid), contains_edge(uid,vid), integration tests (~450 lines) | ✅ DONE |
| 3.1e.4 | Update `tests/CMakeLists.txt` | ✅ DONE |
| 3.1e.5 | Build and verify all tests pass | ✅ DONE |

**Phase 3.1f: uol_graph_traits (unordered_map + list) - COMPLETE** ✅
Mirrors mol but with hash-based O(1) average lookup, unordered iteration.

| Step | Task | Status |
|------|------|--------|
| 3.1f.1 | Create `uol_graph_traits.hpp` | ✅ DONE |
| 3.1f.2 | Create `test_dynamic_graph_uol.cpp` (~1100 lines) | ✅ DONE |
| 3.1f.3 | Create `test_dynamic_graph_cpo_uol.cpp` (~1800 lines) | ✅ DONE |
| 3.1f.4 | Update `tests/CMakeLists.txt` | ✅ DONE |
| 3.1f.5 | Build and verify all tests pass | ✅ DONE |

**Phase 3.1g: uov_graph_traits (unordered_map + vector) - COMPLETE** ✅
Mirrors mov but with hash-based O(1) average lookup, unordered iteration.

| Step | Task | Status |
|------|------|--------|
| 3.1g.1 | Create `uov_graph_traits.hpp` | ✅ DONE |
| 3.1g.2 | Create `test_dynamic_graph_uov.cpp` (~1100 lines) | ✅ DONE |
| 3.1g.3a | Create uov CPO tests Part 1: Header, type aliases, vertices, num_vertices, find_vertex, vertex_id (~300 lines) | ✅ DONE |
| 3.1g.3b | Create uov CPO tests Part 2: num_edges, edges(g,u), edges(g,uid), degree (~300 lines) | ✅ DONE |
| 3.1g.3c | Create uov CPO tests Part 3: target_id, target, find_vertex_edge(g,u,v) (~250 lines) | ✅ DONE |
| 3.1g.3d | Create uov CPO tests Part 4: contains_edge(g,u,v), vertex_value, edge_value, graph_value (~250 lines) | ✅ DONE |
| 3.1g.3e | Create uov CPO tests Part 5: has_edge, source_id, source, partition CPOs (~250 lines) | ✅ DONE |
| 3.1g.3f | Create uov CPO tests Part 6: find_vertex_edge(uid,vid), contains_edge(uid,vid), integration tests (~450 lines) | ✅ DONE |
| 3.1g.4 | Update `tests/CMakeLists.txt` | ✅ DONE |
| 3.1g.5 | Build and verify all tests pass | ✅ DONE |

**Phase 3.1h: uod_graph_traits (unordered_map + deque) - COMPLETE** ✅
Mirrors mod but with hash-based O(1) average lookup, unordered iteration.

| Step | Task | Status |
|------|------|--------|
| 3.1h.1 | Create `uod_graph_traits.hpp` | ✅ DONE |
| 3.1h.2 | Create `test_dynamic_graph_uod.cpp` (~1100 lines) | ✅ DONE |
| 3.1h.3a | Create uod CPO tests Part 1: Header, type aliases, vertices, num_vertices, find_vertex, vertex_id (~300 lines) | ✅ DONE |
| 3.1h.3b | Create uod CPO tests Part 2: num_edges, edges(g,u), edges(g,uid), degree (~300 lines) | ✅ DONE |
| 3.1h.3c | Create uod CPO tests Part 3: target_id, target, find_vertex_edge(g,u,v) (~250 lines) | ✅ DONE |
| 3.1h.3d | Create uod CPO tests Part 4: contains_edge(g,u,v), vertex_value, edge_value, graph_value (~250 lines) | ✅ DONE |
| 3.1h.3e | Create uod CPO tests Part 5: has_edge, source_id, source, partition CPOs (~250 lines) | ✅ DONE |
| 3.1h.3f | Create uod CPO tests Part 6: find_vertex_edge(uid,vid), contains_edge(uid,vid), integration tests (~450 lines) | ✅ DONE |
| 3.1h.4 | Update `tests/CMakeLists.txt` | ✅ DONE |
| 3.1h.5 | Build and verify all tests pass | ✅ DONE |

**Unordered Map Key Differences from Map:**
1. **Hash-based storage** - O(1) average lookup vs O(log n) for map
2. **Unordered iteration** - vertices do NOT iterate in key order
3. **Requires hashable keys** - std::hash specialization needed (std::string has it)
4. **bucket_count()** - exposes hash table internals for testing

**New Traits Structures:**
```cpp
template <class EV, class VV, class GV, class VId, bool Sourced>
struct mofl_graph_traits {  // map + forward_list ✅ COMPLETE
  using vertices_type = std::map<VId, vertex_type>;
  using edges_type = std::forward_list<edge_type>;
};

template <class EV, class VV, class GV, class VId, bool Sourced>
struct mol_graph_traits {  // map + list ✅ COMPLETE
  using vertices_type = std::map<VId, vertex_type>;
  using edges_type = std::list<edge_type>;
};

template <class EV, class VV, class GV, class VId, bool Sourced>
struct mov_graph_traits {  // map + vector ✅ COMPLETE
  using vertices_type = std::map<VId, vertex_type>;
  using edges_type = std::vector<edge_type>;
};

template <class EV, class VV, class GV, class VId, bool Sourced>
struct mod_graph_traits {  // map + deque ✅ COMPLETE
  using vertices_type = std::map<VId, vertex_type>;
  using edges_type = std::deque<edge_type>;
};

template <class EV, class VV, class GV, class VId, bool Sourced>
struct uofl_graph_traits {  // unordered_map + forward_list ⏳ IN PROGRESS
  using vertices_type = std::unordered_map<VId, vertex_type>;
  using edges_type = std::forward_list<edge_type>;
};

template <class EV, class VV, class GV, class VId, bool Sourced>
struct uol_graph_traits {  // unordered_map + list ⏳ PENDING
  using vertices_type = std::unordered_map<VId, vertex_type>;
  using edges_type = std::list<edge_type>;
};

template <class EV, class VV, class GV, class VId, bool Sourced>
struct uov_graph_traits {  // unordered_map + vector ⏳ PENDING
  using vertices_type = std::unordered_map<VId, vertex_type>;
  using edges_type = std::vector<edge_type>;
};

template <class EV, class VV, class GV, class VId, bool Sourced>
struct uod_graph_traits {  // unordered_map + deque ⏳ PENDING
  using vertices_type = std::unordered_map<VId, vertex_type>;
  using edges_type = std::deque<edge_type>;
};
```

**Required Vertex ID Types for Associative Containers:**
- `std::string` (named vertices - primary test type)
- `std::pair<int, int>` (coordinate pairs)
- `std::tuple<int, int, int>` (3D coordinates)
- Custom struct `USAAddress` with operator<, hash support

**Critical Differences from Sequential Containers:**
1. **No auto-extension** - only creates vertices explicitly referenced in edges
2. **Key-based lookup** - `g["alice"]` not `g[0]`
3. **Bidirectional iterators only** - no random access, no iterator arithmetic
4. **Different CPO implementations** - vertex_id() returns key, find_vertex() uses find()
5. **Different construction semantics** - sparse vertex IDs by design

**Implementation Changes Needed:**
1. Update `vertex_id()` CPO for bidirectional iterators (map) - returns key not index
2. Update `find_vertex()` to use map's find() member instead of begin() + offset
3. Adjust construction logic for non-contiguous vertex IDs
4. Handle vertex insertion vs. assignment semantics
5. No auto-extension - vertices must be explicitly created

**Test Files:**
```
tests/test_dynamic_graph_associative_common.cpp  # ~1600 lines (unified for 3-6 traits)
tests/test_dynamic_graph_map_traits.cpp          # ~700 lines (map-specific)
tests/test_dynamic_graph_unordered_map_traits.cpp # ~700 lines (unordered_map-specific)
```

**Test Coverage (Associative Common File):**
- Construction with string vertex IDs
- Key-based vertex lookup (not index-based)
- No auto-extension behavior
- Sparse vertex ID sequences
- Bidirectional iteration only (no random access)
- All edge access patterns (same as sequential)
- Value access (same as sequential where applicable)
- CPO implementations specific to associative containers

**Test Coverage (Map-Specific):**
- Ordered iteration (sorted by key)
- Custom comparators
- Key ordering requirements
- Iterator invalidation rules specific to std::map
- Lower/upper bound operations

**Test Coverage (Unordered_Map-Specific):**
- Custom hash functions (for unordered_map)
- Hash collision handling
- Bucket management
- Load factor behavior
- Hash consistency across copies
- Iterator invalidation rules specific to std::unordered_map

**Why Separate Common File is Required:**
- Sequential containers use `uint64_t` with auto-extension: `g[5]` creates vertices 0-5
- Associative containers use `std::string` without auto-extension: `g["alice"]` only creates "alice"
- Test assertions are fundamentally different: `REQUIRE(g[0].id() == 0)` vs `REQUIRE(g["alice"].id() == "alice")`
- CPO behavior differs: `vertex_id()` returns index vs key, `find_vertex()` uses arithmetic vs find()
- Cannot share TEMPLATE_TEST_CASE between integral and non-integral vertex ID types

---

## Phase 4: Extend Edge Containers (Associative/Set)

Add support for set/map-based edge containers. This phase requires implementation changes to dynamic_edge to support comparison operators and hashing.

**Prerequisites:**
- Phase 1-3 complete ✅
- Need to add operator<=> and hash support to dynamic_edge

---

### Phase 4.1: Set-Based Edge Containers (std::set)

**Overview:** Add vos_graph_traits (vector vertices + set edges) for all vertex container types.

**Step 4.1.1: Add operator<=> and operator== to dynamic_edge - COMPLETE** ✅

Using `operator<=>` (spaceship operator) provides ordering operators (`<`, `>`, `<=`, `>=`).
Using explicit `operator==` provides equality operators (`==`, `!=`) and is required for:
- `std::unordered_set` (Phase 4.2) which needs `operator==` for collision resolution
- Better performance when equality check is cheaper than full ordering
- Clarity when `operator<=>` is custom (not defaulted)

| Step | Task | Status |
|------|------|--------|
| 4.1.1a | Analyze dynamic_edge class hierarchy for comparison operator locations | ✅ DONE |
| 4.1.1b | Add operator<=> to all 4 dynamic_edge specializations (Sourced=true: source_id+target_id, Sourced=false: target_id only) | ✅ DONE |
| 4.1.1c | Add operator== to all 4 dynamic_edge specializations (matches <=> semantics) | ✅ DONE |
| 4.1.1d | Add std::hash<dynamic_edge> specialization (Sourced=true: hash source_id+target_id, Sourced=false: hash target_id) | ✅ DONE |
| 4.1.1e | Create test_dynamic_edge_comparison.cpp (380 lines, 15 test cases, 83 assertions) | ✅ DONE |
| 4.1.1f | Build and verify all comparison operators work (2749 tests pass) | ✅ DONE |

**Implementation Notes:**
- Added `#include <compare>` to dynamic_graph.hpp
- Operators added directly to dynamic_edge (not base classes) for all 4 specializations
- Edge values intentionally excluded from comparison (only structural IDs compared)
- Tests verify integration with std::set and std::unordered_set containers

**Step 4.1.2: Create vos_graph_traits (vector + set) - COMPLETE** ✅

| Step | Task | Status |
|------|------|--------|
| 4.1.2a | Create vos_graph_traits.hpp | ✅ DONE |
| 4.1.2b | Verify dynamic_graph compiles with std::set edges | ✅ DONE |
| 4.1.2c | Create test_dynamic_graph_vos.cpp basic tests (~600 lines, 20 test cases, 113 assertions) | ✅ DONE |
| 4.1.2d | Create test_dynamic_graph_cpo_vos.cpp CPO tests (~1200 lines, 26 TEST_CASE) | ✅ DONE |
| 4.1.2e | Update CMakeLists.txt | ✅ DONE |
| 4.1.2f | Build and verify all vos tests pass | ✅ DONE |

**Step 4.1.3: Create dos_graph_traits (deque + set) - COMPLETE** ✅

| Step | Task | Status |
|------|------|--------|
| 4.1.3a | Create dos_graph_traits.hpp | ✅ DONE |
| 4.1.3b | Create test_dynamic_graph_dos.cpp (~800 lines) | ✅ DONE |
| 4.1.3c | Create test_dynamic_graph_cpo_dos.cpp (~1200 lines) | ✅ DONE |
| 4.1.3d | Update CMakeLists.txt and verify tests pass | ✅ DONE |

**Step 4.1.4: Create mos_graph_traits (map + set) - COMPLETE** ✅

| Step | Task | Status |
|------|------|--------|
| 4.1.4a | Create mos_graph_traits.hpp | ✅ DONE |
| 4.1.4b | Create test_dynamic_graph_mos.cpp (~800 lines, 20 test cases) | ✅ DONE |
| 4.1.4c | Create test_dynamic_graph_cpo_mos.cpp (~1773 lines, 28 test cases, 377 assertions) | ✅ DONE |
| 4.1.4d | Update CMakeLists.txt and verify tests pass | ✅ DONE |

**Test Results:**
- All 28 CPO test cases passing ✅
- 377 assertions validated ✅
- Comprehensive testing of map vertices + set edges
- String and uint32_t vertex ID configurations tested
- Set deduplication and ordering verified
- Map sparse vertex behavior confirmed

**Step 4.1.5: Create uos_graph_traits (unordered_map + set)**

| Step | Task | Status |
|------|------|--------|
| 4.1.5a | Create uos_graph_traits.hpp | ⏳ PENDING |
| 4.1.5b | Create test_dynamic_graph_uos.cpp (~800 lines) | ⏳ PENDING |
| 4.1.5c | Create test_dynamic_graph_cpo_uos.cpp (~1200 lines) | ⏳ PENDING |
| 4.1.5d | Update CMakeLists.txt and verify tests pass | ⏳ PENDING |

**Phase 4.1 Current Status Summary:**
- ✅ vos (vector + set): Basic tests COMPLETE, CPO tests COMPLETE
- ✅ dos (deque + set): Basic tests COMPLETE, CPO tests COMPLETE  
- ✅ mos (map + set): Basic tests COMPLETE, CPO tests COMPLETE
- ⏳ uos (unordered_map + set): PENDING
- **Progress: 75% complete (3/4 traits done)**

---

### Phase 4.2: Unordered Set Edge Containers (std::unordered_set) ✅ **100% COMPLETE**

**Overview:** Add traits using std::unordered_set for edges. Requires hash specialization for dynamic_edge.

**Step 4.2.1: Add hash specialization for dynamic_edge - COMPLETE** ✅

| Step | Task | Status |
|------|------|--------|
| 4.2.1a | Create std::hash specialization for dynamic_edge in std namespace | ✅ DONE |
| 4.2.1b | Hash should combine target_id (and source_id if Sourced=true) | ✅ DONE |
| 4.2.1c | Add hash tests in test_dynamic_edge_comparison.cpp | ✅ DONE |
| 4.2.1d | Verify hash works with std::unordered_set | ✅ DONE |

**Implementation Details:**
- Hash specialization in [dynamic_graph.hpp](include/graph/container/dynamic_graph.hpp) lines 2118-2153
- Hashes only structural identifiers (source_id + target_id), not edge values
- Uses boost-style hash combination: `h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2))`
- Tests in [test_dynamic_edge_comparison.cpp](tests/test_dynamic_edge_comparison.cpp) (451 lines, 11 test cases)
- All hash tests passing: 7 assertions in 2 test cases ✅
- Integration with std::unordered_set validated: 6 assertions ✅

**Note:** Hash specialization was implemented alongside operator<=> and operator== in Phase 4.1.1d.
All prerequisites for both std::set (Phase 4.1) and std::unordered_set (Phase 4.2) are complete.

**Step 4.2.2: Create vous_graph_traits (vector + unordered_set) - COMPLETE** ✅

| Step | Task | Status |
|------|------|--------|
| 4.2.2a | Create vous_graph_traits.hpp | ✅ DONE |
| 4.2.2b | Verify dynamic_graph compiles with std::unordered_set edges | ✅ DONE |
| 4.2.2c | Create test_dynamic_graph_vous.cpp basic tests (~486 lines, 11 test cases, 60 assertions) | ✅ DONE |
| 4.2.2d | Create test_dynamic_graph_cpo_vous.cpp CPO tests (~1281 lines, 26 test cases, 178 assertions) | ✅ DONE |
| 4.2.2e | Update CMakeLists.txt and verify tests pass | ✅ DONE |

**Implementation Details:**
- vous_graph_traits.hpp: 51 lines, uses std::unordered_set for edges
- O(1) average edge operations vs O(log n) for vos
- Forward iterators only (vs bidirectional for vos)
- Automatic deduplication like vos
- Test adaptations: All order-dependent assertions updated to sort before comparing
- All 37 test cases passing (11 basic + 26 CPO) ✅

**Step 4.2.3: Create dous_graph_traits (deque + unordered_set)** ✅ COMPLETE

| Step | Task | Status |
|------|------|--------|
| 4.2.3a | Create dous_graph_traits.hpp | ✅ COMPLETE |
| 4.2.3b | Create test_dynamic_graph_dous.cpp (486 lines, 11 tests) | ✅ COMPLETE |
| 4.2.3c | Create test_dynamic_graph_cpo_dous.cpp (1281 lines, 26 tests) | ✅ COMPLETE |
| 4.2.3d | Update CMakeLists.txt and verify tests pass (238 assertions) | ✅ COMPLETE |

**Step 4.2.4: Create mous_graph_traits (map + unordered_set)** ✅ COMPLETE

| Step | Task | Status |
|------|------|--------|
| 4.2.4a | Create mous_graph_traits.hpp | ✅ COMPLETE |
| 4.2.4b | Create test_dynamic_graph_mous.cpp (1129 lines, 25 tests) | ✅ COMPLETE |
| 4.2.4c | Create test_dynamic_graph_cpo_mous.cpp (1781 lines, 28 tests) | ✅ COMPLETE |
| 4.2.4d | Update CMakeLists.txt and verify tests pass (578 assertions) | ✅ COMPLETE |

**Step 4.2.5: Create uous_graph_traits (unordered_map + unordered_set)** ✅ **COMPLETE** (2024-01-XX)

**Files:** `uous_graph_traits.hpp` (55 lines), `test_dynamic_graph_uous.cpp` (1131 lines, 25 tests), `test_dynamic_graph_cpo_uous.cpp` (1777 lines, 28 tests)

**Results:** All 53 test cases passing (563 assertions). O(1) average complexity for both vertices and edges.

| Step | Task | Status |
|------|------|--------|
| 4.2.5a | Create uous_graph_traits.hpp | ✅ DONE |
| 4.2.5b | Create test_dynamic_graph_uous.cpp (~800 lines) | ✅ DONE (1131 lines, 25 tests) |
| 4.2.5c | Create test_dynamic_graph_cpo_uous.cpp (~1200 lines) | ✅ DONE (1777 lines, 28 tests) |
| 4.2.5d | Update CMakeLists.txt and verify tests pass | ✅ DONE (563 assertions) |

**Phase 4.2 Status:** 100% COMPLETE ✅ (4/4 traits implemented: vous, dous, mous, uous)

---

### Phase 4.3: Map-Based Edge Containers (std::map keyed by target_id)

**Overview:** Add traits using std::map<VId, edge_type> for edges, enabling O(log n) edge lookup by target.

**Step 4.3.1: Analyze edge container interface changes needed** ✅ **COMPLETE** (2024-12-28)

**Analysis Document:** [docs/edge_map_analysis.md](../docs/edge_map_analysis.md)

**Key Findings:**
- Minimal changes required - existing infrastructure largely compatible
- Need: concept detection, pair construction helper, edge_descriptor pair unwrapping
- Map key: `VId` (target_id) - simpler than pair<VId,VId>
- O(log n) lookup benefit, automatic deduplication, sorted order
- Memory overhead: ~8-16 bytes/edge for pair wrapper

| Step | Task | Status |
|------|------|--------|
| 4.3.1a | Review dynamic_vertex edge container usage patterns | ✅ DONE |
| 4.3.1b | Identify changes needed for map-based edge access (key vs iterator) | ✅ DONE |
| 4.3.1c | Design edge_descriptor changes for map-based edges (if any) | ✅ DONE |

**Step 4.3.2: Create voem_graph_traits (vector + edge map)** ✅ **COMPLETE** (2024-12-28)

**Implementation Summary:**
- Added is_map_based_edge_container concept to container_utility.hpp
- Added emplace_edge helper for pair-wrapped edge insertion
- Updated edge_descriptor::target_id() to unwrap map pairs
- Updated edge_value CPO to extract values from map pairs  
- Created voem_graph_traits.hpp with std::map<VId, edge_type> edges
- Created test_dynamic_graph_voem.cpp (~741 lines, 25 test cases)
- Created test_dynamic_graph_cpo_voem.cpp (~1273 lines, 21 test cases)
- All 46 test cases passing

| Step | Task | Status |
|------|------|--------|
| 4.3.2a | Create voem_graph_traits.hpp | ✅ DONE |
| 4.3.2b | Update load_edges or edge insertion for map semantics | ✅ DONE |
| 4.3.2c | Create test_dynamic_graph_voem.cpp basic tests (~800 lines) | ✅ DONE |
| 4.3.2d | Create test_dynamic_graph_cpo_voem.cpp CPO tests (~1200 lines) | ✅ DONE |
| 4.3.2e | Update CMakeLists.txt and verify tests pass | ✅ DONE |

**Step 4.3.3: Create moem_graph_traits (map vertices + edge map)** ✅ **COMPLETE** (2024-12-28)

**Implementation Summary:**
- Created moem_graph_traits.hpp with std::map vertices and std::map edges
- Simplified operator[] using at() for both container types (throws if not found)
- Added is_map_based_vertex_container concept
- Tests derived from mos (which also has map vertices)
- Created test_dynamic_graph_moem.cpp (~1123 lines, 27 test cases)
- Created test_dynamic_graph_cpo_moem.cpp (~1274 lines, 26 test cases)
- All 53 test cases passing

| Step | Task | Status |
|------|------|--------|
| 4.3.3a | Create moem_graph_traits.hpp | ✅ DONE |
| 4.3.3b | Create test_dynamic_graph_moem.cpp (~800 lines) | ✅ DONE |
| 4.3.3c | Create test_dynamic_graph_cpo_moem.cpp (~1200 lines) | ✅ DONE |
| 4.3.3d | Update CMakeLists.txt and verify tests pass | ✅ DONE |

---

### Phase 4 Summary

**Total New Traits:**
- Set edges: vos, dos, mos, uos (4 traits)
- Unordered set edges: vous, dous, mous, uous (4 traits)
- Map edges: voem, moem (2 traits)

**Implementation Changes:**
- operator<=> for dynamic_edge (generates <, >, <=, >=)
- operator== for dynamic_edge (generates ==, !=; required for unordered_set)
- std::hash specialization for dynamic_edge
- Possible edge container interface updates for map-based edges

**Estimated Lines:**
- Edge comparison/hash support: ~500 lines
- 10 new traits × ~2,000 lines each: ~20,000 lines
- Total Phase 4: ~20,500 lines

**Key Considerations:**
1. Set-based edges prevent duplicate edges (same source→target)
2. Map-based edges key by target_id, allowing O(log n) or O(1) lookup
3. Edge values can still be stored alongside target_id in map values
4. find_vertex_edge CPO may have optimized implementations for set/map edges

---

## Phase 5: Non-Integral Vertex IDs

Extend and validate support for non-integral vertex ID types. This phase focuses on verifying
that map/unordered_map-based vertex containers work correctly with various ID types beyond
the standard integral types.

**Current State Analysis:**
- Map-based containers (mos, moem, mol, etc.) already support `std::string` vertex IDs
- Tests exist with `std::string` IDs in: mos, mous, moem, mol, mov, mod, mofl tests
- Basic string ID functionality is validated but not comprehensively tested
- No tests exist for: double IDs, compound types, custom types with complex comparison

**New VId Types to Test:**
- `double` (floating-point IDs with precision considerations)
- `std::string` (already partially tested - needs comprehensive coverage)
- Compound types: `struct PersonId { std::string name; int department; }`
- Custom types with complex hash/comparison logic

**Requirements:**
- Must use map/unordered_map vertex containers (vector/deque require integral IDs for indexing)
- std::map requires: `operator<` or custom comparator
- std::unordered_map requires: `std::hash` specialization + `operator==`
- CPO functions must work correctly with non-integral IDs

---

### Phase 5.1: Analysis and Preparation ✅ COMPLETE

**Goal:** Understand current support and identify gaps.

| Step | Task | Status |
|------|------|--------|
| 5.1.1 | Audit existing string ID tests across all map-based traits | ✅ DONE |
| 5.1.2 | Verify vertex_id() CPO works with string IDs | ✅ DONE |
| 5.1.3 | Verify all CPOs work with string IDs in map containers | ✅ DONE |
| 5.1.4 | Document which traits support which ID types | ✅ DONE |

**Deliverable:** Analysis document identifying gaps in non-integral ID coverage.

---

#### Phase 5.1 Analysis Results (December 28, 2025)

**1. Trait ID Type Support Matrix:**

| Trait Category | Vertex Container | Supports Non-Integral VId | Reason |
|----------------|------------------|--------------------------|--------|
| Sequential (vo*, do*) | vector/deque | ❌ NO | Requires integral for indexing |
| Ordered Associative (mo*) | std::map | ✅ YES | Any type with operator< |
| Unordered Associative (uo*) | std::unordered_map | ✅ YES | Any type with hash + operator== |

**2. Existing String ID Test Coverage:**

| Trait | Basic Tests | CPO Tests | String Sections |
|-------|-------------|-----------|-----------------|
| mos | 3 test cases | 22 sections | ✅ Comprehensive |
| mous | 3 test cases | 22 sections | ✅ Comprehensive |
| moem | 3 test cases | 22 sections | ✅ Comprehensive |
| mol | 3 test cases | ~20 sections | ✅ Comprehensive |
| mov | 3 test cases | ~20 sections | ✅ Comprehensive |
| mod | 3 test cases | ~20 sections | ✅ Comprehensive |
| mofl | 3 test cases | ~20 sections | ✅ Comprehensive |
| uofl | 3 test cases | 20 sections | ✅ Comprehensive |
| uol | 3 test cases | ~20 sections | ✅ Comprehensive |
| uov | 3 test cases | ~20 sections | ✅ Comprehensive |
| uod | 3 test cases | ~20 sections | ✅ Comprehensive |
| uous | 3 test cases | ~20 sections | ✅ Comprehensive |

**Total:** 36 basic test cases + ~260 CPO string sections across 12 traits

**3. CPO Functions Verified with String IDs (mos as reference):**

All major CPO functions have string ID test sections:
- `vertices(g)` ✅
- `num_vertices(g)` ✅
- `find_vertex(g, id)` ✅
- `vertex_id(g, u)` ✅ (returns std::string)
- `edges(g, u)` ✅
- `num_edges(g)` ✅
- `degree(g, u)` ✅
- `target_id(g, uv)` ✅
- `target(g, uv)` ✅
- `source_id(g, uv)` ✅ (sourced graphs)
- `source(g, uv)` ✅ (sourced graphs)
- `find_vertex_edge(g, u, v)` ✅
- `find_vertex_edge(g, uid, vid)` ✅
- `contains_edge(g, u, v)` ✅
- `contains_edge(g, uid, vid)` ✅
- `vertex_value(g, u)` ✅
- `edge_value(g, uv)` ✅
- `partition_id(g, u)` ✅
- `num_partitions(g)` ✅

**4. Test Execution Results:**
- `[string]` tag tests: 128 assertions in 36 test cases - ALL PASSED ✅
- `[mos][cpo]` tests: 377 assertions in 28 test cases - ALL PASSED ✅

**5. Gaps Identified:**

| Gap | Priority | Notes |
|-----|----------|-------|
| No double/float ID tests | Medium | May have precision edge cases |
| No compound type ID tests | Low | No user-defined VId types tested |
| No Unicode string tests | Low | Basic ASCII strings only tested |
| No empty string edge cases | Low | Not explicitly tested |
| No very long string tests | Low | Not tested for performance |
| No graph_value CPO with string IDs | Low | Most graphs use void GV |

**6. Conclusions:**

✅ **String vertex IDs are well-supported** - All map/unordered_map traits work with std::string VId
✅ **CPO layer is fully compatible** - All CPO functions work correctly with string IDs
✅ **Existing coverage is comprehensive** - ~260 string ID test sections across 12 traits

**Recommended Phase 5 Adjustments:**

Given the extensive existing coverage, Phase 5.2 (comprehensive string tests) can be **simplified** 
to focus only on edge cases not currently covered:
- Empty strings, Unicode, very long strings
- Performance with string IDs
- Error conditions

Phase 5.3-5.4 (double/custom IDs) remain as planned since no coverage exists.

---

### Phase 5.2-5.4: Non-Integral ID Tests ✅ COMPLETE (December 28, 2025)

**Implementation:** Combined string edge cases, double IDs, and custom type IDs into single test file.

**Test File:** `tests/test_dynamic_graph_nonintegral_ids.cpp` (~740 lines)

**Results:** 19 test cases, 133 assertions - ALL PASSED ✅

**Coverage:**

| Category | Test Cases | Assertions | Status |
|----------|------------|------------|--------|
| String edge cases (empty, Unicode, long) | 5 | ~35 | ✅ DONE |
| Double/floating-point IDs | 7 | ~50 | ✅ DONE |
| Custom compound type (PersonId) | 5 | ~35 | ✅ DONE |
| Type traits verification | 1 | 4 | ✅ DONE |
| Integration tests | 1 | 9 | ✅ DONE |
| **Total** | **19** | **133** | ✅ DONE |

**Key Tests Implemented:**

*String Edge Cases:*
- Empty strings as vertex IDs
- Whitespace (space, tab, newline) in IDs
- Unicode (CJK, emoji, Greek) vertex IDs
- Very long strings (10K+ characters)
- Sourced edges with string IDs

*Double IDs:*
- Basic construction and lookup
- Ordering (negative before positive)
- Close values (epsilon separation)
- Special values (0.0, -0.0, infinity)
- CPO access (vertex_id, target_id, find_vertex, contains_edge)
- unordered_map with double keys

*Custom Type IDs (PersonId):*
- Compound type with name + department
- Three-way comparison (operator<=>)
- Hash specialization for unordered_map
- Ordering verification
- CPO access with custom types
- Edge values with custom IDs

**Infrastructure Added:**
- `PersonId` struct with `operator<=>` and `operator==`
- `std::hash<PersonId>` specialization using boost-style hash combine

---

### Phase 5.5: CPO Non-Integral ID Validation
---

## Phase 6: Comprehensive Integration Tests

### Phase 5 Summary ✅ COMPLETE

**Implementation Date:** December 28, 2025

**Test File:** `tests/test_dynamic_graph_nonintegral_ids.cpp` (~850 lines)

**Results:** 21 test cases, 156 assertions - ALL PASSED ✅

**What Was Implemented:**
- String edge cases: empty strings, whitespace, Unicode (CJK, emoji, Greek), long strings (10K+)
- Double IDs: construction, ordering, special values (0.0, -0.0, infinity), CPO access
- Custom PersonId type: compound struct with `operator<=>`, `std::hash` specialization
- Integration tests: verify iteration works across all non-integral types
- **load_vertices/load_edges tests**: Explicit testing of `load_vertices` and `load_edges` with 
  non-integral VId types (string, PersonId, double)

**Bug Fix:** Fixed `load_vertices` in `dynamic_graph.hpp` to handle `VV=void` case correctly.
When vertex values are void, `copyable_vertex_t<VId, void>` has only 1 element (id), not 2.
The fix uses `if constexpr (is_void_v<VV>)` to handle both cases properly.

**Risk Assessment Results:**
- ✅ LOW RISK confirmed: String IDs work correctly with all edge cases
- ✅ LOW RISK confirmed: Double IDs work with special values (except NaN which is documented)
- ✅ LOW RISK confirmed: Custom types work with proper hash/comparison implementations
- ✅ LOW RISK confirmed: load_vertices and load_edges work with non-integral VId types

**Note:** Phase 5.5 (explicit CPO validation) was folded into the main test file since CPO 
functions are tested throughout the string, double, and PersonId test cases.

---

## Phase 6: Comprehensive Integration Tests

After all phases, create integration tests for complex scenarios.

**Test File:**
```
tests/test_dynamic_graph_integration.cpp  (~1200 lines)
```

### Phase 6: Integration Test Scenarios

**Test Coverage:**
- Mixed container types in same codebase
- Large-scale graphs (100k+ vertices, 1M+ edges)
- Interoperability with compressed_graph
- Conversion between graph types
- STL algorithm compatibility
- Range adaptor compatibility

**Specific Tests:**
1. **Graph Conversion** (50 tests)
   - dynamic_graph → compressed_graph
   - compressed_graph → dynamic_graph
   - Preserve vertex/edge values
   - Preserve partition information
   - Handle large graphs efficiently

2. **Cross-Container Operations** (40 tests)
   - Copy between different traits types
   - Transform graph structure
   - Merge multiple graphs
   - Extract subgraphs

3. **STL Algorithm Integration** (50 tests)
   - `std::ranges::for_each` on vertices/edges
   - `std::ranges::find_if` for search
   - `std::ranges::sort` on edge containers
   - `std::ranges::transform` on values
   - Pipeline operations with views
   - `std::ranges::count_if` for filtering
   - `std::ranges::accumulate` for aggregation

4. **Performance Validation** (40 tests)
   - Benchmark construction time
   - Benchmark iteration time
   - Memory usage measurement
   - Compare container choices
   - Identify performance regressions
   - Cache locality impact

5. **CPO Interoperability** (60 tests)
   - Use CPOs across different graph types
   - Generic functions using CPOs
   - Verify CPO consistency
   - Test CPO with all container combinations

**Note:** Graph algorithms (BFS, DFS, shortest path, etc.) will be implemented and tested separately using the CPO functions once the CPO layer is complete and stable. This ensures algorithms are generic and work across all graph types.

---

## Phase 7: Advanced Features and Edge Cases

Test advanced functionality and stress scenarios.

### Phase 7.1: Graph Mutation Operations

**Test File:** `test_dynamic_graph_mutation.cpp` (~1200 lines)

**Features to Test:**

1. **Edge Mutation** (100 tests)
   - `add_edge(g, uid, vid)` - Add single edge
   - `add_edge(g, uid, vid, value)` - Add edge with value
   - `remove_edge(g, uid, vid)` - Remove specific edge
   - `remove_first_edge(g, uid, vid)` - Remove first of parallel edges
   - `remove_all_edges(g, uid, vid)` - Remove all parallel edges
   - `clear_edges(g, uid)` - Remove all edges from vertex
   - `update_edge_value(g, uid, vid, value)` - Modify edge value

2. **Vertex Mutation** (80 tests)
   - `add_vertex(g)` - Add vertex with auto-ID
   - `add_vertex(g, uid)` - Add vertex with specific ID
   - `add_vertex(g, uid, value)` - Add vertex with value
   - `remove_vertex(g, uid)` - Remove vertex and incident edges
   - `update_vertex_value(g, uid, value)` - Modify vertex value

3. **Iterator Stability** (60 tests)
   - Vertex iterator invalidation rules
   - Edge iterator invalidation rules
   - Reference validity after mutations
   - Safe mutation during iteration

4. **Exception Safety** (40 tests)
   - Strong guarantee for add operations
   - Basic guarantee for bulk operations
   - Rollback on exception
   - Resource cleanup

5. **Concurrent Modification** (20 tests)
   - Multiple readers, single writer patterns
   - Document thread-safety guarantees
   - Race condition detection (if applicable)

### Phase 7.2: Stress and Performance Tests

**Test File:** `test_dynamic_graph_stress.cpp` (~800 lines)

**Stress Scenarios:**

1. **Scale Tests** (60 tests)
   - 1M vertices, sparse edges
   - 10k vertices, dense edges (near-complete)
   - 10M edges with 100k vertices
   - Memory limit testing
   - Construction time benchmarks

2. **Special Graph Structures** (50 tests)
   - Complete graphs K_n (all-to-all)
   - Star graphs (one hub, many spokes)
   - Chain graphs (linear path)
   - Binary trees (perfect balance)
   - Random graphs (Erdős–Rényi)
   - Scale-free graphs (power-law)
   - Small-world graphs (Watts-Strogatz)

3. **Extreme Degrees** (30 tests)
   - Vertex with 1M+ out-edges
   - Graph with all isolated vertices
   - Graph with maximum degree variance
   - Self-loop dominated graphs

4. **Parallel Edges** (25 tests)
   - 1000+ parallel edges between two vertices
   - Many vertex pairs with parallel edges
   - Edge count accuracy with duplicates
   - Performance with parallel edges

5. **Value Type Stress** (35 tests)
   - Large string values (1MB+ each)
   - Complex nested structures
   - Move-only types at scale
   - Custom allocators under load

### Phase 7.3: Edge Cases and Corner Cases

**Test File:** `test_dynamic_graph_edge_cases.cpp` (~1000 lines)

**Edge Case Categories:**

1. **Empty and Single-Element** (60 tests)
   - Empty graph (no vertices, no edges)
   - Single vertex, no edges (isolated)
   - Single vertex, self-loop only
   - Single edge graph
   - All possible void combinations

2. **Boundary Values** (70 tests)
   - VId = 0, VId = max
   - Negative VId (for signed types)
   - VId near overflow boundary
   - Size = 0, Size = max
   - Partition boundaries

3. **Self-Loops** (50 tests)
   - Single self-loop per vertex
   - Multiple self-loops per vertex
   - Graph with only self-loops
   - Degree calculation with self-loops
   - Iteration behavior with self-loops
   - Remove self-loops

4. **Duplicate Edges** (50 tests)
   - Duplicate detection in set containers
   - Duplicate preservation in list containers
   - Edge count with duplicates
   - Remove one vs. remove all duplicates
   - Find with multiple matches

5. **Bidirectional Simulation** (40 tests)
   - Add reverse edges manually
   - In-degree calculation (counting incoming)
   - Predecessor lookup
   - Bidirectional traversal
   - Symmetric edge verification

6. **Malformed Input** (60 tests)
   - Null ranges
   - Empty ranges
   - Invalid projection functions
   - Inconsistent vertex IDs
   - Target ID exceeds vertex count
   - Negative IDs (when invalid)

7. **Numeric Edge Cases** (50 tests)
   - Integer overflow detection
   - Wraparound behavior
   - Signed/unsigned mismatch
   - Size_t limits
   - Vertex count at capacity

8. **Value Semantics Edge Cases** (40 tests)
   - Non-copyable values (move-only)
   - Non-movable values (copy-only)
   - Throwing copy constructors
   - Throwing move constructors
   - Throwing destructors
   - Deleted assignment operators

9. **Container-Specific Edge Cases** (40 tests)
   - Map with non-default comparator
   - Unordered_map with poor hash function
   - Set ordering preservation
   - Forward_list limitations (no size, no backward)
   - Deque iterator invalidation

10. **Allocator Edge Cases** (40 tests)
    - Stateful allocators
    - Allocator propagation on copy/move
    - Allocator mismatch detection
    - Custom allocators with different allocation strategies
    - Memory pool exhaustion

---

## Additional Features for Future Consideration

### Feature Set 1: Bidirectional Graph Support

**Implementation:**
```cpp
template <class EV, class VV, class GV, class VId, class Traits>
struct bidirectional_graph_traits : public Traits {
  using in_edges_type = typename Traits::edges_type;
};

// Add to dynamic_vertex:
in_edges_type in_edges_;  // Store incoming edges
```

**New CPOs:**
- `in_edges(g, u)` - Get incoming edges
- `in_degree(g, u)` - Count incoming edges
- `predecessors(g, u)` - Get predecessor vertices

**Test File:** `test_dynamic_graph_bidirectional.cpp` (~800 lines)

### Feature Set 2: Graph Views and Adaptors

**View Types:**
- `vertex_filter_view` - Filter vertices by predicate
- `edge_filter_view` - Filter edges by predicate
- `reverse_graph_view` - Swap source/target semantics
- `subgraph_view` - View subset of vertices

**Test File:** `test_dynamic_graph_views.cpp` (~600 lines)

### Feature Set 3: Property Maps

**External Property Storage:**
```cpp
template <typename G, typename T>
class vertex_property_map {
  std::vector<T> properties_;
  // or std::map<vertex_id_type, T> for sparse
};
```

**Use Cases:**
- Vertex colors for graph coloring
- Distances for shortest path
- Discovery/finish times for DFS
- Avoid modifying graph structure for temporary data

**Test File:** `test_dynamic_graph_property_maps.cpp` (~500 lines)

### Feature Set 4: Serialization

**Formats to Support:**
- Edge list (CSV, TSV)
- Adjacency list
- GraphML (XML-based)
- JSON
- Binary (custom format)

**Operations:**
- `save_to_edge_list(g, ostream)`
- `load_from_edge_list(istream)`
- `save_to_json(g, ostream)`
- `load_from_json(istream)`

**Test File:** `test_dynamic_graph_serialization.cpp` (~700 lines)

### Feature Set 5: Graph Generators

**Generator Functions:**
- `make_complete_graph(n)` - Complete graph K_n
- `make_cycle_graph(n)` - Cycle graph C_n
- `make_path_graph(n)` - Path graph P_n
- `make_star_graph(n)` - Star graph S_n
- `make_grid_graph(rows, cols)` - 2D grid
- `make_random_graph(n, p)` - Erdős–Rényi G(n,p)
- `make_barabasi_albert_graph(n, m)` - Scale-free
- `make_watts_strogatz_graph(n, k, p)` - Small-world

**Test File:** `test_dynamic_graph_generators.cpp` (~600 lines)

### Feature Set 6: Graph Metrics

**Statistical Functions:**
- `density(g)` - Edge density (E / V*(V-1))
- `avg_degree(g)` - Mean degree
- `degree_distribution(g)` - Histogram
- `clustering_coefficient(g, u)` - Local clustering
- `diameter(g)` - Maximum shortest path
- `radius(g)` - Minimum eccentricity
- `center_vertices(g)` - Vertices with minimum eccentricity

**Test File:** `test_dynamic_graph_metrics.cpp` (~500 lines)

---

## Enhanced Test Scenarios by Phase

### Phase 1 Enhancements:

Add to each existing test file:

**11. Value Type Advanced Tests** (40 tests per file)
   - Move-only types (unique_ptr, movable_struct)
   - Non-movable types (copy-only)
   - Throwing constructors/destructors
   - Large value types (>1KB)
   - Polymorphic types with virtual functions
   - Aggregate types
   - Types with const members

**12. Numeric Limits Tests** (30 tests per file)
   - VId at type limits (min, max)
   - VId overflow prevention
   - Size calculations near limits
   - Signed vs unsigned VId behavior
   - Negative VId handling (if applicable)

**13. Memory and Resource Tests** (25 tests per file)
   - Custom allocators with tracking
   - Allocator propagation semantics
   - Memory leak detection
   - RAII verification
   - Exception-induced leaks

**14. Self-Loop Tests** (20 tests per file)
   - Add self-loops during construction
   - Iterate self-loops
   - Count self-loops in degree
   - Remove self-loops
   - Multiple self-loops per vertex

**15. Parallel Edge Tests** (20 tests per file)
   - Multiple edges same endpoints
   - Count parallel edges
   - Iterate all parallel edges
   - Remove specific parallel edge
   - Edge container behavior (set vs list)

### Phase 2 CPO Enhancements:

Add to `test_dynamic_graph_cpo.cpp`:

**Additional CPOs to Implement:**

17. `contains_vertex(g, uid)` - Check vertex existence (40 tests)
18. `contains_edge(g, uid, vid)` - Check edge existence (40 tests)
19. `find_vertex_edge(g, uid, vid)` - Find specific edge (40 tests)
20. `in_edges(g, u)` - Incoming edges (if bidirectional) (30 tests)
21. `in_degree(g, u)` - Incoming degree (if bidirectional) (30 tests)
22. `is_adjacent(g, u, v)` - Check adjacency (30 tests)
23. `is_isolated(g, u)` - Check if vertex has no edges (25 tests)
24. `isolated_vertices(g)` - Range of isolated vertices (25 tests)

**CPO Testing Enhancements:**
- Customization point testing (ADL, member, default)
- Noexcept specifications
- Forwarding references
- Perfect forwarding
- SFINAE behavior
- Concept constraints

### Phase 3-5 Enhancements:

**Complex Key Types to Add:**

For Phase 3 (map vertices):
- `std::variant<int, string>` - Variant keys
- `std::optional<int>` - Optional keys
- Pointer types (identity comparison)
- Custom types with complex comparison logic
- Types with expensive comparison operators

For Phase 5 (non-integral):
- `double` with epsilon comparison
- `std::complex<double>` - Complex numbers
- Multi-field compound types
- Hierarchical key types

**Hash Function Tests:**
- Custom hash with collisions
- Perfect hash functions
- Poor hash functions (performance)
- Hash consistency across copies

---

## Coverage Goals (Revised)

**Target Coverage by Phase:**
- Phase 1: 75% of dynamic_graph.hpp
- Phase 2: 88% of dynamic_graph.hpp + CPO functions  
- Phase 3: 92% overall
- Phase 4: 94% overall
- Phase 5: 96% overall
- Phase 6: 97% with algorithm validation
- Phase 7: 98%+ with stress and edge cases

**Coverage Measurement Tools:**
- Use `gcov` or `llvm-cov` for line coverage
- Track branch coverage for all conditionals
- Monitor template instantiation coverage
- Focus on exception handling paths
- Measure cyclomatic complexity coverage

---

## Implementation Timeline

### Completed Phases:
1. ✅ **Phase 1.1-1.8** - All sequential containers (vofl, vol, vov, vod, dofl, dol, dov, dod) - COMPLETE
2. ✅ **Phase 2** - CPO implementation and tests for all sequential containers - COMPLETE  
3. ✅ **Phase 3.1a-h** - All associative containers (mofl, mol, mov, mod, uofl, uol, uov, uod) - COMPLETE
4. ✅ **Phase 4.1.1** - Edge comparison operators and hash support - COMPLETE
5. ✅ **Phase 4.1.2** - vos (vector + set) basic + CPO tests - COMPLETE
6. ✅ **Phase 4.1.3** - dos (deque + set) basic + CPO tests - COMPLETE
7. ⏳ **Phase 4.1.4** - mos (map + set) basic tests COMPLETE, CPO tests IN PROGRESS

### Next Steps (Priority Order):
1. **Finish Phase 4.1.4** - Complete mos CPO tests (1-2 days)
2. **Phase 4.1.5** - uos (unordered_map + set) basic + CPO tests (2-3 days)
3. **Phase 4.2** - Unordered set edge containers (vous, dous, mous, uous) (4-5 days)
4. **Phase 4.3** - Map-based edge containers (voem, moem) (3-4 days)
5. **Phase 7.3** - Edge cases (ongoing, 2-3 days)
6. **Phase 6** - Integration tests (2-3 days)
7. **Phase 7.1** - Mutation operations (2-3 days)
8. **Phase 7.2** - Stress tests (2-3 days)

**Estimated Remaining Time:** 3-4 weeks for comprehensive coverage of all planned phases

**Note:** Graph algorithms will be implemented in a future phase using CPO functions, ensuring generic implementations that work across all graph container types (dynamic_graph, compressed_graph, etc.).

### Safety Considerations (Enhanced):
- After each phase, run **all** existing tests to ensure no regressions
- Use **feature branches** for each phase with PR review
- Commit after each test file passes completely
- Maintain **backward compatibility** throughout all phases
- Document **any breaking changes** with migration guide
- Use **continuous integration** to catch issues early
- Perform **static analysis** (clang-tidy, cppcheck) regularly
- Run **sanitizers** (ASan, UBSan, TSan, MSan) on all tests
- Conduct **code reviews** focusing on edge cases
- Maintain **changelog** for each phase completion

---

## Summary of Test Organization

```
tests/
  # Phase 1: Sequential containers (Optimized with TEMPLATE_TEST_CASE)
  test_dynamic_graph_vofl.cpp           # ~2673 lines ✅ COMPLETE
  test_dynamic_graph_vol.cpp            # ~2677 lines ✅ COMPLETE
  test_dynamic_graph_vov.cpp            # ~2677 lines ✅ COMPLETE
  test_dynamic_graph_common.cpp         # ~1800 lines (6 sequential traits, uint64_t IDs)
  test_dynamic_graph_vector_traits.cpp  # ~600 lines (vector-specific)
  test_dynamic_graph_deque_traits.cpp   # ~800 lines (deque-specific + CPO)
  test_dynamic_graph_edge_containers.cpp # ~400 lines (edge container deltas)
  
  # Phase 2: CPO support
  test_dynamic_graph_cpo.cpp       # ~2000 lines
  
  # Phase 3: Associative vertex containers (Separate TEMPLATE_TEST_CASE)
  test_dynamic_graph_associative_common.cpp  # ~1600 lines (3-6 assoc traits, string IDs)
  test_dynamic_graph_map_traits.cpp          # ~700 lines (map-specific: ordering, comparators)
  test_dynamic_graph_unordered_map_traits.cpp # ~700 lines (unordered_map-specific: hashing)
  
  # Phase 4: Set/map edge containers
  test_dynamic_graph_set.cpp       # ~1200 lines
  test_dynamic_graph_unordered_set.cpp  # ~1200 lines
  test_dynamic_graph_edge_map.cpp  # ~1200 lines
  
  # Phase 5: Non-integral IDs
  test_dynamic_graph_non_integral_id.cpp  # ~1000 lines
  
  # Phase 6: Integration
  test_dynamic_graph_integration.cpp  # ~1200 lines
  
  # Phase 7: Advanced features
  test_dynamic_graph_mutation.cpp      # ~1200 lines
  test_dynamic_graph_stress.cpp        # ~800 lines
  test_dynamic_graph_edge_cases.cpp    # ~1000 lines
```

**Total Estimated Lines (Optimized):** ~68,000 lines of test code
**Total Estimated Tests:** ~4,500-5,000 test cases
**Current Test Count:** 2,400 test cases, 25,926 assertions ✅
**Sequential Container Coverage:** 100% complete (all 8 combinations tested)
**Phase 3.1 mofl Coverage:** 100% complete (map + forward_list tested)
**Phase 3.1 mol Basic Coverage:** 100% complete (map + list basic tests)
**Test Executions:** ~14,000+ (TEMPLATE_TEST_CASE multiplying tests across 6 sequential + 3-6 associative traits)
**Code Reduction:** Opportunity exists for future optimization with TEMPLATE_TEST_CASE

**Optimization Strategy:**
- **Two separate "common" files**: One for sequential containers (uint64_t IDs), one for associative (string IDs)
- Use Catch2 TEMPLATE_TEST_CASE to run identical tests across container types within each category
- Separate container-specific behavior tests (iterator invalidation, memory layout, CPO differences)
- Cannot unify sequential and associative due to fundamental differences in vertex ID types and semantics
- Maintain full coverage with significantly less code duplication
- Clear separation between common behavior and container-specific quirks

**Why Two Common Files Are Required:**
- **Sequential (vector/deque)**: Auto-extends to accommodate any integral vertex ID, random/bidirectional access
- **Associative (map/unordered_map)**: Only creates explicitly referenced vertices, bidirectional access only, key-based lookup
- Different vertex ID types: `uint64_t` vs `std::string` (cannot template over both)
- Different CPO implementations: `vertex_id()` returns index vs key
- Different test assertions: `g[5]` vs `g["alice"]`

**Note:** Graph algorithms (BFS, DFS, shortest path, MST, flow algorithms, etc.) will be implemented and tested in a separate phase after the CPO layer is complete and stable, ensuring they work generically across all graph types.

---

## Implementation Timeline

### Recommended Order:
1. **Phase 1.1** - Core vofl tests (most common use case)
2. **Phase 1.2** - Core vol tests (bidirectional support)
3. **Phase 1.3** - Core vov tests (performance-oriented)
4. **Phase 2** - CPO implementation (enables uniformity)
5. **Phase 1.4** - Deque vertex tests (completes Phase 1)
6. **Phase 3** - Map vertex containers (major feature)
7. **Phase 4** - Set/map edge containers (major feature)
8. **Phase 5** - Non-integral IDs (advanced feature)
9. **Phase 6** - Integration tests (validation)

### Safety Considerations:
- After each phase, run all existing tests to ensure no regressions
- Use feature branches for each phase
- Commit after each test file passes
- Maintain backward compatibility throughout
- Document any breaking changes

---

## Coverage Goals

**Target Coverage by Phase:**
- Phase 1: 70% of dynamic_graph.hpp
- Phase 2: 85% of dynamic_graph.hpp + CPO functions
- Phase 3: 90% overall
- Phase 4: 93% overall
- Phase 5: 95% overall
- Phase 6: 95%+ with edge case validation

**Coverage Measurement:**
- Use `gcov` or `llvm-cov` for line coverage
- Track branch coverage for conditionals
- Monitor template instantiation coverage
- Focus on untested error paths in later phases

---

## Notes

1. **Container Selection Strategy:**
   - Tests prioritize most common combinations first
   - Coverage matrix targets 95%+ with ~30% of total combinations
   - Combinations chosen based on: common usage, unique edge cases, performance characteristics

2. **Value Type Testing:**
   - Each test file covers void/int/string combinations
   - String tests stress move semantics and non-trivial destruction
   - Void tests verify zero-overhead abstraction
   - Add move-only and copy-only types for advanced scenarios

3. **Test Generation:**
   - ✅ **Use Catch2 TEMPLATE_TEST_CASE** for cross-container unified tests (Phase 1.4+)
   - Use Catch2 GENERATE for cartesian products of parameters
   - TEMPLATE_TEST_CASE_SIG for complex type parameter combinations
   - Separate container-specific tests from common behavior tests
   - Macros for repetitive assertion patterns (minimize use)
   - Property-based testing for algorithm validation
   
   **TEMPLATE_TEST_CASE Example:**
   ```cpp
   TEMPLATE_TEST_CASE("construction works", "[construction]",
       vofl_graph_traits, vol_graph_traits, vov_graph_traits,
       dofl_graph_traits, dol_graph_traits, dov_graph_traits) {
       using Graph = dynamic_graph<int, int, void, uint64_t, false, TestType>;
       Graph g;
       REQUIRE(g.size() == 0);
       // Test body runs 6 times (once per traits type)
   }
   ```

4. **Performance Benchmarks:**
   - Include micro-benchmarks in Phase 6 and 7.2
   - Compare container choices for typical graph operations
   - Document performance characteristics in comments
   - Use Google Benchmark or similar for consistent measurements
   - Track performance regressions across versions

5. **Future Extensions:**
   - **Phase 8**: Graph algorithms using CPOs (BFS, DFS, shortest path, MST, connectivity, topological sort, flow algorithms)
   - **Phase 9**: Bidirectional graph support (in-edges)
   - **Phase 9**: Multi-graph explicit support (parallel edges policy)
   - **Phase 10**: Property maps as separate entities
   - **Phase 10**: Graph views and adaptors
   - **Phase 11**: Serialization (GraphML, JSON, binary)
   - **Phase 11**: Graph generators (complete, random, scale-free)
   - **Phase 12**: Graph metrics (density, clustering, diameter)
   - **Future research**: Thread-safe variants with mutex protection

6. **Edge Case Philosophy:**
   - Test boundary conditions explicitly
   - Document undefined behavior clearly
   - Prefer compile-time errors over runtime errors
   - Use concepts/static_assert for type safety
   - Validate preconditions with assertions in debug builds

7. **Mutation Operations Design:**
   - Document iterator invalidation rules clearly
   - Strong exception guarantee for single-element operations
   - Basic exception guarantee for bulk operations
   - Consider copy-on-write for certain scenarios
   - Provide both checked and unchecked variants

8. **Memory Testing:**
   - Use custom allocators to track allocations
   - Verify no leaks with sanitizers
   - Test behavior under allocation failure
   - Measure memory overhead vs. theoretical minimum
   - Validate empty base optimization for void types

10. **Compatibility Testing:**
    - Ensure C++20 concepts are satisfied
    - Test with different standard library implementations
    - Verify ADL behavior is correct
    - Check for ODR violations
    - Ensure header-only compatibility if applicable

11. **Documentation Requirements:**
    - Document time complexity for all operations
    - Document space complexity and memory layout
    - Provide usage examples for each feature
    - Document design decisions and trade-offs
    - Include migration guides for API changes

12. **Continuous Improvement:**
    - Review coverage reports after each phase
    - Add tests for discovered bugs immediately
    - Refactor tests to reduce duplication
    - Update test plan based on implementation discoveries
    - Maintain test execution time under reasonable limits (~5 min total)

---

## Appendix A: Example Custom Types for Testing

### USAAddress (for map keys)
```cpp
struct USAAddress {
    std::string street;
    std::string city;
    std::string state;
    std::string zip;
    
    auto operator<=>(const USAAddress&) const = default;
    
    friend std::ostream& operator<<(std::ostream& os, const USAAddress& addr) {
        return os << addr.street << ", " << addr.city << ", " 
                  << addr.state << " " << addr.zip;
    }
};

namespace std {
    template<>
    struct hash<USAAddress> {
        size_t operator()(const USAAddress& addr) const {
            size_t h1 = hash<string>{}(addr.street);
            size_t h2 = hash<string>{}(addr.city);
            size_t h3 = hash<string>{}(addr.state);
            size_t h4 = hash<string>{}(addr.zip);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };
}
```

### MoveOnlyValue (for testing move semantics)
```cpp
struct MoveOnlyValue {
    std::unique_ptr<int> data;
    
    MoveOnlyValue(int val = 0) : data(std::make_unique<int>(val)) {}
    MoveOnlyValue(const MoveOnlyValue&) = delete;
    MoveOnlyValue(MoveOnlyValue&&) = default;
    MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;
    MoveOnlyValue& operator=(MoveOnlyValue&&) = default;
    
    int value() const { return *data; }
};
```

### ThrowingValue (for exception safety)
```cpp
struct ThrowingValue {
    int data;
    static int throw_countdown;
    
    ThrowingValue(int val = 0) : data(val) {
        if (throw_countdown > 0 && --throw_countdown == 0)
            throw std::runtime_error("Throwing constructor");
    }
    
    ThrowingValue(const ThrowingValue& other) : data(other.data) {
        if (throw_countdown > 0 && --throw_countdown == 0)
            throw std::runtime_error("Throwing copy constructor");
    }
    
    static void reset(int countdown = 0) { throw_countdown = countdown; }
};
```

### TrackingAllocator (for memory testing)
```cpp
template<typename T>
class TrackingAllocator {
    static inline size_t allocation_count = 0;
    static inline size_t deallocation_count = 0;
    static inline size_t bytes_allocated = 0;
    
public:
    using value_type = T;
    
    T* allocate(size_t n) {
        ++allocation_count;
        bytes_allocated += n * sizeof(T);
        return std::allocator<T>{}.allocate(n);
    }
    
    void deallocate(T* p, size_t n) {
        ++deallocation_count;
        std::allocator<T>{}.deallocate(p, n);
    }
    
    static void reset() {
        allocation_count = 0;
        deallocation_count = 0;
        bytes_allocated = 0;
    }
    
    static auto stats() {
        return std::tuple{allocation_count, deallocation_count, bytes_allocated};
    }
};
```

---

## Appendix B: Test Execution Guidelines

### Running Tests

```bash
# Run all tests
./graph3_tests

# Run specific phase
./graph3_tests "[vofl]"
./graph3_tests "[cpo]"

# Run with coverage
cmake --build build --target graph3_tests_coverage
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# Run with sanitizers
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined" ..
./graph3_tests

# Run performance benchmarks
./graph3_tests "[.benchmark]" --benchmark-samples 100
```

### Test Organization Best Practices

1. **Use SECTION for related tests**
   ```cpp
   TEST_CASE("load_edges behavior", "[load][edges]") {
       SECTION("empty range") { /* ... */ }
       SECTION("single edge") { /* ... */ }
       SECTION("multiple edges") { /* ... */ }
   }
   ```

2. **Use TEST_CASE_TEMPLATE for type parameterization**
   ```cpp
   TEMPLATE_TEST_CASE("works with various value types", "[values]",
                      int, std::string, double) {
       using Graph = dynamic_graph<TestType, void, void>;
       // ...
   }
   ```

3. **Use generators for combinations**
   ```cpp
   auto value = GENERATE(0, 42, -1, INT_MAX);
   auto sourced = GENERATE(true, false);
   ```

4. **Tag tests appropriately**
   - `[.]` - Hidden, must be explicitly selected
   - `[.benchmark]` - Performance tests
   - `[!throws]` - Expected to throw
   - `[!mayfail]` - May fail in some environments

---

## Appendix C: Implementation Checklist

### Before Starting Each Phase:
- [ ] Review previous phase coverage report
- [ ] Update test plan based on findings
- [ ] Create feature branch
- [ ] Set up CI pipeline for new tests

### During Implementation:
- [ ] Write tests before implementation (TDD)
- [ ] Run tests frequently during development
- [ ] Use compiler warnings (-Wall -Wextra -Wpedantic)
- [ ] Run static analyzers regularly
- [ ] Document complex test scenarios

### After Completing Each Phase:
- [ ] Achieve target coverage percentage
- [ ] Run all tests with sanitizers
- [ ] Generate and review coverage report
- [ ] Update documentation
- [ ] Code review
- [ ] Merge to main branch
- [ ] Tag release/milestone

### Quality Gates:
- [ ] Zero compiler warnings
- [ ] Zero sanitizer errors
- [ ] Zero static analyzer errors
- [ ] Coverage target met
- [ ] All tests passing
- [ ] Performance benchmarks within acceptable range
- [ ] Documentation updated
- [ ] Code review approved

---
