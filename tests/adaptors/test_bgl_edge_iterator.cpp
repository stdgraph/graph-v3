#include <catch2/catch_test_macros.hpp>

#include <graph/adaptors/bgl/bgl_edge_iterator.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>

#include <iterator>
#include <ranges>
#include <type_traits>
#include <vector>

// ── BGL graph type aliases ──────────────────────────────────────────────────

using bgl_directed_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
using out_edge_iter_t = boost::graph_traits<bgl_directed_t>::out_edge_iterator;

using bgl_csr_t = boost::compressed_sparse_row_graph<boost::directedS>;
using csr_out_edge_iter_t = boost::graph_traits<bgl_csr_t>::out_edge_iterator;

// ── Wrapped iterator aliases ────────────────────────────────────────────────

using wrapped_adj_iter = graph::bgl::bgl_edge_iterator<out_edge_iter_t>;
using wrapped_csr_iter = graph::bgl::bgl_edge_iterator<csr_out_edge_iter_t>;

// ── Static assertions ───────────────────────────────────────────────────────

// 1. forward_iterator satisfaction
static_assert(std::forward_iterator<wrapped_adj_iter>,
              "wrapped adjacency_list out_edge_iterator must satisfy std::forward_iterator");
static_assert(std::forward_iterator<wrapped_csr_iter>,
              "wrapped CSR out_edge_iterator must satisfy std::forward_iterator");

// 2. reference type matches underlying BGL iterator's reference type
static_assert(std::is_same_v<wrapped_adj_iter::reference,
                             std::iterator_traits<out_edge_iter_t>::reference>,
              "adjacency_list wrapper reference must match BGL iterator reference");
static_assert(std::is_same_v<wrapped_csr_iter::reference,
                             std::iterator_traits<csr_out_edge_iter_t>::reference>,
              "CSR wrapper reference must match BGL iterator reference");

// 3. adjacency_list out_edge_iterator returns by value (proxy); CSR returns lvalue ref
static_assert(!std::is_reference_v<wrapped_adj_iter::reference>,
              "adjacency_list out_edge_iterator dereferences to a prvalue (proxy)");
static_assert(std::is_reference_v<wrapped_csr_iter::reference>,
              "CSR out_edge_iterator dereferences to an lvalue reference");

// ── Runtime tests ───────────────────────────────────────────────────────────

TEST_CASE("bgl_edge_iterator wraps adjacency_list out-edges", "[bgl][iterator]") {
  // Build a small directed graph: 3 vertices, 4 edges
  //   0 → 1, 0 → 2, 1 → 2, 2 → 0
  bgl_directed_t g(3);
  boost::add_edge(0, 1, g);
  boost::add_edge(0, 2, g);
  boost::add_edge(1, 2, g);
  boost::add_edge(2, 0, g);

  SECTION("iterate out-edges of vertex 0 and verify targets") {
    auto [bgl_begin, bgl_end] = boost::out_edges(0, g);
    wrapped_adj_iter begin(bgl_begin);
    wrapped_adj_iter end(bgl_end);

    std::vector<std::size_t> targets;
    for (auto it = begin; it != end; ++it) {
      targets.push_back(boost::target(*it, g));
    }

    REQUIRE(targets.size() == 2);
    CHECK(targets[0] == 1);
    CHECK(targets[1] == 2);
  }

  SECTION("subrange models forward_range") {
    auto [bgl_begin, bgl_end] = boost::out_edges(0, g);
    auto sr = std::ranges::subrange(wrapped_adj_iter(bgl_begin),
                                    wrapped_adj_iter(bgl_end));

    static_assert(std::ranges::forward_range<decltype(sr)>,
                  "subrange of wrapped iterators must model forward_range");

    std::size_t count = 0;
    for ([[maybe_unused]] auto&& edge : sr) {
      ++count;
    }
    CHECK(count == 2);
  }
}

TEST_CASE("bgl_edge_iterator CSR operator-> availability", "[bgl][iterator][csr]") {
  // CSR requires sorted edge list at construction
  using edge_t = std::pair<std::size_t, std::size_t>;
  std::vector<edge_t> edges = {{0, 1}, {0, 2}, {1, 2}};

  bgl_csr_t g(boost::edges_are_sorted, edges.begin(), edges.end(), 3);

  using vertex_t = boost::graph_traits<bgl_csr_t>::vertex_descriptor;
  auto [bgl_begin, bgl_end] = boost::out_edges(vertex_t(0), g);
  wrapped_csr_iter it(bgl_begin);
  wrapped_csr_iter end(bgl_end);

  REQUIRE(it != end);

  // operator-> should be available for CSR (lvalue reference)
  auto* ptr = it.operator->();
  CHECK(ptr == std::addressof(*it));
}
