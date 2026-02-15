#include <catch2/catch_test_macros.hpp>
#include "graph/container/compressed_graph.hpp"
#include <string>
#include <vector>
#include <numeric> // for std::accumulate

using namespace std;
using namespace graph;
using namespace graph::adj_list;
using namespace graph::container;

// =============================================================================
// Category 1: Basic Value Type Tests
// =============================================================================

TEST_CASE("compressed_graph default constructor", "[constructors]") {
  SECTION("void/void/void") {
    compressed_graph<void, void, void> g;
    REQUIRE(g.empty());
    REQUIRE(g.size() == 0);
  }

  SECTION("int/int/int") {
    compressed_graph<int, int, int> g(42);
    REQUIRE(g.empty());
    REQUIRE(g.graph_value() == 42);
  }

  SECTION("string/string/string") {
    compressed_graph<string, string, string> g(string("test"));
    REQUIRE(g.empty());
    REQUIRE(g.graph_value() == "test");
  }
}

TEST_CASE("compressed_graph load_edges with void edge values", "[api][edges]") {
  compressed_graph<void, int, void> g;

  vector<copyable_edge_t<int, void>> ee = {{0, 1}, {0, 2}, {1, 2}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
  REQUIRE(g.size() >= 2); // At least 2 vertices
}

TEST_CASE("compressed_graph load_edges with int edge values", "[api][edges]") {
  compressed_graph<int, int, void> g;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
  REQUIRE(g.size() >= 2);
}

TEST_CASE("compressed_graph load_edges with string edge values", "[api][edges]") {
  compressed_graph<string, int, void> g;

  vector<copyable_edge_t<int, string>> ee = {{0, 1, string("a")}, {0, 2, string("b")}, {1, 2, string("c")}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph load_vertices with void vertex values", "[api][vertices]") {
  compressed_graph<void, void, void> g;

  vector<copyable_vertex_t<int, void>> vv = {{0}, {1}, {2}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 3);
}

TEST_CASE("compressed_graph load_vertices with int vertex values", "[api][vertices]") {
  compressed_graph<void, int, void> g;

  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 3);
}

TEST_CASE("compressed_graph load_vertices with string vertex values", "[api][vertices]") {
  compressed_graph<void, string, void> g;

  vector<copyable_vertex_t<int, string>> vv = {{0, string("v0")}, {1, string("v1")}, {2, string("v2")}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 3);
}

// =============================================================================
// Issue #4 Fix Tests: load_vertices with void vertex values
// =============================================================================

TEST_CASE("load_vertices with void VV after load_edges", "[issue4][load_vertices][void]") {
  compressed_graph<int, void, void> g;

  // Load edges first - creates vertices implicitly
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};
  g.load_edges(ee);

  // Now load void vertices - should be no-op but not crash
  vector<copyable_vertex_t<int, void>> vv = {{0}, {1}, {2}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 3);
}

TEST_CASE("load_vertices with void VV before load_edges", "[issue4][load_vertices][void]") {
  compressed_graph<int, void, void> g;

  // Load edges first to create graph structure
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};
  g.load_edges(ee);

  REQUIRE(g.size() == 3);

  // Load void vertices - no-op since VV is void
  vector<copyable_vertex_t<int, void>> vv = {{0}, {1}, {2}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 3);
}

TEST_CASE("load_vertices with void VV on empty graph", "[issue4][load_vertices][void]") {
  compressed_graph<void, void, void> g;

  // Load void vertices on empty graph - no-op since VV is void
  vector<copyable_vertex_t<int, void>> vv = {{0}, {1}, {2}, {3}, {4}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 5);
}

TEST_CASE("load_vertices void VV with rvalue reference", "[issue4][load_vertices][void]") {
  compressed_graph<int, void, void> g;

  // Test rvalue reference overload of load_row_values
  g.load_vertices(vector<copyable_vertex_t<int, void>>{{0}, {1}, {2}});

  REQUIRE(g.size() == 3);
}

TEST_CASE("load_vertices void VV with custom projection", "[issue4][load_vertices][void]") {
  compressed_graph<void, void, void> g;

  // Test with custom projection function
  struct VertexData {
    int id;
  };
  vector<VertexData> data = {{0}, {1}, {2}};

  auto projection = [](const VertexData& vd) { return copyable_vertex_t<int, void>{vd.id}; };

  g.load_vertices(data, projection);

  REQUIRE(g.size() == 3);
}

TEST_CASE("load_vertices void VV combined with non-void edges", "[issue4][load_vertices][void]") {
  compressed_graph<string, void, void> g;

  // Load edges with string values
  vector<copyable_edge_t<int, string>> ee = {{0, 1, string("edge1")}, {1, 2, string("edge2")}, {2, 3, string("edge3")}};
  g.load_edges(ee);

  // Load void vertices
  vector<copyable_vertex_t<int, void>> vv = {{0}, {1}, {2}, {3}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 4);
}

TEST_CASE("load_vertices void VV with explicit vertex count", "[issue4][load_vertices][void]") {
  compressed_graph<void, void, void> g;

  vector<copyable_vertex_t<int, void>> vv = {{0}, {1}};

  // Provide explicit vertex count larger than range size
  g.load_vertices(vv, identity(), 5);

  // Should use explicit vertex count (5) as it's larger than max_id + 1 (2)
  REQUIRE(g.size() == 5);
}

// =============================================================================
// Category 2: VId and EIndex Type Variations
// =============================================================================

