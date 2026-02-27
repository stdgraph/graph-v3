# Review the library design

Review the design and organization of the library. 

The library does not have algorithms defined yet. They will be implemented after this review is done and resolved.

## General Criteria

- It is desired for it to be in the C++ standard library, so it should match the expectations of that.
- This should also conform to standard open source package organization and layout
- Verify alignment with P1709 conventions and naming
- Check naming patterns against Ranges and other C++20 standard libraries

## 1. Namespace Organization

**Current State:**
- `graph::` - Root namespace, algorithms, common types, info structures
- `graph::adj_list::` - Adjacency list abstractions (CPOs, descriptors, concepts, traits)
- `graph::edge_list::` - Edge list abstractions (CPOs, descriptors, concepts, traits)
- `graph::container::` - Concrete graph containers
- `graph::views::` - Graph views (vertex list, edge list, neighbors, BFS, DFS, etc.)

**Issues to Review:**
- Views are in `graph::views::` but most CPOs are in `graph::adj_list::` - is this consistent?
- Should views be `graph::views::` or `graph::adj_list::views::`?
- Types from `graph_data.hpp` are in `graph::` - should they be in a dedicated namespace?
- What symbols should be imported into `graph::` namespace vs staying in subnamespaces?
- Are descriptor types appropriately placed in `graph::adj_list::`?

**Questions:**
- Should separate namespaces exist for adj_list and edge_list, or should they be combined?
- Shared code should be in common directories and common code should be in the same namespace - is this achieved?
- Is there a clear pattern for users to understand what namespace to use?

## 2. Header Organization

**Current State:**
- `graph.hpp` - Main umbrella header, includes descriptors, concepts, traits, utilities
- `views.hpp` - Views umbrella header
- Container headers under `include/graph/container/`
- `graph.hpp` does NOT currently include views or containers

**Issues to Review:**
 - Should `graph.hpp` include views and containers, or keep them separate?
- What's the minimal include for basic graph usage?
- Are header dependencies appropriate (can users include minimal headers)?
- Relationship between umbrella headers and individual component headers
- Should there be `adj_list.hpp` and `edge_list.hpp` umbrella headers?

**Questions:**
- Should there be separate umbrella headers or one comprehensive header?
- What should a user include for different use cases (containers only, views only, everything)?
- Are internal/detail headers properly separated from public API?

## 3. CPO Implementation Review

**Current State:**
- CPOs use MSVC-style pattern with `_cpo` namespace
- Adjacency list CPOs in `graph::adj_list::detail::graph_cpo.hpp`
- Edge list CPOs in edge_list headers

**Issues to Review:**
- Should CPO implementation be split into two sections: one for adjacency lists and another for edge lists?
- **Critical:** `target_id(x,uv)`, `source_id(x,uv)`, and `edge_value(x,uv)` must behave identically when x is an adjacency list or an edge list
- Are CPO patterns consistent across the library?
- Is `graph_utility.hpp` the right place for adjacency list CPOs, or should they be in a dedicated CPO header?
- CPO organization in detail vs public headers - is the boundary clear?

**Questions:**
- Should there be a unified CPO header that works for both adj_list and edge_list?
- Are the customization points properly documented and easy to customize?
- Do CPOs properly constrain with concepts?

## 4. Container Implementation Review

**Current State:**
- Three containers: `dynamic_graph`, `compressed_graph`, `undirected_adjacency_list`
- Located in `graph::container::` namespace
- Multiple trait specializations in `container/traits/`

**Issues to Review:**
- Should containers be in `graph::container::` or directly in `graph::`?
- Naming consistency with standard library containers (e.g., should it be `adjacency_list` not `undirected_adjacency_list`)?
- Are trait specialization patterns consistent?
- Is the container API consistent across implementations?
- Should there be a base class or common interface?

**Questions:**
- How do containers relate to the abstract adjacency list concepts?
- Are containers easy to instantiate and use?
- Is memory management and performance characteristics documented?

## 5. Views Integration

**Current State:**
- Views implemented: `vertexlist`, `edgelist`, `neighbors`, `incidence`, `bfs`, `dfs`, `topological_sort`
- Adaptors: `filter`, `transform`
- Located in `graph::views::` namespace

