/**
 * @file test_type_aliases.cpp
 * @brief Tests for graph type aliases
 */

#include <graph/adj_list/detail/graph_cpo.hpp>
#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <map>
#include <deque>
#include <type_traits>

using namespace graph;
using namespace graph::adj_list;

TEST_CASE("Type aliases - vertices(g) based types", "[type_aliases][vertices]") {
  SECTION("vector<vector<int>> - vertex_range_t") {
    using Graph  = std::vector<std::vector<int>>;
    using VRange = vertex_range_t<Graph>;

    // Should be a vertex_descriptor_view
    STATIC_REQUIRE(is_vertex_descriptor_view_v<VRange>);

    // Should be a forward_range
    STATIC_REQUIRE(std::ranges::forward_range<VRange>);
  }

  SECTION("vector<vector<int>> - vertex_iterator_t") {
    using Graph = std::vector<std::vector<int>>;
    using VIter = vertex_iterator_t<Graph>;

    // Should be an iterator
    STATIC_REQUIRE(std::input_or_output_iterator<VIter>);
    STATIC_REQUIRE(std::forward_iterator<VIter>);
  }

  SECTION("vector<vector<int>> - vertex_t") {
    using Graph = std::vector<std::vector<int>>;
    using V     = vertex_t<Graph>;

    // Should be a vertex_descriptor
    STATIC_REQUIRE(is_vertex_descriptor_v<V>);

    // Should have vertex_id() method
    Graph g     = {{1, 2}, {3}, {}};
    auto  verts = vertices(g);
    auto  it    = std::ranges::begin(verts);
    V     v     = *it;
    REQUIRE(v.vertex_id() == 0);
  }

  SECTION("map<int, vector<int>> - vertex_range_t") {
    using Graph  = std::map<int, std::vector<int>>;
    using VRange = vertex_range_t<Graph>;

    // Should be a vertex_descriptor_view
    STATIC_REQUIRE(is_vertex_descriptor_view_v<VRange>);

    // Should be a forward_range
    STATIC_REQUIRE(std::ranges::forward_range<VRange>);
  }

  SECTION("map<int, vector<int>> - vertex_t") {
    using Graph = std::map<int, std::vector<int>>;
    using V     = vertex_t<Graph>;

    // Should be a vertex_descriptor
    STATIC_REQUIRE(is_vertex_descriptor_v<V>);

    // Should have vertex_id() method that returns the key
    Graph g     = {{100, {200}}, {200, {300}}, {300, {}}};
    auto  verts = vertices(g);
    auto  it    = std::ranges::begin(verts);
    V     v     = *it;
    REQUIRE(v.vertex_id() == 100);
  }

  SECTION("deque<deque<int>> - all type aliases") {
    using Graph  = std::deque<std::deque<int>>;
    using VRange = vertex_range_t<Graph>;
    using VIter  = vertex_iterator_t<Graph>;
    using V      = vertex_t<Graph>;

    STATIC_REQUIRE(is_vertex_descriptor_view_v<VRange>);
    STATIC_REQUIRE(std::forward_iterator<VIter>);
    STATIC_REQUIRE(is_vertex_descriptor_v<V>);

    // Verify usage
    Graph  g     = {{1, 2}, {3, 4, 5}, {6}};
    VRange verts = vertices(g);
    REQUIRE(std::ranges::distance(verts) == 3);

    VIter it = std::ranges::begin(verts);
    V     v0 = *it++;
    V     v1 = *it++;
    V     v2 = *it;

    REQUIRE(v0.vertex_id() == 0);
    REQUIRE(v1.vertex_id() == 1);
    REQUIRE(v2.vertex_id() == 2);
  }
}

TEST_CASE("Type aliases - consistency across graph types", "[type_aliases][consistency]") {
  SECTION("vertex_t is consistent with iterator dereference") {
    using Graph = std::vector<std::vector<int>>;
    using V     = vertex_t<Graph>;
    using VIter = vertex_iterator_t<Graph>;

    // vertex_t should be the same as dereferencing vertex_iterator_t
    STATIC_REQUIRE(std::is_same_v<V, std::iter_value_t<VIter>>);
  }

  SECTION("vertex_iterator_t is consistent with range iterator") {
    using Graph  = std::vector<std::vector<int>>;
    using VRange = vertex_range_t<Graph>;
    using VIter  = vertex_iterator_t<Graph>;

    // vertex_iterator_t should be the same as the iterator of vertex_range_t
    STATIC_REQUIRE(std::is_same_v<VIter, std::ranges::iterator_t<VRange>>);
  }
}

