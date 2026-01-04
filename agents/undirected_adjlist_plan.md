# Undirected Adjacency List Implementation Plan

**Status:** Phase 0 - Planning Complete  
**Last Updated:** January 4, 2026  
**Estimated Total Time:** 2.5-3 weeks  

---

## Phase Summary

| Phase | Status | Priority | Effort | Description |
|-------|--------|----------|--------|-------------|
| 0 | ✅ COMPLETE | - | 1 day | Planning and review |
| 1 | ✅ COMPLETE | CRITICAL | 2 days | Bug fixes and quick wins |
| 2 | ⏳ PENDING | CRITICAL | 2-3 days | Interface conformance verification |
| 3 | ⏳ PENDING | CRITICAL | 2-3 days | API standardization and modernization |
| 4 | ⏳ PENDING | BLOCKING | 4-5 days | Comprehensive test suite |
| 5 | ⏳ PENDING | HIGH | 2 days | Documentation updates |
| 6 | ⏳ PENDING | MEDIUM | 1-2 days | Code cleanup and polish |
| 7 | ⏳ PENDING | OPTIONAL | 2-3 days | Performance optimizations |

**Legend:**
- ✅ COMPLETE - Phase finished and verified
- 🚧 IN PROGRESS - Currently being worked on
- ⏳ PENDING - Not yet started
- ⚠️ BLOCKED - Waiting on dependencies

---

## Detailed Phase Plans

### Phase 1: Bug Fixes and Quick Wins (CRITICAL - 2 days)

**Goal:** Fix identified bugs that are safe, isolated, and don't require major refactoring.

**Dependencies:** None

**Tasks:**
1. **Fix `size_` decrement in `unlink()` method** (30 min)
   - File: `include/graph/container/detail/undirected_adjacency_list_impl.hpp`
   - Location: `ual_vertex_edge_list::unlink()` template function
   - Action: Add `--size_;` after unlink operations
   - Verification: Add assertion `assert(size_ >= 0)`

2. **Fix `contains_vertex()` return type** (15 min)
   - File: `include/graph/container/detail/undirected_adjacency_list_api.hpp`
   - Current: `void contains_vertex(...)`
   - Change to: `bool contains_vertex(...)`
   - Add: `return ukey >= 0 && ukey < g.vertices().size();`

3. **Add `[[nodiscard]]` attributes** (1 hour)
   - Add to all query functions that return values
   - Files: Main header and API files
   - Prevents accidental discard of important return values

4. **Clean up commented-out code** (30 min)
   - File: `include/graph/container/detail/undirected_adjacency_list_api.hpp`
   - Remove lines ~895+ (dead code section)

**Deliverables:**
- Bug-fixed implementation
- Code cleanup complete
- No breaking changes to API

**Success Criteria:**
- All bugs fixed
- Code compiles without warnings
- No behavioral changes (beyond fixes)

---

### Phase 2: Interface Conformance Verification (CRITICAL - 2-3 days)

**Goal:** Verify that all CPO implementations conform to Graph Container Interface requirements.

**Dependencies:** Phase 1 complete

**Tasks:**
1. **Document interface requirements** (4 hours)
   - Review `docs/container_interface.md`
   - Create checklist of required operations
   - Note expected signatures and semantics
   - Document undirected-specific requirements

2. **Create conformance matrix** (4 hours)
   - Map each interface requirement to implementation
   - Identify missing operations
   - Flag signature mismatches
   - Note semantic differences

3. **Fix signature mismatches** (1 day)
   - Update function signatures to match interface exactly
   - Fix return type inconsistencies
   - Correct parameter types/order
   - Update type traits as needed

4. **Implement missing operations** (4 hours)
   - Add any required operations not yet implemented
   - Examples: `num_vertices()`, `num_edges()`, `degree()` if required

5. **Create concept verification tests** (4 hours)
   - File: `tests/test_undirected_adjlist_concepts.cpp`
   - Use `static_assert` for concept checks
   - Verify with generic algorithm usage
   - Test iterator concept compliance

**Deliverables:**
- Interface conformance matrix document
- All interface requirements satisfied
- Concept verification test file

**Success Criteria:**
- 100% interface conformance
- All concept checks pass
- Works with generic algorithms

---

### Phase 3: API Standardization and Modernization (CRITICAL - 2-3 days)

**Goal:** Update legacy API to match current library standards and terminology.

**Dependencies:** Phase 2 complete

**Tasks:**
1. **Update constructor signatures** (1 day)
   - Current: Uses `ekey_fnc`, `evalue_fnc` separately
   - Target: Unified `eproj` (edge projection) pattern
   - Update parameter order to match `dynamic_graph`
   - Support optional vertex range + projection
   - Files: Main header and implementation

