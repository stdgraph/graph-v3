/**
 * @file test_edge_descriptor.cpp
 * @brief Comprehensive unit tests for edge_descriptor and edge_descriptor_view
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <graph/adj_list/edge_descriptor.hpp>
#include <graph/adj_list/edge_descriptor_view.hpp>

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Edge Descriptor Tests - Random Access Iterator (vector)
// =============================================================================

TEST_CASE("edge_descriptor with random access iterator - vector<int>", "[edge_descriptor][random_access]") {
  using VectorIter = std::vector<int>::iterator;
  using EdgeIter   = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VectorIter>;
  using ED         = edge_descriptor<EdgeIter, VectorIter>;

  SECTION("Default construction") {
    ED ed;
    REQUIRE(ed.value() == 0);
    REQUIRE(ed.source().value() == 0);
  }

  SECTION("Construction from edge index and source vertex") {
    VD source{5};
    ED ed{3, source};

    REQUIRE(ed.value() == 3);
    REQUIRE(ed.source().value() == 5);
    REQUIRE(ed.source().vertex_id() == 5);
  }

  SECTION("Copy semantics") {
    VD source{10};
    ED ed1{7, source};
    ED ed2 = ed1;

    REQUIRE(ed2.value() == 7);
    REQUIRE(ed2.source().value() == 10);

    ED ed3{1, VD{2}};
    ed3 = ed1;
    REQUIRE(ed3.value() == 7);
    REQUIRE(ed3.source().value() == 10);
  }

  SECTION("Move semantics") {
    VD source{15};
    ED ed1{8, source};
    ED ed2 = std::move(ed1);

    REQUIRE(ed2.value() == 8);
    REQUIRE(ed2.source().value() == 15);
  }

  SECTION("Pre-increment advances edge, keeps source") {
    VD source{5};
    ED ed{3, source};

    ++ed;
    REQUIRE(ed.value() == 4);
    REQUIRE(ed.source().value() == 5); // Source unchanged
  }

  SECTION("Post-increment") {
    VD source{5};
    ED ed{3, source};

    ED old = ed++;
    REQUIRE(old.value() == 3);
    REQUIRE(ed.value() == 4);
    REQUIRE(ed.source().value() == 5);
  }

  SECTION("Comparison operators") {
    VD source1{5};
    VD source2{10};

    ED ed1{3, source1};
    ED ed2{7, source1};
    ED ed3{3, source1};
    ED ed4{3, source2}; // Same edge index, different source

    REQUIRE(ed1 == ed3);
    REQUIRE(ed1 != ed2);
    REQUIRE(ed1 != ed4); // Different source makes them different
    REQUIRE(ed1 < ed2);
    REQUIRE(ed2 > ed1);
  }

  SECTION("Hash consistency") {
    VD source{42};
    ED ed1{10, source};
    ED ed2{10, source};
    ED ed3{11, source};

    std::hash<ED> hasher;
    REQUIRE(hasher(ed1) == hasher(ed2));
    // Different edge index should produce different hash (usually)
  }

  SECTION("Use in std::set") {
    VD           source{5};
    std::set<ED> ed_set;

    ed_set.insert(ED{3, source});
    ed_set.insert(ED{1, source});
    ed_set.insert(ED{3, source}); // duplicate

    REQUIRE(ed_set.size() == 2);
  }

  SECTION("Use in std::unordered_map") {
    VD                                  source{5};
    std::unordered_map<ED, std::string> ed_map;

    ed_map[ED{1, source}] = "edge1";
    ed_map[ED{2, source}] = "edge2";

    REQUIRE(ed_map.size() == 2);
    REQUIRE(ed_map[ED{1, source}] == "edge1");
  }
}

// =============================================================================
// Edge Descriptor Tests - Forward Iterator (list)
// =============================================================================

TEST_CASE("edge_descriptor with forward iterator - list<int>", "[edge_descriptor][forward]") {
  using VectorIter = std::vector<int>::iterator;
  using ListIter   = std::list<int>::iterator;
  using VD         = vertex_descriptor<VectorIter>;
  using ED         = edge_descriptor<ListIter, VectorIter>;

  std::list<int> edges = {100, 200, 300};

  SECTION("Construction from iterator and source") {
    auto it = edges.begin();
    VD   source{10};
    ED   ed{it, source};

    REQUIRE(ed.value() == it);
    REQUIRE(ed.source().value() == 10);
  }

  SECTION("Pre-increment advances iterator") {
    auto it = edges.begin();
    VD   source{10};
    ED   ed{it, source};

    ++ed;
    REQUIRE(ed.value() == ++edges.begin());
    REQUIRE(ed.source().value() == 10); // Source unchanged
  }

  SECTION("Post-increment") {
    auto it = edges.begin();
    VD   source{10};
    ED   ed{it, source};

    ED old = ed++;
    REQUIRE(old.value() == edges.begin());
    REQUIRE(ed.value() == ++edges.begin());
  }

  SECTION("Comparison operators") {
    auto it1 = edges.begin();
    auto it2 = ++edges.begin();
    VD   source{10};

    ED ed1{it1, source};
    ED ed2{it2, source};
    ED ed3{it1, source};

    REQUIRE(ed1 == ed3);
    REQUIRE(ed1 != ed2);
  }
}

// =============================================================================
// Edge Descriptor View Tests - Random Access (vector)
// =============================================================================

TEST_CASE("edge_descriptor_view with vector - per-vertex adjacency", "[edge_descriptor_view][random_access]") {
  using VectorIter = std::vector<int>::iterator;
  using EdgeIter   = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VectorIter>;
  using ED         = edge_descriptor<EdgeIter, VectorIter>;
  using View       = edge_descriptor_view<EdgeIter, VectorIter>;

  // Simulate per-vertex adjacency: edges from vertex 5
  std::vector<int> edges_from_v5 = {10, 20, 30, 40}; // Target vertex IDs
  VD               source{5};

  SECTION("Construction from container and source") {
    View view{edges_from_v5, source};

    REQUIRE(view.size() == 4);
    REQUIRE(!view.empty());
    REQUIRE(view.source().value() == 5);
  }

  SECTION("Forward iteration yields edge descriptors") {
    View view{edges_from_v5, source};
    auto it = view.begin();

    ED ed0 = *it;
    REQUIRE(ed0.value() == 0);
    REQUIRE(ed0.source().value() == 5);

    ++it;
    ED ed1 = *it;
    REQUIRE(ed1.value() == 1);
    REQUIRE(ed1.source().value() == 5);
  }

  SECTION("Range-based for loop") {
    View                     view{edges_from_v5, source};
    std::vector<std::size_t> collected_indices;

    for (auto ed : view) {
      collected_indices.push_back(ed.value());
      REQUIRE(ed.source().value() == 5); // All have same source
    }

    REQUIRE(collected_indices == std::vector<std::size_t>{0, 1, 2, 3});
  }

  SECTION("View satisfies forward_range") {
    View view{edges_from_v5, source};
    static_assert(std::ranges::forward_range<View>);
    static_assert(std::ranges::view<View>);
  }

  SECTION("Iterator value_type is edge_descriptor") {
    View view{edges_from_v5, source};
    using IterValueType = typename View::iterator::value_type;
    static_assert(std::same_as<IterValueType, ED>);
  }

  SECTION("Empty view") {
    std::vector<int> empty_edges;
    View             view{empty_edges, source};

    REQUIRE(view.size() == 0);
    REQUIRE(view.empty());
    REQUIRE(view.begin() == view.end());
  }

  SECTION("Works with std::ranges algorithms") {
    View view{edges_from_v5, source};

    auto count = std::ranges::distance(view);
    REQUIRE(count == 4);

    // Find edge at specific index
    auto it = std::ranges::find_if(view, [](ED ed) { return ed.value() == 2; });

    REQUIRE(it != view.end());
    REQUIRE((*it).value() == 2);
    REQUIRE((*it).source().value() == 5);
  }
}

// =============================================================================
// Edge Descriptor View Tests - Forward Iterator (list)
// =============================================================================

TEST_CASE("edge_descriptor_view with list", "[edge_descriptor_view][forward]") {
  using VectorIter = std::vector<int>::iterator;
  using ListIter   = std::list<int>::iterator;
  using VD         = vertex_descriptor<VectorIter>;
  using View       = edge_descriptor_view<ListIter, VectorIter>;

  std::list<int> edges = {100, 200, 300};
  VD             source{42};

  SECTION("Construction from container and source") {
    View view{edges, source};
    REQUIRE(!view.empty());
    REQUIRE(view.source().value() == 42);
  }

  SECTION("Forward iteration yields edge descriptors with correct source") {
    View view{edges, source};

    int count = 0;
    for (auto ed : view) {
      REQUIRE(ed.source().value() == 42);
      count++;
    }

    REQUIRE(count == 3);
  }

  SECTION("View satisfies forward_range") {
    View view{edges, source};
    static_assert(std::ranges::forward_range<View>);
    static_assert(std::ranges::view<View>);
  }

  SECTION("Works with std::ranges algorithms") {
    View view{edges, source};

    auto count = std::ranges::distance(view);
    REQUIRE(count == 3);
  }
}

// =============================================================================
// Various Edge Data Types Tests
// =============================================================================

TEST_CASE("edge_descriptor_view with various edge data types", "[edge_descriptor_view][data_types]") {
  using VectorIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VectorIter>;

  VD source{100};

  SECTION("Edge container with pairs (target, weight)") {
    using PairVec = std::vector<std::pair<int, double>>;
    using View    = edge_descriptor_view<PairVec::iterator, VectorIter>;

    PairVec edges = {{10, 1.5}, {20, 2.5}, {30, 3.5}};
    View    view{edges, source};

    REQUIRE(view.size() == 3);

    for (auto ed : view) {
      REQUIRE(ed.source().value() == 100);
    }
  }

  SECTION("Edge container with tuples (target, weight, color)") {
    using TupleVec = std::vector<std::tuple<int, double, std::string>>;
    using View     = edge_descriptor_view<TupleVec::iterator, VectorIter>;

    TupleVec edges = {{10, 1.5, "red"}, {20, 2.5, "blue"}};
    View     view{edges, source};

    REQUIRE(view.size() == 2);

    for (auto ed : view) {
      REQUIRE(ed.source().value() == 100);
    }
  }

  SECTION("Edge container with simple integers (just target IDs)") {
    std::vector<int>     edges = {5, 10, 15, 20};
    edge_descriptor_view view{edges, source};

    REQUIRE(view.size() == 4);

    int idx = 0;
    for (auto ed : view) {
      REQUIRE(ed.value() == static_cast<size_t>(idx));
      REQUIRE(ed.source().value() == 100);
      idx++;
    }
  }
}

// =============================================================================
// Type Safety Tests
// =============================================================================

TEST_CASE("Type safety - edge descriptors with different iterators", "[edge_descriptor][type_safety]") {
  using VectorIter = std::vector<int>::iterator;
  using ListIter   = std::list<int>::iterator;
  using VD         = vertex_descriptor<VectorIter>;

  using VectorEdgeDesc = edge_descriptor<VectorIter, VectorIter>;
  using ListEdgeDesc   = edge_descriptor<ListIter, VectorIter>;

  // These types should be distinct
  static_assert(!std::same_as<VectorEdgeDesc, ListEdgeDesc>);

  SECTION("Cannot accidentally mix descriptor types") {
    VD             source{5};
    VectorEdgeDesc ed_vec{3, source};
    // ListEdgeDesc ed_list = ed_vec; // Would not compile

    SUCCEED("Types are properly distinct");
  }
}

// =============================================================================
// Multiple Sources / Graph Simulation
// =============================================================================

TEST_CASE("Multiple edge views for different source vertices", "[edge_descriptor_view][graph_simulation]") {
  using VectorIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VectorIter>;
  using View       = edge_descriptor_view<std::vector<int>::iterator, VectorIter>;

  // Simulate adjacency lists for different vertices
  std::vector<int> edges_from_v0 = {1, 2, 3};
  std::vector<int> edges_from_v1 = {2, 3};
  std::vector<int> edges_from_v2 = {3};

  View view0{edges_from_v0, VD{0}};
  View view1{edges_from_v1, VD{1}};
  View view2{edges_from_v2, VD{2}};

  SECTION("Each view has correct source") {
    REQUIRE(view0.source().value() == 0);
    REQUIRE(view1.source().value() == 1);
    REQUIRE(view2.source().value() == 2);
  }

  SECTION("Each view has correct edge count") {
    REQUIRE(view0.size() == 3);
    REQUIRE(view1.size() == 2);
    REQUIRE(view2.size() == 1);
  }

  SECTION("All edges from each view have correct source") {
    for (auto ed : view0) {
      REQUIRE(ed.source().value() == 0);
    }

    for (auto ed : view1) {
      REQUIRE(ed.source().value() == 1);
    }

    for (auto ed : view2) {
      REQUIRE(ed.source().value() == 2);
    }
  }
}

// =============================================================================
// Target ID Extraction Tests
// =============================================================================

TEST_CASE("edge_descriptor::target_id() with simple int edges", "[edge_descriptor][target_id]") {
  std::vector<int> edges    = {1, 2, 3, 4, 5};
  std::vector<int> vertices = {10, 20, 30, 40, 50};

  using EdgeIter   = std::vector<int>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  VD source{0};
  ED ed{2, source}; // Points to edge at index 2 (value 3)

  REQUIRE(ed.target_id(edges) == 3);
}

TEST_CASE("edge_descriptor::target_id() with pair edges", "[edge_descriptor][target_id]") {
  std::vector<std::pair<int, double>> edges    = {{1, 1.5}, {2, 2.5}, {3, 3.5}, {4, 4.5}};
  std::vector<int>                    vertices = {10, 20, 30, 40, 50};

  using EdgeIter   = std::vector<std::pair<int, double>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  VD source{0};
  ED ed{1, source}; // Points to edge at index 1: (2, 2.5)

  REQUIRE(ed.target_id(edges) == 2); // First element of pair
}

TEST_CASE("edge_descriptor::target_id() with tuple edges", "[edge_descriptor][target_id]") {
  std::vector<std::tuple<size_t, size_t, double>> edges    = {{1, 0, 1.0}, {2, 0, 2.0}, {3, 1, 3.0}};
  std::vector<int>                                vertices = {10, 20, 30, 40};

  using EdgeIter   = std::vector<std::tuple<size_t, size_t, double>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  VD source{0};
  ED ed{2, source}; // Points to edge at index 2: (3, 1, 3.0)

  REQUIRE(ed.target_id(edges) == 3); // First element of tuple
}

TEST_CASE("edge_descriptor::target_id() with forward iterator - list", "[edge_descriptor][target_id]") {
  std::list<int>   edges    = {5, 10, 15, 20};
  std::vector<int> vertices = {100, 200, 300}; // Use vector for vertices (random access)

  using EdgeIter   = std::list<int>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  auto edge_it = edges.begin();
  std::advance(edge_it, 2); // Point to 15

  VD source{0}; // Random access vertex descriptor uses index
  ED ed{edge_it, source};

  REQUIRE(ed.target_id(edges) == 15); // Dereferences iterator
}

// =============================================================================
// Underlying Value Access Tests
// =============================================================================

TEST_CASE("edge_descriptor::underlying_value() with simple int edges", "[edge_descriptor][underlying_value]") {
  std::vector<int> edges    = {10, 20, 30, 40, 50};
  std::vector<int> vertices = {100, 200, 300};

  using EdgeIter   = std::vector<int>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("Access underlying edge value") {
    VD source{0};
    ED ed{2, source};

    REQUIRE(ed.underlying_value(edges) == 30);
  }

  SECTION("Modify underlying edge value") {
    VD source{1};
    ED ed{3, source};

    ed.underlying_value(edges) = 999;
    REQUIRE(edges[3] == 999);
    REQUIRE(ed.underlying_value(edges) == 999);
  }

  SECTION("Const access") {
    const std::vector<int> const_edges = {1, 2, 3};
    VD                     source{0};
    ED                     ed{1, source};

    REQUIRE(ed.underlying_value(const_edges) == 2);
  }
}

TEST_CASE("edge_descriptor::underlying_value() with pair edges", "[edge_descriptor][underlying_value]") {
  std::vector<std::pair<int, double>> edges    = {{10, 1.5}, {20, 2.5}, {30, 3.5}, {40, 4.5}};
  std::vector<int>                    vertices = {100, 200, 300};

  using EdgeIter   = std::vector<std::pair<int, double>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("Access pair through underlying_value") {
    VD source{0};
    ED ed{1, source};

    const auto& edge_pair = ed.underlying_value(edges);
    REQUIRE(edge_pair.first == 20);
    REQUIRE(edge_pair.second == 2.5);
  }

  SECTION("Modify pair members") {
    VD source{1};
    ED ed{2, source};

    ed.underlying_value(edges).first  = 99;
    ed.underlying_value(edges).second = 9.9;

    REQUIRE(edges[2].first == 99);
    REQUIRE(edges[2].second == 9.9);
  }
}

TEST_CASE("edge_descriptor::underlying_value() with tuple edges", "[edge_descriptor][underlying_value]") {
  std::vector<std::tuple<int, int, double>> edges    = {{1, 10, 1.0}, {2, 20, 2.0}, {3, 30, 3.0}};
  std::vector<int>                          vertices = {100, 200, 300};

  using EdgeIter   = std::vector<std::tuple<int, int, double>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("Access tuple through underlying_value") {
    VD source{0};
    ED ed{1, source};

    const auto& edge_tuple = ed.underlying_value(edges);
    REQUIRE(std::get<0>(edge_tuple) == 2);
    REQUIRE(std::get<1>(edge_tuple) == 20);
    REQUIRE(std::get<2>(edge_tuple) == 2.0);
  }

  SECTION("Modify tuple members") {
    VD source{1};
    ED ed{0, source};

    std::get<0>(ed.underlying_value(edges)) = 99;
    std::get<2>(ed.underlying_value(edges)) = 9.9;

    REQUIRE(std::get<0>(edges[0]) == 99);
    REQUIRE(std::get<2>(edges[0]) == 9.9);
  }
}

TEST_CASE("edge_descriptor::underlying_value() with custom struct", "[edge_descriptor][underlying_value]") {
  struct Edge {
    int         target;
    std::string label;
    double      weight;
  };

  std::vector<Edge> edges    = {{10, "A", 1.5}, {20, "B", 2.5}, {30, "C", 3.5}};
  std::vector<int>  vertices = {100, 200, 300};

  using EdgeIter   = std::vector<Edge>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("Access struct members through underlying_value") {
    VD source{0};
    ED ed{1, source};

    const auto& edge = ed.underlying_value(edges);
    REQUIRE(edge.target == 20);
    REQUIRE(edge.label == "B");
    REQUIRE(edge.weight == 2.5);
  }

  SECTION("Modify struct through underlying_value") {
    VD source{1};
    ED ed{2, source};

    ed.underlying_value(edges).label  = "Modified";
    ed.underlying_value(edges).weight = 9.9;

    REQUIRE(edges[2].label == "Modified");
    REQUIRE(edges[2].weight == 9.9);
  }
}

TEST_CASE("edge_descriptor::underlying_value() with forward iterator", "[edge_descriptor][underlying_value]") {
  std::list<std::pair<int, double>> edges    = {{10, 1.0}, {20, 2.0}, {30, 3.0}};
  std::vector<int>                  vertices = {100, 200, 300};

  using EdgeIter   = std::list<std::pair<int, double>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("Access through iterator-based descriptor") {
    auto edge_it = edges.begin();
    std::advance(edge_it, 1);

    VD source{0};
    ED ed{edge_it, source};

    const auto& edge_pair = ed.underlying_value(edges);
    REQUIRE(edge_pair.first == 20);
    REQUIRE(edge_pair.second == 2.0);
  }

  SECTION("Modify through iterator-based descriptor") {
    auto edge_it = edges.begin();

    VD source{1};
    ED ed{edge_it, source};

    ed.underlying_value(edges).second = 99.9;
    REQUIRE(edges.begin()->second == 99.9);
  }
}

// =============================================================================
// Inner Value Access Tests
// =============================================================================

TEST_CASE("edge_descriptor::inner_value() with simple int edges", "[edge_descriptor][inner_value]") {
  std::vector<int> edges    = {10, 20, 30, 40};
  std::vector<int> vertices = {100, 200, 300};

  using EdgeIter   = std::vector<int>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("For simple int edges, inner_value returns the int itself") {
    VD source{0};
    ED ed{2, source};

    // Simple int edges: the value is just the target, so inner_value returns it
    REQUIRE(ed.inner_value(edges) == 30);
  }
}

TEST_CASE("edge_descriptor::inner_value() with pair edges", "[edge_descriptor][inner_value]") {
  std::vector<std::pair<int, double>> edges    = {{10, 1.5}, {20, 2.5}, {30, 3.5}};
  std::vector<int>                    vertices = {100, 200, 300};

  using EdgeIter   = std::vector<std::pair<int, double>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("For pairs, inner_value returns .second (the weight/property)") {
    VD source{0};
    ED ed{1, source};

    // Pair: first is target, second is property
    REQUIRE(ed.inner_value(edges) == 2.5);
  }

  SECTION("Modify through inner_value") {
    VD source{1};
    ED ed{0, source};

    ed.inner_value(edges) = 9.9;
    REQUIRE(edges[0].second == 9.9);
    REQUIRE(ed.inner_value(edges) == 9.9);
  }

  SECTION("Const access") {
    const std::vector<std::pair<int, double>> const_edges = {{1, 1.1}, {2, 2.2}};
    VD                                        source{0};
    ED                                        ed{1, source};

    REQUIRE(ed.inner_value(const_edges) == 2.2);
  }
}

TEST_CASE("edge_descriptor::inner_value() with 2-element tuple", "[edge_descriptor][inner_value]") {
  std::vector<std::tuple<int, double>> edges    = {{10, 1.5}, {20, 2.5}, {30, 3.5}};
  std::vector<int>                     vertices = {100, 200, 300};

  using EdgeIter   = std::vector<std::tuple<int, double>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("For 2-element tuple, inner_value returns second element") {
    VD source{0};
    ED ed{1, source};

    REQUIRE(ed.inner_value(edges) == 2.5);
  }

  SECTION("Modify through inner_value") {
    VD source{1};
    ED ed{2, source};

    ed.inner_value(edges) = 7.7;
    REQUIRE(std::get<1>(edges[2]) == 7.7);
  }
}

TEST_CASE("edge_descriptor::inner_value() with 3-element tuple", "[edge_descriptor][inner_value]") {
  std::vector<std::tuple<int, double, std::string>> edges    = {{10, 1.5, "A"}, {20, 2.5, "B"}, {30, 3.5, "C"}};
  std::vector<int>                                  vertices = {100, 200, 300};

  using EdgeIter   = std::vector<std::tuple<int, double, std::string>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("For 3+ element tuple, inner_value returns tuple of last N-1 elements") {
    VD source{0};
    ED ed{1, source};

    // Should return tuple of (double, string) - the property parts
    auto props = ed.inner_value(edges);
    REQUIRE(std::get<0>(props) == 2.5);
    REQUIRE(std::get<1>(props) == "B");
  }

  SECTION("Modify through inner_value tuple") {
    VD source{1};
    ED ed{0, source};

    auto props         = ed.inner_value(edges);
    std::get<0>(props) = 9.9;
    std::get<1>(props) = "Modified";

    REQUIRE(std::get<1>(edges[0]) == 9.9);
    REQUIRE(std::get<2>(edges[0]) == "Modified");
  }
}

TEST_CASE("edge_descriptor::inner_value() with 4-element tuple", "[edge_descriptor][inner_value]") {
  std::vector<std::tuple<int, int, double, std::string>> edges    = {{1, 10, 1.5, "label1"}, {2, 20, 2.5, "label2"}};
  std::vector<int>                                       vertices = {100, 200, 300};

  using EdgeIter   = std::vector<std::tuple<int, int, double, std::string>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("For 4-element tuple, returns tuple of last 3 elements") {
    VD source{0};
    ED ed{0, source};

    auto props = ed.inner_value(edges);
    REQUIRE(std::get<0>(props) == 10);       // 2nd element
    REQUIRE(std::get<1>(props) == 1.5);      // 3rd element
    REQUIRE(std::get<2>(props) == "label1"); // 4th element
  }
}

TEST_CASE("edge_descriptor::inner_value() with custom struct", "[edge_descriptor][inner_value]") {
  struct Edge {
    int         target;
    double      weight;
    std::string label;
  };

  std::vector<Edge> edges    = {{10, 1.5, "A"}, {20, 2.5, "B"}, {30, 3.5, "C"}};
  std::vector<int>  vertices = {100, 200, 300};

  using EdgeIter   = std::vector<Edge>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("For custom struct, inner_value returns the whole struct") {
    VD source{0};
    ED ed{1, source};

    // Custom structs: return whole value (user manages property semantics)
    const auto& edge = ed.inner_value(edges);
    REQUIRE(edge.target == 20);
    REQUIRE(edge.weight == 2.5);
    REQUIRE(edge.label == "B");
  }

  SECTION("Modify through inner_value") {
    VD source{1};
    ED ed{2, source};

    ed.inner_value(edges).weight = 9.9;
    ed.inner_value(edges).label  = "Modified";

    REQUIRE(edges[2].weight == 9.9);
    REQUIRE(edges[2].label == "Modified");
  }
}

TEST_CASE("edge_descriptor::inner_value() with list iterator", "[edge_descriptor][inner_value]") {
  std::list<std::pair<int, double>> edges    = {{10, 1.0}, {20, 2.0}, {30, 3.0}};
  std::vector<int>                  vertices = {100, 200, 300};

  using EdgeIter   = std::list<std::pair<int, double>>::iterator;
  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;
  using ED         = edge_descriptor<EdgeIter, VertexIter>;

  SECTION("inner_value works with forward iterators") {
    auto edge_it = edges.begin();
    std::advance(edge_it, 1);

    VD source{0};
    ED ed{edge_it, source};

    REQUIRE(ed.inner_value(edges) == 2.0);
  }
}

// =============================================================================
// Const Semantics Tests
// =============================================================================

TEST_CASE("edge_descriptor_view - const container with vector", "[edge_descriptor_view][const]") {
  const std::vector<int> edges    = {10, 20, 30, 40};
  std::vector<int>       vertices = {100, 200, 300};

  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;

  VD source{0};

  // Construct view from const container
  edge_descriptor_view view{edges, source};

  // The view should deduce const_iterator
  using ViewType = decltype(view);
  using IterType = typename ViewType::edge_desc::edge_iterator_type;
  static_assert(std::is_same_v<IterType, std::vector<int>::const_iterator>,
                "Should deduce const_iterator for const container");

  // Iterate and verify we can access values
  std::vector<int> target_ids;
  for (auto e : view) {
    target_ids.push_back(e.target_id(edges));
    REQUIRE(e.source().value() == 0);
  }

  REQUIRE(target_ids.size() == 4);
  REQUIRE(target_ids[0] == 10);
  REQUIRE(target_ids[1] == 20);
  REQUIRE(target_ids[2] == 30);
  REQUIRE(target_ids[3] == 40);

  // Verify we can call underlying_value with const container
  auto        e   = *view.begin();
  const auto& val = e.underlying_value(edges);
  REQUIRE(val == 10);
}

TEST_CASE("edge_descriptor_view - non-const container with vector", "[edge_descriptor_view][const]") {
  std::vector<int> edges    = {10, 20, 30, 40};
  std::vector<int> vertices = {100, 200, 300};

  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;

  VD source{0};

  // Construct view from non-const container
  edge_descriptor_view view{edges, source};

  // The view should deduce non-const iterator
  using ViewType = decltype(view);
  using IterType = typename ViewType::edge_desc::edge_iterator_type;
  static_assert(std::is_same_v<IterType, std::vector<int>::iterator>, "Should deduce iterator for non-const container");

  // Verify we can modify through the descriptor
  auto e                    = *view.begin();
  e.underlying_value(edges) = 999;

  REQUIRE(edges[0] == 999);
}

TEST_CASE("edge_descriptor_view - const container with pairs", "[edge_descriptor_view][const]") {
  const std::vector<std::pair<int, double>> edges    = {{10, 1.5}, {20, 2.5}, {30, 3.5}};
  std::vector<int>                          vertices = {100, 200, 300};

  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;

  VD source{1};

  // Construct view from const container
  edge_descriptor_view view{edges, source};

  // The view should deduce const_iterator
  using ViewType = decltype(view);
  using IterType = typename ViewType::edge_desc::edge_iterator_type;
  static_assert(std::is_same_v<IterType, std::vector<std::pair<int, double>>::const_iterator>,
                "Should deduce const_iterator for const container");

  // Iterate and verify we can access target IDs
  std::vector<int> target_ids;
  for (auto e : view) {
    target_ids.push_back(e.target_id(edges));
  }

  REQUIRE(target_ids.size() == 3);
  REQUIRE(target_ids[0] == 10);
  REQUIRE(target_ids[1] == 20);
  REQUIRE(target_ids[2] == 30);

  // Verify we can call inner_value with const container
  auto        e      = *view.begin();
  const auto& weight = e.inner_value(edges);
  REQUIRE(weight == 1.5);
}

TEST_CASE("edge_descriptor_view - non-const container with pairs", "[edge_descriptor_view][const]") {
  std::vector<std::pair<int, double>> edges    = {{10, 1.5}, {20, 2.5}, {30, 3.5}};
  std::vector<int>                    vertices = {100, 200, 300};

  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;

  VD source{2};

  // Construct view from non-const container
  edge_descriptor_view view{edges, source};

  // The view should deduce non-const iterator
  using ViewType = decltype(view);
  using IterType = typename ViewType::edge_desc::edge_iterator_type;
  static_assert(std::is_same_v<IterType, std::vector<std::pair<int, double>>::iterator>,
                "Should deduce iterator for non-const container");

  // Verify we can modify through the descriptor
  auto e               = *view.begin();
  e.inner_value(edges) = 9.9; // Modify the weight (second element of pair)

  REQUIRE(edges[0].second == 9.9);
}

TEST_CASE("edge_descriptor_view - const vs non-const distinction", "[edge_descriptor_view][const]") {
  std::vector<int>       mutable_edges = {1, 2, 3};
  const std::vector<int> const_edges   = {4, 5, 6};
  std::vector<int>       vertices      = {100, 200};

  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;

  VD source{0};

  edge_descriptor_view mutable_view{mutable_edges, source};
  edge_descriptor_view const_view{const_edges, source};

  // These should be different types
  using MutableViewType = decltype(mutable_view);
  using ConstViewType   = decltype(const_view);

  static_assert(!std::is_same_v<MutableViewType, ConstViewType>,
                "Views from const and non-const containers should be different types");

  // Verify iterator types are different
  using MutableIter = typename MutableViewType::edge_desc::edge_iterator_type;
  using ConstIter   = typename ConstViewType::edge_desc::edge_iterator_type;

  static_assert(std::is_same_v<MutableIter, std::vector<int>::iterator>);
  static_assert(std::is_same_v<ConstIter, std::vector<int>::const_iterator>);
}

TEST_CASE("edge_descriptor_view - const with list container", "[edge_descriptor_view][const]") {
  const std::list<std::pair<int, double>> edges    = {{5, 1.1}, {10, 2.2}, {15, 3.3}};
  std::vector<int>                        vertices = {100, 200, 300};

  using VertexIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VertexIter>;

  VD source{0};

  // Construct view from const list
  edge_descriptor_view view{edges, source};

  // Should deduce const_iterator for list
  using ViewType = decltype(view);
  using IterType = typename ViewType::edge_desc::edge_iterator_type;
  static_assert(std::is_same_v<IterType, std::list<std::pair<int, double>>::const_iterator>,
                "Should deduce const_iterator for const list");

  // Verify iteration works
  int count = 0;
  for (auto e : view) {
    REQUIRE(e.source().value() == 0);
    count++;
  }
  REQUIRE(count == 3);

  // Verify we can access but not modify
  auto        e      = *view.begin();
  const auto& weight = e.inner_value(edges);
  REQUIRE(weight == 1.1);
}