TEST_CASE("compressed_graph with uint32_t VId", "[types][vid]") {
  compressed_graph<void, void, void, uint32_t, uint32_t> g;

  vector<copyable_edge_t<uint32_t, void>> ee = {{0, 1}, {1, 2}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph with int32_t VId", "[types][vid]") {
  compressed_graph<void, void, void, int32_t, int32_t> g;

  vector<copyable_edge_t<int32_t, void>> ee = {{0, 1}, {1, 2}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph with uint64_t VId", "[types][vid]") {
  compressed_graph<void, void, void, uint64_t, uint64_t> g;

  vector<copyable_edge_t<uint64_t, void>> ee = {{0, 1}, {1, 2}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph with int8_t VId", "[types][vid]") {
  compressed_graph<void, void, void, int8_t, int8_t> g;

  vector<copyable_edge_t<int8_t, void>> ee = {{0, 1}, {1, 2}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph with mixed signed/unsigned types", "[types][vid]") {
  compressed_graph<void, void, void, int32_t, uint32_t> g;

  vector<copyable_edge_t<int32_t, void>> ee = {{0, 1}, {1, 2}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
}

// =============================================================================
// Category 3: Constructor Tests
// =============================================================================

TEST_CASE("compressed_graph default constructor creates empty graph", "[constructors]") {
  compressed_graph<void, void, void> g;

  REQUIRE(g.empty());
  REQUIRE(g.size() == 0);
}

TEST_CASE("compressed_graph constructor with graph value", "[constructors][graph_value]") {
  SECTION("int graph value") {
    compressed_graph<void, void, int> g(42);
    REQUIRE(g.graph_value() == 42);
    REQUIRE(g.empty());
  }

  SECTION("string graph value") {
    compressed_graph<void, void, string> g(string("graph1"));
    REQUIRE(g.graph_value() == "graph1");
    REQUIRE(g.empty());
  }
}

TEST_CASE("compressed_graph copy constructor", "[constructors][copy]") {
  compressed_graph<int, int, int>   g1(100);
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};
  g1.load_edges(ee);

  compressed_graph<int, int, int> g2(g1);

  REQUIRE(g2.graph_value() == 100);
  REQUIRE(g2.size() == g1.size());
  REQUIRE_FALSE(g2.empty());
}

TEST_CASE("compressed_graph move constructor", "[constructors][move]") {
  compressed_graph<int, int, int>   g1(100);
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};
  g1.load_edges(ee);

  auto                            original_size = g1.size();
  compressed_graph<int, int, int> g2(std::move(g1));

  REQUIRE(g2.graph_value() == 100);
  REQUIRE(g2.size() == original_size);
}

// =============================================================================
// Category 4: Public API Tests
// =============================================================================

TEST_CASE("compressed_graph reserve allocates space", "[api][reserve]") {
  compressed_graph<void, void, void> g;
  g.reserve(200, 100);

  REQUIRE(g.empty());
  REQUIRE(g.size() == 0);
}

TEST_CASE("compressed_graph operator subscript accesses edge values by index", "[api][operator_subscript]") {
  compressed_graph<int, int, void>  g;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}};
  g.load_edges(ee);

  // edge_value() with edge id 0, 1, 2 gives us the edge values
  REQUIRE(g.edge_value(0) == 10); // First edge: 0->1 with value 10
  REQUIRE(g.edge_value(1) == 20); // Second edge: 0->2 with value 20
  REQUIRE(g.edge_value(2) == 30); // Third edge: 1->3 with value 30

  // Test mutability
  g.edge_value(0) = 15;
  REQUIRE(g.edge_value(0) == 15);
}

TEST_CASE("compressed_graph graph_value() accesses graph value", "[api][graph_value]") {
  SECTION("int graph value") {
    compressed_graph<void, void, int> g(42);

    // Test const accessor
    const auto& cg = g;
    REQUIRE(cg.graph_value() == 42);

    // Test non-const accessor
    REQUIRE(g.graph_value() == 42);

    // Test mutability
    g.graph_value() = 100;
    REQUIRE(g.graph_value() == 100);
    REQUIRE(cg.graph_value() == 100);
  }

  SECTION("string graph value") {
    compressed_graph<void, void, string> g(string("initial"));

    REQUIRE(g.graph_value() == "initial");

    // Test modification
    g.graph_value() = "modified";
    REQUIRE(g.graph_value() == "modified");

    // Test const correctness
    const auto& cg = g;
    REQUIRE(cg.graph_value() == "modified");
  }

  SECTION("double graph value") {
    compressed_graph<void, void, double> g(3.14);

    REQUIRE(g.graph_value() == 3.14);

    g.graph_value() = 2.71;
    REQUIRE(g.graph_value() == 2.71);
  }

  SECTION("graph_value() mutability") {
    compressed_graph<void, void, int> g(999);

    REQUIRE(g.graph_value() == 999);

    g.graph_value() = 111;
    REQUIRE(g.graph_value() == 111);

    g.graph_value() = 222;
    REQUIRE(g.graph_value() == 222);
  }

  SECTION("graph_value() with edges and vertices") {
    compressed_graph<int, int, string> g(string("metadata"));

    vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(ee);

    vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv);

    REQUIRE(g.graph_value() == "metadata");
    REQUIRE(g.size() == 3);

    // Modify graph value independently
    g.graph_value() = "updated";
    REQUIRE(g.graph_value() == "updated");
    REQUIRE(g.size() == 3); // Structure unchanged
  }
}

TEST_CASE("compressed_graph graph_value() with copy and move", "[api][graph_value][copy][move]") {
  SECTION("copy preserves graph_value") {
    compressed_graph<void, void, int>  g1(42);
    vector<copyable_edge_t<int, void>> ee = {{0, 1}, {1, 2}};
    g1.load_edges(ee);

    compressed_graph<void, void, int> g2(g1);

    REQUIRE(g2.graph_value() == 42);
    REQUIRE(g2.size() == g1.size());

    // Modify copy's graph value
    g2.graph_value() = 100;
    REQUIRE(g2.graph_value() == 100);
    REQUIRE(g1.graph_value() == 42); // Original unchanged
  }

  SECTION("move transfers graph_value") {
    compressed_graph<void, void, string> g1(string("original"));
    vector<copyable_edge_t<int, void>>   ee = {{0, 1}, {1, 2}};
    g1.load_edges(ee);

    compressed_graph<void, void, string> g2(std::move(g1));

    REQUIRE(g2.graph_value() == "original");
  }

  SECTION("copy assignment preserves graph_value") {
    compressed_graph<void, void, int> g1(42);
    compressed_graph<void, void, int> g2(999);

    g2 = g1;
    REQUIRE(g2.graph_value() == 42);
    REQUIRE(g1.graph_value() == 42);
  }

  SECTION("move assignment transfers graph_value") {
    compressed_graph<void, void, string> g1(string("source"));
    compressed_graph<void, void, string> g2(string("dest"));

    g2 = std::move(g1);
    REQUIRE(g2.graph_value() == "source");
  }
}

// =============================================================================
// Category 5: Boundary Condition Tests
// =============================================================================

TEST_CASE("compressed_graph handles empty graph", "[boundary][empty]") {
  compressed_graph<void, void, void> g;

  REQUIRE(g.empty());
  REQUIRE(g.size() == 0);

  // Test vertex_ids() on empty graph
  size_t count = 0;
  for ([[maybe_unused]] auto id : g.vertex_ids()) {
    ++count;
  }
  REQUIRE(count == 0);
}

TEST_CASE("compressed_graph handles single vertex", "[boundary][single]") {
  compressed_graph<void, int, void>   g;
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 1);
  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph handles single edge", "[boundary][single]") {
  compressed_graph<void, int, void>  g;
  vector<copyable_edge_t<int, void>> ee = {{0, 1}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
  REQUIRE(g.size() >= 2);
}

TEST_CASE("compressed_graph handles vertices with no edges", "[boundary][isolated]") {
  compressed_graph<void, int, void>   g;
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 3);
}

