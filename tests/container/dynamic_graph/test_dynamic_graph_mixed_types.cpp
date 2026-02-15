#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <vector>
#include <string>

using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

// Graph type aliases with uint64_t IDs
using vov_void = dynamic_graph<void, void, void, uint64_t, false, vov_graph_traits<void, void, void, uint64_t, false>>;
using mos_void = dynamic_graph<void, void, void, uint64_t, false, mos_graph_traits<void, void, void, uint64_t, false>>;
using dofl_void =
      dynamic_graph<void, void, void, uint64_t, false, dofl_graph_traits<void, void, void, uint64_t, false>>;

// Graph type with string IDs
using mos_string =
      dynamic_graph<void, void, void, std::string, false, mos_graph_traits<void, void, void, std::string, false>>;

// Generic edge counting function
template <typename G>
size_t count_edges(const G& g) {
  size_t count = 0;
  for (auto&& u : vertices(g)) {
    count += static_cast<size_t>(std::ranges::distance(edges(g, u)));
  }
  return count;
}

// Generic vertex counting function
template <typename G>
size_t count_vertices(const G& g) {
  size_t count = 0;
  for (auto&& u : vertices(g)) {
    (void)u;
    ++count;
  }
  return count;
}

// Generic has_edge function
template <typename G, typename VId>
bool has_edge_generic(const G& g, const VId& uid, const VId& vid) {
  for (auto&& u : vertices(g)) {
    if (vertex_id(g, u) == uid) {
      for (auto&& e : edges(g, u)) {
        if (target_id(g, e) == vid) {
          return true;
        }
      }
      break;
    }
  }
  return false;
}

TEST_CASE("Multiple graph types coexist - vov, mos, dofl", "[6.4.1][mixed][coexist]") {
  vov_void  g1({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});
  mos_void  g2({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});
  dofl_void g3({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});

  REQUIRE(count_edges(g1) == 2);
  REQUIRE(count_edges(g2) == 2);
  REQUIRE(count_edges(g3) == 2);
}

TEST_CASE("Generic functions work on all graph types", "[6.4.1][mixed][generic]") {
  vov_void  g1({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}, {.source_id = 2, .target_id = 0}});
  mos_void  g2({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}, {.source_id = 2, .target_id = 0}});
  dofl_void g3({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}, {.source_id = 2, .target_id = 0}});

  REQUIRE(count_vertices(g1) == 3);
  REQUIRE(count_vertices(g2) == 3);
  REQUIRE(count_vertices(g3) == 3);

  REQUIRE(count_edges(g1) == 3);
  REQUIRE(count_edges(g2) == 3);
  REQUIRE(count_edges(g3) == 3);
}

TEST_CASE("has_edge_generic works across types", "[6.4.1][mixed][has-edge]") {
  vov_void  g1({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});
  mos_void  g2({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});
  dofl_void g3({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});

  REQUIRE(has_edge_generic(g1, 0ul, 1ul));
  REQUIRE(has_edge_generic(g2, 0ul, 1ul));
  REQUIRE(has_edge_generic(g3, 0ul, 1ul));

  REQUIRE_FALSE(has_edge_generic(g1, 2ul, 0ul));
  REQUIRE_FALSE(has_edge_generic(g2, 2ul, 0ul));
  REQUIRE_FALSE(has_edge_generic(g3, 2ul, 0ul));
}

TEST_CASE("Empty graphs of different types", "[6.4.1][mixed][empty]") {
  vov_void  g1;
  mos_void  g2;
  dofl_void g3;

  REQUIRE(count_vertices(g1) == 0);
  REQUIRE(count_vertices(g2) == 0);
  REQUIRE(count_vertices(g3) == 0);

  REQUIRE(count_edges(g1) == 0);
  REQUIRE(count_edges(g2) == 0);
  REQUIRE(count_edges(g3) == 0);
}

TEST_CASE("Single vertex graphs of different types", "[6.4.1][mixed][single-vertex]") {
  vov_void  g1({{.source_id = 0, .target_id = 0}}); // Self-loop
  mos_void  g2({{.source_id = 0, .target_id = 0}}); // Self-loop
  dofl_void g3({{.source_id = 0, .target_id = 0}}); // Self-loop

  REQUIRE(count_vertices(g1) == 1);
  REQUIRE(count_vertices(g2) == 1);
  REQUIRE(count_vertices(g3) == 1);

  REQUIRE(count_edges(g1) == 1);
  REQUIRE(count_edges(g2) == 1);
  REQUIRE(count_edges(g3) == 1);
}

