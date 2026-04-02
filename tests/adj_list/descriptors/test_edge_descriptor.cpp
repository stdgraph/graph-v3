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

  // We need a live container for iterators
  std::vector<int> edge_data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120};

  SECTION("Default construction") {
    ED ed;
    // Default-constructed edge_descriptor exists; source is default-constructed
    REQUIRE(ed.source().value() == 0);
  }

  SECTION("Construction from edge iterator and source vertex") {
    VD source{5};
    ED ed{edge_data.begin() + 3, source};

    REQUIRE(ed.value() == edge_data.begin() + 3);
    REQUIRE(ed.source().value() == 5);
    REQUIRE(ed.source().vertex_id() == 5);
  }

  SECTION("Copy semantics") {
    VD source{10};
    ED ed1{edge_data.begin() + 7, source};
    ED ed2 = ed1;

    REQUIRE(ed2.value() == edge_data.begin() + 7);
    REQUIRE(ed2.source().value() == 10);

    ED ed3{edge_data.begin() + 1, VD{2}};
    ed3 = ed1;
    REQUIRE(ed3.value() == edge_data.begin() + 7);
    REQUIRE(ed3.source().value() == 10);
  }

  SECTION("Move semantics") {
    VD source{15};
    ED ed1{edge_data.begin() + 8, source};
    ED ed2 = std::move(ed1);

    REQUIRE(ed2.value() == edge_data.begin() + 8);
    REQUIRE(ed2.source().value() == 15);
  }

  SECTION("Pre-increment advances edge, keeps source") {
    VD source{5};
    ED ed{edge_data.begin() + 3, source};

    ++ed;
    REQUIRE(ed.value() == edge_data.begin() + 4);
    REQUIRE(ed.source().value() == 5); // Source unchanged
  }

  SECTION("Post-increment") {
    VD source{5};
    ED ed{edge_data.begin() + 3, source};

    ED old = ed++;
    REQUIRE(old.value() == edge_data.begin() + 3);
    REQUIRE(ed.value() == edge_data.begin() + 4);
    REQUIRE(ed.source().value() == 5);
  }

  SECTION("Comparison operators") {
    VD source1{5};
    VD source2{10};

    ED ed1{edge_data.begin() + 3, source1};
    ED ed2{edge_data.begin() + 7, source1};
    ED ed3{edge_data.begin() + 3, source1};
    ED ed4{edge_data.begin() + 3, source2}; // Same edge position, different source

    REQUIRE(ed1 == ed3);
    REQUIRE(ed1 != ed2);
    REQUIRE(ed1 != ed4); // Different source makes them different
    REQUIRE(ed1 < ed2);
    REQUIRE(ed2 > ed1);
  }

  SECTION("Hash consistency") {
    VD source{42};
    ED ed1{edge_data.begin() + 10, source};
    ED ed2{edge_data.begin() + 10, source};
    ED ed3{edge_data.begin() + 11, source};

    std::hash<ED> hasher;
    REQUIRE(hasher(ed1) == hasher(ed2));
    // Different edge position should produce different hash (usually)
  }

  SECTION("Use in std::set") {
    VD           source{5};
    std::set<ED> ed_set;

    ed_set.insert(ED{edge_data.begin() + 3, source});
    ed_set.insert(ED{edge_data.begin() + 1, source});
    ed_set.insert(ED{edge_data.begin() + 3, source}); // duplicate

    REQUIRE(ed_set.size() == 2);
  }

  SECTION("Use in std::unordered_map") {
    VD                                  source{5};
    std::unordered_map<ED, std::string> ed_map;

    ed_map[ED{edge_data.begin() + 1, source}] = "edge1";
    ed_map[ED{edge_data.begin() + 2, source}] = "edge2";

    REQUIRE(ed_map.size() == 2);
    REQUIRE(ed_map[ED{edge_data.begin() + 1, source}] == "edge1");
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
    REQUIRE(ed0.value() == edges_from_v5.begin());
    REQUIRE(ed0.source().value() == 5);

    ++it;
    ED ed1 = *it;
    REQUIRE(ed1.value() == edges_from_v5.begin() + 1);
    REQUIRE(ed1.source().value() == 5);
  }

  SECTION("Range-based for loop") {
    View view{edges_from_v5, source};
    std::size_t count = 0;

    for (auto ed : view) {
      REQUIRE(ed.value() == edges_from_v5.begin() + count);
      REQUIRE(ed.source().value() == 5); // All have same source
      ++count;
    }

    REQUIRE(count == 4);
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

    // Find edge at specific position
    auto target_it = edges_from_v5.begin() + 2;
    auto it = std::ranges::find_if(view, [&](ED ed) { return ed.value() == target_it; });

    REQUIRE(it != view.end());
    REQUIRE((*it).value() == target_it);
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

    auto edge_it = edges.begin();
    for (auto ed : view) {
      REQUIRE(ed.value() == edge_it);
      REQUIRE(ed.source().value() == 100);
      ++edge_it;
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
    VD                   source{5};
    std::vector<int>     vec = {10, 20, 30};
    VectorEdgeDesc ed_vec{vec.begin() + 1, source};
    // ListEdgeDesc ed_list = ed_vec; // Would not compile

    SUCCEED("Types are properly distinct");
  }
}
