#include <catch2/catch_test_macros.hpp>
#include <graph/edge_list/edge_list.hpp>
#include <graph/edge_list/edge_list_descriptor.hpp>
#include <graph/graph_data.hpp>
#include <vector>
#include <tuple>

using namespace graph;
// Don't use `using namespace graph::edge_list` to avoid ambiguity with adj_list::edge_descriptor


// =============================================================================
// Concept Satisfaction Tests
// =============================================================================

TEST_CASE("basic_sourced_edgelist concept with pairs", "[edge_list][concepts]") {
  using edge_list_type = std::vector<std::pair<int, int>>;
  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::basic_sourced_index_edgelist<edge_list_type>);
  STATIC_REQUIRE(!edge_list::has_edge_value<edge_list_type>);
}

TEST_CASE("basic_sourced_edgelist concept with 2-tuples", "[edge_list][concepts]") {
  using edge_list_type = std::vector<std::tuple<int, int>>;
  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::basic_sourced_index_edgelist<edge_list_type>);
  STATIC_REQUIRE(!edge_list::has_edge_value<edge_list_type>);
}

TEST_CASE("basic_sourced_edgelist concept with 3-tuples", "[edge_list][concepts]") {
  using edge_list_type = std::vector<std::tuple<int, int, double>>;
  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::basic_sourced_index_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::has_edge_value<edge_list_type>);
}

TEST_CASE("basic_sourced_edgelist concept with edge_data (no value)", "[edge_list][concepts]") {
  using edge_type      = graph::edge_data<int, true, void, void>;
  using edge_list_type = std::vector<edge_type>;
  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::basic_sourced_index_edgelist<edge_list_type>);
  STATIC_REQUIRE(!edge_list::has_edge_value<edge_list_type>);
}

TEST_CASE("basic_sourced_edgelist concept with edge_data (with value)", "[edge_list][concepts]") {
  using edge_type      = graph::edge_data<int, true, void, double>;
  using edge_list_type = std::vector<edge_type>;
  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::basic_sourced_index_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::has_edge_value<edge_list_type>);
}

TEST_CASE("basic_sourced_edgelist concept with edge_descriptor (no value)", "[edge_list][concepts]") {
  using edge_list_type = std::vector<edge_list::edge_descriptor<int, void>>;
  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::basic_sourced_index_edgelist<edge_list_type>);
  STATIC_REQUIRE(!edge_list::has_edge_value<edge_list_type>);
}

TEST_CASE("basic_sourced_edgelist concept with edge_descriptor (with value)", "[edge_list][concepts]") {
  using edge_list_type = std::vector<edge_list::edge_descriptor<int, double>>;
  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::basic_sourced_index_edgelist<edge_list_type>);
  STATIC_REQUIRE(edge_list::has_edge_value<edge_list_type>);
}

TEST_CASE("basic_sourced_edgelist concept with string vertex IDs", "[edge_list][concepts]") {
  using edge_list_type = std::vector<std::pair<std::string, std::string>>;
  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<edge_list_type>);
  STATIC_REQUIRE(!edge_list::basic_sourced_index_edgelist<edge_list_type>); // strings are not integral
  STATIC_REQUIRE(!edge_list::has_edge_value<edge_list_type>);
}

TEST_CASE("Nested ranges should NOT satisfy basic_sourced_edgelist", "[edge_list][concepts]") {
  // Adjacency list pattern - vector of vectors
  using nested_type = std::vector<std::vector<int>>;
  STATIC_REQUIRE(!edge_list::basic_sourced_edgelist<nested_type>);
}

// =============================================================================
// Type Alias Tests
// =============================================================================

TEST_CASE("edge_list type aliases", "[edge_list][types]") {
  using EL = std::vector<std::tuple<int, int, double>>;

  // These should compile
  using edge_range = edge_list::edge_range_t<EL>;
  using edge_iter  = edge_list::edge_iterator_t<EL>;
  using edge       = edge_list::edge_t<EL>;
  using edge_val   = edge_list::edge_value_t<EL>;
  using vid        = edge_list::vertex_id_t<EL>;

  STATIC_REQUIRE(std::is_same_v<edge, std::tuple<int, int, double>>);
  STATIC_REQUIRE(std::is_same_v<vid, int>);
  STATIC_REQUIRE(std::is_same_v<edge_val, double>);

  SUCCEED("Type aliases compile successfully");
}

TEST_CASE("edge_list type aliases without edge value", "[edge_list][types]") {
  using EL = std::vector<std::pair<int, int>>;

  using edge_range = edge_list::edge_range_t<EL>;
  using edge_iter  = edge_list::edge_iterator_t<EL>;
  using edge       = edge_list::edge_t<EL>;
  using vid        = edge_list::vertex_id_t<EL>;

  STATIC_REQUIRE(std::is_same_v<edge, std::pair<int, int>>);
  STATIC_REQUIRE(std::is_same_v<vid, int>);

  SUCCEED("Type aliases compile successfully");
}

// =============================================================================
// Runtime Behavior Tests
// =============================================================================

TEST_CASE("basic_sourced_edgelist runtime behavior with pairs", "[edge_list][runtime]") {
  std::vector<std::pair<int, int>> edges = {{1, 2}, {2, 3}, {3, 4}};

  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<decltype(edges)>);

  for (const auto& e : edges) {
    auto src = graph::source_id(edges, e);
    auto tgt = graph::target_id(edges, e);
    REQUIRE(src < tgt); // All our test edges have src < tgt
  }
}

TEST_CASE("basic_sourced_edgelist runtime behavior with edge_descriptor", "[edge_list][runtime]") {
  int s1 = 1, t1 = 2;
  int s2 = 2, t2 = 3;

  edge_list::edge_descriptor<int, void> e1(s1, t1);
  edge_list::edge_descriptor<int, void> e2(s2, t2);

  std::vector<edge_list::edge_descriptor<int, void>> edges = {e1, e2};

  STATIC_REQUIRE(edge_list::basic_sourced_edgelist<decltype(edges)>);

  auto src = graph::source_id(edges, e1);
  auto tgt = graph::target_id(edges, e1);

  REQUIRE(src == 1);
  REQUIRE(tgt == 2);
}

TEST_CASE("has_edge_value runtime behavior", "[edge_list][runtime]") {
  int                                     s = 5, t = 6;
  double                                  v = 3.14;
  edge_list::edge_descriptor<int, double> e(s, t, v);

  std::vector<edge_list::edge_descriptor<int, double>> edges = {e};

  STATIC_REQUIRE(edge_list::has_edge_value<decltype(edges)>);

  auto val = graph::edge_value(edges, e);
  REQUIRE(val == 3.14);
}
