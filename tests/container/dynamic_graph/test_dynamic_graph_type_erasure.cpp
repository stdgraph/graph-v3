#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <memory>
#include <vector>
#include <string>
#include <any>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

// Type-erased graph wrapper using virtual interface
template <typename VId>
class graph_view {
public:
  virtual ~graph_view() = default;

  virtual size_t                           num_vertices() const                       = 0;
  virtual size_t                           num_edges() const                          = 0;
  virtual bool                             has_edges(const VId& u, const VId& v) const = 0;
  virtual std::vector<VId>                 get_vertex_ids() const                     = 0;
  virtual std::vector<std::pair<VId, VId>> get_edges() const                          = 0;
};

// Concrete wrapper for any graph type
template <typename G>
class graph_wrapper : public graph_view<typename std::remove_cvref_t<G>::vertex_id_type> {
public:
  using VId = typename std::remove_cvref_t<G>::vertex_id_type;

  explicit graph_wrapper(const G& g) : graph_(g) {}

  size_t num_vertices() const override {
    size_t count = 0;
    for (auto&& u : vertices(graph_)) {
      (void)u;
      ++count;
    }
    return count;
  }

  size_t num_edges() const override {
    size_t count = 0;
    for (auto&& u : vertices(graph_)) {
      count += static_cast<size_t>(std::ranges::distance(edges(graph_, u)));
    }
    return count;
  }

  bool has_edges(const VId& uid, const VId& vid) const override {
    for (auto&& u : vertices(graph_)) {
      if (vertex_id(graph_, u) == uid) {
        for (auto&& e : edges(graph_, u)) {
          if (target_id(graph_, e) == vid) {
            return true;
          }
        }
        break;
      }
    }
    return false;
  }

  std::vector<VId> get_vertex_ids() const override {
    std::vector<VId> ids;
    for (auto&& u : vertices(graph_)) {
      ids.push_back(vertex_id(graph_, u));
    }
    return ids;
  }

  std::vector<std::pair<VId, VId>> get_edges() const override {
    std::vector<std::pair<VId, VId>> edge_list;
    for (auto&& u : vertices(graph_)) {
      VId uid = vertex_id(graph_, u);
      for (auto&& e : edges(graph_, u)) {
        edge_list.emplace_back(uid, target_id(graph_, e));
      }
    }
    return edge_list;
  }

private:
  const G& graph_;
};

// Helper function to create type-erased wrapper
template <typename G>
std::unique_ptr<graph_view<typename std::remove_cvref_t<G>::vertex_id_type>> make_graph_view(const G& g) {
  return std::make_unique<graph_wrapper<G>>(g);
}

// Test fixtures
using vov_void = dynamic_graph<void, void, void, uint64_t, false, vov_graph_traits<void, void, void, uint64_t, false>>;
using mos_void = dynamic_graph<void, void, void, uint64_t, false, mos_graph_traits<void, void, void, uint64_t, false>>;
using dofl_void =
      dynamic_graph<void, void, void, uint64_t, false, dofl_graph_traits<void, void, void, uint64_t, false>>;

TEST_CASE("graph_view wraps empty graph", "[6.3.5][type-erasure][empty]") {
  vov_void g;
  auto     view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 0);
  REQUIRE(view->num_edges() == 0);
  REQUIRE(view->get_vertex_ids().empty());
  REQUIRE(view->get_edges().empty());
}

TEST_CASE("graph_view wraps single vertex graph", "[6.3.5][type-erasure][single]") {
  vov_void g({{0, 1}}); // Creates vertices 0 and 1 with edge 0->1

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 2);
  REQUIRE(view->num_edges() == 1);

  auto ids = view->get_vertex_ids();
  REQUIRE(ids.size() == 2);
}