TEST_CASE("compressed_graph handles large vertex IDs", "[boundary][large_ids]") {
  compressed_graph<void, void, void, uint64_t, uint64_t> g;
  vector<copyable_edge_t<uint64_t, void>>                ee = {{0, 1000000}, {1000000, 2000000}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
}

// =============================================================================
// Category 6: Const-Correctness Tests
// =============================================================================

TEST_CASE("compressed_graph const methods work correctly", "[const][api]") {
  compressed_graph<void, int, void>   g;
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}};
  g.load_vertices(vv);

  const auto& cg = g;

  REQUIRE_FALSE(cg.empty());
  REQUIRE(cg.size() == 2);

  // Test const vertex_ids()
  size_t count = 0;
  for ([[maybe_unused]] auto id : cg.vertex_ids()) {
    ++count;
  }
  REQUIRE(count == 2);
}

TEST_CASE("compressed_graph const operator subscript", "[const][operator_subscript]") {
  compressed_graph<string, int, void>  g;
  vector<copyable_edge_t<int, string>> ee = {{0, 1, string("edge0")},
                                             {0, 2, string("edge1")},
                                             {1, 3, string("edge2")},
                                             {2, 4, string("edge3")},
                                             {3, 5, string("edge4")}};
  g.load_edges(ee);

  const auto& cg = g;
  // Access edge values by edge index
  REQUIRE(cg.edge_value(0) == "edge0");
  REQUIRE(cg.edge_value(2) == "edge2");
  REQUIRE(cg.edge_value(4) == "edge4");
}

// =============================================================================
// Category 7: Additional Tests
// =============================================================================

TEST_CASE("compressed_graph copy assignment", "[copy][assignment]") {
  compressed_graph<int, int, int>   g1(100);
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};
  g1.load_edges(ee);

  compressed_graph<int, int, int> g2(200);
  g2 = g1;

  REQUIRE(g2.graph_value() == 100);
  REQUIRE(g2.size() == g1.size());
}

TEST_CASE("compressed_graph move assignment", "[move][assignment]") {
  compressed_graph<int, int, int>   g1(100);
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};
  g1.load_edges(ee);

  auto                            original_size = g1.size();
  compressed_graph<int, int, int> g2(200);
  g2 = std::move(g1);

  REQUIRE(g2.graph_value() == 100);
  REQUIRE(g2.size() == original_size);
}

TEST_CASE("compressed_graph handles self-loops", "[edges][self_loop]") {
  compressed_graph<void, void, void> g;
  vector<copyable_edge_t<int, void>> ee = {{0, 0}, {1, 1}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph handles duplicate edges", "[edges][duplicates]") {
  compressed_graph<int, void, void> g;
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 1, 20}, {0, 1, 30}};
  g.load_edges(ee);

  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph load_edges and load_vertices together", "[api][combined]") {
  compressed_graph<int, int, void> g;

  // Load edges first (this creates the CSR structure)
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};
  g.load_edges(ee);

  // Then load vertices (this populates vertex values)
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
  g.load_vertices(vv);

  REQUIRE(g.size() == 3);
  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph clear empties the graph", "[api][clear]") {
  compressed_graph<void, int, void>   g;
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}};
  g.load_vertices(vv);

  REQUIRE_FALSE(g.empty());

  g.clear();

  REQUIRE(g.empty());
  REQUIRE(g.size() == 0);
}

TEST_CASE("compressed_graph iteration over vertices", "[api][iteration]") {
  compressed_graph<void, int, void>   g;
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
  g.load_vertices(vv);

  size_t count = 0;
  for ([[maybe_unused]] auto id : g.vertex_ids()) {
    ++count;
  }

  REQUIRE(count == 3);
}

TEST_CASE("compressed_graph range-based for loop", "[api][iteration][range]") {
  compressed_graph<void, int, void>   g;
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
  g.load_vertices(vv);

  size_t count = 0;
  for ([[maybe_unused]] auto id : g.vertex_ids()) {
    ++count;
  }

  REQUIRE(count == 3);
}

// =============================================================================
// Category: size(), empty(), and clear() Tests (Issue #1 Fix)
// =============================================================================

TEST_CASE("compressed_graph size() returns vertex count", "[api][size][issue1]") {
  SECTION("empty graph has size 0") {
    compressed_graph<void, void, void> g;
    REQUIRE(g.size() == 0);
  }

  SECTION("graph with vertices from load_vertices") {
    compressed_graph<void, int, void>   g;
    vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}, {3, 400}};
    g.load_vertices(vv);
    REQUIRE(g.size() == 4);
  }

  SECTION("graph with vertices from load_edges") {
    compressed_graph<void, int, void>  g;
    vector<copyable_edge_t<int, void>> ee = {{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}};
    g.load_edges(ee);
    REQUIRE(g.size() == 5); // vertices 0-4
  }

  SECTION("graph with mixed edge and vertex loading") {
    compressed_graph<int, int, void>  g;
    vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(ee);

    vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv);

    REQUIRE(g.size() == 3);
  }

  SECTION("graph with large number of vertices") {
    compressed_graph<void, void, void> g;
    vector<copyable_edge_t<int, void>> ee;
    for (int i = 0; i < 100; ++i) {
      ee.push_back({i, i + 1});
    }
    g.load_edges(ee);
    REQUIRE(g.size() == 101); // 0-100
  }
}

TEST_CASE("compressed_graph empty() checks if graph has vertices", "[api][empty][issue1]") {
  SECTION("newly constructed graph is empty") {
    compressed_graph<void, void, void> g;
    REQUIRE(g.empty());
  }

  SECTION("graph with graph value but no vertices is empty") {
    compressed_graph<void, void, int> g(42);
    REQUIRE(g.empty());
  }

  SECTION("graph after loading vertices is not empty") {
    compressed_graph<void, int, void>   g;
    vector<copyable_vertex_t<int, int>> vv = {{0, 100}};
    g.load_vertices(vv);
    REQUIRE_FALSE(g.empty());
  }

  SECTION("graph after loading edges is not empty") {
    compressed_graph<void, void, void> g;
    vector<copyable_edge_t<int, void>> ee = {{0, 1}};
    g.load_edges(ee);
    REQUIRE_FALSE(g.empty());
  }

  SECTION("graph becomes empty after clear") {
    compressed_graph<void, int, void>   g;
    vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}};
    g.load_vertices(vv);

    REQUIRE_FALSE(g.empty());
    g.clear();
    REQUIRE(g.empty());
  }
}

