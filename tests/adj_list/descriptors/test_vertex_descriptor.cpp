/**
 * @file test_vertex_descriptor.cpp
 * @brief Comprehensive unit tests for vertex_descriptor and vertex_descriptor_view
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <graph/adj_list/vertex_descriptor.hpp>
#include <graph/adj_list/vertex_descriptor_view.hpp>

#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>

using namespace graph;
using namespace graph::adj_list;

// =============================================================================
// Random Access Iterator Tests (std::vector)
// =============================================================================

TEST_CASE("vertex_descriptor with random access iterator - vector<int>", "[vertex_descriptor][random_access]") {
  using VectorIter = std::vector<int>::iterator;
  using VD         = vertex_descriptor<VectorIter>;

  SECTION("Default construction") {
    VD vd;
    REQUIRE(vd.value() == 0);
    REQUIRE(vd.vertex_id() == 0);
  }

  SECTION("Construction from index") {
    VD vd{5};
    REQUIRE(vd.value() == 5);
    REQUIRE(vd.vertex_id() == 5);
  }

  SECTION("Copy semantics") {
    VD vd1{10};
    VD vd2 = vd1;
    REQUIRE(vd2.value() == 10);
    REQUIRE(vd2.vertex_id() == 10);

    VD vd3{20};
    vd3 = vd1;
    REQUIRE(vd3.value() == 10);
  }

  SECTION("Move semantics") {
    VD vd1{15};
    VD vd2 = std::move(vd1);
    REQUIRE(vd2.value() == 15);
  }

  SECTION("Pre-increment") {
    VD vd{5};
    ++vd;
    REQUIRE(vd.value() == 6);
    REQUIRE(vd.vertex_id() == 6);
  }

  SECTION("Post-increment") {
    VD vd{5};
    VD old = vd++;
    REQUIRE(old.value() == 5);
    REQUIRE(vd.value() == 6);
  }

  SECTION("Comparison operators") {
    VD vd1{5};
    VD vd2{10};
    VD vd3{5};

    REQUIRE(vd1 == vd3);
    REQUIRE(vd1 != vd2);
    REQUIRE(vd1 < vd2);
    REQUIRE(vd2 > vd1);
    REQUIRE(vd1 <= vd3);
    REQUIRE(vd1 >= vd3);
  }

  SECTION("Hash consistency") {
    VD vd1{42};
    VD vd2{42};
    VD vd3{43};

    std::hash<VD> hasher;
    REQUIRE(hasher(vd1) == hasher(vd2));
    REQUIRE(hasher(vd1) != hasher(vd3));
  }

  SECTION("Use in std::set") {
    std::set<VD> vd_set;
    vd_set.insert(VD{5});
    vd_set.insert(VD{3});
    vd_set.insert(VD{5}); // duplicate

    REQUIRE(vd_set.size() == 2);
    REQUIRE(vd_set.contains(VD{3}));
    REQUIRE(vd_set.contains(VD{5}));
  }

  SECTION("Use in std::unordered_map") {
    std::unordered_map<VD, std::string> vd_map;
    vd_map[VD{1}] = "one";
    vd_map[VD{2}] = "two";

    REQUIRE(vd_map.size() == 2);
    REQUIRE(vd_map[VD{1}] == "one");
    REQUIRE(vd_map[VD{2}] == "two");
  }
}

// =============================================================================
// Underlying Value Access Tests
// =============================================================================

TEST_CASE("vertex_descriptor::underlying_value() with vector", "[vertex_descriptor][underlying_value]") {
  std::vector<int> vertices = {100, 200, 300, 400, 500};
  using VectorIter          = std::vector<int>::iterator;
  using VD                  = vertex_descriptor<VectorIter>;

  SECTION("Access underlying value through descriptor") {
    VD vd{2};
    REQUIRE(vd.underlying_value(vertices) == 300);
  }

  SECTION("Modify underlying value through descriptor") {
    VD vd{3};
    vd.underlying_value(vertices) = 999;
    REQUIRE(vertices[3] == 999);
    REQUIRE(vd.underlying_value(vertices) == 999);
  }

  SECTION("Const access to underlying value") {
    const std::vector<int> const_vertices = {10, 20, 30};
    VD                     vd{1};
    REQUIRE(vd.underlying_value(const_vertices) == 20);
  }
}

TEST_CASE("vertex_descriptor::underlying_value() with map", "[vertex_descriptor][underlying_value]") {
  using MapType = std::map<int, std::string>;
  using MapIter = MapType::iterator;
  using VD      = vertex_descriptor<MapIter>;

  MapType vertex_map = {{10, "data_10"}, {20, "data_20"}, {30, "data_30"}};

  SECTION("Access underlying pair from map") {
    auto it = vertex_map.find(20);
    VD   vd{it};

    const auto& pair = vd.underlying_value(vertex_map);
    REQUIRE(pair.first == 20);
    REQUIRE(pair.second == "data_20");
  }

  SECTION("Modify value through underlying_value") {
    auto it = vertex_map.find(10);
    VD   vd{it};

    vd.underlying_value(vertex_map).second = "modified";
    REQUIRE(vertex_map[10] == "modified");
  }

  SECTION("Const access to map") {
    // For const access, we still use the non-const VD but with const container reference
    MapType test_map = {{1, "one"}, {2, "two"}};
    auto    it       = test_map.find(2);
    VD      vd{it};

    // Access through const reference to container
    const MapType& const_ref = test_map;
    const auto&    pair      = vd.underlying_value(const_ref);
    REQUIRE(pair.first == 2);
    REQUIRE(pair.second == "two");
  }
}

TEST_CASE("vertex_descriptor::underlying_value() with custom struct", "[vertex_descriptor][underlying_value]") {
  struct Vertex {
    int         id;
    std::string name;
    double      weight;
  };

  std::vector<Vertex> vertices = {{1, "A", 1.5}, {2, "B", 2.5}, {3, "C", 3.5}};

  using VectorIter = std::vector<Vertex>::iterator;
  using VD         = vertex_descriptor<VectorIter>;

  SECTION("Access struct members through underlying_value") {
    VD          vd{1};
    const auto& vertex = vd.underlying_value(vertices);

    REQUIRE(vertex.id == 2);
    REQUIRE(vertex.name == "B");
    REQUIRE(vertex.weight == 2.5);
  }

  SECTION("Modify struct through underlying_value") {
    VD vd{0};
    vd.underlying_value(vertices).name   = "Modified";
    vd.underlying_value(vertices).weight = 9.9;

    REQUIRE(vertices[0].name == "Modified");
    REQUIRE(vertices[0].weight == 9.9);
  }
}

// =============================================================================
// Inner Value Access Tests
// =============================================================================

TEST_CASE("vertex_descriptor::inner_value() with vector", "[vertex_descriptor][inner_value]") {
  std::vector<int> vertices = {100, 200, 300, 400};
  using VectorIter          = std::vector<int>::iterator;
  using VD                  = vertex_descriptor<VectorIter>;

  SECTION("For vectors, inner_value returns the whole value") {
    VD vd{2};
    REQUIRE(vd.inner_value(vertices) == 300);

    vd.inner_value(vertices) = 999;
    REQUIRE(vertices[2] == 999);
  }
}

TEST_CASE("vertex_descriptor::inner_value() with map", "[vertex_descriptor][inner_value]") {
  using MapType = std::map<int, std::string>;
  using MapIter = MapType::iterator;
  using VD      = vertex_descriptor<MapIter>;

  MapType vertex_map = {{10, "data_10"}, {20, "data_20"}, {30, "data_30"}};

  SECTION("For maps, inner_value returns .second (the value part)") {
    auto it = vertex_map.find(20);
    VD   vd{it};

    // inner_value returns reference to the value part (not the key)
    REQUIRE(vd.inner_value(vertex_map) == "data_20");
  }

  SECTION("Modify through inner_value") {
    auto it = vertex_map.find(10);
    VD   vd{it};

    vd.inner_value(vertex_map) = "modified";
    REQUIRE(vertex_map[10] == "modified");
    REQUIRE(vd.inner_value(vertex_map) == "modified");
  }

  SECTION("Const access") {
    MapType test_map = {{5, "five"}, {6, "six"}};
    auto    it       = test_map.find(6);
    VD      vd{it};

    const MapType& const_ref = test_map;
    const auto&    value     = vd.inner_value(const_ref);
    REQUIRE(value == "six");
  }
}

TEST_CASE("vertex_descriptor::inner_value() with custom struct in map", "[vertex_descriptor][inner_value]") {
  struct VertexData {
    std::string name;
    double      weight;
  };

  using MapType = std::map<int, VertexData>;
  using MapIter = MapType::iterator;
  using VD      = vertex_descriptor<MapIter>;

  MapType vertex_map = {{1, {"A", 1.5}}, {2, {"B", 2.5}}, {3, {"C", 3.5}}};

  SECTION("Access struct through inner_value (excludes key)") {
    auto it = vertex_map.find(2);
    VD   vd{it};

    auto&& data = vd.inner_value(vertex_map);
    REQUIRE(data.name == "B");
    REQUIRE(data.weight == 2.5);
  }

  SECTION("Modify struct members") {
    auto it = vertex_map.find(1);
    VD   vd{it};

    vd.inner_value(vertex_map).name   = "Modified";
    vd.inner_value(vertex_map).weight = 9.9;

    REQUIRE(vertex_map[1].name == "Modified");
    REQUIRE(vertex_map[1].weight == 9.9);
  }
}

// =============================================================================
// Bidirectional Iterator Tests (std::map)
// =============================================================================

TEST_CASE("vertex_descriptor with bidirectional iterator - map<int,string>", "[vertex_descriptor][bidirectional]") {
  using MapType = std::map<int, std::string>;
  using MapIter = MapType::iterator;
  using VD      = vertex_descriptor<MapIter>;

  MapType vertex_map = {{10, "vertex_10"}, {20, "vertex_20"}, {30, "vertex_30"}};

  SECTION("Construction from iterator") {
    auto it = vertex_map.begin();
    VD   vd{it};

    REQUIRE(vd.vertex_id() == 10);
    REQUIRE(vd.value() == it);
  }

  SECTION("vertex_id extracts key from pair") {
    auto it = vertex_map.find(20);
    VD   vd{it};

    REQUIRE(vd.vertex_id() == 20);
  }

  SECTION("Pre-increment advances iterator") {
    auto it = vertex_map.begin();
    VD   vd{it};
    REQUIRE(vd.vertex_id() == 10);

    ++vd;
    REQUIRE(vd.vertex_id() == 20);
  }

  SECTION("Post-increment") {
    auto it = vertex_map.begin();
    VD   vd{it};

    VD old = vd++;
    REQUIRE(old.vertex_id() == 10);
    REQUIRE(vd.vertex_id() == 20);
  }

  SECTION("Comparison operators") {
    auto it1 = vertex_map.begin();
    auto it2 = ++vertex_map.begin();

    VD vd1{it1};
    VD vd2{it2};
    VD vd3{it1};

    REQUIRE(vd1 == vd3);
    REQUIRE(vd1 != vd2);
  }

  SECTION("Hash consistency with map iterators") {
    auto it = vertex_map.find(20);
    VD   vd1{it};
    VD   vd2{it};

    std::hash<VD> hasher;
    REQUIRE(hasher(vd1) == hasher(vd2));
  }
}

// =============================================================================
// Vertex Descriptor View Tests - Random Access
// =============================================================================

TEST_CASE("vertex_descriptor_view with vector", "[vertex_descriptor_view][random_access]") {
  std::vector<int> vertices = {100, 200, 300, 400, 500};
  using VectorIter          = std::vector<int>::iterator;
  using View                = vertex_descriptor_view<VectorIter>;
  using VD                  = vertex_descriptor<VectorIter>;

  SECTION("Construction from container") {
    View view{vertices};

    REQUIRE(view.size() == 5);
    REQUIRE(!view.empty());
  }

  SECTION("Forward iteration yields descriptors") {
    View view{vertices};
    auto it = view.begin();

    VD vd0 = *it;
    REQUIRE(vd0.vertex_id() == 0);

    ++it;
    VD vd1 = *it;
    REQUIRE(vd1.vertex_id() == 1);
  }

  SECTION("Range-based for loop") {
    View                     view{vertices};
    std::vector<std::size_t> collected_ids;

    for (auto vd : view) {
      collected_ids.push_back(vd.vertex_id());
    }

    REQUIRE(collected_ids == std::vector<std::size_t>{0, 1, 2, 3, 4});
  }

  SECTION("std::ranges algorithms work") {
    View view{vertices};

    auto count = std::ranges::distance(view);
    REQUIRE(count == 5);

    // Find specific descriptor
    auto it = std::ranges::find_if(view, [](VD vd) { return vd.vertex_id() == 2; });

    REQUIRE(it != view.end());
    REQUIRE((*it).vertex_id() == 2);
  }

  SECTION("View satisfies forward_range") {
    View view{vertices};
    static_assert(std::ranges::forward_range<View>);
    static_assert(std::ranges::view<View>);
  }

  SECTION("Iterator value_type is vertex_descriptor") {
    View view{vertices};
    using IterValueType = typename View::iterator::value_type;
    static_assert(std::same_as<IterValueType, VD>);
  }

  SECTION("Empty view") {
    std::vector<int> empty_vec;
    View             view{empty_vec};

    REQUIRE(view.size() == 0);
    REQUIRE(view.empty());
    REQUIRE(view.begin() == view.end());
  }
}

// =============================================================================
// Vertex Descriptor View Tests - Bidirectional
// =============================================================================

TEST_CASE("vertex_descriptor_view with map", "[vertex_descriptor_view][bidirectional]") {
  std::map<int, std::string> vertex_map = {{5, "A"}, {10, "B"}, {15, "C"}};

  using MapIter = std::map<int, std::string>::iterator;
  using View    = vertex_descriptor_view<MapIter>;
  using VD      = vertex_descriptor<MapIter>;

  SECTION("Construction from map") {
    View view{vertex_map};

    REQUIRE(!view.empty());
  }

  SECTION("Forward iteration yields descriptors with correct keys") {
    View view{vertex_map};
    auto it = view.begin();

    VD vd0 = *it;
    REQUIRE(vd0.vertex_id() == 5);

    ++it;
    VD vd1 = *it;
    REQUIRE(vd1.vertex_id() == 10);

    ++it;
    VD vd2 = *it;
    REQUIRE(vd2.vertex_id() == 15);
  }

  SECTION("Range-based for loop collects keys") {
    View             view{vertex_map};
    std::vector<int> collected_keys;

    for (auto vd : view) {
      collected_keys.push_back(vd.vertex_id());
    }

    REQUIRE(collected_keys == std::vector<int>{5, 10, 15});
  }

  SECTION("View satisfies forward_range") {
    View view{vertex_map};
    static_assert(std::ranges::forward_range<View>);
    static_assert(std::ranges::view<View>);
  }

  SECTION("Works with std::ranges algorithms") {
    View view{vertex_map};

    auto it = std::ranges::find_if(view, [](VD vd) { return vd.vertex_id() == 10; });

    REQUIRE(it != view.end());
    REQUIRE((*it).vertex_id() == 10);
  }
}

// =============================================================================
// Type Safety Tests
// =============================================================================

TEST_CASE("Type safety - vertex descriptors from different containers", "[vertex_descriptor][type_safety]") {
  using VectorDesc = vertex_descriptor<std::vector<int>::iterator>;
  using MapDesc    = vertex_descriptor<std::map<int, int>::iterator>;

  // These types should be distinct
  static_assert(!std::same_as<VectorDesc, MapDesc>);

  SECTION("Cannot accidentally mix descriptor types") {
    VectorDesc vd_vec{5};
    // MapDesc vd_map = vd_vec; // Would not compile - types are distinct

    SUCCEED("Types are properly distinct");
  }
}

// =============================================================================
// Const Semantics Tests
// =============================================================================

TEST_CASE("vertex_descriptor_view - const container with vector", "[vertex_descriptor_view][const]") {
  const std::vector<int> data = {10, 20, 30};

  // Construct view from const container
  vertex_descriptor_view view{data};

  // The view should deduce const_iterator
  using ViewType = decltype(view);
  using IterType = typename ViewType::vertex_desc::iterator_type;
  static_assert(std::is_same_v<IterType, std::vector<int>::const_iterator>,
                "Should deduce const_iterator for const container");

  // Iterate and verify we can access values
  std::vector<int> ids;
  for (auto v : view) {
    ids.push_back(static_cast<int>(v.vertex_id()));
  }

  REQUIRE(ids.size() == 3);
  REQUIRE(ids[0] == 0);
  REQUIRE(ids[1] == 1);
  REQUIRE(ids[2] == 2);

  // Verify we can call underlying_value with const container
  auto        v   = *view.begin();
  const auto& val = v.underlying_value(data);
  REQUIRE(val == 10);

  // This should not compile (const correctness):
  // auto& mutable_val = v.underlying_value(data); // Error: binding to const
}

TEST_CASE("vertex_descriptor_view - non-const container with vector", "[vertex_descriptor_view][const]") {
  std::vector<int> data = {10, 20, 30};

  // Construct view from non-const container
  vertex_descriptor_view view{data};

  // The view should deduce non-const iterator
  using ViewType = decltype(view);
  using IterType = typename ViewType::vertex_desc::iterator_type;
  static_assert(std::is_same_v<IterType, std::vector<int>::iterator>, "Should deduce iterator for non-const container");

  // Verify we can modify through the descriptor
  auto v                   = *view.begin();
  v.underlying_value(data) = 100;

  REQUIRE(data[0] == 100);
}

TEST_CASE("vertex_descriptor_view - const container with map", "[vertex_descriptor_view][const]") {
  const std::map<int, std::string> data = {{100, "A"}, {200, "B"}, {300, "C"}};

  // Construct view from const container
  vertex_descriptor_view view{data};

  // The view should deduce const_iterator
  using ViewType = decltype(view);
  using IterType = typename ViewType::vertex_desc::iterator_type;
  static_assert(std::is_same_v<IterType, std::map<int, std::string>::const_iterator>,
                "Should deduce const_iterator for const map");

  // Iterate and verify we can access keys
  std::vector<int> ids;
  for (auto v : view) {
    ids.push_back(v.vertex_id());
  }

  REQUIRE(ids.size() == 3);
  REQUIRE(ids[0] == 100);
  REQUIRE(ids[1] == 200);
  REQUIRE(ids[2] == 300);

  // Verify we can call inner_value with const container
  auto        v   = *view.begin();
  const auto& val = v.inner_value(data);
  REQUIRE(val == "A");
}

TEST_CASE("vertex_descriptor_view - non-const container with map", "[vertex_descriptor_view][const]") {
  std::map<int, std::string> data = {{100, "A"}, {200, "B"}, {300, "C"}};

  // Construct view from non-const container
  vertex_descriptor_view view{data};

  // The view should deduce non-const iterator
  using ViewType = decltype(view);
  using IterType = typename ViewType::vertex_desc::iterator_type;
  static_assert(std::is_same_v<IterType, std::map<int, std::string>::iterator>,
                "Should deduce iterator for non-const map");

  // Verify we can modify through the descriptor
  auto v              = *view.begin();
  v.inner_value(data) = "Modified";

  REQUIRE(data[100] == "Modified");
}

TEST_CASE("vertex_descriptor_view - const vs non-const distinction", "[vertex_descriptor_view][const]") {
  std::vector<int>       mutable_data = {1, 2, 3};
  const std::vector<int> const_data   = {4, 5, 6};

  vertex_descriptor_view mutable_view{mutable_data};
  vertex_descriptor_view const_view{const_data};

  // These should be different types
  using MutableViewType = decltype(mutable_view);
  using ConstViewType   = decltype(const_view);

  static_assert(!std::is_same_v<MutableViewType, ConstViewType>,
                "Views from const and non-const containers should be different types");

  // Verify iterator types are different
  using MutableIter = typename MutableViewType::vertex_desc::iterator_type;
  using ConstIter   = typename ConstViewType::vertex_desc::iterator_type;

  static_assert(std::is_same_v<MutableIter, std::vector<int>::iterator>);
  static_assert(std::is_same_v<ConstIter, std::vector<int>::const_iterator>);
}
