# Comparison with modernized boost graph library (BGL2)

The original boost graph library (BGL1, located at `/home/phil/dev_graph/boost/libs/graph/`) has been 
modernized to use C++20 (`BGL2`, located at `/home/phil/dev_graph/boost/libs/graph/modern/`).

**BGL2 Container Selectors (Now Fully Implemented):**

BGL2 now supports 10 container selectors defined in `container_selectors.hpp`:

| Selector | Container | Characteristics |
|----------|-----------|-----------------|
| `vecS` | `std::vector` | O(1) random access, index-based descriptors |
| `listS` | `std::list` | O(1) insertion/removal, stable iterator-based descriptors |
| `setS` | `std::set` | O(log n), ordered unique elements, stable iterators |
| `mapS` | `std::set` (for edges) | O(log n), key-value storage, prevents parallel edges |
| `multisetS` | `std::multiset` | O(log n), ordered with duplicates allowed |
| `multimapS` | `std::multiset` (for edges) | O(log n), duplicate keys allowed |
| `hash_setS` | `std::unordered_set` | O(1) average, hash-based unique |
| `hash_mapS` | `std::unordered_set` (for edges) | O(1) average, hash-based |
| `hash_multisetS` | `std::unordered_multiset` | O(1) average, duplicates allowed |
| `hash_multimapS` | `std::unordered_multiset` (for edges) | O(1) average, duplicate keys |

The `adjacency_list.hpp` provides implementations for:
- **Vertex list selectors:** `vecS` (index-based), `listS` (iterator-based), `setS` (iterator-based)
- **Out-edge list selectors:** All 10 selectors work for edge containers

**Key BGL2 Design Features:**
- Selector concepts: `ContainerSelector`, `SequenceSelector`, `AssociativeSelector`, `UnorderedSelector`
- Selector traits: `is_random_access`, `has_stable_iterators`, `is_ordered`, `is_unique`
- `container_gen<Selector, ValueType>` maps selectors to container types
- `parallel_edge_category_for<OutEdgeListS>` determines if parallel edges are allowed (based on `is_unique`)

Compare and contrast BGL2 with the current library (graph-v3), making sure that the following topics are covered:
- The differences between the vertex and edge descriptors.
- The ability to adapt to pre-existing graph data structures.
- Strengths and weaknesses in the capabilities of each library.
- Container flexibility: BGL2's selector-based approach vs graph-v3's trait-based approach.
- Other areas of interest in the design and flexibility for the libraries.

Algorithms have not been implemented for graph-v3 and is a known limitation.

Output the result to `agents/bgl2_comparison_result.md`.