TEST_CASE("compressed_graph clear() removes all data", "[api][clear][issue1]") {
  SECTION("clear on empty graph is safe") {
    compressed_graph<void, void, void> g;
    REQUIRE_NOTHROW(g.clear());
    REQUIRE(g.empty());
    REQUIRE(g.size() == 0);
  }

  SECTION("clear removes vertices") {
    compressed_graph<void, int, void>   g;
    vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv);

    REQUIRE(g.size() == 3);
    g.clear();
    REQUIRE(g.size() == 0);
    REQUIRE(g.empty());
  }

  SECTION("clear removes edges") {
    compressed_graph<int, void, void> g;
    vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}};
    g.load_edges(ee);

    REQUIRE_FALSE(g.empty());
    g.clear();
    REQUIRE(g.empty());
  }

  SECTION("clear removes both edges and vertices") {
    compressed_graph<int, int, void>  g;
    vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(ee);

    vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv);

    REQUIRE(g.size() == 3);
    g.clear();
    REQUIRE(g.size() == 0);
    REQUIRE(g.empty());
  }

  SECTION("graph can be reused after clear") {
    compressed_graph<void, int, void> g;

    // First load
    vector<copyable_vertex_t<int, int>> vertices1 = {{0, 100}, {1, 200}};
    g.load_vertices(vertices1);
    REQUIRE(g.size() == 2);

    // Clear and reload
    g.clear();
    vector<copyable_vertex_t<int, int>> vertices2 = {{0, 300}, {1, 400}, {2, 500}};
    g.load_vertices(vertices2);
    REQUIRE(g.size() == 3);
  }

  SECTION("clear with graph value") {
    compressed_graph<void, void, int>  g(42);
    vector<copyable_edge_t<int, void>> ee = {{0, 1}, {1, 2}};
    g.load_edges(ee);

    g.clear();
    REQUIRE(g.empty());
    REQUIRE(g.size() == 0);
    // Note: graph value is not cleared by clear()
  }
}

TEST_CASE("compressed_graph size/empty/clear work with all value types", "[api][types][issue1]") {
  SECTION("void/void/void") {
    compressed_graph<void, void, void> g;
    REQUIRE(g.empty());
    REQUIRE(g.size() == 0);

    vector<copyable_edge_t<int, void>> ee = {{0, 1}, {1, 2}};
    g.load_edges(ee);
    REQUIRE_FALSE(g.empty());
    REQUIRE(g.size() == 3);

    g.clear();
    REQUIRE(g.empty());
    REQUIRE(g.size() == 0);
  }

  SECTION("int/int/int") {
    compressed_graph<int, int, int> g(999);
    REQUIRE(g.empty());

    vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};
    g.load_edges(ee);
    vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}};
    g.load_vertices(vv);

    REQUIRE_FALSE(g.empty());
    REQUIRE(g.size() == 2);

    g.clear();
    REQUIRE(g.empty());
  }

  SECTION("string/string/void") {
    compressed_graph<string, string, void> g;
    vector<copyable_edge_t<int, string>>   ee = {{0, 1, string("edge1")}, {1, 2, string("edge2")}};
    g.load_edges(ee);

    vector<copyable_vertex_t<int, string>> vv = {{0, string("v0")}, {1, string("v1")}, {2, string("v2")}};
    g.load_vertices(vv);

    REQUIRE(g.size() == 3);
    g.clear();
    REQUIRE(g.size() == 0);
  }
}

TEST_CASE("compressed_graph size/empty are consistent", "[api][size][empty][issue1]") {
  compressed_graph<void, int, void> g;

  SECTION("size == 0 implies empty") {
    REQUIRE(g.size() == 0);
    REQUIRE(g.empty());
  }

  SECTION("size > 0 implies not empty") {
    vector<copyable_vertex_t<int, int>> vv = {{0, 100}};
    g.load_vertices(vv);
    REQUIRE(g.size() > 0);
    REQUIRE_FALSE(g.empty());
  }

  SECTION("after clear, size == 0 and empty") {
    vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}};
    g.load_vertices(vv);
    g.clear();
    REQUIRE(g.size() == 0);
    REQUIRE(g.empty());
  }
}

TEST_CASE("compressed_graph clear preserves graph invariants", "[api][clear][issue1]") {
  compressed_graph<int, int, void> g;

  // Load some data
  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 2, 30}, {2, 3, 40}};
  g.load_edges(ee);

  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}, {3, 400}};
  g.load_vertices(vv);

  size_t original_size = g.size();
  REQUIRE(original_size == 4);

  // Clear the graph
  g.clear();

  // Verify all data is gone
  REQUIRE(g.size() == 0);
  REQUIRE(g.empty());

  // Verify we can reload new data
  vector<copyable_edge_t<int, int>> new_edges = {{0, 1, 50}, {1, 2, 60}};
  g.load_edges(new_edges);

  REQUIRE(g.size() == 3);
  REQUIRE_FALSE(g.empty());
}

TEST_CASE("compressed_graph const-correctness of size/empty", "[api][const][issue1]") {
  compressed_graph<void, int, void>   g;
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}};
  g.load_vertices(vv);

  const auto& cg = g;

  REQUIRE(cg.size() == 2);
  REQUIRE_FALSE(cg.empty());
}

// =============================================================================
// Category: vertex_ids() Tests (Issue #2 Fix)
// =============================================================================

TEST_CASE("compressed_graph vertex_ids() returns correct range", "[api][vertex_ids][issue2]") {
  SECTION("empty graph") {
    compressed_graph<void, void, void> g;

    size_t count = 0;
    for ([[maybe_unused]] auto id : g.vertex_ids()) {
      ++count;
    }

    REQUIRE(count == 0);
  }

  SECTION("graph with 5 vertices") {
    compressed_graph<void, void, void> g;
    vector<copyable_edge_t<int, void>> ee = {{0, 1}, {1, 2}, {2, 3}, {3, 4}};
    g.load_edges(ee);

    REQUIRE(g.size() == 5);

    vector<int> ids;
    for (auto id : g.vertex_ids()) {
      ids.push_back(static_cast<int>(id));
    }

    REQUIRE(ids.size() == 5);
    REQUIRE(ids == vector<int>{0, 1, 2, 3, 4});
  }

  SECTION("graph with explicit vertex values") {
    compressed_graph<void, int, void>   g;
    vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
    g.load_vertices(vv);

    size_t count = 0;
    for ([[maybe_unused]] auto id : g.vertex_ids()) {
      ++count;
    }

    REQUIRE(count == 3);
    REQUIRE(count == g.size());
  }

  SECTION("graph with vertices from edges") {
    compressed_graph<void, void, void> g;
    vector<copyable_edge_t<int, void>> ee = {{0, 1}, {0, 2}, {1, 2}, {1, 3}};
    g.load_edges(ee);

    size_t count = 0;
    for ([[maybe_unused]] auto id : g.vertex_ids()) {
      ++count;
    }

    REQUIRE(count == g.size());
    REQUIRE(count == 4); // vertices 0, 1, 2, 3
  }

  SECTION("const version") {
    compressed_graph<void, int, void>  g;
    vector<copyable_edge_t<int, void>> ee = {{0, 1}, {1, 2}, {2, 3}};
    g.load_edges(ee);

    const auto& cg = g;

    size_t count = 0;
    for ([[maybe_unused]] auto id : cg.vertex_ids()) {
      ++count;
    }

    REQUIRE(count == cg.size());
    REQUIRE(count == 4);
  }
}