TEST_CASE("Different graph types with same structure", "[6.4.1][mixed][same-structure]") {
  vov_void  g1({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 3},
                {.source_id = 3, .target_id = 0}});
  mos_void  g2({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 3},
                {.source_id = 3, .target_id = 0}});
  dofl_void g3({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 3},
                {.source_id = 3, .target_id = 0}});

  REQUIRE(count_vertices(g1) == 4);
  REQUIRE(count_vertices(g2) == 4);
  REQUIRE(count_vertices(g3) == 4);

  REQUIRE(count_edges(g1) == 4);
  REQUIRE(count_edges(g2) == 4);
  REQUIRE(count_edges(g3) == 4);
}

TEST_CASE("Generic function with string ID graph", "[6.4.1][mixed][string-ids]") {
  mos_string g({{.source_id = "A", .target_id = "B"},
                {.source_id = "B", .target_id = "C"},
                {.source_id = "C", .target_id = "A"}});

  REQUIRE(count_vertices(g) == 3);
  REQUIRE(count_edges(g) == 3);
  REQUIRE(has_edge_generic(g, std::string("A"), std::string("B")));
  REQUIRE_FALSE(has_edge_generic(g, std::string("A"), std::string("C")));
}

TEST_CASE("Mixed integral and string ID graphs", "[6.4.1][mixed][integral-string]") {
  mos_void   g1({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});
  mos_string g2({{.source_id = "A", .target_id = "B"}, {.source_id = "B", .target_id = "C"}});

  // Both use same generic functions
  REQUIRE(count_edges(g1) == 2);
  REQUIRE(count_edges(g2) == 2);

  REQUIRE(count_vertices(g1) == 3);
  REQUIRE(count_vertices(g2) == 3);
}

TEST_CASE("Disconnected graphs of different types", "[6.4.1][mixed][disconnected]") {
  vov_void  g1({{.source_id = 0, .target_id = 1}, {.source_id = 2, .target_id = 3}});
  mos_void  g2({{.source_id = 0, .target_id = 1}, {.source_id = 2, .target_id = 3}});
  dofl_void g3({{.source_id = 0, .target_id = 1}, {.source_id = 2, .target_id = 3}});

  REQUIRE(count_vertices(g1) == 4);
  REQUIRE(count_vertices(g2) == 4);
  REQUIRE(count_vertices(g3) == 4);

  REQUIRE(count_edges(g1) == 2);
  REQUIRE(count_edges(g2) == 2);
  REQUIRE(count_edges(g3) == 2);
}

TEST_CASE("Star topology across types", "[6.4.1][mixed][star]") {
  vov_void  g1({{.source_id = 0, .target_id = 1},
                {.source_id = 0, .target_id = 2},
                {.source_id = 0, .target_id = 3},
                {.source_id = 0, .target_id = 4}});
  mos_void  g2({{.source_id = 0, .target_id = 1},
                {.source_id = 0, .target_id = 2},
                {.source_id = 0, .target_id = 3},
                {.source_id = 0, .target_id = 4}});
  dofl_void g3({{.source_id = 0, .target_id = 1},
                {.source_id = 0, .target_id = 2},
                {.source_id = 0, .target_id = 3},
                {.source_id = 0, .target_id = 4}});

  REQUIRE(count_vertices(g1) == 5);
  REQUIRE(count_vertices(g2) == 5);
  REQUIRE(count_vertices(g3) == 5);

  REQUIRE(count_edges(g1) == 4);
  REQUIRE(count_edges(g2) == 4);
  REQUIRE(count_edges(g3) == 4);
}

TEST_CASE("Chain topology across types", "[6.4.1][mixed][chain]") {
  vov_void  g1({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 3},
                {.source_id = 3, .target_id = 4}});
  mos_void  g2({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 3},
                {.source_id = 3, .target_id = 4}});
  dofl_void g3({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 3},
                {.source_id = 3, .target_id = 4}});

  REQUIRE(count_vertices(g1) == 5);
  REQUIRE(count_vertices(g2) == 5);
  REQUIRE(count_vertices(g3) == 5);

  REQUIRE(count_edges(g1) == 4);
  REQUIRE(count_edges(g2) == 4);
  REQUIRE(count_edges(g3) == 4);
}

