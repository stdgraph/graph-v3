# Prepare for algorithm development

All the required elements are in place so graph algorithms can be created.

We need a good environment where we can develop, test and evaluate new algorithms,
or to compare new implementations with existing implementations.

## Requirements Overview

The following items are needed to establish a robust algorithm development environment:

### 1. Graph Test Data Suite

A suite of useful graph examples with data that is useful for testing in multiple use-cases with the following criteria:

- **Graph categories to include:**
  - Trivial cases: empty graph, single vertex, single edge
  - Common patterns: path graphs, cycle graphs, complete graphs (K_n), star graphs
  - Tree structures: binary trees, balanced trees, chains
  - DAGs: simple DAGs, topological sort test cases
  - Weighted graphs: positive weights, negative weights, mixed weights
  - Special cases: disconnected graphs, self-loops, multi-edges (multiple edges between same vertices), bipartite graphs
  - Scale variations: small (< 10 vertices), medium (10-100), large (100-1000+)

- **Real-world themed examples:**
  - Where practical, use relatable scenarios rather than abstract graphs
  - Example themes:
    - Social networks: actors and movies (6 degrees of Kevin Bacon), friendships, collaborations
    - Transportation: cities and routes, airline connections, road networks
    - Communication: computer networks, message routing, dependency graphs
    - Organizational: company hierarchy, project dependencies, workflow graphs
  - Benefits: easier to understand, memorable, demonstrates practical applications
  - Balance: maintain some abstract examples for pure algorithm testing

- **Container flexibility:**
  - At least one directed and one undirected example is needed per category (maximum of 10 total fixtures)
  - We should be able to select whichever adjacency list representation we want for the data
  - This includes ones built from standard containers, `compressed_graph`, `dynamic_graph`, and `undirected_adjacency_list`
  - `undirected_adjacency_list` is limited to undirected examples
  - For `dynamic_graph`, we should be able to determine which vertex/edge combination is used for each situation
  - Ideally, we should be able to select a subset of adjacency list types and run the algorithm against that subset

- **Reusable test fixture infrastructure:**
  - Location: `tests/common/graph_fixtures.hpp`
  - Template-based factory functions for different graph types
  - Support for all container combinations
  - Include expected results structure (similar to existing `tests/common/map_graph_test_data.hpp`)
  - Type lists for parameterized testing (similar to `tests/common/graph_test_types.hpp`)
  - Support filtering by category (e.g., directed_only, undirected_only, weighted_only)

- **Example structure for parameterized tests:**
  ```cpp
  template<typename GraphFixture>
  void test_algorithm() { /* ... */ }
  
  TEMPLATE_TEST_CASE_METHOD(GraphFixture, "algorithm name", "[algorithm]",
                            all_directed_containers,
                            all_undirected_containers)
  ```

### 2. Benchmarking Infrastructure

Benchmarking of algorithms with regression tracking capability:

- **Benchmarking framework selection:**
  - **Google Benchmark (Recommended):** Already in use in the project
    - Industry-standard library with sophisticated statistical analysis
    - Automatic iteration count adjustment and warmup runs
    - Built-in outlier detection and confidence intervals
    - Excellent JSON output format for regression tracking
    - Better for comparing multiple implementations and container types
    - More accurate for complex performance analysis
  - **Catch2 Benchmarking (Alternative):** Simpler but less powerful
    - Integrated with existing test framework
    - Good for quick performance checks within tests
    - Simpler API and easier setup
    - Less sophisticated statistical analysis
    - Limited tooling for historical regression tracking
    - May be sufficient for basic performance validation
  - **Recommendation:** Use Google Benchmark for dedicated algorithm benchmarking (already established in `benchmark/` directory), reserve Catch2 benchmarks for quick performance checks within unit tests if needed

- **Integration with existing infrastructure:**
  - Extend existing Google Benchmark setup in `benchmark/` directory
  - Create algorithm-specific benchmark files:
    - `benchmark/algorithms/benchmark_shortest_path.cpp`
    - `benchmark/algorithms/benchmark_search.cpp`
    - `benchmark/algorithms/benchmark_spanning_tree.cpp`
    - `benchmark/algorithms/benchmark_connectivity.cpp`

- **Regression tracking:**
  - Use `benchmark_main` with `--benchmark_format=json` output
  - Create `scripts/compare_benchmarks.py` for historical comparison
  - Document baseline results in `docs/benchmark_results.md`
  - Store historical benchmarks in `benchmark/baselines/`

