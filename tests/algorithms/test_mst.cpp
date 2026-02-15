/**
 * @file test_mst.cpp
 * @brief Tests for minimum spanning tree algorithms (Kruskal and Prim)
 */

#include <catch2/catch_test_macros.hpp>
#include <graph/algorithm/mst.hpp>
#include <graph/container/undirected_adjacency_list.hpp>
#include "../common/algorithm_test_types.hpp"
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::test::algorithm;

// =============================================================================
// Simple Edge Descriptor for Testing
// =============================================================================

/**
 * @brief Simple edge structure matching MST algorithm expectations
 */
template <typename VId, typename EV>
struct simple_edge {
  VId source_id;
  VId target_id;
  EV  value;

  using source_id_type = VId;
  using target_id_type = VId;
  using value_type     = EV;

  constexpr simple_edge() = default;
  constexpr simple_edge(VId s, VId t, EV v) : source_id(s), target_id(t), value(v) {}

  auto operator<=>(const simple_edge&) const = default;
};

// =============================================================================
// Helper Functions
// =============================================================================

/**
 * @brief Check if edges form a tree (connected and acyclic)
 * For N vertices, a tree must have exactly N-1 edges
 */
template <typename EdgeList>
bool is_tree(size_t num_vertices, const EdgeList& edges) {
  return edges.size() == num_vertices - 1;
}

/**
 * @brief Calculate total weight of edges in a spanning tree
 */
template <typename EdgeList>
auto total_weight(const EdgeList& edges) {
  using edge_type   = typename EdgeList::value_type;
  using weight_type = typename edge_type::value_type;

  weight_type total = 0;
  for (const auto& e : edges) {
    total += e.value;
  }
  return total;
}

/**
 * @brief Check if all edges in tree connect vertices that exist
 */