TEST_CASE("Complex graph with multiple components", "[6.4.1][mixed][complex]") {
  vov_void g1({{.source_id = 0, .target_id = 1},
               {.source_id = 1, .target_id = 2},
               {.source_id = 3, .target_id = 4},
               {.source_id = 4, .target_id = 5},
               {.source_id = 5, .target_id = 3}});
  mos_void g2({{.source_id = 0, .target_id = 1},
               {.source_id = 1, .target_id = 2},
               {.source_id = 3, .target_id = 4},
               {.source_id = 4, .target_id = 5},
               {.source_id = 5, .target_id = 3}});

  REQUIRE(count_vertices(g1) == 6);
  REQUIRE(count_vertices(g2) == 6);

  REQUIRE(count_edges(g1) == 5);
  REQUIRE(count_edges(g2) == 5);
}

TEST_CASE("Self-loops across different types", "[6.4.1][mixed][self-loops]") {
  vov_void  g1({{.source_id = 0, .target_id = 0}, {.source_id = 1, .target_id = 1}, {.source_id = 2, .target_id = 2}});
  mos_void  g2({{.source_id = 0, .target_id = 0}, {.source_id = 1, .target_id = 1}, {.source_id = 2, .target_id = 2}});
  dofl_void g3({{.source_id = 0, .target_id = 0}, {.source_id = 1, .target_id = 1}, {.source_id = 2, .target_id = 2}});

  REQUIRE(count_vertices(g1) == 3);
  REQUIRE(count_vertices(g2) == 3);
  REQUIRE(count_vertices(g3) == 3);

  REQUIRE(count_edges(g1) == 3);
  REQUIRE(count_edges(g2) == 3);
  REQUIRE(count_edges(g3) == 3);
}

TEST_CASE("Bidirectional edges across types", "[6.4.1][mixed][bidirectional]") {
  vov_void  g1({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 0},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 1}});
  mos_void  g2({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 0},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 1}});
  dofl_void g3({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 0},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 1}});

  REQUIRE(count_edges(g1) == 4);
  REQUIRE(count_edges(g2) == 4);
  REQUIRE(count_edges(g3) == 4);
}

TEST_CASE("Large graph across types", "[6.4.1][mixed][large]") {
  std::vector<copyable_edge_t<uint64_t, void>> edges;
  for (uint64_t i = 0; i < 100; ++i) {
    edges.push_back({.source_id = i, .target_id = i + 1});
  }

  vov_void g1;
  g1.load_edges(edges);
  mos_void g2;
  g2.load_edges(edges);
  dofl_void g3;
  g3.load_edges(edges);

  REQUIRE(count_vertices(g1) == 101);
  REQUIRE(count_vertices(g2) == 101);
  REQUIRE(count_vertices(g3) == 101);

  REQUIRE(count_edges(g1) == 100);
  REQUIRE(count_edges(g2) == 100);
  REQUIRE(count_edges(g3) == 100);
}

TEST_CASE("Generic function with dense graph", "[6.4.1][mixed][dense]") {
  std::vector<copyable_edge_t<uint64_t, void>> edges;
  for (uint64_t i = 0; i < 10; ++i) {
    for (uint64_t j = 0; j < 10; ++j) {
      if (i != j) {
        edges.push_back({.source_id = i, .target_id = j});
      }
    }
  }

  vov_void g1;
  g1.load_edges(edges);
  mos_void g2;
  g2.load_edges(edges);

  REQUIRE(count_vertices(g1) == 10);
  REQUIRE(count_vertices(g2) == 10);

  REQUIRE(count_edges(g1) == 90);
  REQUIRE(count_edges(g2) == 90);
}

TEST_CASE("Mixed graph types in vector", "[6.4.1][mixed][vector]") {
  vov_void  g1({{.source_id = 0, .target_id = 1}});
  mos_void  g2({{.source_id = 10, .target_id = 20}});
  dofl_void g3({{.source_id = 5, .target_id = 6}});

  // Count edges for each graph type
  size_t total = 0;
  total += count_edges(g1);
  total += count_edges(g2);
  total += count_edges(g3);

  REQUIRE(total == 3);
}