TEST_CASE("compressed_graph vertex_ids() can access vertices", "[api][vertex_ids][issue2]") {
  compressed_graph<void, int, void>   g;
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}};
  g.load_vertices(vv);

  // Use vertex_ids() to iterate and verify all IDs are valid
  for (auto id : g.vertex_ids()) {
    REQUIRE(id < g.size()); // Verify ID is within valid range
  }
}

TEST_CASE("compressed_graph vertex_ids() with different template parameters", "[api][vertex_ids][types][issue2]") {
  SECTION("void/void/void") {
    compressed_graph<void, void, void> g;
    vector<copyable_edge_t<int, void>> ee = {{0, 1}, {1, 2}};
    g.load_edges(ee);

    size_t count = 0;
    for ([[maybe_unused]] auto id : g.vertex_ids()) {
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("int/int/void") {
    compressed_graph<int, int, void>  g;
    vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {1, 2, 20}};
    g.load_edges(ee);

    size_t count = 0;
    for ([[maybe_unused]] auto id : g.vertex_ids()) {
      ++count;
    }
    REQUIRE(count == 3);
  }

  SECTION("void/void/string (non-void GV)") {
    compressed_graph<void, void, string> g("test graph");
    vector<copyable_edge_t<int, void>>   ee = {{0, 1}, {1, 2}};
    g.load_edges(ee);

    size_t count = 0;
    for ([[maybe_unused]] auto id : g.vertex_ids()) {
      ++count;
    }
    REQUIRE(count == 3);
    REQUIRE(g.graph_value() == "test graph");
  }
}

TEST_CASE("compressed_graph vertex_ids() can be used with STL algorithms", "[api][vertex_ids][algorithms][issue2]") {
  compressed_graph<void, void, void> g;
  vector<copyable_edge_t<int, void>> ee = {{0, 2}, {2, 4}, {4, 6}};
  g.load_edges(ee);

  auto ids = g.vertex_ids();

  // Test std::accumulate (sum of IDs)
  int sum = std::accumulate(ids.begin(), ids.end(), 0);
  // IDs are 0, 1, 2, 3, 4, 5, 6 = 21
  REQUIRE(sum == 21);

  // Test std::any_of
  bool has_zero = std::any_of(ids.begin(), ids.end(), [](auto id) { return id == 0; });
  REQUIRE(has_zero);
}
TEST_CASE("compressed_graph vertex_value(id) returns correct value", "[api][vertex_value]") {
  using Graph = compressed_graph<void, int, void>;

  vector<copyable_edge_t<int, void>>  ee = {{0, 1}, {0, 2}, {1, 2}, {2, 3}};
  vector<copyable_vertex_t<int, int>> vv = {{0, 100}, {1, 200}, {2, 300}, {3, 400}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vv);

  REQUIRE(g.vertex_value(0) == 100);
  REQUIRE(g.vertex_value(1) == 200);
  REQUIRE(g.vertex_value(2) == 300);
  REQUIRE(g.vertex_value(3) == 400);
}

TEST_CASE("compressed_graph vertex_value(id) is mutable", "[api][vertex_value]") {
  using Graph = compressed_graph<void, string, void>;

  vector<copyable_edge_t<int, void>>     ee = {{0, 1}, {1, 2}};
  vector<copyable_vertex_t<int, string>> vv = {{0, string("a")}, {1, string("b")}, {2, string("c")}};

  Graph g;
  g.load_edges(ee);
  g.load_vertices(vv);

  REQUIRE(g.vertex_value(0) == "a");
  REQUIRE(g.vertex_value(1) == "b");
  REQUIRE(g.vertex_value(2) == "c");

  // Modify values
  g.vertex_value(0) = "x";
  g.vertex_value(1) = "y";
  g.vertex_value(2) = "z";

  REQUIRE(g.vertex_value(0) == "x");
  REQUIRE(g.vertex_value(1) == "y");
  REQUIRE(g.vertex_value(2) == "z");
}

TEST_CASE("compressed_graph vertex_value(id) works with const graph", "[api][vertex_value]") {
  using Graph = compressed_graph<int, double, void>;

  vector<copyable_edge_t<int, int>>      ee = {{0, 1, 10}, {1, 2, 20}};
  vector<copyable_vertex_t<int, double>> vv = {{0, 1.5}, {1, 2.5}, {2, 3.5}};

  Graph g_temp;
  g_temp.load_edges(ee);
  g_temp.load_vertices(vv);

  const Graph g = std::move(g_temp);

  REQUIRE(g.vertex_value(0) == 1.5);
  REQUIRE(g.vertex_value(1) == 2.5);
  REQUIRE(g.vertex_value(2) == 3.5);
}

TEST_CASE("compressed_graph vertex_value(id) with graph value", "[api][vertex_value]") {
  using Graph = compressed_graph<void, int, string>;

  vector<copyable_edge_t<int, void>>  ee = {{0, 1}, {1, 2}};
  vector<copyable_vertex_t<int, int>> vv = {{0, 10}, {1, 20}, {2, 30}};

  Graph g(string("graph_metadata"));
  g.load_edges(ee);
  g.load_vertices(vv);

  REQUIRE(g.graph_value() == "graph_metadata");
  REQUIRE(g.vertex_value(0) == 10);
  REQUIRE(g.vertex_value(1) == 20);
  REQUIRE(g.vertex_value(2) == 30);
}

// =============================================================================
// Category: edge_ids() Tests
// =============================================================================

TEST_CASE("compressed_graph edge_ids() returns correct range of indices", "[api][edge_ids]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {
        {0, 1, 10}, {0, 2, 20}, {0, 3, 30}, // vertex 0 has 3 edges (indices 0, 1, 2)
        {1, 2, 40}, {1, 3, 50},             // vertex 1 has 2 edges (indices 3, 4)
        {2, 3, 60}                          // vertex 2 has 1 edge (index 5)
  };

  Graph g;
  g.load_edges(ee);

  SECTION("vertex 0 edge indices") {
    auto                 ids = g.edge_ids(0);
    vector<unsigned int> collected(ids.begin(), ids.end());
    REQUIRE(collected.size() == 3);
    REQUIRE(collected == vector<unsigned int>{0, 1, 2});
  }

  SECTION("vertex 1 edge indices") {
    auto                 ids = g.edge_ids(1);
    vector<unsigned int> collected(ids.begin(), ids.end());
    REQUIRE(collected.size() == 2);
    REQUIRE(collected == vector<unsigned int>{3, 4});
  }

  SECTION("vertex 2 edge indices") {
    auto                 ids = g.edge_ids(2);
    vector<unsigned int> collected(ids.begin(), ids.end());
    REQUIRE(collected.size() == 1);
    REQUIRE(collected == vector<unsigned int>{5});
  }

  SECTION("vertex 3 (no edges) has empty range") {
    auto ids = g.edge_ids(3);
    REQUIRE(ids.begin() == ids.end());
    REQUIRE(std::ranges::distance(ids) == 0);
  }
}