- **Performance metrics to track:**
  - Time complexity verification (O(V+E), O(E log V), etc.)
  - Memory usage patterns
  - Comparison with standard library algorithms where applicable
  - Scalability analysis across different graph sizes
  - Container type performance comparison

- **Benchmark structure pattern:**
  - Follow existing `benchmark/benchmark_views.cpp` patterns
  - Include multiple graph sizes per algorithm
  - Test with different graph topologies
  - Measure both best-case and worst-case scenarios

### 3. Algorithm Documentation Template

A template for algorithm descriptions that follows algorithm descriptions in the standard C++ library:

- **Create `docs/algorithm_template.md` with sections:**
  - **Brief description and purpose** - One-paragraph overview
  - **Complexity analysis** - Time and space complexity with Big-O notation
    - Time complexity for best, average, and worst cases
    - Space complexity including all temporary memory requirements (auxiliary data structures, recursion stack, etc.)
    - Note: Graph algorithms often require temporary storage for visited flags, predecessor maps, priority queues, etc.
  - **Supported graph properties** - Clearly state which graph types and special cases are supported
    - Directedness: directed, undirected, or both
    - Multi-edges: whether multiple edges between same vertices are supported/handled correctly
    - Self-loops: whether self-loops are supported/handled correctly
    - Cycles: whether graph can contain cycles or must be acyclic (DAG)
    - Connectivity: whether graph must be connected or disconnected graphs are handled
    - Other constraints: weighted/unweighted, simple graphs only, etc.
  - **Requirements and constraints** - Graph properties, concepts, preconditions
  - **Mandates** - Compile-time requirements that must be satisfied (e.g., type traits, static assertions)
  - **Parameters** - Detailed descriptions with constraints
  - **Return value and side effects** - What the function returns and modifies
  - **Preconditions and postconditions** - Contract specification (runtime requirements)
  - **Exception safety guarantees** - noexcept, strong, basic, or none
  - **Example usage** - Complete, runnable code snippet
  - **Implementation notes** - Details about the specific approach taken
  - **References** - Academic papers, standards, or classic algorithms texts

- **Doxygen documentation style:**
  - Reference existing `examples/dijkstra_clrs.hpp` as a model
  - Ensure consistency with graph library CPO documentation style
  - Include `@ingroup graph_algorithms` tag
  - Document all template parameters with `@tparam`
  - Document all function parameters with `@param`
  - Include `@brief` summary
  - Add complexity information with `@complexity` or in description

- **Code comment standards:**
  - Inline comments for non-obvious algorithm steps
  - References to algorithm description sources (e.g., CLRS, Sedgewick)
  - Notes on implementation choices and trade-offs

### 4. Algorithm Testing Requirements

- **Correctness tests:**
  - Known results validation (e.g., shortest path on example graphs)
  - Edge case handling (empty graphs, disconnected components, single vertex)
  - Boundary condition testing
  - Property-based testing where applicable (e.g., path existence)
  - Cross-validation between different implementations when available

- **Test organization:**
  - Location: `tests/algorithms/test_<algorithm_name>.cpp`
  - Use TEMPLATE_TEST_CASE for parameterized container testing
  - Separate test files per algorithm or algorithm family
  - Include both unit tests and integration tests

### 5. Example Programs

- **Integration with existing examples:**
  - Each algorithm needs a corresponding example in `examples/`
  - Follow `examples/dijkstra_clrs_example.cpp` pattern
  - Include CMakeLists.txt updates for new executables
  - Demonstrate practical usage with real-world scenarios
  - Show different graph configurations and use cases
  - Include output visualization or interpretation

- **Example program structure:**
  - Clear comments explaining each step
  - Multiple test cases within single program
  - Demonstrate error handling and edge cases
  - Include timing/performance output when relevant

### 6. Documentation Integration

- **Algorithm overview documentation:**
  - Create `docs/algorithms/` directory
  - Individual markdown file per algorithm
  - Overview document: `docs/algorithms/README.md`
  - Cross-reference with related CPOs and views

- **Project-level documentation:**
  - Update main `README.md` with algorithm list
  - Add algorithms section to project documentation
  - Link to examples and API reference
  - Include complexity reference table

## Implementation Plan