TEST_CASE("String ID graph with longer strings", "[6.4.1][mixed][long-strings]") {
  mos_string g({{.source_id = "Alice", .target_id = "Bob"},
                {.source_id = "Bob", .target_id = "Charlie"},
                {.source_id = "Charlie", .target_id = "David"}});

  REQUIRE(count_vertices(g) == 4);
  REQUIRE(count_edges(g) == 3);
  REQUIRE(has_edge_generic(g, std::string("Alice"), std::string("Bob")));
  REQUIRE(has_edge_generic(g, std::string("Bob"), std::string("Charlie")));
  REQUIRE(has_edge_generic(g, std::string("Charlie"), std::string("David")));
}

TEST_CASE("Generic function with cycle of different sizes", "[6.4.1][mixed][cycles]") {
  vov_void  g1({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 0}}); // 2-cycle
  mos_void  g2({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 0}}); // 3-cycle
  dofl_void g3({{.source_id = 0, .target_id = 1},
                {.source_id = 1, .target_id = 2},
                {.source_id = 2, .target_id = 3},
                {.source_id = 3, .target_id = 0}}); // 4-cycle

  REQUIRE(count_edges(g1) == 2);
  REQUIRE(count_edges(g2) == 3);
  REQUIRE(count_edges(g3) == 4);
}

TEST_CASE("All graph types with isolated vertex", "[6.4.1][mixed][isolated]") {
  vov_void g1(
        {{.source_id = 0, .target_id = 1},
         {.source_id = 2, .target_id = 3},
         {.source_id = 4, .target_id = 4}}); // vertex 4 has self-loop, but conceptually could have isolated vertices
  mos_void  g2({{.source_id = 0, .target_id = 1}, {.source_id = 2, .target_id = 3}});
  dofl_void g3({{.source_id = 0, .target_id = 1}});

  REQUIRE(count_edges(g1) == 3);
  REQUIRE(count_edges(g2) == 2);
  REQUIRE(count_edges(g3) == 1);
}
// ============================================================================
// Test Cases: Range Construction with Projection
// ============================================================================

TEST_CASE("vov graph constructed from range with identity projection", "[6.4.1][range-construct][identity]") {
  std::vector<copyable_edge_t<uint64_t, void>> edges = {
        {.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}, {.source_id = 0, .target_id = 2}};
  std::vector<uint64_t> partitions;
  vov_void              g(2ul, edges, identity{}, partitions);

  REQUIRE(count_vertices(g) == 3);
  REQUIRE(count_edges(g) == 3);
}

TEST_CASE("mos graph constructed from range with identity projection", "[6.4.1][range-construct][mos]") {
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{.source_id = 10, .target_id = 20},
                                                        {.source_id = 20, .target_id = 30}};
  std::vector<uint64_t>                        partitions;
  mos_void                                     g(30ul, edges, identity{}, partitions);

  REQUIRE(count_vertices(g) == 3);
  REQUIRE(count_edges(g) == 2);
  REQUIRE(has_edge_generic(g, 10ul, 20ul));
  REQUIRE(has_edge_generic(g, 20ul, 30ul));
}

TEST_CASE("dofl graph constructed from range with identity projection", "[6.4.1][range-construct][dofl]") {
  std::vector<copyable_edge_t<uint64_t, void>> edges = {
        {.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}, {.source_id = 2, .target_id = 0}};
  std::vector<uint64_t> partitions;
  dofl_void             g(2ul, edges, identity{}, partitions);

  REQUIRE(count_vertices(g) == 3);
  REQUIRE(count_edges(g) == 3);
}

TEST_CASE("graph constructed from range with custom projection", "[6.4.1][range-construct][custom-projection]") {
  struct EdgeData {
    uint64_t from;
    uint64_t to;
  };

  std::vector<EdgeData> data = {{0, 1}, {1, 2}, {0, 2}};
  std::vector<uint64_t> partitions;

  auto proj = [](const EdgeData& e) -> copyable_edge_t<uint64_t, void> {
    return {.source_id = e.from, .target_id = e.to};
  };

  vov_void g(2ul, data, proj, partitions);

  REQUIRE(count_vertices(g) == 3);
  REQUIRE(count_edges(g) == 3);
  REQUIRE(has_edge_generic(g, 0ul, 1ul));
  REQUIRE(has_edge_generic(g, 1ul, 2ul));
  REQUIRE(has_edge_generic(g, 0ul, 2ul));
}