TEST_CASE("Type aliases - with custom vertices() member", "[type_aliases][custom]") {
  struct CustomGraph {
    std::vector<int> data = {10, 20, 30};

    auto vertices() { return vertex_descriptor_view(data); }
  };

  SECTION("Custom graph type aliases") {
    using VRange = vertex_range_t<CustomGraph>;
    using VIter  = vertex_iterator_t<CustomGraph>;
    using V      = vertex_t<CustomGraph>;

    STATIC_REQUIRE(is_vertex_descriptor_view_v<VRange>);
    STATIC_REQUIRE(std::forward_iterator<VIter>);
    STATIC_REQUIRE(is_vertex_descriptor_v<V>);

    CustomGraph g;
    VRange      verts = vertices(g);
    REQUIRE(std::ranges::distance(verts) == 3);

    VIter it = std::ranges::begin(verts);
    V     v  = *it;
    REQUIRE(v.vertex_id() == 0);
  }
}

TEST_CASE("Type aliases - reference vs value types", "[type_aliases][references]") {
  SECTION("vertex_range_t returns view (not reference)") {
    using Graph  = std::vector<std::vector<int>>;
    using VRange = vertex_range_t<Graph>;

    // Should not be a reference type
    STATIC_REQUIRE(!std::is_reference_v<VRange>);

    // But should be a view
    STATIC_REQUIRE(std::ranges::view<VRange>);
  }

  SECTION("vertex_t is value type (not reference)") {
    using Graph = std::vector<std::vector<int>>;
    using V     = vertex_t<Graph>;

    // Should not be a reference
    STATIC_REQUIRE(!std::is_reference_v<V>);

    // Should be copyable
    STATIC_REQUIRE(std::copyable<V>);
  }
}

TEST_CASE("Type aliases - compile-time computation", "[type_aliases][constexpr]") {
  SECTION("All type aliases computed at compile time") {
    using Graph = std::vector<std::vector<int>>;

    // These should all be computable at compile time
    using VRange = vertex_range_t<Graph>;
    using VIter  = vertex_iterator_t<Graph>;
    using V      = vertex_t<Graph>;
    using VId    = vertex_id_t<Graph>;

    // Just having these compile proves they work at compile time
    STATIC_REQUIRE(sizeof(VRange) > 0);
    STATIC_REQUIRE(sizeof(VIter) > 0);
    STATIC_REQUIRE(sizeof(V) > 0);
    STATIC_REQUIRE(sizeof(VId) > 0);
  }
}

TEST_CASE("Type aliases - vertex_id(g,u) based types", "[type_aliases][vertex_id]") {
  SECTION("vector<vector<int>> - vertex_id_t is size_t") {
    using Graph = std::vector<std::vector<int>>;

    STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, size_t>);
  }

  SECTION("map<int, vector<int>> - vertex_id_t is int") {
    using Graph = std::map<int, std::vector<int>>;

    STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, int>);
  }

  SECTION("map<string, vector<int>> - vertex_id_t is string") {
    using Graph = std::map<std::string, std::vector<int>>;

    STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, std::string>);
  }

  SECTION("deque<deque<int>> - vertex_id_t is size_t") {
    using Graph = std::deque<std::deque<int>>;

    STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, size_t>);
  }

  SECTION("vertex_id_t matches actual return value") {
    using Graph = std::vector<std::vector<int>>;
    Graph g     = {{}};

    vertex_range_t<Graph> verts = vertices(g);
    vertex_t<Graph>       v     = *std::ranges::begin(verts);

    auto id = vertex_id(g, v);
    STATIC_REQUIRE(std::same_as<decltype(id), vertex_id_t<Graph>>);
    REQUIRE(id == 0);
  }
}

TEST_CASE("Type aliases - vertex_id_t with different key types", "[type_aliases][vertex_id]") {
  SECTION("map with unsigned key") {
    using Graph = std::map<unsigned int, std::vector<int>>;

    STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, unsigned int>);
  }

  SECTION("map with long key") {
    using Graph = std::map<long, std::vector<int>>;

    STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, long>);
  }

  SECTION("map with custom key type") {
    struct CustomKey {
      int  value;
      bool operator<(const CustomKey& other) const { return value < other.value; }
    };
    using Graph = std::map<CustomKey, std::vector<int>>;

    STATIC_REQUIRE(std::same_as<vertex_id_t<Graph>, CustomKey>);
  }
}