2. **Rename legacy terminology** (4 hours)
   - Replace "function" → "projection" in names
   - Update template parameter names
   - Update documentation comments
   - Ensure consistency across all files

3. **Standardize type trait names** (2 hours)
   - Review against `dynamic_graph` patterns
   - Update any non-standard trait names
   - Verify `graph_traits` specialization

4. **Update API documentation** (4 hours)
   - Reflect new constructor signatures
   - Document projection patterns
   - Add migration guide from legacy API
   - Update examples in comments

**Deliverables:**
- Modernized constructor API
- Consistent terminology throughout
- Migration guide for users

**Success Criteria:**
- API matches library standards
- Constructors follow common pattern
- All terminology updated

---

### Phase 4: Comprehensive Test Suite (BLOCKING - 4-5 days)

**Goal:** Create thorough test coverage for all functionality.

**Dependencies:** Phases 1-3 complete

**Tasks:**
1. **Basic operations tests** (1 day)
   - File: `tests/test_undirected_adjlist_basic.cpp`
   - Construction (all overloads)
   - Vertex operations (create, access, iterate)
   - Edge operations (create, access, iterate, remove)
   - Empty graph behavior
   - Single vertex/edge cases

2. **Iterator tests** (1 day)
   - File: `tests/test_undirected_adjlist_iterators.cpp`
   - Vertex iteration (forward, const)
   - Edge iteration (forward, const)
   - Vertex-edge iteration (bidirectional, const)
   - Vertex-vertex iteration
   - Iterator equality/inequality
   - Edge cases (empty, single element)

3. **Edge cases and stress tests** (1 day)
   - File: `tests/test_undirected_adjlist_edge_cases.cpp`
   - Self-loops behavior
   - Parallel edges
   - High-degree vertices
   - Maximum vertex keys
   - Edge deletion during iteration
   - Large graphs (stress test)

4. **Memory management tests** (4 hours)
   - File: `tests/test_undirected_adjlist_memory.cpp`
   - Custom allocator usage
   - Move semantics
   - Copy semantics (if implemented)
   - Clear and destructor behavior
   - Exception safety

5. **CPO tests** (1 day)
   - File: `tests/test_undirected_adjlist_cpo.cpp`
   - All graph-level CPOs
   - All vertex CPOs
   - All edge CPOs
   - Integration with generic algorithms
   - Verify undirected semantics

6. **Interface conformance tests** (from Phase 2)
   - Concept satisfaction checks
   - Generic algorithm compatibility
   - Return type verification

**Deliverables:**
- 6 test files with comprehensive coverage
- All tests passing
- Coverage report

**Success Criteria:**
- 90%+ code coverage
- All edge cases tested
- No memory leaks
- All CPOs verified

---

### Phase 5: Documentation Updates (HIGH - 2 days)

**Goal:** Complete and modernize all documentation.

**Dependencies:** Phases 1-4 complete

**Tasks:**
1. **Update class-level documentation** (4 hours)
   - Add comprehensive overview of dual-list design
   - Document design rationale and tradeoffs
   - Add memory overhead analysis
   - Include usage recommendations
   - Add example code

2. **Document interface conformance** (2 hours)
   - Reference conformance matrix from Phase 2
   - Document any special undirected semantics
   - Note edge iteration behavior (double visitation)

3. **Update function documentation** (4 hours)
   - Document iterator invalidation rules
   - Add pre/postconditions
   - Document complexity guarantees
   - Add usage examples for complex functions

4. **Create migration guide** (3 hours)
   - Document legacy → modern API changes
   - Provide code examples
   - List breaking changes
   - Add conversion table

5. **Add performance documentation** (2 hours)
   - Document algorithmic complexity
   - Add comparison table vs alternatives
   - Document memory overhead
   - When to use this vs other containers

6. **Document thread safety** (1 hour)
   - Explicitly state not thread-safe
   - Document safe usage patterns
   - Note concurrent read guarantees

**Deliverables:**
- Complete class/function documentation
- Migration guide
- Performance comparison documentation

**Success Criteria:**
- All public APIs documented
- Examples provided for complex usage
- Migration path clear

---

### Phase 6: Code Cleanup and Polish (MEDIUM - 1-2 days)

**Goal:** Improve code quality and maintainability.

**Dependencies:** Phases 1-4 complete

**Tasks:**
1. **Fix `const_cast` issues** (4 hours)
   - File: Iterator constructors
   - Refactor to avoid const_cast where possible
   - Document why needed if unavoidable
   - Ensure const-correctness

2. **Reduce code duplication** (4 hours)
   - Iterator const/non-const pairs
   - Link/unlink patterns
   - Consider CRTP or helper templates