TEST_CASE("graph_view wraps graph with edges", "[6.3.5][type-erasure][edges]") {
  vov_void g({{0, 1}, {1, 2}, {0, 2}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 3);
  REQUIRE(view->num_edges() == 3);
  REQUIRE(view->has_edges(0, 1));
  REQUIRE(view->has_edges(1, 2));
  REQUIRE(view->has_edges(0, 2));
  REQUIRE_FALSE(view->has_edges(2, 0));
}

TEST_CASE("graph_view has_edges on non-existent edge", "[6.3.5][type-erasure][has-edge]") {
  vov_void g({{0, 1}});

  auto view = make_graph_view(g);

  REQUIRE(view->has_edges(0, 1));
  REQUIRE_FALSE(view->has_edges(1, 0));
  REQUIRE_FALSE(view->has_edges(0, 2));
}

TEST_CASE("graph_view get_vertex_ids returns all vertices", "[6.3.5][type-erasure][vertices]") {
  vov_void g({{0, 1}, {1, 2}});

  auto view = make_graph_view(g);
  auto ids  = view->get_vertex_ids();

  REQUIRE(ids.size() == 3);
  std::sort(ids.begin(), ids.end());
  REQUIRE(ids == std::vector<uint64_t>{0, 1, 2});
}

TEST_CASE("graph_view get_edges returns all edges", "[6.3.5][type-erasure][get-edges]") {
  vov_void g({{0, 1}, {1, 0}});

  auto view  = make_graph_view(g);
  auto edges = view->get_edges();

  REQUIRE(edges.size() == 2);

  std::sort(edges.begin(), edges.end());
  REQUIRE(edges[0] == std::pair{0ul, 1ul});
  REQUIRE(edges[1] == std::pair{1ul, 0ul});
}

TEST_CASE("graph_view with self-loop", "[6.3.5][type-erasure][self-loop]") {
  vov_void g({{0, 0}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 1);
  REQUIRE(view->num_edges() == 1);
  REQUIRE(view->has_edges(0, 0));
}

TEST_CASE("graph_view wraps mos graph", "[6.3.5][type-erasure][mos]") {
  mos_void g({{10, 20}, {20, 30}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 3);
  REQUIRE(view->num_edges() == 2);
  REQUIRE(view->has_edges(10, 20));
  REQUIRE(view->has_edges(20, 30));
  REQUIRE_FALSE(view->has_edges(10, 30));
}

TEST_CASE("graph_view wraps dofl graph", "[6.3.5][type-erasure][dofl]") {
  dofl_void g({{0, 1}, {1, 2}, {2, 0}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 3);
  REQUIRE(view->num_edges() == 3);

  auto edges = view->get_edges();
  REQUIRE(edges.size() == 3);
}

TEST_CASE("multiple graph_views in container", "[6.3.5][type-erasure][container]") {
  vov_void  g1({{0, 1}});
  mos_void  g2({{10, 20}});
  dofl_void g3({{0, 1}, {1, 2}});

  std::vector<std::unique_ptr<graph_view<uint64_t>>> graphs;
  graphs.push_back(make_graph_view(g1));
  graphs.push_back(make_graph_view(g2));
  graphs.push_back(make_graph_view(g3));

  REQUIRE(graphs.size() == 3);
  REQUIRE(graphs[0]->num_edges() == 1);
  REQUIRE(graphs[1]->num_edges() == 1);
  REQUIRE(graphs[2]->num_edges() == 2);
}

TEST_CASE("graph_view supports polymorphic iteration", "[6.3.5][type-erasure][polymorphic]") {
  vov_void g1({{0, 1}});
  mos_void g2({{5, 6}, {6, 7}, {5, 7}});

  std::vector<std::unique_ptr<graph_view<uint64_t>>> graphs;
  graphs.push_back(make_graph_view(g1));
  graphs.push_back(make_graph_view(g2));

  // Count total edges across all graphs
  size_t total_edges = 0;
  for (const auto& view : graphs) {
    total_edges += view->num_edges();
  }

  REQUIRE(total_edges == 4);
}

TEST_CASE("graph_view get_vertex_ids with map graph", "[6.3.5][type-erasure][map-vertices]") {
  mos_void g({{100, 200}, {150, 200}});

  auto view = make_graph_view(g);
  auto ids  = view->get_vertex_ids();

  REQUIRE(ids.size() == 3);
  std::sort(ids.begin(), ids.end());
  REQUIRE(ids == std::vector<uint64_t>{100, 150, 200});
}

TEST_CASE("graph_view handles cycle", "[6.3.5][type-erasure][cycle]") {
  vov_void g({{0, 1}, {1, 2}, {2, 0}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 3);
  REQUIRE(view->num_edges() == 3);
  REQUIRE(view->has_edges(0, 1));
  REQUIRE(view->has_edges(1, 2));
  REQUIRE(view->has_edges(2, 0));
}

TEST_CASE("graph_view with disconnected components", "[6.3.5][type-erasure][disconnected]") {
  vov_void g({{0, 1}, {2, 3}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 4);
  REQUIRE(view->num_edges() == 2);
  REQUIRE(view->has_edges(0, 1));
  REQUIRE(view->has_edges(2, 3));
  REQUIRE_FALSE(view->has_edges(0, 2));
  REQUIRE_FALSE(view->has_edges(1, 3));
}

TEST_CASE("graph_view polymorphic function call", "[6.3.5][type-erasure][function]") {
  auto count_total_edges = [](const std::vector<std::unique_ptr<graph_view<uint64_t>>>& graphs) {
    size_t total = 0;
    for (const auto& g : graphs) {
      total += g->num_edges();
    }
    return total;
  };

  vov_void g1({{0, 1}});
  mos_void g2({{10, 20}, {20, 10}});

  std::vector<std::unique_ptr<graph_view<uint64_t>>> graphs;
  graphs.push_back(make_graph_view(g1));
  graphs.push_back(make_graph_view(g2));

  REQUIRE(count_total_edges(graphs) == 3);
}

TEST_CASE("graph_view with complex graph structure", "[6.3.5][type-erasure][complex]") {
  vov_void g({{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}, {4, 0}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 5);
  REQUIRE(view->num_edges() == 6);

  auto edges = view->get_edges();
  REQUIRE(edges.size() == 6);

  auto ids = view->get_vertex_ids();
  REQUIRE(ids.size() == 5);
}

TEST_CASE("graph_view empty edge list", "[6.3.5][type-erasure][no-edges]") {
  vov_void g({{0, 1}, {1, 2}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 3);
  REQUIRE(view->num_edges() == 2);

  auto edges = view->get_edges();
  REQUIRE(edges.size() == 2);
}

TEST_CASE("graph_view multiple self-loops", "[6.3.5][type-erasure][multi-self-loops]") {
  vov_void g({{0, 0}, {1, 1}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 2);
  REQUIRE(view->num_edges() == 2);
  REQUIRE(view->has_edges(0, 0));
  REQUIRE(view->has_edges(1, 1));
}

TEST_CASE("graph_view with star topology", "[6.3.5][type-erasure][star]") {
  mos_void g({{0, 1}, {0, 2}, {0, 3}, {0, 4}});

  auto view = make_graph_view(g);

  REQUIRE(view->num_vertices() == 5);
  REQUIRE(view->num_edges() == 4);
  REQUIRE(view->has_edges(0, 1));
  REQUIRE(view->has_edges(0, 2));
  REQUIRE(view->has_edges(0, 3));
  REQUIRE(view->has_edges(0, 4));
}
