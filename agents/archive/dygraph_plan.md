# Plan for testing and improving dynamic_graph

Create comprehensive tests for dynamic_graph for the following requirements. Include additional tests for areas you think are important. Do not write tests for CPOs; they will be done later.
1. Vertices kept in vector, deque. The vertex_id type must be uint64_t, int, int8_t, for all container types; additional key types must be a class for a USA address, pair, tuple and string for the map and unordered_map.
2. Edges kept in vector, deque, list and forward_list. 
3. Edge values tested must be void, int, string.
4. Vertex values tested must be void, int, string.
5. Graph values tested must be void, int, string.
6. All public member functions must have at least one test.
7. The goal is to achieve 95% test coverage.
Ideally all combinations of choices should be chosen, but there will be too many combinations to test effectively. Choose a combination of features with a confidence of at least 95% coverage of the functionality.

Add support for a minimal set of CPO functions (use default implementations whenever possible). 

Incrementally add support for new container types for vertices and edges, using the same test criteria as before.
1. Add support for vertices in map and unordered_map.
2. Add support for edges in map, unordered_map, set and unordered_set.

Add support for non-integral vertex ids including double, string, and a compound type (e.g. first and last name), using the same test criteria as before.

The number of tests is expected to be large. Determine a good strategy to organize the remaining tests in separate test files that is clear
and supports estensibility.

Create a phased plan that focuses on testing the existing functionality first, and that is safe in
order to minimize the stability of the code.

Output the plan to `dynamic_graph_todo.md`