3. **Add C++20 iterator concepts** (4 hours)
   - Satisfy `std::bidirectional_iterator`
   - Add concept requirements
   - Verify with concept checks

4. **Add `operator<=>` for comparisons** (2 hours)
   - C++20 three-way comparison
   - For iterators where applicable

5. **Code style consistency** (2 hours)
   - Follow project formatting guidelines
   - Consistent naming patterns
   - Remove any style violations

**Deliverables:**
- Cleaner, more maintainable code
- C++20 iterator concepts satisfied
- Reduced duplication

**Success Criteria:**
- No const_cast warnings
- Iterator concepts pass
- Code style consistent

---

### Phase 7: Performance Optimizations (OPTIONAL - 2-3 days)

**Goal:** Improve runtime performance and memory efficiency.

**Dependencies:** Phases 1-6 complete

**Tasks:**
1. **Profile current implementation** (4 hours)
   - Identify hotspots
   - Measure memory usage
   - Benchmark against alternatives
   - Document baseline performance

2. **Edge pool allocator** (1-2 days)
   - Implement memory pool for edges
   - Pre-allocate chunks
   - Reduce allocation overhead
   - Maintain allocator-aware design

3. **Small buffer optimization** (1 day)
   - For low-degree vertices
   - Store first N edges inline
   - Reduces allocations for common case

4. **Optimize iterator advancement** (4 hours)
   - Cache list type in iterator?
   - Reduce branching
   - Profile-guided optimization

5. **Add `unique_edges()` view** (4 hours)
   - Return each edge exactly once
   - Implement as view/adaptor
   - Useful for algorithms

**Deliverables:**
- Performance improvements documented
- Optional optimizations implemented
- Benchmark results

**Success Criteria:**
- 20%+ performance improvement (if optimizations applied)
- No correctness regressions
- Memory usage documented

---

## Implementation Guidelines

### Agent-Friendly Practices

1. **One phase at a time**: Complete and verify each phase before moving to next
2. **Small, atomic commits**: Commit after each task completion
3. **Test after each change**: Run relevant tests immediately
4. **Document as you go**: Update comments with changes
5. **Ask for clarification**: If requirements unclear, ask before implementing

### Testing Strategy

- Run full test suite after each phase
- Use `ctest` or direct test execution
- Verify no regressions in other graph containers
- Check compilation with `-Wall -Wextra -Werror`

### Rollback Strategy

- Each phase creates a commit
- Can rollback to previous phase if issues arise
- Keep phases small for easy debugging

### Review Points

- After Phase 1: Quick review of bug fixes
- After Phase 2: Interface conformance review (CRITICAL)
- After Phase 3: API consistency review
- After Phase 4: Test coverage review
- Final: Complete code review before merge

---

## Risk Mitigation

### High-Risk Areas

1. **Constructor changes (Phase 3)**: May break existing code
   - Mitigation: Provide deprecation warnings, keep old API temporarily
   
2. **Interface conformance (Phase 2)**: May require significant changes
   - Mitigation: Verify early, get stakeholder buy-in

3. **Memory management changes (Phase 7)**: Could introduce bugs
   - Mitigation: Extensive testing, optional feature

### Testing Requirements

- Minimum 90% code coverage for Phases 1-4
- All interface conformance tests must pass
- No memory leaks (valgrind clean)
- No undefined behavior (sanitizers clean)

---

## Dependencies and Prerequisites

### Required Knowledge

- C++20 features (concepts, ranges, etc.)
- Graph theory (undirected graphs, adjacency lists)
- Template metaprogramming
- Memory management and allocators

### Required Tools

- C++20 compliant compiler (GCC 10+, Clang 12+)
- CMake 3.20+
- Catch2 testing framework
- Git for version control

### Reference Materials

- `docs/container_interface.md` - Interface specification
- `agents/unordered_adjlist.review.md` - Detailed review
- `include/graph/container/dynamic_graph.hpp` - Reference implementation
- Boost.Graph documentation - Concept reference

---

## Progress Tracking

Update the Phase Summary table at the top as phases complete:
- ⏳ → 🚧 when starting a phase
- 🚧 → ✅ when phase complete and verified
- Add ⚠️ if blocked with reason

**Current Phase:** Phase 0 (Planning) - COMPLETE ✅

**Next Action:** Begin Phase 1 (Bug Fixes and Quick Wins)

**Estimated Completion Date:** ~3 weeks from start (assuming full-time work)

---

## Notes

- Phase 7 is optional and can be deferred
- Phases 1-4 are critical path to production
- Phase 5 should not be skipped but can be done incrementally
- Phase 6 improves quality but not strictly required for MVP