TEST_CASE("mos string graph constructed from range with projection", "[6.4.1][range-construct][string-projection]") {
  struct NamedEdge {
    std::string source;
    std::string target;
  };

  std::vector<NamedEdge>   data = {{"A", "B"}, {"B", "C"}, {"A", "C"}};
  std::vector<std::string> partitions;

  auto proj = [](const NamedEdge& e) -> copyable_edge_t<std::string, void> {
    return {.source_id = e.source, .target_id = e.target};
  };

  mos_string g(data, std::vector<copyable_vertex_t<std::string, void>>{}, proj, identity{}, partitions);

  REQUIRE(count_vertices(g) == 3);
  REQUIRE(count_edges(g) == 3);
}

TEST_CASE("range construction with complex data structure", "[6.4.1][range-construct][complex]") {
  struct Connection {
    uint64_t    src_id;
    uint64_t    dst_id;
    std::string label; // Not used in graph, just to show projection flexibility
  };

  std::vector<Connection> connections = {{0, 1, "first"}, {1, 2, "second"}, {2, 3, "third"}};
  std::vector<uint64_t>   partitions;

  auto edge_proj = [](const Connection& c) -> copyable_edge_t<uint64_t, void> {
    return {.source_id = c.src_id, .target_id = c.dst_id};
  };

  vov_void g(3ul, connections, edge_proj, partitions);

  REQUIRE(count_vertices(g) == 4);
  REQUIRE(count_edges(g) == 3);
}

TEST_CASE("mixed construction methods coexist", "[6.4.1][range-construct][mixed-methods]") {
  // Initializer list construction
  vov_void g1({{.source_id = 0, .target_id = 1}, {.source_id = 1, .target_id = 2}});

  // Range with identity projection
  std::vector<copyable_edge_t<uint64_t, void>> edges = {{.source_id = 0, .target_id = 1},
                                                        {.source_id = 1, .target_id = 2}};
  std::vector<uint64_t>                        partitions;
  mos_void                                     g2(2ul, edges, identity{}, partitions);

  // Custom projection
  struct Edge {
    uint64_t u, v;
  };
  std::vector<Edge> edge_data = {{0, 1}, {1, 2}};
  auto      proj = [](const Edge& e) { return copyable_edge_t<uint64_t, void>{.source_id = e.u, .target_id = e.v}; };
  dofl_void g3(2ul, edge_data, proj, partitions);

  // All should have same structure
  REQUIRE(count_edges(g1) == 2);
  REQUIRE(count_edges(g2) == 2);
  REQUIRE(count_edges(g3) == 2);
}

TEST_CASE("range construction with empty range", "[6.4.1][range-construct][empty]") {
  std::vector<copyable_edge_t<uint64_t, void>> edges;
  vov_void                                     g;
  g.load_edges(edges);

  REQUIRE(count_vertices(g) == 0);
  REQUIRE(count_edges(g) == 0);
}

TEST_CASE("projection function with stateful capture", "[6.4.1][range-construct][stateful]") {
  struct RawEdge {
    int from;
    int to;
  };

  std::vector<RawEdge>  data = {{0, 1}, {1, 2}, {2, 3}};
  std::vector<uint64_t> partitions;

  int  conversion_count = 0;
  auto proj             = [&conversion_count](const RawEdge& e) -> copyable_edge_t<uint64_t, void> {
    ++conversion_count;
    return {.source_id = static_cast<uint64_t>(e.from), .target_id = static_cast<uint64_t>(e.to)};
  };

  vov_void g(3ul, data, proj, partitions);

  REQUIRE(count_edges(g) == 3);
  REQUIRE(conversion_count == 3); // Projection was called for each edge
}

TEST_CASE("string graph with complex projection", "[6.4.1][range-construct][string-complex]") {
  struct Person {
    std::string name;
    std::string knows;
  };

  std::vector<Person>      relationships = {{"Alice", "Bob"}, {"Bob", "Charlie"}, {"Charlie", "Alice"}};
  std::vector<std::string> partitions;

  auto proj = [](const Person& p) -> copyable_edge_t<std::string, void> {
    return {.source_id = p.name, .target_id = p.knows};
  };

  mos_string g(relationships, std::vector<copyable_vertex_t<std::string, void>>{}, proj, identity{}, partitions);

  REQUIRE(count_vertices(g) == 3);
  REQUIRE(count_edges(g) == 3);
}