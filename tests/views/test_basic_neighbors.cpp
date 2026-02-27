/**
 * @file test_basic_neighbors.cpp
 * @brief Tests for basic_neighbors view
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/neighbors.hpp>
#include <graph/container/undirected_adjacency_list.hpp>

#include <vector>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <algorithm>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// =============================================================================
// basic_neighbors — target_id only
// =============================================================================

TEST_CASE("basic_neighbors - empty vertex", "[basic_neighbors][empty]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{}, {0}};

  SECTION("no value function") {
    auto nbrs = basic_neighbors(g, std::size_t(0));

    REQUIRE(std::ranges::distance(nbrs) == 0);
    REQUIRE(nbrs.begin() == nbrs.end());
  }

  SECTION("with value function") {
    auto nbrs = basic_neighbors(g, std::size_t(0), [](const auto&, auto) { return 0; });

    REQUIRE(std::ranges::distance(nbrs) == 0);
    REQUIRE(nbrs.begin() == nbrs.end());
  }
}

TEST_CASE("basic_neighbors - single neighbor", "[basic_neighbors][single]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {0}};

  SECTION("no value function - yields target_id only") {
    auto nbrs = basic_neighbors(g, std::size_t(0));

    std::vector<std::size_t> targets;
    for (auto [tid] : nbrs) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{1});
  }

  SECTION("with value function") {
    auto nbrs = basic_neighbors(g, std::size_t(0),
                                [](const auto&, auto v) { return static_cast<int>(v.vertex_id()) * 10; });

    auto it = nbrs.begin();
    REQUIRE(it != nbrs.end());
    auto [tid, val] = *it;
    REQUIRE(tid == 1);
    REQUIRE(val == 10);
  }
}

TEST_CASE("basic_neighbors - multiple neighbors", "[basic_neighbors][multiple]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2, 3}, // vertex 0 → neighbors 1, 2, 3
        {0, 2},    // vertex 1 → neighbors 0, 2
        {},        // vertex 2 → no neighbors
        {0}        // vertex 3 → neighbor 0
  };

  SECTION("vertex 0 - three neighbors") {
    auto nbrs = basic_neighbors(g, std::size_t(0));

    std::vector<std::size_t> targets;
    for (auto [tid] : nbrs) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{1, 2, 3});
  }

  SECTION("vertex 1 - two neighbors") {
    auto nbrs = basic_neighbors(g, std::size_t(1));

    std::vector<std::size_t> targets;
    for (auto [tid] : nbrs) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{0, 2});
  }

  SECTION("vertex 2 - no neighbors") {
    auto nbrs = basic_neighbors(g, std::size_t(2));

    REQUIRE(std::ranges::distance(nbrs) == 0);
  }

  SECTION("with value function") {
    auto nbrs = basic_neighbors(g, std::size_t(0),
                                [](const auto&, auto v) { return static_cast<int>(v.vertex_id()) * 10; });

    std::vector<int> values;
    for (auto [tid, val] : nbrs) {
      values.push_back(val);
    }

    REQUIRE(values == std::vector<int>{10, 20, 30});
  }
}

TEST_CASE("basic_neighbors - info_type has no vertex field", "[basic_neighbors][info]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexIdType = vertex_id_t<Graph>;

  SECTION("no value function - info type") {
    using ViewType = basic_neighbors_view<Graph, void>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::value_type>);
  }

  SECTION("with value function - info type") {
    auto  vvf      = [](const auto&, auto) { return 42; };
    using VVFType  = decltype(vvf);
    using ViewType = basic_neighbors_view<Graph, VVFType>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::vertex_type>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::value_type, int>);
  }
}

TEST_CASE("basic_neighbors - range concepts", "[basic_neighbors][concepts]") {
  using Graph       = std::vector<std::vector<int>>;
  using ViewNoVVF   = basic_neighbors_view<Graph, void>;
  using ViewWithVVF = basic_neighbors_view<Graph, decltype([](const auto&, auto) { return 0; })>;

  SECTION("input_range satisfied") {
    STATIC_REQUIRE(std::ranges::input_range<ViewNoVVF>);
    STATIC_REQUIRE(std::ranges::input_range<ViewWithVVF>);
  }

  SECTION("forward_range satisfied") {
    STATIC_REQUIRE(std::ranges::forward_range<ViewNoVVF>);
    STATIC_REQUIRE(std::ranges::forward_range<ViewWithVVF>);
  }

  SECTION("view satisfied") {
    STATIC_REQUIRE(std::ranges::view<ViewNoVVF>);
    STATIC_REQUIRE(std::ranges::view<ViewWithVVF>);
  }
}

TEST_CASE("basic_neighbors - deque-based graph", "[basic_neighbors][deque]") {
  using Graph = std::deque<std::deque<int>>;
  Graph g     = {{1, 2}, {0}, {0, 1}};

  SECTION("no value function") {
    auto nbrs = basic_neighbors(g, std::size_t(0));

    std::vector<std::size_t> targets;
    for (auto [tid] : nbrs) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{1, 2});
  }

  SECTION("with value function") {
    auto nbrs = basic_neighbors(g, std::size_t(2),
                                [](const auto&, auto v) { return static_cast<int>(v.vertex_id()); });

    std::vector<int> targets;
    for (auto [tid, val] : nbrs) {
      targets.push_back(val);
    }

    REQUIRE(targets == std::vector<int>{0, 1});
  }
}

TEST_CASE("basic_neighbors - const graph", "[basic_neighbors][const]") {
  using Graph   = std::vector<std::vector<int>>;
  const Graph g = {{1, 2}, {0}, {}};

  SECTION("no value function") {
    auto nbrs = basic_neighbors(g, std::size_t(0));

    std::vector<std::size_t> targets;
    for (auto [tid] : nbrs) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{1, 2});
  }
}

TEST_CASE("basic_neighbors - iterator properties", "[basic_neighbors][iterator]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2, 3}, {}, {}};

  SECTION("pre-increment") {
    auto nbrs = basic_neighbors(g, std::size_t(0));
    auto it   = nbrs.begin();

    REQUIRE((*it).target_id == 1);
    ++it;
    REQUIRE((*it).target_id == 2);
    ++it;
    REQUIRE((*it).target_id == 3);
    ++it;
    REQUIRE(it == nbrs.end());
  }

  SECTION("post-increment") {
    auto nbrs = basic_neighbors(g, std::size_t(0));
    auto it   = nbrs.begin();

    auto old = it++;
    REQUIRE((*old).target_id == 1);
    REQUIRE((*it).target_id == 2);
  }

  SECTION("equality comparison") {
    auto nbrs = basic_neighbors(g, std::size_t(0));
    auto it1  = nbrs.begin();
    auto it2  = nbrs.begin();

    REQUIRE(it1 == it2);
    ++it1;
    REQUIRE(it1 != it2);
    ++it2;
    REQUIRE(it1 == it2);
  }

  SECTION("default constructed iterators are equal") {
    using Iter = decltype(basic_neighbors(g, std::size_t(0)).begin());
    Iter it1;
    Iter it2;
    REQUIRE(it1 == it2);
  }
}

TEST_CASE("basic_neighbors - value function types", "[basic_neighbors][vvf]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {0}, {}};

  SECTION("returning string") {
    auto nbrs = basic_neighbors(g, std::size_t(0),
                                [](const auto&, auto v) { return "neighbor_" + std::to_string(v.vertex_id()); });

    std::vector<std::string> names;
    for (auto [tid, name] : nbrs) {
      names.push_back(name);
    }

    REQUIRE(names == std::vector<std::string>{"neighbor_1", "neighbor_2"});
  }

  SECTION("returning double") {
    auto nbrs = basic_neighbors(g, std::size_t(0),
                                [](const auto&, auto v) { return static_cast<double>(v.vertex_id()) * 1.5; });

    std::vector<double> values;
    for (auto [tid, val] : nbrs) {
      values.push_back(val);
    }

    REQUIRE(values[0] == 1.5);
    REQUIRE(values[1] == 3.0);
  }
}

// =============================================================================
// basic_neighbors with undirected_adjacency_list
// =============================================================================

TEST_CASE("basic_neighbors - undirected_adjacency_list", "[basic_neighbors][undirected]") {
  using Graph = graph::container::undirected_adjacency_list<int, int>;
  Graph g;

  g.create_vertex(100);
  g.create_vertex(200);
  g.create_vertex(300);
  g.create_edge(0, 1, 10);
  g.create_edge(0, 2, 20);
  g.create_edge(1, 2, 12);

  SECTION("basic_neighbors(g, uid) - basic iteration") {
    auto nbrs = basic_neighbors(g, 0u);
    REQUIRE(nbrs.size() == 2);

    std::set<unsigned int> targets;
    for (auto [tid] : nbrs) {
      targets.insert(tid);
    }
    REQUIRE(targets.count(1) == 1);
    REQUIRE(targets.count(2) == 1);
  }

  SECTION("basic_neighbors(g, uid, vvf) - with value function") {
    auto nbrs = basic_neighbors(g, 0u, [](const auto& g, auto v) { return vertex_value(g, v); });

    std::vector<int> values;
    for (auto [tid, val] : nbrs) {
      values.push_back(val);
    }

    REQUIRE(values.size() == 2);
    std::sort(values.begin(), values.end());
    REQUIRE(values[0] == 200);
    REQUIRE(values[1] == 300);
  }

  SECTION("basic_neighbors(g, uid) from different vertices") {
    auto nbrs1 = basic_neighbors(g, 1u);
    REQUIRE(nbrs1.size() == 2);

    auto nbrs2 = basic_neighbors(g, 2u);
    REQUIRE(nbrs2.size() == 2);
  }
}

// =============================================================================
// Verify return types match goal specification
// =============================================================================

TEST_CASE("neighbors - return type verification", "[neighbors][return_type]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexIdType = vertex_id_t<Graph>;
  using VertexType   = vertex_t<Graph>;

  Graph g  = {{1, 2}, {0}, {}};
  auto  v0 = *find_vertex(g, std::size_t(0));

  SECTION("neighbors(g, u) returns neighbor_data<VId, false, V, void>") {
    auto      nbrs = neighbors(g, v0);
    using ActualInfo = decltype(*nbrs.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::vertex_type, VertexType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::value_type>);
  }

  SECTION("neighbors(g, u, vvf) returns neighbor_data<VId, false, V, VV>") {
    auto nbrs = neighbors(g, v0, [](const auto&, auto) { return 42; });
    using ActualInfo = decltype(*nbrs.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::vertex_type, VertexType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::value_type, int>);
  }

  SECTION("basic_neighbors(g, uid) returns neighbor_data<VId, false, void, void>") {
    auto      nbrs = basic_neighbors(g, std::size_t(0));
    using ActualInfo = decltype(*nbrs.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::value_type>);
  }

  SECTION("basic_neighbors(g, uid, vvf) returns neighbor_data<VId, false, void, VV>") {
    auto nbrs = basic_neighbors(g, std::size_t(0), [](const auto&, auto) { return 42; });
    using ActualInfo = decltype(*nbrs.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::vertex_type>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::value_type, int>);
  }
}