### Phase 1: Test Infrastructure ✅ COMPLETE (2026-02-03)
- [x] Create `tests/common/graph_fixtures.hpp` - 15+ reusable fixtures
- [x] Port `map_graph_test_data.hpp` patterns to fixture system
- [x] Add `tests/common/algorithm_test_types.hpp` - Type lists for parameterized testing
- [x] Implement common graph patterns (path, cycle, tree, DAG, etc.)
- [x] Create expected results structures for each fixture
- [x] Document fixture usage patterns
- [x] Add `tests/algorithms/` directory structure
- [x] Create `tests/algorithms/CMakeLists.txt` (ready for algorithms)
- [x] Create `tests/algorithms/README.md` with comprehensive testing guide

### Phase 2: Documentation Standards ✅ COMPLETE (2026-02-03)
- [x] Create `docs/algorithm_template.md` - Comprehensive documentation template
- [x] Create `docs/algorithms/README.md` - Algorithm overview and catalog
- [x] Create `include/graph/algorithm/README.md` - Implementation guidelines
- [x] Define Doxygen style guide for algorithms
- [x] Establish code comment conventions
- [x] Create master include file `include/graph/algorithms.hpp`
- [ ] Document existing `dijkstra_clrs.hpp` as reference implementation (deferred)

### Phase 3: Benchmarking Setup ✅ COMPLETE (2026-02-03)
- [x] Extend `benchmark/` infrastructure with algorithms subdirectory
- [x] Create `benchmark/algorithms/CMakeLists.txt` (ready for benchmarks)
- [x] Create `benchmark/algorithms/README.md` with benchmarking guide
- [x] Document benchmarking procedures and regression tracking
- [ ] Add regression tracking scripts in `scripts/` (deferred until first algorithm)
- [ ] Create initial baseline measurements (pending algorithm implementation)
- [ ] Set up automated performance testing (optional, future work)

### Phase 4: Algorithm Implementation (Ready to Start)
- [ ] Implement Dijkstra algorithm from graph-v2
- [ ] Use established patterns for each new algorithm
- [ ] Parallel development of tests, examples, and benchmarks
- [ ] Code review process for correctness and performance
- [ ] Documentation review for clarity and completeness

**Infrastructure Status:** All foundational infrastructure complete. Ready for algorithm implementation.

## Reference Examples in Codebase

Good examples to follow:
- **Test data structure:** `tests/common/map_graph_test_data.hpp`
- **Algorithm fixtures:** `tests/common/graph_fixtures.hpp` ✨ NEW
- **Algorithm type lists:** `tests/common/algorithm_test_types.hpp` ✨ NEW
- **Parameterized testing:** `tests/container/dynamic_graph/test_dynamic_graph_common.cpp`
- **Benchmarking patterns:** `benchmark/benchmark_views.cpp`
- **Algorithm documentation:** `examples/dijkstra_clrs.hpp`
- **Example structure:** `examples/dijkstra_clrs_example.cpp`
- **Type parameterization:** `tests/common/graph_test_types.hpp`

## New Infrastructure Documentation

Created documentation for algorithm development:
- **Documentation template:** `docs/algorithm_template.md` ✨
- **Algorithm overview:** `docs/algorithms/README.md` ✨
- **Implementation guide:** `include/graph/algorithm/README.md` ✨
- **Testing guide:** `tests/algorithms/README.md` ✨
- **Benchmark guide:** `benchmark/algorithms/README.md` ✨
- **Infrastructure summary:** `agents/algorithm_infrastructure_complete.md` ✨

## Success Criteria

The algorithm development environment is ready when:
- [x] At least 8-10 graph fixtures are available and documented - **15+ fixtures created**
- [x] Fixtures work with all major container types - **Template-based, works with all containers**
- [x] Benchmarking infrastructure can track performance over time - **Google Benchmark setup complete**
- [x] Documentation template is complete and demonstrated - **Comprehensive template created**
- [ ] At least 2-3 algorithms are fully implemented using the new patterns - **Ready to implement**
- [ ] Tests achieve high coverage for implemented algorithms - **Infrastructure ready**
- [ ] Examples compile and run successfully - **Infrastructure ready**
- [x] Documentation is clear and comprehensive - **Complete with guides and templates**

**Status:** Infrastructure phases complete (Phase 1-3). Ready for Phase 4 algorithm implementation.
 