TEST_CASE("compressed_graph edge_ids() works with empty graph", "[api][edge_ids][empty]") {
  compressed_graph<int, void, void> g;

  auto ids = g.edge_ids(0);
  REQUIRE(ids.begin() == ids.end());
  REQUIRE(std::ranges::distance(ids) == 0);
}

TEST_CASE("compressed_graph edge_ids() handles out of bounds gracefully", "[api][edge_ids][bounds]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}};

  Graph g;
  g.load_edges(ee);

  SECTION("vertex ID beyond graph size returns empty range") {
    auto ids = g.edge_ids(100);
    REQUIRE(ids.begin() == ids.end());
    REQUIRE(std::ranges::distance(ids) == 0);
  }

  SECTION("vertex at boundary with no edges") {
    auto ids = g.edge_ids(2); // vertex 2 exists but has no edges
    REQUIRE(ids.begin() == ids.end());
  }
}

TEST_CASE("compressed_graph edge_ids() works with STL algorithms", "[api][edge_ids][algorithms]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}, {0, 4, 40}};

  Graph g;
  g.load_edges(ee);

  auto ids = g.edge_ids(0);

  SECTION("std::ranges::count works") {
    auto count = std::ranges::distance(ids);
    REQUIRE(count == 4);
  }

  SECTION("can be used with std::accumulate") {
    auto sum = std::accumulate(ids.begin(), ids.end(), 0u);
    REQUIRE(sum == 0 + 1 + 2 + 3); // sum of indices 0, 1, 2, 3
  }

  SECTION("can be used with range-based for") {
    int count = 0;
    for (auto id : ids) {
      (void)id; // suppress unused warning
      ++count;
    }
    REQUIRE(count == 4);
  }
}

TEST_CASE("compressed_graph edge_ids() is lightweight view", "[api][edge_ids][view]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {0, 3, 30}};

  Graph g;
  g.load_edges(ee);

  SECTION("can create multiple views without overhead") {
    auto ids1 = g.edge_ids(0);
    auto ids2 = g.edge_ids(0);

    REQUIRE(std::ranges::distance(ids1) == 3);
    REQUIRE(std::ranges::distance(ids2) == 3);
  }

  SECTION("views are independent") {
    auto ids1 = g.edge_ids(0);
    auto it1  = ids1.begin();
    ++it1;

    auto ids2 = g.edge_ids(0);
    auto it2  = ids2.begin();

    REQUIRE(*it1 == 1);
    REQUIRE(*it2 == 0);
  }
}

TEST_CASE("compressed_graph edge_ids() returns all edge IDs", "[api][edge_ids][all]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {
        {0, 1, 10}, {0, 2, 20}, {0, 3, 30}, // 3 edges
        {1, 2, 40}, {1, 3, 50},             // 2 edges
        {2, 3, 60}                          // 1 edge
  };

  Graph g;
  g.load_edges(ee);

  SECTION("returns range with correct size") {
    auto ids = g.edge_ids();
    REQUIRE(std::ranges::distance(ids) == 6);
  }

  SECTION("returns indices 0 through n-1") {
    auto                 ids = g.edge_ids();
    vector<unsigned int> collected(ids.begin(), ids.end());
    REQUIRE(collected == vector<unsigned int>{0, 1, 2, 3, 4, 5});
  }

  SECTION("can iterate with range-based for") {
    auto ids   = g.edge_ids();
    int  count = 0;
    for (auto id : ids) {
      REQUIRE(id < 6);
      ++count;
    }
    REQUIRE(count == 6);
  }
}

TEST_CASE("compressed_graph edge_ids() with empty graph", "[api][edge_ids][all][empty]") {
  compressed_graph<int, void, void> g;

  auto ids = g.edge_ids();
  REQUIRE(ids.begin() == ids.end());
  REQUIRE(std::ranges::distance(ids) == 0);
}

TEST_CASE("compressed_graph edge_ids() with single edge", "[api][edge_ids][all][single]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}};

  Graph g;
  g.load_edges(ee);

  auto                 ids = g.edge_ids();
  vector<unsigned int> collected(ids.begin(), ids.end());
  REQUIRE(collected.size() == 1);
  REQUIRE(collected == vector<unsigned int>{0});
}

TEST_CASE("compressed_graph edge_ids() works with STL algorithms", "[api][edge_ids][all][algorithms]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}, {2, 3, 40}};

  Graph g;
  g.load_edges(ee);

  auto ids = g.edge_ids();

  SECTION("std::ranges::count works") {
    auto count = std::ranges::distance(ids);
    REQUIRE(count == 4);
  }

  SECTION("can be used with std::accumulate") {
    auto sum = std::accumulate(ids.begin(), ids.end(), 0u);
    REQUIRE(sum == 0 + 1 + 2 + 3);
  }

  SECTION("can filter with std::views::filter") {
    auto                 even_ids = ids | std::views::filter([](auto id) { return id % 2 == 0; });
    vector<unsigned int> collected(even_ids.begin(), even_ids.end());
    REQUIRE(collected == vector<unsigned int>{0, 2});
  }
}

TEST_CASE("compressed_graph edge_ids() can access edge data", "[api][edge_ids][all][access]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 10, 100}, {0, 20, 200}, {1, 30, 300}};

  Graph g;
  g.load_edges(ee);

  SECTION("can use edge_ids() to access target_id()") {
    auto                 ids = g.edge_ids();
    vector<unsigned int> targets;
    for (auto edge_id : ids) {
      targets.push_back(g.target_id(edge_id));
    }
    REQUIRE(targets == vector<unsigned int>{10, 20, 30});
  }

  SECTION("can use edge_ids() to access edge values") {
    auto        ids = g.edge_ids();
    vector<int> values;
    for (auto edge_id : ids) {
      values.push_back(g.edge_value(edge_id));
    }
    REQUIRE(values == vector<int>{100, 200, 300});
  }
}

TEST_CASE("compressed_graph edge_ids() is lightweight view", "[api][edge_ids][all][view]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}};

  Graph g;
  g.load_edges(ee);

  SECTION("can create multiple views without overhead") {
    auto ids1 = g.edge_ids();
    auto ids2 = g.edge_ids();

    REQUIRE(std::ranges::distance(ids1) == 3);
    REQUIRE(std::ranges::distance(ids2) == 3);
  }

  SECTION("views are independent") {
    auto ids1 = g.edge_ids();
    auto it1  = ids1.begin();
    ++it1;

    auto ids2 = g.edge_ids();
    auto it2  = ids2.begin();

    REQUIRE(*it1 == 1);
    REQUIRE(*it2 == 0);
  }
}

