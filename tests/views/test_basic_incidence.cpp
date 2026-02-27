/**
 * @file test_basic_incidence.cpp
 * @brief Tests for basic_incidence view
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/incidence.hpp>
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
// basic_incidence — target_id only
// =============================================================================

TEST_CASE("basic_incidence - empty vertex", "[basic_incidence][empty]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{}, {0}};

  SECTION("no value function") {
    auto inc = basic_incidence(g, std::size_t(0));

    REQUIRE(std::ranges::distance(inc) == 0);
    REQUIRE(inc.begin() == inc.end());
  }

  SECTION("with value function") {
    auto inc = basic_incidence(g, std::size_t(0), [](const auto&, auto) { return 0; });

    REQUIRE(std::ranges::distance(inc) == 0);
    REQUIRE(inc.begin() == inc.end());
  }
}

TEST_CASE("basic_incidence - single edge", "[basic_incidence][single]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {0}};

  SECTION("no value function - yields target_id only") {
    auto inc = basic_incidence(g, std::size_t(0));

    std::vector<std::size_t> targets;
    for (auto [tid] : inc) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{1});
  }

  SECTION("with value function") {
    auto inc = basic_incidence(g, std::size_t(0),
                               [](const auto& g, auto e) { return static_cast<int>(adj_list::target_id(g, e)); });

    auto it = inc.begin();
    REQUIRE(it != inc.end());
    auto [tid, val] = *it;
    REQUIRE(tid == 1);
  }
}

TEST_CASE("basic_incidence - multiple edges", "[basic_incidence][multiple]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2, 3}, // vertex 0 → edges to 1, 2, 3
        {0, 2},    // vertex 1 → edges to 0, 2
        {},        // vertex 2 → no edges
        {0}        // vertex 3 → edge to 0
  };

  SECTION("vertex 0 - three edges") {
    auto inc = basic_incidence(g, std::size_t(0));

    std::vector<std::size_t> targets;
    for (auto [tid] : inc) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{1, 2, 3});
  }

  SECTION("vertex 1 - two edges") {
    auto inc = basic_incidence(g, std::size_t(1));

    std::vector<std::size_t> targets;
    for (auto [tid] : inc) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{0, 2});
  }

  SECTION("vertex 2 - no edges") {
    auto inc = basic_incidence(g, std::size_t(2));

    REQUIRE(std::ranges::distance(inc) == 0);
  }

  SECTION("with value function") {
    auto inc = basic_incidence(g, std::size_t(0),
                               [](const auto& g, auto e) { return static_cast<int>(adj_list::target_id(g, e)) * 10; });

    std::vector<int> values;
    for (auto [tid, val] : inc) {
      values.push_back(val);
    }

    REQUIRE(values == std::vector<int>{10, 20, 30});
  }
}

TEST_CASE("basic_incidence - info_type has no edge field", "[basic_incidence][info]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexIdType = vertex_id_t<Graph>;
  using EdgeType     = edge_t<Graph>;

  SECTION("no value function - info type") {
    using ViewType = basic_incidence_view<Graph, void>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::edge_type>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::value_type>);
  }

  SECTION("with value function - info type") {
    auto  evf      = [](const auto&, auto) { return 42; };
    using EVFType  = decltype(evf);
    using ViewType = basic_incidence_view<Graph, EVFType>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::edge_type>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::value_type, int>);
  }
}

TEST_CASE("basic_incidence - range concepts", "[basic_incidence][concepts]") {
  using Graph       = std::vector<std::vector<int>>;
  using ViewNoEVF   = basic_incidence_view<Graph, void>;
  using ViewWithEVF = basic_incidence_view<Graph, decltype([](const auto&, auto) { return 0; })>;

  SECTION("input_range satisfied") {
    STATIC_REQUIRE(std::ranges::input_range<ViewNoEVF>);
    STATIC_REQUIRE(std::ranges::input_range<ViewWithEVF>);
  }

  SECTION("forward_range satisfied") {
    STATIC_REQUIRE(std::ranges::forward_range<ViewNoEVF>);
    STATIC_REQUIRE(std::ranges::forward_range<ViewWithEVF>);
  }

  SECTION("view satisfied") {
    STATIC_REQUIRE(std::ranges::view<ViewNoEVF>);
    STATIC_REQUIRE(std::ranges::view<ViewWithEVF>);
  }
}

TEST_CASE("basic_incidence - deque-based graph", "[basic_incidence][deque]") {
  using Graph = std::deque<std::deque<int>>;
  Graph g     = {{1, 2}, {0}, {0, 1}};

  SECTION("no value function") {
    auto inc = basic_incidence(g, std::size_t(0));

    std::vector<std::size_t> targets;
    for (auto [tid] : inc) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{1, 2});
  }

  SECTION("with value function") {
    auto inc = basic_incidence(g, std::size_t(2),
                               [](const auto& g, auto e) { return static_cast<int>(adj_list::target_id(g, e)); });

    std::vector<int> targets;
    for (auto [tid, val] : inc) {
      targets.push_back(val);
    }

    REQUIRE(targets == std::vector<int>{0, 1});
  }
}

TEST_CASE("basic_incidence - const graph", "[basic_incidence][const]") {
  using Graph   = std::vector<std::vector<int>>;
  const Graph g = {{1, 2}, {0}, {}};

  SECTION("no value function") {
    auto inc = basic_incidence(g, std::size_t(0));

    std::vector<std::size_t> targets;
    for (auto [tid] : inc) {
      targets.push_back(tid);
    }

    REQUIRE(targets == std::vector<std::size_t>{1, 2});
  }
}

TEST_CASE("basic_incidence - iterator properties", "[basic_incidence][iterator]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2, 3}, {}, {}};

  SECTION("pre-increment") {
    auto inc = basic_incidence(g, std::size_t(0));
    auto it  = inc.begin();

    REQUIRE((*it).target_id == 1);
    ++it;
    REQUIRE((*it).target_id == 2);
    ++it;
    REQUIRE((*it).target_id == 3);
    ++it;
    REQUIRE(it == inc.end());
  }

  SECTION("post-increment") {
    auto inc = basic_incidence(g, std::size_t(0));
    auto it  = inc.begin();

    auto old = it++;
    REQUIRE((*old).target_id == 1);
    REQUIRE((*it).target_id == 2);
  }

  SECTION("equality comparison") {
    auto inc = basic_incidence(g, std::size_t(0));
    auto it1 = inc.begin();
    auto it2 = inc.begin();

    REQUIRE(it1 == it2);
    ++it1;
    REQUIRE(it1 != it2);
    ++it2;
    REQUIRE(it1 == it2);
  }

  SECTION("default constructed iterators are equal") {
    using Iter = decltype(basic_incidence(g, std::size_t(0)).begin());
    Iter it1;
    Iter it2;
    REQUIRE(it1 == it2);
  }
}

TEST_CASE("basic_incidence - value function types", "[basic_incidence][evf]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {0}, {}};

  SECTION("returning string") {
    auto inc = basic_incidence(g, std::size_t(0),
                               [](const auto& g, auto e) { return "edge_to_" + std::to_string(adj_list::target_id(g, e)); });

    std::vector<std::string> names;
    for (auto [tid, name] : inc) {
      names.push_back(name);
    }

    REQUIRE(names == std::vector<std::string>{"edge_to_1", "edge_to_2"});
  }

  SECTION("returning double") {
    auto inc = basic_incidence(g, std::size_t(0),
                               [](const auto& g, auto e) { return static_cast<double>(adj_list::target_id(g, e)) * 1.5; });

    std::vector<double> values;
    for (auto [tid, val] : inc) {
      values.push_back(val);
    }

    REQUIRE(values[0] == 1.5);
    REQUIRE(values[1] == 3.0);
  }
}

// =============================================================================
// basic_incidence with undirected_adjacency_list
// =============================================================================

TEST_CASE("basic_incidence - undirected_adjacency_list", "[basic_incidence][undirected]") {
  using Graph = graph::container::undirected_adjacency_list<int, int>;
  Graph g;

  g.create_vertex(100);
  g.create_vertex(200);
  g.create_vertex(300);
  g.create_edge(0, 1, 10);
  g.create_edge(0, 2, 20);
  g.create_edge(1, 2, 12);

  SECTION("basic_incidence(g, uid) - basic iteration") {
    auto inc = basic_incidence(g, 0u);
    REQUIRE(inc.size() == 2);

    std::set<unsigned int> targets;
    for (auto [tid] : inc) {
      targets.insert(tid);
    }
    REQUIRE(targets.count(1) == 1);
    REQUIRE(targets.count(2) == 1);
  }

  SECTION("basic_incidence(g, uid, evf) - with value function") {
    auto inc = basic_incidence(g, 0u, [](const auto& g, auto e) { return edge_value(g, e); });

    std::vector<int> weights;
    for (auto [tid, w] : inc) {
      weights.push_back(w);
    }

    REQUIRE(weights.size() == 2);
    std::sort(weights.begin(), weights.end());
    REQUIRE(weights[0] == 10);
    REQUIRE(weights[1] == 20);
  }

  SECTION("basic_incidence(g, uid) from different vertices") {
    auto inc1 = basic_incidence(g, 1u);
    REQUIRE(inc1.size() == 2);

    auto inc2 = basic_incidence(g, 2u);
    REQUIRE(inc2.size() == 2);
  }
}

// =============================================================================
// Verify return types match goal specification
// =============================================================================

TEST_CASE("incidence - return type verification", "[incidence][return_type]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexIdType = vertex_id_t<Graph>;
  using VertexType   = vertex_t<Graph>;
  using EdgeType     = edge_t<Graph>;

  Graph g  = {{1, 2}, {0}, {}};
  auto  v0 = *find_vertex(g, std::size_t(0));

  SECTION("incidence(g, u) returns edge_data<VId, false, E, void>") {
    auto      inc = incidence(g, v0);
    using ActualInfo = decltype(*inc.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::edge_type, EdgeType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::value_type>);
  }

  SECTION("incidence(g, u, evf) returns edge_data<VId, false, E, EV>") {
    auto inc = incidence(g, v0, [](const auto&, auto) { return 42; });
    using ActualInfo = decltype(*inc.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::edge_type, EdgeType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::value_type, int>);
  }

  SECTION("basic_incidence(g, uid) returns edge_data<VId, false, void, void>") {
    auto      inc = basic_incidence(g, std::size_t(0));
    using ActualInfo = decltype(*inc.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::edge_type>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::value_type>);
  }

  SECTION("basic_incidence(g, uid, evf) returns edge_data<VId, false, void, EV>") {
    auto inc = basic_incidence(g, std::size_t(0), [](const auto&, auto) { return 42; });
    using ActualInfo = decltype(*inc.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::edge_type>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::value_type, int>);
  }
}