**Issues to Review:**
- Integration with the adjacency list interface - do views work seamlessly with all containers?
- Should views have their own CPO layer or use existing CPOs?
- Consistency of view naming (e.g., `vertexlist` vs `vertex_list`)
- View composition and chaining - limitations documented?
- Are views properly constrained with concepts?

**Questions:**
- Should views be in their own namespace or integrated with adj_list?
- Do views work with both adjacency lists and edge lists where appropriate?
- Is the adaptor pattern consistent with C++20 ranges adaptors?

## 6. Type Alias Placement

**Current State:**
- Type aliases defined in traits headers
- Some imported into `graph::` namespace in `graph.hpp`

**Issues to Review:**
- Where should common type aliases live (e.g., `vertex_t`, `edge_t`, `vertex_id_t`)?
- Should type aliases be in the same namespace as the graph types?
- Are `_t` suffix conventions followed consistently?
- What aliases should be imported into root `graph::` namespace?

**Questions:**
- Are type aliases easy to discover and use?
- Is there duplication of type aliases across namespaces?

## 7. Test Organization

**Current State:**
- `tests/adj_list/` - Adjacency list tests
- `tests/views/` - View tests  
- `tests/edge_list/` - Edge list tests
- `tests/common/` - Common utilities
- `tests/container/` - Does this exist? Should it?

**Issues to Review:**
- Does test directory structure match include structure?
- Should there be `tests/container/` for concrete containers?
- Are common test utilities properly shared?
- Test naming conventions consistent?

**Questions:**
- Is it clear where new tests should be added?
- Are integration tests vs unit tests properly organized?

## 8. Build System and Packaging

**Issues to Review:**
- CMake configuration for library consumption
- Package config generation (`graph3-config.cmake`)
- Install targets and public API surface
- Header installation structure
- Version management
- CPM integration for dependencies

**Questions:**
- Can users easily consume the library via CMake?
- Are internal headers excluded from installation?
- Is version compatibility properly managed?

## 9. Documentation Review

**Current State:**
- `docs/` contains various design documents
- README.md provides overview
- Code comments and examples exist

**Issues to Review:**
- Clarity: Are concepts and usage patterns clearly explained?
- Completeness: Are all major components documented?
- Correctness: Does documentation match implementation?
- Organization: Is documentation easy to navigate?
- Examples: Are there sufficient examples for common use cases?

**Specific Documentation Files to Review:**
- `docs/cpo.md` - CPO implementation guide
- `docs/container_interface.md` - Container interface specification
- `docs/views.md` - Views documentation
- `docs/common_graph_guidelines.md` - Naming conventions

**Questions:**
- Is there a clear getting started guide?
- Are advanced features documented?
- Is migration from other graph libraries documented?

## 10. API Surface and User Experience

**Issues to Review:**
- What's the minimal include for basic usage?
- What's included by default in `graph.hpp`?
- Clear user-facing vs internal API boundaries
- Consistency of function naming and parameter order
- Error messages and diagnostics quality
- Compile-time performance

**Questions:**
- Can a new user easily understand what to include?
- Are common tasks easy to accomplish?
- Is there clear separation between stable and experimental APIs?
- Are deprecation paths clear if APIs need to change?

## 11. Standard Library Alignment

**Issues to Review:**
- Alignment with P1709 Graph Library Proposal
- Consistency with C++20 Ranges library patterns
- Consistency with C++20 concepts and constraints
- Iterator and range interface conformance
- Naming alignment with standard library

**Questions:**
- Would this library fit naturally into `std::` namespace?
- Are there any non-standard patterns that should be reconsidered?
- Does it follow modern C++ best practices?

## Review Process

For each section above:
1. Document current state
2. Identify issues and inconsistencies  
3. Make specific recommendations for changes
4. Prioritize changes (critical, important, nice-to-have)
5. Consider backward compatibility impacts
6. Propose implementation approach

## Success Criteria

The review is complete when:
- All namespaces follow a clear, consistent pattern
- Header organization is intuitive and well-documented
- CPOs work uniformly across adjacency lists and edge lists
- Container, views, and core abstractions are properly integrated
- Documentation is complete and accurate
- The library is ready for algorithm implementation
- The library could be proposed for standardization