// =============================================================================
// Category: target_id() Tests
// =============================================================================

TEST_CASE("compressed_graph target_id() returns correct target vertex", "[api][target_id]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {
        {0, 10, 100}, {0, 20, 200}, {0, 30, 300}, // vertex 0 edges to 10, 20, 30
        {1, 40, 400}, {1, 50, 500},               // vertex 1 edges to 40, 50
        {2, 60, 600}                              // vertex 2 edge to 60
  };

  Graph g;
  g.load_edges(ee);

  SECTION("vertex 0 edge targets") {
    auto                 edge_ids = g.edge_ids(0);
    vector<unsigned int> ids(edge_ids.begin(), edge_ids.end());

    REQUIRE(ids.size() == 3);
    REQUIRE(g.target_id(ids[0]) == 10);
    REQUIRE(g.target_id(ids[1]) == 20);
    REQUIRE(g.target_id(ids[2]) == 30);
  }

  SECTION("vertex 1 edge targets") {
    auto                 edge_ids = g.edge_ids(1);
    vector<unsigned int> ids(edge_ids.begin(), edge_ids.end());

    REQUIRE(ids.size() == 2);
    REQUIRE(g.target_id(ids[0]) == 40);
    REQUIRE(g.target_id(ids[1]) == 50);
  }

  SECTION("vertex 2 edge target") {
    auto                 edge_ids = g.edge_ids(2);
    vector<unsigned int> ids(edge_ids.begin(), edge_ids.end());

    REQUIRE(ids.size() == 1);
    REQUIRE(g.target_id(ids[0]) == 60);
  }
}

TEST_CASE("compressed_graph target_id() works with edge iteration", "[api][target_id][iteration]") {
  using Graph = compressed_graph<void, void, void>;

  vector<copyable_edge_t<int, void>> ee = {{0, 1}, {0, 2}, {1, 3}, {2, 3}, {3, 4}};

  Graph g;
  g.load_edges(ee);

  SECTION("iterate edges and get targets") {
    vector<int> expected_targets = {1, 2, 3, 3, 4};
    size_t      idx              = 0;

    for (auto vid : g.vertex_ids()) {
      for (auto edge_id : g.edge_ids(vid)) {
        REQUIRE(g.target_id(edge_id) == static_cast<unsigned>(expected_targets[idx]));
        ++idx;
      }
    }

    REQUIRE(idx == expected_targets.size());
  }
}

TEST_CASE("compressed_graph target_id() with self-loops", "[api][target_id][self_loop]") {
  using Graph = compressed_graph<void, void, void>;

  vector<copyable_edge_t<int, void>> ee = {{0, 0}, {0, 1}, {1, 1}};

  Graph g;
  g.load_edges(ee);

  auto                 ids_0 = g.edge_ids(0);
  vector<unsigned int> v0(ids_0.begin(), ids_0.end());
  REQUIRE(g.target_id(v0[0]) == 0); // self-loop
  REQUIRE(g.target_id(v0[1]) == 1);

  auto                 ids_1 = g.edge_ids(1);
  vector<unsigned int> v1(ids_1.begin(), ids_1.end());
  REQUIRE(g.target_id(v1[0]) == 1); // self-loop
}

TEST_CASE("compressed_graph target_id() consistency", "[api][target_id][consistency]") {
  using Graph = compressed_graph<void, void, void>;

  vector<copyable_edge_t<int, void>> ee = {{0, 10}, {0, 20}, {1, 30}, {2, 40}};

  Graph g;
  g.load_edges(ee);

  // Verify target_id() returns correct targets
  auto                 ids_0 = g.edge_ids(0);
  vector<unsigned int> collected_0(ids_0.begin(), ids_0.end());
  REQUIRE(collected_0.size() == 2);
  REQUIRE(g.target_id(collected_0[0]) == 10);
  REQUIRE(g.target_id(collected_0[1]) == 20);

  auto                 ids_1 = g.edge_ids(1);
  vector<unsigned int> collected_1(ids_1.begin(), ids_1.end());
  REQUIRE(collected_1.size() == 1);
  REQUIRE(g.target_id(collected_1[0]) == 30);

  auto                 ids_2 = g.edge_ids(2);
  vector<unsigned int> collected_2(ids_2.begin(), ids_2.end());
  REQUIRE(collected_2.size() == 1);
  REQUIRE(g.target_id(collected_2[0]) == 40);
}

TEST_CASE("compressed_graph target_id() with large vertex IDs", "[api][target_id][large_ids]") {
  using Graph = compressed_graph<void, void, void, uint64_t, uint64_t>;

  vector<copyable_edge_t<uint64_t, void>> ee = {{0, 1000000}, {1000000, 2000000}};

  Graph g;
  g.load_edges(ee);

  auto ids_0 = g.edge_ids(0);
  REQUIRE(g.target_id(*ids_0.begin()) == 1000000);

  auto ids_1m = g.edge_ids(1000000);
  REQUIRE(g.target_id(*ids_1m.begin()) == 2000000);
}

// =============================================================================
// Category: edge_value() Tests
// =============================================================================

TEST_CASE("compressed_graph edge_value() returns correct value", "[api][edge_value]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {
        {0, 1, 100}, {0, 2, 200}, {0, 3, 300}, // vertex 0 edges with values
        {1, 2, 400}, {1, 3, 500},              // vertex 1 edges with values
        {2, 3, 600}                            // vertex 2 edge with value
  };

  Graph g;
  g.load_edges(ee);

  SECTION("vertex 0 edge values") {
    auto                 edge_ids = g.edge_ids(0);
    vector<unsigned int> ids(edge_ids.begin(), edge_ids.end());

    REQUIRE(ids.size() == 3);
    REQUIRE(g.edge_value(ids[0]) == 100);
    REQUIRE(g.edge_value(ids[1]) == 200);
    REQUIRE(g.edge_value(ids[2]) == 300);
  }

  SECTION("vertex 1 edge values") {
    auto                 edge_ids = g.edge_ids(1);
    vector<unsigned int> ids(edge_ids.begin(), edge_ids.end());

    REQUIRE(ids.size() == 2);
    REQUIRE(g.edge_value(ids[0]) == 400);
    REQUIRE(g.edge_value(ids[1]) == 500);
  }

  SECTION("vertex 2 edge value") {
    auto                 edge_ids = g.edge_ids(2);
    vector<unsigned int> ids(edge_ids.begin(), edge_ids.end());

    REQUIRE(ids.size() == 1);
    REQUIRE(g.edge_value(ids[0]) == 600);
  }
}