template <typename EdgeList>
bool edges_valid(size_t num_vertices, const EdgeList& edges) {
  for (const auto& e : edges) {
    if (e.source_id >= num_vertices || e.target_id >= num_vertices) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Check connectivity using union-find
 */
template <typename EdgeList>
bool is_connected(size_t num_vertices, const EdgeList& edges) {
  if (num_vertices == 0)
    return true;
  if (num_vertices == 1)
    return true;
  if (edges.empty())
    return false;

  std::vector<size_t> parent(num_vertices);
  std::iota(parent.begin(), parent.end(), 0);

  auto find = [&](size_t x) {
    while (parent[x] != x) {
      parent[x] = parent[parent[x]];
      x         = parent[x];
    }
    return x;
  };

  auto unite = [&](size_t x, size_t y) {
    size_t px = find(x);
    size_t py = find(y);
    if (px != py) {
      parent[px] = py;
      return true;
    }
    return false;
  };

  for (const auto& e : edges) {
    unite(e.source_id, e.target_id);
  }

  // Check if all vertices have the same root
  size_t root = find(0);
  for (size_t i = 1; i < num_vertices; ++i) {
    if (find(i) != root) {
      return false;
    }
  }
  return true;
}

// =============================================================================
// Kruskal's Algorithm Tests
// =============================================================================

TEST_CASE("kruskal - simple triangle", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  // Triangle with edges of weight 1, 2, 3
  // MST should select edges of weight 1 and 2
  std::vector<Edge> input = {{0, 1, 1}, {1, 2, 2}, {2, 0, 3}};

  std::vector<Edge> mst;
  auto [total_wt, num_components] = kruskal(input, mst);

  REQUIRE(mst.size() == 2); // Tree with 3 vertices has 2 edges
  REQUIRE(edges_valid(3, mst));
  REQUIRE(is_connected(3, mst));
  REQUIRE(total_weight(mst) == 3); // 1 + 2 = 3
  REQUIRE(total_wt == 3);          // Verify return value matches
  REQUIRE(num_components == 1);    // Single connected component
}

TEST_CASE("kruskal - linear graph", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  // Linear: 0-1-2-3 with weights 1, 2, 3
  std::vector<Edge> input = {{0, 1, 1}, {1, 2, 2}, {2, 3, 3}};

  std::vector<Edge> mst;
  auto [total_wt, num_components] = kruskal(input, mst);

  REQUIRE(mst.size() == 3); // 4 vertices, 3 edges
  REQUIRE(edges_valid(4, mst));
  REQUIRE(is_connected(4, mst));
  REQUIRE(total_weight(mst) == 6); // 1 + 2 + 3 = 6
  REQUIRE(total_wt == 6);          // Verify return value
  REQUIRE(num_components == 1);    // Connected
}

TEST_CASE("kruskal - complete graph K4", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  // Complete graph on 4 vertices
  std::vector<Edge> input = {{0, 1, 1}, {0, 2, 4}, {0, 3, 3}, {1, 2, 2}, {1, 3, 5}, {2, 3, 6}};

  std::vector<Edge> mst;
  kruskal(input, mst);

  REQUIRE(mst.size() == 3); // 4 vertices, 3 edges
  REQUIRE(edges_valid(4, mst));
  REQUIRE(is_connected(4, mst));
  REQUIRE(total_weight(mst) == 6); // Should select edges 1, 2, 3
}

TEST_CASE("kruskal - graph with equal weights", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  // All edges have weight 1
  std::vector<Edge> input = {{0, 1, 1}, {1, 2, 1}, {2, 0, 1}};

  std::vector<Edge> mst;
  kruskal(input, mst);

  REQUIRE(mst.size() == 2);
  REQUIRE(edges_valid(3, mst));
  REQUIRE(is_connected(3, mst));
  REQUIRE(total_weight(mst) == 2); // Any 2 edges work
}

TEST_CASE("kruskal - disconnected components", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  // Two separate triangles
  std::vector<Edge> input = {
        {0, 1, 1}, {1, 2, 1}, {2, 0, 1}, // First triangle
        {3, 4, 2}, {4, 5, 2}, {5, 3, 2}  // Second triangle
  };

  std::vector<Edge> mst;
  auto [total_wt, num_components] = kruskal(input, mst);

  // MST of disconnected graph is a forest
  // Each component should have its MST
  REQUIRE(mst.size() == 4); // 2 edges per component
  REQUIRE(edges_valid(6, mst));
  REQUIRE(total_wt == 6);       // 2 edges in each component of weight 1 and 2
  REQUIRE(num_components == 2); // Two separate components
}

TEST_CASE("kruskal - single vertex", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  std::vector<Edge> input = {}; // No edges, single vertex
  std::vector<Edge> mst;
  kruskal(input, mst);

  REQUIRE(mst.empty()); // MST of single vertex has no edges
}

TEST_CASE("kruskal - custom comparator (maximum spanning tree)", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  // Triangle with edges of weight 1, 2, 3
  std::vector<Edge> input = {{0, 1, 1}, {1, 2, 2}, {2, 0, 3}};

  std::vector<Edge> mst;
  // Use greater<> to find maximum spanning tree
  kruskal(input, mst, std::greater<int>());

  REQUIRE(mst.size() == 2);
  REQUIRE(edges_valid(3, mst));
  REQUIRE(is_connected(3, mst));
  REQUIRE(total_weight(mst) == 5); // Should select edges 2 and 3 (maximum)
}

TEST_CASE("kruskal - larger graph (CLRS example)", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  // Example from CLRS textbook (simplified)
  std::vector<Edge> input = {{0, 1, 4}, {0, 7, 8},  {1, 2, 8},  {1, 7, 11}, {2, 3, 7}, {2, 5, 4}, {2, 8, 2},
                             {3, 4, 9}, {3, 5, 14}, {4, 5, 10}, {5, 6, 2},  {6, 7, 1}, {6, 8, 6}, {7, 8, 7}};

  std::vector<Edge> mst;
  kruskal(input, mst);

  REQUIRE(mst.size() == 8); // 9 vertices, 8 edges
  REQUIRE(edges_valid(9, mst));
  REQUIRE(is_connected(9, mst));
  REQUIRE(total_weight(mst) == 37); // Known MST weight
}

// =============================================================================
// Inplace Kruskal Tests
// =============================================================================

TEST_CASE("inplace_kruskal - simple triangle", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  std::vector<Edge> input = {{0, 1, 1}, {1, 2, 2}, {2, 0, 3}};

  std::vector<Edge> mst;
  inplace_kruskal(input, mst);

  REQUIRE(mst.size() == 2);
  REQUIRE(edges_valid(3, mst));
  REQUIRE(is_connected(3, mst));
  REQUIRE(total_weight(mst) == 3);
}

TEST_CASE("inplace_kruskal - input is modified", "[algorithm][mst][kruskal]") {
  using Edge = simple_edge<uint32_t, int>;

  std::vector<Edge> input = {{0, 1, 3}, {1, 2, 1}, {2, 0, 2}};

  auto original_order = input;

  std::vector<Edge> mst;
  inplace_kruskal(input, mst);

  // Input should be sorted by weight
  REQUIRE(input != original_order);
  REQUIRE(input[0].value <= input[1].value);
  REQUIRE(input[1].value <= input[2].value);

  REQUIRE(mst.size() == 2);
  REQUIRE(total_weight(mst) == 3);
}

// =============================================================================
// Prim's Algorithm Tests
// =============================================================================

TEST_CASE("prim - simple triangle", "[algorithm][mst][prim]") {
  using Graph = vov_weighted;

  // Create undirected triangle graph using initializer list
  Graph g({{0, 1, 1}, {1, 0, 1}, {1, 2, 2}, {2, 1, 2}, {2, 0, 3}, {0, 2, 3}});

  std::vector<uint32_t> predecessor(3);
  std::vector<int>      weight(3);

  auto total_wt = prim(g, predecessor, weight, 0);

  // Check MST properties
  REQUIRE(predecessor[0] == 0); // Root

  // Calculate MST weight from tree edges
  int mst_weight = weight[1] + weight[2];
  REQUIRE(mst_weight == 3); // Edges 1 and 2
  REQUIRE(total_wt == 3);   // Verify return value matches
}

TEST_CASE("prim - linear graph", "[algorithm][mst][prim]") {
  using Graph = vov_weighted;

  // Linear: 0-1-2-3
  Graph g({{0, 1, 1}, {1, 0, 1}, {1, 2, 2}, {2, 1, 2}, {2, 3, 3}, {3, 2, 3}});

  std::vector<uint32_t> predecessor(4);
  std::vector<int>      weight(4);

  prim(g, predecessor, weight, 0);

  REQUIRE(predecessor[0] == 0); // Root

  int mst_weight = weight[1] + weight[2] + weight[3];
  REQUIRE(mst_weight == 6); // 1 + 2 + 3
}


TEST_CASE("prim - complete graph K4", "[algorithm][mst][prim]") {
  using Graph = vov_weighted;

  // Complete graph on 4 vertices
  Graph g({{0, 1, 1},
           {1, 0, 1},
           {0, 2, 4},
           {2, 0, 4},
           {0, 3, 3},
           {3, 0, 3},
           {1, 2, 2},
           {2, 1, 2},
           {1, 3, 5},
           {3, 1, 5},
           {2, 3, 6},
           {3, 2, 6}});

  std::vector<uint32_t> predecessor(4);
  std::vector<int>      weight(4);

  prim(g, predecessor, weight, 0);

  REQUIRE(predecessor[0] == 0);

  int mst_weight = weight[1] + weight[2] + weight[3];
  REQUIRE(mst_weight == 6); // Should match Kruskal
}

TEST_CASE("kruskal and prim produce same MST weight", "[algorithm][mst]") {
  using Graph = vov_weighted;
  using Edge  = simple_edge<uint32_t, int>;

  // Create a graph
  std::vector<Edge> edges = {{0, 1, 2}, {0, 3, 6}, {1, 2, 3}, {1, 3, 8}, {1, 4, 5}, {2, 4, 7}, {3, 4, 9}};

  // Build graph for Prim with bidirectional edges
  Graph g({{0, 1, 2},
           {1, 0, 2},
           {0, 3, 6},
           {3, 0, 6},
           {1, 2, 3},
           {2, 1, 3},
           {1, 3, 8},
           {3, 1, 8},
           {1, 4, 5},
           {4, 1, 5},
           {2, 4, 7},
           {4, 2, 7},
           {3, 4, 9},
           {4, 3, 9}});

  // Run Kruskal
  std::vector<Edge> kruskal_mst;
  kruskal(edges, kruskal_mst);
  int kruskal_weight = total_weight(kruskal_mst);

  // Run Prim
  std::vector<uint32_t> predecessor(5);
  std::vector<int>      weight(5);
  prim(g, predecessor, weight, 0);

  int prim_weight = weight[1] + weight[2] + weight[3] + weight[4];

  // Both algorithms should produce the same MST weight
  REQUIRE(kruskal_weight == prim_weight);
  REQUIRE(kruskal_mst.size() == 4); // 5 vertices, 4 edges
}

// =============================================================================
// Undirected Graph Tests
// =============================================================================

TEST_CASE("prim - undirected_adjacency_list triangle", "[algorithm][mst][prim][undirected]") {
  using namespace graph::container;
  using Graph = undirected_adjacency_list<int, int>;

  // Create undirected triangle - edges only declared once (not bidirectional)
  Graph g({{0, 1, 1}, {1, 2, 2}, {2, 0, 3}});

  std::vector<uint32_t> predecessor(3);
  std::vector<int>      weight(3);

  auto total_wt = prim(g, predecessor, weight, 0);

  // Check MST properties
  REQUIRE(predecessor[0] == 0); // Root

  // Calculate MST weight from tree edges
  int mst_weight = weight[1] + weight[2];
  REQUIRE(mst_weight == 3); // Edges 1 and 2
  REQUIRE(total_wt == 3);   // Verify return value matches
}

TEST_CASE("prim - undirected_adjacency_list linear graph", "[algorithm][mst][prim][undirected]") {
  using namespace graph::container;
  using Graph = undirected_adjacency_list<int, int>;

  // Linear: 0-1-2-3 - edges only declared once  (not bidirectional)
  Graph g({{0, 1, 1}, {1, 2, 2}, {2, 3, 3}});

  std::vector<uint32_t> predecessor(4);
  std::vector<int>      weight(4);

  auto total_wt = prim(g, predecessor, weight, 0);

  REQUIRE(predecessor[0] == 0); // Root

  int mst_weight = weight[1] + weight[2] + weight[3];
  REQUIRE(mst_weight == 6); // 1 + 2 + 3
  REQUIRE(total_wt == 6);
}

TEST_CASE("prim - undirected_adjacency_list complete graph K4", "[algorithm][mst][prim][undirected]") {
  using namespace graph::container;
  using Graph = undirected_adjacency_list<int, int>;

  // Complete graph on 4 vertices - edges only declared once (not bidirectional)
  Graph g({{0, 1, 1}, {0, 2, 4}, {0, 3, 3}, {1, 2, 2}, {1, 3, 5}, {2, 3, 6}});

  std::vector<uint32_t> predecessor(4);
  std::vector<int>      weight(4);

  auto total_wt = prim(g, predecessor, weight, 0);

  REQUIRE(predecessor[0] == 0);

  int mst_weight = weight[1] + weight[2] + weight[3];
  REQUIRE(mst_weight == 6); // 1 + 2 + 3
  REQUIRE(total_wt == 6);
}

TEST_CASE("prim - undirected_adjacency_list CLRS example", "[algorithm][mst][prim][undirected]") {
  using namespace graph::container;
  using Graph = undirected_adjacency_list<int, int>;

  // CLRS example graph (Figure 23.1) - edges only declared once (not bidirectional)
  Graph g({{0, 1, 2}, {0, 3, 6}, {1, 2, 3}, {1, 3, 8}, {1, 4, 5}, {2, 4, 7}, {3, 4, 9}});

  std::vector<uint32_t> predecessor(5);
  std::vector<int>      weight(5);

  auto total_wt = prim(g, predecessor, weight, 0);

  REQUIRE(predecessor[0] == 0); // Root

  // MST weight should be 16 (same as directed graph tests)
  int mst_weight = weight[1] + weight[2] + weight[3] + weight[4];
  REQUIRE(mst_weight == 16);
  REQUIRE(total_wt == 16);
}
