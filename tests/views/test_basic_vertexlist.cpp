/**
 * @file test_basic_vertexlist.cpp
 * @brief Tests for basic_vertexlist view and vertexlist subrange overloads
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/views/vertexlist.hpp>

#include <vector>
#include <deque>
#include <map>
#include <string>

using namespace graph;
using namespace graph::views;
using namespace graph::adj_list;

// =============================================================================
// basic_vertexlist — id only
// =============================================================================

TEST_CASE("basic_vertexlist - empty graph", "[basic_vertexlist][empty]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g;

  SECTION("no value function") {
    auto vlist = basic_vertexlist(g);

    REQUIRE(vlist.size() == 0);
    REQUIRE(vlist.begin() == vlist.end());

    std::size_t count = 0;
    for ([[maybe_unused]] auto vi : vlist) {
      ++count;
    }
    REQUIRE(count == 0);
  }

  SECTION("with value function") {
    auto vlist = basic_vertexlist(g, [](const auto&, auto v) { return v.vertex_id(); });

    REQUIRE(vlist.size() == 0);
    REQUIRE(vlist.begin() == vlist.end());
  }
}

TEST_CASE("basic_vertexlist - single vertex", "[basic_vertexlist][single]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{}};

  SECTION("no value function") {
    auto vlist = basic_vertexlist(g);

    REQUIRE(vlist.size() == 1);

    auto it = vlist.begin();
    REQUIRE(it != vlist.end());

    auto vi = *it;
    REQUIRE(vi.id == 0);

    ++it;
    REQUIRE(it == vlist.end());
  }

  SECTION("structured binding - single field") {
    auto vlist = basic_vertexlist(g);
    for (auto [uid] : vlist) {
      REQUIRE(uid == 0);
    }
  }

  SECTION("with value function") {
    auto vlist = basic_vertexlist(g, [](const auto&, auto v) { return v.vertex_id() * 2; });

    REQUIRE(vlist.size() == 1);

    auto vi = *vlist.begin();
    REQUIRE(vi.id == 0);
    REQUIRE(vi.value == 0);
  }
}

TEST_CASE("basic_vertexlist - multiple vertices", "[basic_vertexlist][multiple]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {
        {1, 2}, // vertex 0
        {2, 3}, // vertex 1
        {3},    // vertex 2
        {}      // vertex 3
  };

  SECTION("no value function - yields ids only") {
    auto vlist = basic_vertexlist(g);

    REQUIRE(vlist.size() == 4);

    std::vector<std::size_t> ids;
    for (auto [uid] : vlist) {
      ids.push_back(uid);
    }

    REQUIRE(ids == std::vector<std::size_t>{0, 1, 2, 3});
  }

  SECTION("with value function") {
    auto vlist = basic_vertexlist(g, [](const auto&, auto v) { return static_cast<int>(v.vertex_id() * 10); });

    std::vector<int> values;
    for (auto [uid, val] : vlist) {
      REQUIRE(uid == static_cast<std::size_t>(val / 10));
      values.push_back(val);
    }

    REQUIRE(values == std::vector<int>{0, 10, 20, 30});
  }

  SECTION("structured binding - with value function") {
    auto vlist = basic_vertexlist(g, [](const auto& g, auto v) {
      return g[v.vertex_id()].size(); // number of edges
    });

    std::vector<std::size_t> edge_counts;
    for (auto [uid, count] : vlist) {
      edge_counts.push_back(count);
    }

    REQUIRE(edge_counts == std::vector<std::size_t>{2, 2, 1, 0});
  }
}

TEST_CASE("basic_vertexlist - info_type has no vertex field", "[basic_vertexlist][info]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexIdType = vertex_id_t<Graph>;

  SECTION("no value function - info type") {
    using ViewType = basic_vertexlist_view<Graph, void>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<typename InfoType::id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::value_type>);
  }

  SECTION("with value function - info type") {
    auto  vvf      = [](const auto&, auto) { return 42; };
    using VVFType  = decltype(vvf);
    using ViewType = basic_vertexlist_view<Graph, VVFType>;
    using InfoType = typename ViewType::info_type;

    STATIC_REQUIRE(std::is_same_v<typename InfoType::id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename InfoType::vertex_type>);
    STATIC_REQUIRE(std::is_same_v<typename InfoType::value_type, int>);
  }
}

TEST_CASE("basic_vertexlist - range concepts", "[basic_vertexlist][concepts]") {
  using Graph       = std::vector<std::vector<int>>;
  using ViewNoVVF   = basic_vertexlist_view<Graph, void>;
  using ViewWithVVF = basic_vertexlist_view<Graph, decltype([](const auto&, auto) { return 0; })>;

  SECTION("input_range satisfied") {
    STATIC_REQUIRE(std::ranges::input_range<ViewNoVVF>);
    STATIC_REQUIRE(std::ranges::input_range<ViewWithVVF>);
  }

  SECTION("forward_range satisfied") {
    STATIC_REQUIRE(std::ranges::forward_range<ViewNoVVF>);
    STATIC_REQUIRE(std::ranges::forward_range<ViewWithVVF>);
  }

  SECTION("sized_range satisfied") {
    STATIC_REQUIRE(std::ranges::sized_range<ViewNoVVF>);
    STATIC_REQUIRE(std::ranges::sized_range<ViewWithVVF>);
  }

  SECTION("view satisfied") {
    STATIC_REQUIRE(std::ranges::view<ViewNoVVF>);
    STATIC_REQUIRE(std::ranges::view<ViewWithVVF>);
  }
}

TEST_CASE("basic_vertexlist - deque-based graph", "[basic_vertexlist][deque]") {
  using Graph = std::deque<std::deque<int>>;
  Graph g     = {{1}, {2}, {0}};

  SECTION("no value function") {
    auto vlist = basic_vertexlist(g);

    REQUIRE(vlist.size() == 3);

    std::vector<std::size_t> ids;
    for (auto [uid] : vlist) {
      ids.push_back(uid);
    }

    REQUIRE(ids == std::vector<std::size_t>{0, 1, 2});
  }

  SECTION("with value function") {
    auto vlist = basic_vertexlist(g, [](const auto& g, auto v) {
      return g[v.vertex_id()].front();
    });

    std::vector<int> targets;
    for (auto [uid, target] : vlist) {
      targets.push_back(target);
    }

    REQUIRE(targets == std::vector<int>{1, 2, 0});
  }
}

TEST_CASE("basic_vertexlist - const graph", "[basic_vertexlist][const]") {
  using Graph   = std::vector<std::vector<int>>;
  const Graph g = {{1}, {2}, {}};

  SECTION("no value function") {
    auto vlist = basic_vertexlist(g);

    REQUIRE(vlist.size() == 3);

    std::size_t count = 0;
    for (auto [uid] : vlist) {
      REQUIRE(uid == count);
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("with value function") {
    auto vlist = basic_vertexlist(g, [](const auto&, auto v) { return v.vertex_id(); });

    std::vector<std::size_t> ids;
    for (auto [uid, val_id] : vlist) {
      REQUIRE(uid == val_id);
      ids.push_back(val_id);
    }

    REQUIRE(ids == std::vector<std::size_t>{0, 1, 2});
  }
}

TEST_CASE("basic_vertexlist - map-based graph", "[basic_vertexlist][map]") {
  using Graph = std::map<int, std::vector<int>>;
  Graph g     = {
        {100, {200, 300}},
        {200, {300}},
        {300, {}}
  };

  SECTION("iteration over sparse vertex IDs") {
    auto vlist = basic_vertexlist(g);

    REQUIRE(vlist.size() == 3);

    std::vector<int> ids;
    for (auto [uid] : vlist) {
      ids.push_back(uid);
    }

    REQUIRE(ids == std::vector<int>{100, 200, 300});
  }

  SECTION("with value function") {
    auto vlist = basic_vertexlist(g, [](const auto& g, auto v) {
      return g.at(v.vertex_id()).size();
    });

    std::vector<std::size_t> edge_counts;
    for (auto [uid, count] : vlist) {
      edge_counts.push_back(count);
    }

    REQUIRE(edge_counts == std::vector<std::size_t>{2, 1, 0});
  }
}

TEST_CASE("basic_vertexlist - iterator properties", "[basic_vertexlist][iterator]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2}, {}};

  SECTION("pre-increment") {
    auto vlist = basic_vertexlist(g);
    auto it    = vlist.begin();

    REQUIRE((*it).id == 0);
    ++it;
    REQUIRE((*it).id == 1);
    ++it;
    REQUIRE((*it).id == 2);
    ++it;
    REQUIRE(it == vlist.end());
  }

  SECTION("post-increment") {
    auto vlist = basic_vertexlist(g);
    auto it    = vlist.begin();

    auto old = it++;
    REQUIRE((*old).id == 0);
    REQUIRE((*it).id == 1);
  }

  SECTION("equality comparison") {
    auto vlist = basic_vertexlist(g);
    auto it1   = vlist.begin();
    auto it2   = vlist.begin();

    REQUIRE(it1 == it2);
    ++it1;
    REQUIRE(it1 != it2);
    ++it2;
    REQUIRE(it1 == it2);
  }

  SECTION("default constructed iterators are equal") {
    using Iter = decltype(basic_vertexlist(g).begin());
    Iter it1;
    Iter it2;
    REQUIRE(it1 == it2);
  }
}

TEST_CASE("basic_vertexlist - value function types", "[basic_vertexlist][vvf]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1}, {2}, {}};

  SECTION("returning string") {
    auto vlist = basic_vertexlist(g, [](const auto&, auto v) { return "vertex_" + std::to_string(v.vertex_id()); });

    std::vector<std::string> names;
    for (auto [uid, name] : vlist) {
      names.push_back(name);
    }

    REQUIRE(names == std::vector<std::string>{"vertex_0", "vertex_1", "vertex_2"});
  }

  SECTION("returning double") {
    auto vlist = basic_vertexlist(g, [](const auto&, auto v) { return static_cast<double>(v.vertex_id()) * 1.5; });

    std::vector<double> values;
    for (auto [uid, val] : vlist) {
      values.push_back(val);
    }

    REQUIRE(values[0] == 0.0);
    REQUIRE(values[1] == 1.5);
    REQUIRE(values[2] == 3.0);
  }
}

// =============================================================================
// basic_vertexlist — id-based subrange
// =============================================================================

TEST_CASE("basic_vertexlist - id subrange", "[basic_vertexlist][subrange]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2, 3}, {3}, {}, {0}};

  SECTION("subrange [1, 3) yields ids 1, 2") {
    auto vlist = basic_vertexlist(g, std::size_t(1), std::size_t(3));

    REQUIRE(vlist.size() == 2);

    std::vector<std::size_t> ids;
    for (auto [uid] : vlist) {
      ids.push_back(uid);
    }

    REQUIRE(ids == std::vector<std::size_t>{1, 2});
  }

  SECTION("subrange [0, 5) yields all vertices") {
    auto vlist = basic_vertexlist(g, std::size_t(0), std::size_t(5));

    REQUIRE(vlist.size() == 5);

    std::vector<std::size_t> ids;
    for (auto [uid] : vlist) {
      ids.push_back(uid);
    }

    REQUIRE(ids == std::vector<std::size_t>{0, 1, 2, 3, 4});
  }

  SECTION("subrange [2, 2) is empty") {
    auto vlist = basic_vertexlist(g, std::size_t(2), std::size_t(2));

    REQUIRE(vlist.size() == 0);
    REQUIRE(vlist.begin() == vlist.end());
  }

  SECTION("subrange with value function") {
    auto vlist = basic_vertexlist(g, std::size_t(1), std::size_t(4),
                                 [](const auto& g, auto v) { return g[v.vertex_id()].size(); });

    REQUIRE(vlist.size() == 3);

    std::vector<std::size_t> edge_counts;
    for (auto [uid, count] : vlist) {
      edge_counts.push_back(count);
    }

    // vertices 1, 2, 3: edge counts are 2, 1, 0
    REQUIRE(edge_counts == std::vector<std::size_t>{2, 1, 0});
  }
}

// =============================================================================
// basic_vertexlist — vertex range overload
// =============================================================================

TEST_CASE("basic_vertexlist - vertex range overload", "[basic_vertexlist][vr]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2, 3}, {3}, {}, {0}};

  SECTION("passing vertices(g) yields all vertices") {
    auto vr    = vertices(g);
    auto vlist = basic_vertexlist(g, vr);

    REQUIRE(vlist.size() == 5);

    std::vector<std::size_t> ids;
    for (auto [uid] : vlist) {
      ids.push_back(uid);
    }

    REQUIRE(ids == std::vector<std::size_t>{0, 1, 2, 3, 4});
  }

  SECTION("vertex range with value function") {
    auto vr    = vertices(g);
    auto vlist = basic_vertexlist(g, vr, [](const auto&, auto v) { return v.vertex_id() * 10; });

    REQUIRE(vlist.size() == 5);

    for (auto [uid, val] : vlist) {
      REQUIRE(val == uid * 10);
    }
  }
}

// =============================================================================
// vertexlist — descriptor-based subrange
// =============================================================================

TEST_CASE("vertexlist - descriptor subrange", "[vertexlist][subrange]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2, 3}, {3}, {}, {0}};

  SECTION("subrange [1, 3) yields vertices 1, 2 with descriptors") {
    auto vr      = vertices(g);
    auto first_u = *std::ranges::next(std::ranges::begin(vr), 1);
    auto last_u  = *std::ranges::next(std::ranges::begin(vr), 3);

    auto vlist = vertexlist(g, first_u, last_u);

    REQUIRE(vlist.size() == 2);

    std::vector<std::size_t> ids;
    for (auto [uid, u] : vlist) {
      REQUIRE(uid == u.vertex_id());
      ids.push_back(uid);
    }

    REQUIRE(ids == std::vector<std::size_t>{1, 2});
  }

  SECTION("full range via descriptors") {
    auto vr      = vertices(g);
    auto first_u = *std::ranges::begin(vr);
    auto last_u  = *std::ranges::end(vr);

    auto vlist = vertexlist(g, first_u, last_u);

    REQUIRE(vlist.size() == 5);
  }

  SECTION("empty range") {
    auto vr = vertices(g);
    auto u  = *std::ranges::next(std::ranges::begin(vr), 2);

    auto vlist = vertexlist(g, u, u);

    REQUIRE(vlist.size() == 0);
    REQUIRE(vlist.begin() == vlist.end());
  }

  SECTION("subrange with value function") {
    auto vr      = vertices(g);
    auto first_u = *std::ranges::next(std::ranges::begin(vr), 1);
    auto last_u  = *std::ranges::next(std::ranges::begin(vr), 4);

    auto vlist = vertexlist(g, first_u, last_u,
                            [](const auto& g, auto v) { return g[v.vertex_id()].size(); });

    REQUIRE(vlist.size() == 3);

    std::vector<std::size_t> edge_counts;
    for (auto [uid, u, count] : vlist) {
      REQUIRE(uid == u.vertex_id());
      edge_counts.push_back(count);
    }

    // vertices 1, 2, 3: edge counts are 2, 1, 0
    REQUIRE(edge_counts == std::vector<std::size_t>{2, 1, 0});
  }
}

// =============================================================================
// vertexlist — vertex range overload
// =============================================================================

TEST_CASE("vertexlist - vertex range overload", "[vertexlist][vr]") {
  using Graph = std::vector<std::vector<int>>;
  Graph g     = {{1, 2}, {2, 3}, {3}, {}, {0}};

  SECTION("passing vertices(g) yields all vertices with descriptors") {
    auto vr    = vertices(g);
    auto vlist = vertexlist(g, vr);

    REQUIRE(vlist.size() == 5);

    std::vector<std::size_t> ids;
    for (auto [uid, u] : vlist) {
      REQUIRE(uid == u.vertex_id());
      ids.push_back(uid);
    }

    REQUIRE(ids == std::vector<std::size_t>{0, 1, 2, 3, 4});
  }

  SECTION("vertex range with value function") {
    auto vr    = vertices(g);
    auto vlist = vertexlist(g, vr, [](const auto&, auto v) { return v.vertex_id() * 10; });

    REQUIRE(vlist.size() == 5);

    for (auto [uid, u, val] : vlist) {
      REQUIRE(uid == u.vertex_id());
      REQUIRE(val == uid * 10);
    }
  }
}

// =============================================================================
// Verify return types match goal specification
// =============================================================================

TEST_CASE("vertexlist - return type verification", "[vertexlist][return_type]") {
  using Graph        = std::vector<std::vector<int>>;
  using VertexType   = vertex_t<Graph>;
  using VertexIdType = vertex_id_t<Graph>;

  Graph g = {{1}, {2}, {}};

  SECTION("vertexlist(g) returns vertex_info<VId, V, void>") {
    auto                     vlist = vertexlist(g);
    using ActualInfo               = decltype(*vlist.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::vertex_type, VertexType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::value_type>);
  }

  SECTION("vertexlist(g, vvf) returns vertex_info<VId, V, VV>") {
    auto vlist = vertexlist(g, [](const auto&, auto) { return 42; });
    using ActualInfo = decltype(*vlist.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::vertex_type, VertexType>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::value_type, int>);
  }

  SECTION("basic_vertexlist(g) returns vertex_info<VId, void, void>") {
    auto                     vlist = basic_vertexlist(g);
    using ActualInfo               = decltype(*vlist.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::vertex_type>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::value_type>);
  }

  SECTION("basic_vertexlist(g, vvf) returns vertex_info<VId, void, VV>") {
    auto vlist = basic_vertexlist(g, [](const auto&, auto) { return 42; });
    using ActualInfo = decltype(*vlist.begin());
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::id_type, VertexIdType>);
    STATIC_REQUIRE(std::is_void_v<typename ActualInfo::vertex_type>);
    STATIC_REQUIRE(std::is_same_v<typename ActualInfo::value_type, int>);
  }
}