TEST_CASE("compressed_graph edge_value() is mutable", "[api][edge_value][mutable]") {
  using Graph = compressed_graph<string, void, void>;

  vector<copyable_edge_t<int, string>> ee = {{0, 1, string("a")}, {0, 2, string("b")}, {1, 2, string("c")}};

  Graph g;
  g.load_edges(ee);

  auto                 ids = g.edge_ids(0);
  vector<unsigned int> vid(ids.begin(), ids.end());

  REQUIRE(g.edge_value(vid[0]) == "a");

  // Modify the edge value
  g.edge_value(vid[0]) = "modified";
  REQUIRE(g.edge_value(vid[0]) == "modified");

  // Verify other values unchanged
  REQUIRE(g.edge_value(vid[1]) == "b");
}

TEST_CASE("compressed_graph edge_value() const correctness", "[api][edge_value][const]") {
  using Graph = compressed_graph<double, void, void>;

  vector<copyable_edge_t<int, double>> ee = {{0, 1, 1.5}, {0, 2, 2.5}, {1, 2, 3.5}};

  Graph g;
  g.load_edges(ee);

  const auto& cg = g;

  auto                 ids = cg.edge_ids(0);
  vector<unsigned int> vid(ids.begin(), ids.end());

  REQUIRE(cg.edge_value(vid[0]) == 1.5);
  REQUIRE(cg.edge_value(vid[1]) == 2.5);
}

TEST_CASE("compressed_graph edge_value() with complex types", "[api][edge_value][types]") {
  using Graph = compressed_graph<string, void, void>;

  vector<copyable_edge_t<int, string>> ee = {
        {0, 1, string("edge_0_1")}, {0, 2, string("edge_0_2")}, {1, 3, string("edge_1_3")}};

  Graph g;
  g.load_edges(ee);

  auto                 ids_0 = g.edge_ids(0);
  vector<unsigned int> v0(ids_0.begin(), ids_0.end());

  REQUIRE(g.edge_value(v0[0]) == "edge_0_1");
  REQUIRE(g.edge_value(v0[1]) == "edge_0_2");

  auto ids_1 = g.edge_ids(1);
  REQUIRE(g.edge_value(*ids_1.begin()) == "edge_1_3");
}

TEST_CASE("compressed_graph edge_value() iteration over all edges", "[api][edge_value][iteration]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}, {2, 3, 40}};

  Graph g;
  g.load_edges(ee);

  vector<int> expected_values = {10, 20, 30, 40};
  size_t      idx             = 0;

  for (auto vid : g.vertex_ids()) {
    for (auto edge_id : g.edge_ids(vid)) {
      REQUIRE(g.edge_value(edge_id) == expected_values[idx]);
      ++idx;
    }
  }

  REQUIRE(idx == expected_values.size());
}

TEST_CASE("compressed_graph edge_value() with self-loops", "[api][edge_value][self_loop]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 0, 100}, {0, 1, 200}, {1, 1, 300}};

  Graph g;
  g.load_edges(ee);

  auto                 ids_0 = g.edge_ids(0);
  vector<unsigned int> v0(ids_0.begin(), ids_0.end());
  REQUIRE(g.edge_value(v0[0]) == 100); // self-loop value
  REQUIRE(g.edge_value(v0[1]) == 200);

  auto ids_1 = g.edge_ids(1);
  REQUIRE(g.edge_value(*ids_1.begin()) == 300); // self-loop value
}

TEST_CASE("compressed_graph edge_value() combined with target_id()", "[api][edge_value][combined]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 10, 100}, {0, 20, 200}, {1, 30, 300}};

  Graph g;
  g.load_edges(ee);

  SECTION("verify both target and value for vertex 0") {
    auto                 ids = g.edge_ids(0);
    vector<unsigned int> vid(ids.begin(), ids.end());

    REQUIRE(g.target_id(vid[0]) == 10);
    REQUIRE(g.edge_value(vid[0]) == 100);

    REQUIRE(g.target_id(vid[1]) == 20);
    REQUIRE(g.edge_value(vid[1]) == 200);
  }

  SECTION("iterate and verify both") {
    for (auto vid : g.vertex_ids()) {
      for (auto edge_id : g.edge_ids(vid)) {
        auto targ  = g.target_id(edge_id);
        auto value = g.edge_value(edge_id);

        // Verify relationship: value = target * 10
        REQUIRE(value == static_cast<int>(targ) * 10);
      }
    }
  }
}

TEST_CASE("compressed_graph edge_value() modification test", "[api][edge_value][modify]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 10}, {0, 2, 20}, {1, 3, 30}};

  Graph g;
  g.load_edges(ee);

  // Modify all edge values
  for (auto vid : g.vertex_ids()) {
    for (auto edge_id : g.edge_ids(vid)) {
      g.edge_value(edge_id) *= 2;
    }
  }

  // Verify modifications
  auto                 ids_0 = g.edge_ids(0);
  vector<unsigned int> v0(ids_0.begin(), ids_0.end());
  REQUIRE(g.edge_value(v0[0]) == 20);
  REQUIRE(g.edge_value(v0[1]) == 40);

  auto ids_1 = g.edge_ids(1);
  REQUIRE(g.edge_value(*ids_1.begin()) == 60);
}

TEST_CASE("compressed_graph edge_value() with single edge", "[api][edge_value][single]") {
  using Graph = compressed_graph<int, void, void>;

  vector<copyable_edge_t<int, int>> ee = {{0, 1, 42}};

  Graph g;
  g.load_edges(ee);

  auto ids = g.edge_ids(0);
  REQUIRE(g.edge_value(*ids.begin()) == 42);
}

TEST_CASE("compressed_graph edge_value() with multiple edge types", "[api][edge_value][types]") {
  SECTION("int edge values") {
    using Graph                          = compressed_graph<int, void, void>;
    vector<copyable_edge_t<int, int>> ee = {{0, 1, 100}};
    Graph                             g;
    g.load_edges(ee);
    REQUIRE(g.edge_value(*g.edge_ids(0).begin()) == 100);
  }

  SECTION("double edge values") {
    using Graph                             = compressed_graph<double, void, void>;
    vector<copyable_edge_t<int, double>> ee = {{0, 1, 3.14}};
    Graph                                g;
    g.load_edges(ee);
    REQUIRE(g.edge_value(*g.edge_ids(0).begin()) == 3.14);
  }

  SECTION("string edge values") {
    using Graph                             = compressed_graph<string, void, void>;
    vector<copyable_edge_t<int, string>> ee = {{0, 1, string("test")}};
    Graph                                g;
    g.load_edges(ee);
    REQUIRE(g.edge_value(*g.edge_ids(0).begin()) == "test");
  }
}
