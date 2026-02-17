/**
 * @file test_basic_edgelist.cpp
 * @brief Tests for basic_edgelist view
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/edgelist.hpp>
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
// basic_edgelist — source_id + target_id only
// =============================================================================

TEST_CASE("basic_edgelist - empty graph", "[basic_edgelist][empty]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g;

  SECTION("no value function") {
    auto el = basic_edgelist(g);

    REQUIRE(el.begin() == el.end());

    std::size_t count = 0;
    for ([[maybe_unused]] auto ei : el) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("with value function") {
    auto el = basic_edgelist(g, [](const auto&, auto) { return 0; });

    REQUIRE(el.begin() == el.end());
  }
}

TEST_CASE("basic_edgelist - vertices with no edges", "[basic_edgelist][empty]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{}, {}, {}};

  SECTION("no value function") {
    auto el = basic_edgelist(g);
    REQUIRE(el.begin() == el.end());
  }

  SECTION("with value function") {
    auto el = basic_edgelist(g, [](const auto&, auto) { return 42; });
    REQUIRE(el.begin() == el.end());
  }
}

TEST_CASE("basic_edgelist - single edge", "[basic_edgelist][single]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {}};

  SECTION("no value function - yields source_id + target_id only") {
    auto el = basic_edgelist(g);

    auto it = el.begin();
    REQUIRE(it != el.end());

    auto [sid, tid] = *it;
    REQUIRE(sid == 0);
    REQUIRE(tid == 1);

    ++it;
    REQUIRE(it == el.end());
  }

  SECTION("with value function") {
    auto el = basic_edgelist(g, [](const auto& g, auto e) {
      return static_cast<int>(adj_list::target_id(g, e)) * 10;
    });

    auto it = el.begin();
    REQUIRE(it != el.end());
    auto [sid, tid, val] = *it;
    REQUIRE(sid == 0);
    REQUIRE(tid == 1);
    REQUIRE(val == 10);

    ++it;
    REQUIRE(it == el.end());
  }
}

TEST_CASE("basic_edgelist - multiple edges", "[basic_edgelist][multiple]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2},    // vertex 0 → edges to 1, 2
        {2},       // vertex 1 → edge to 2
        {}         // vertex 2 → no edges
  };

  SECTION("no value function - collects all edges") {
    auto el = basic_edgelist(g);

    std::vector<std::pair<std::size_t, std::size_t>> edges;
    for (auto [sid, tid] : el) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
    REQUIRE(edges[0] == std::pair<std::size_t, std::size_t>{0, 1});
    REQUIRE(edges[1] == std::pair<std::size_t, std::size_t>{0, 2});
    REQUIRE(edges[2] == std::pair<std::size_t, std::size_t>{1, 2});
  }

  SECTION("with value function") {
    auto el = basic_edgelist(g, [](const auto& g, auto e) {
      return static_cast<int>(adj_list::target_id(g, e)) * 10;
    });

    std::vector<int> values;
    for (auto [sid, tid, val] : el) {
      values.push_back(val);
    }

    REQUIRE(values == std::vector<int>{10, 20, 20});
  }
}

TEST_CASE("basic_edgelist - skips empty vertices", "[basic_edgelist][skip]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {},     // vertex 0 → no edges
        {},     // vertex 1 → no edges
        {0, 1}, // vertex 2 → edges to 0, 1
        {},     // vertex 3 → no edges
        {3}     // vertex 4 → edge to 3
  };

  auto el = basic_edgelist(g);

  std::vector<std::pair<std::size_t, std::size_t>> edges;
  for (auto [sid, tid] : el) {
    edges.emplace_back(sid, tid);
  }

  REQUIRE(edges.size() == 3);
  REQUIRE(edges[0] == std::pair<std::size_t, std::size_t>{2, 0});
  REQUIRE(edges[1] == std::pair<std::size_t, std::size_t>{2, 1});
  REQUIRE(edges[2] == std::pair<std::size_t, std::size_t>{4, 3});
}

TEST_CASE("basic_edgelist - info_type has no edge field", "[basic_edgelist][info]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexIdType = vertex_id_t<Graph>;

  SECTION("no value function - info type") {
    using ViewType = basic_edgelist_view<Graph, void>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<typename InfoType::source_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::edge_type>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::value_type>);
  }

  SECTION("with value function - info type") {
    auto  evf      = [](const auto&, auto) { return 42; };
    using EVFType  = decltype(evf);
    using ViewType = basic_edgelist_view<Graph, EVFType>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<typename InfoType::source_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::edge_type>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::value_type, int>);
  }
}

TEST_CASE("basic_edgelist - range concepts", "[basic_edgelist][concepts]") {
  using Graph       = std::vector<std::vector<int>>;
  using ViewNoEVF   = basic_edgelist_view<Graph, void>;
  using ViewWithEVF = basic_edgelist_view<Graph, decltype([](const auto&, auto) { return 0; })>;

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

TEST_CASE("basic_edgelist - deque-based graph", "[basic_edgelist][deque]") {
  using Graph = std::deque<std::deque<int>>;
  Graph g     = {{1, 2}, {0}, {0, 1}};

  SECTION("no value function") {
    auto el = basic_edgelist(g);

    std::vector<std::pair<std::size_t, std::size_t>> edges;
    for (auto [sid, tid] : el) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 5);
    REQUIRE(edges[0] == std::pair<std::size_t, std::size_t>{0, 1});
    REQUIRE(edges[1] == std::pair<std::size_t, std::size_t>{0, 2});
    REQUIRE(edges[2] == std::pair<std::size_t, std::size_t>{1, 0});
    REQUIRE(edges[3] == std::pair<std::size_t, std::size_t>{2, 0});
    REQUIRE(edges[4] == std::pair<std::size_t, std::size_t>{2, 1});
  }

  SECTION("with value function") {
    auto el = basic_edgelist(g, [](const auto& g, auto e) {
      return static_cast<int>(adj_list::target_id(g, e));
    });

    std::vector<int> targets;
    for (auto [sid, tid, val] : el) {
      targets.push_back(val);
    }

    REQUIRE(targets == std::vector<int>{1, 2, 0, 0, 1});
  }
}

TEST_CASE("basic_edgelist - const graph", "[basic_edgelist][const]") {
  using Graph   = std::vector<std::vector<int>>;
  const Graph g = {{1, 2}, {0}, {}};

  SECTION("no value function") {
    auto el = basic_edgelist(g);

    std::vector<std::pair<std::size_t, std::size_t>> edges;
    for (auto [sid, tid] : el) {
      edges.emplace_back(sid, tid);
    }

    REQUIRE(edges.size() == 3);
    REQUIRE(edges[0] == std::pair<std::size_t, std::size_t>{0, 1});
    REQUIRE(edges[1] == std::pair<std::size_t, std::size_t>{0, 2});
    REQUIRE(edges[2] == std::pair<std::size_t, std::size_t>{1, 0});
  }
}

TEST_CASE("basic_edgelist - iterator properties", "[basic_edgelist][iterator]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {0}, {}};

  SECTION("pre-increment") {
    auto el = basic_edgelist(g);
    auto it = el.begin();

    auto [s0, t0] = *it;
    REQUIRE(s0 == 0);
    REQUIRE(t0 == 1);
    ++it;
    auto [s1, t1] = *it;
    REQUIRE(s1 == 0);
    REQUIRE(t1 == 2);
    ++it;
    auto [s2, t2] = *it;
    REQUIRE(s2 == 1);
    REQUIRE(t2 == 0);
    ++it;
    REQUIRE(it == el.end());
  }

  SECTION("post-increment") {
    auto el = basic_edgelist(g);
    auto it = el.begin();

    auto old       = it++;
    auto [s0, t0]  = *old;
    auto [s1, t1]  = *it;
    REQUIRE(s0 == 0);
    REQUIRE(t0 == 1);
    REQUIRE(s1 == 0);
    REQUIRE(t1 == 2);
  }

  SECTION("equality comparison") {
    auto el  = basic_edgelist(g);
    auto it1 = el.begin();
    auto it2 = el.begin();

    REQUIRE(it1 == it2);
    ++it1;
    REQUIRE(it1 != it2);
    ++it2;
    REQUIRE(it1 == it2);
  }

  SECTION("default constructed iterators are equal") {
    using Iter = decltype(basic_edgelist(g).begin());
    Iter it1;
    Iter it2;
    REQUIRE(it1 == it2);
  }
}

TEST_CASE("basic_edgelist - value function types", "[basic_edgelist][evf]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {0}, {}};

  SECTION("returning string") {
    auto el = basic_edgelist(g, [](const auto& g, auto e) {
      return std::to_string(adj_list::target_id(g, e));
    });

    std::vector<std::string> names;
    for (auto [sid, tid, name] : el) {
      names.push_back(name);
    }

    REQUIRE(names == std::vector<std::string>{"1", "2", "0"});
  }

  SECTION("returning double") {
    auto el = basic_edgelist(g, [](const auto& g, auto e) {
      return static_cast<double>(adj_list::target_id(g, e)) * 1.5;
    });

    std::vector<double> values;
    for (auto [sid, tid, val] : el) {
      values.push_back(val);
    }

    REQUIRE(values[0] == 1.5);
    REQUIRE(values[1] == 3.0);
    REQUIRE(values[2] == 0.0);
  }
}

// =============================================================================
// basic_edgelist with undirected_adjacency_list
// =============================================================================

TEST_CASE("basic_edgelist - undirected_adjacency_list", "[basic_edgelist][undirected]") {
  using Graph = graph::container::undirected_adjacency_list<int, int>;
  Graph g;

  g.create_vertex(100);
  g.create_vertex(200);
  g.create_vertex(300);
  g.create_edge(0, 1, 10);
  g.create_edge(0, 2, 20);
  g.create_edge(1, 2, 12);

  SECTION("basic_edgelist(g) - basic iteration") {
    auto el = basic_edgelist(g);

    std::set<std::pair<unsigned int, unsigned int>> edges;
    for (auto [sid, tid] : el) {
      edges.emplace(sid, tid);
    }

    // Undirected: each edge appears in both directions
    REQUIRE(edges.size() == 6);
    REQUIRE(edges.count({0, 1}) == 1);
    REQUIRE(edges.count({1, 0}) == 1);
    REQUIRE(edges.count({0, 2}) == 1);
    REQUIRE(edges.count({2, 0}) == 1);
    REQUIRE(edges.count({1, 2}) == 1);
    REQUIRE(edges.count({2, 1}) == 1);
  }

  SECTION("basic_edgelist(g, evf) - with value function") {
    auto el = basic_edgelist(g, [](const auto& g, auto e) { return edge_value(g, e); });

    std::vector<int> weights;
    for (auto [sid, tid, w] : el) {
      weights.push_back(w);
    }

    REQUIRE(weights.size() == 6);
    std::sort(weights.begin(), weights.end());
    // Each weight appears twice (undirected)
    REQUIRE(weights[0] == 10);
    REQUIRE(weights[1] == 10);
    REQUIRE(weights[2] == 12);
    REQUIRE(weights[3] == 12);
    REQUIRE(weights[4] == 20);
    REQUIRE(weights[5] == 20);
  }
}

// =============================================================================
// Verify return types match goal specification
// =============================================================================

TEST_CASE("edgelist - return type verification", "[edgelist][return_type]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexIdType = vertex_id_t<Graph>;
  using EdgeType     = edge_t<Graph>;

  Graph g = {{1, 2}, {0}, {}};

  SECTION("edgelist(g) returns edge_info<VId, true, E, void>") {
    auto      el = edgelist(g);
    using ActualInfo = decltype(*el.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::source_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::edge_type, EdgeType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::value_type>);
  }

  SECTION("edgelist(g, evf) returns edge_info<VId, true, E, EV>") {
    auto el = edgelist(g, [](const auto&, auto) { return 42; });
    using ActualInfo = decltype(*el.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::source_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::edge_type, EdgeType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::value_type, int>);
  }

  SECTION("basic_edgelist(g) returns edge_info<VId, true, void, void>") {
    auto      el = basic_edgelist(g);
    using ActualInfo = decltype(*el.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::source_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::edge_type>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::value_type>);
  }

  SECTION("basic_edgelist(g, evf) returns edge_info<VId, true, void, EV>") {
    auto el = basic_edgelist(g, [](const auto&, auto) { return 42; });
    using ActualInfo = decltype(*el.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::source_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::target_id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::edge_type>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::value_type, int>);
  }
}
