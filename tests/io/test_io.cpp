/**
 * @file test_io.cpp
 * @brief Tests for graph I/O (DOT, GraphML, JSON).
 */

#include <catch2/catch_test_macros.hpp>

#include <graph/io.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

using namespace graph;
using namespace graph::io;

// ---------------------------------------------------------------------------
// Helper: build a small weighted directed graph
// ---------------------------------------------------------------------------

using weighted_graph_t = container::dynamic_graph<double, void, void, uint32_t, false,
    container::vov_graph_traits<double, void, void, uint32_t, false>>;

static weighted_graph_t make_test_graph() {
  // Triangle: 0->1 (1.5), 0->2 (2.5), 1->2 (3.5)
  using edge_t = graph::copyable_edge_t<uint32_t, double>;
  std::vector<edge_t> edge_list = {{0, 1, 1.5}, {0, 2, 2.5}, {1, 2, 3.5}};
  weighted_graph_t g;
  g.load_edges(edge_list, std::identity{}, uint32_t{3});
  return g;
}

// Unweighted graph (EV = void)
using plain_graph_t = container::dynamic_graph<void, void, void, uint32_t, false,
    container::vov_graph_traits<void, void, void, uint32_t, false>>;

static plain_graph_t make_plain_graph() {
  using edge_t = graph::copyable_edge_t<uint32_t, void>;
  std::vector<edge_t> edge_list = {{0, 1}, {0, 2}, {1, 2}, {2, 0}};
  plain_graph_t g;
  g.load_edges(edge_list, std::identity{}, uint32_t{3});
  return g;
}

// ===========================================================================
// DOT tests
// ===========================================================================

TEST_CASE("write_dot: weighted graph produces valid DOT", "[io][dot]") {
  auto g = make_test_graph();
  std::ostringstream os;
  write_dot(os, g);
  std::string output = os.str();

  REQUIRE(output.find("digraph G {") != std::string::npos);
  REQUIRE(output.find("0 -> 1") != std::string::npos);
  REQUIRE(output.find("0 -> 2") != std::string::npos);
  REQUIRE(output.find("1 -> 2") != std::string::npos);
  // Edge values should appear as labels
  REQUIRE(output.find("1.5") != std::string::npos);
  REQUIRE(output.find("2.5") != std::string::npos);
  REQUIRE(output.find("3.5") != std::string::npos);
  REQUIRE(output.find("}") != std::string::npos);
}

TEST_CASE("write_dot: plain graph omits labels", "[io][dot]") {
  auto g = make_plain_graph();
  std::ostringstream os;
  write_dot(os, g);
  std::string output = os.str();

  REQUIRE(output.find("digraph G {") != std::string::npos);
  REQUIRE(output.find("0 -> 1") != std::string::npos);
  REQUIRE(output.find("[label=") == std::string::npos);
}

TEST_CASE("write_dot: custom graph name", "[io][dot]") {
  auto g = make_plain_graph();
  std::ostringstream os;
  write_dot(os, g, "MyGraph");
  std::string output = os.str();

  REQUIRE(output.find("digraph MyGraph {") != std::string::npos);
}

TEST_CASE("write_dot: user attribute functions", "[io][dot]") {
  auto g = make_test_graph();
  std::ostringstream os;
  write_dot(os, g,
    [](const auto& /*g*/, auto uid) -> std::string {
      return std::format("[label=\"v{}\"]", uid);
    },
    [](const auto& gr, auto /*src*/, auto /*tgt*/, auto uv) -> std::string {
      return std::format("[weight={:.1f}]", edge_value(gr, uv));
    });
  std::string output = os.str();

  REQUIRE(output.find("[label=\"v0\"]") != std::string::npos);
  REQUIRE(output.find("[label=\"v1\"]") != std::string::npos);
  REQUIRE(output.find("[weight=1.5]") != std::string::npos);
  REQUIRE(output.find("[weight=3.5]") != std::string::npos);
}

TEST_CASE("read_dot: parse simple digraph", "[io][dot]") {
  std::istringstream is(R"(
    digraph TestGraph {
      0 [label="start"];
      1 [label="end"];
      0 -> 1 [label="go"];
      1 -> 0;
    }
  )");

  auto result = read_dot(is);
  REQUIRE(result.directed == true);
  REQUIRE(result.name == "TestGraph");
  REQUIRE(result.vertex_ids.size() == 2);
  REQUIRE(result.edges.size() == 2);
  REQUIRE(result.edges[0].source == "0");
  REQUIRE(result.edges[0].target == "1");
  REQUIRE(result.edges[0].label == "go");
  REQUIRE(result.edges[1].source == "1");
  REQUIRE(result.edges[1].target == "0");
  REQUIRE(result.vertex_labels[0] == "start");
  REQUIRE(result.vertex_labels[1] == "end");
}

TEST_CASE("read_dot: parse undirected graph", "[io][dot]") {
  std::istringstream is(R"(
    graph UG {
      A -- B;
      B -- C;
    }
  )");

  auto result = read_dot(is);
  REQUIRE(result.directed == false);
  REQUIRE(result.name == "UG");
  REQUIRE(result.vertex_ids.size() == 3);
  REQUIRE(result.edges.size() == 2);
  REQUIRE(result.edges[0].source == "A");
  REQUIRE(result.edges[0].target == "B");
}

TEST_CASE("DOT roundtrip: write then read", "[io][dot]") {
  auto g = make_test_graph();
  std::ostringstream oss;
  write_dot(oss, g);

  std::istringstream iss(oss.str());
  auto parsed = read_dot(iss);

  REQUIRE(parsed.directed == true);
  REQUIRE(parsed.vertex_ids.size() == 3);
  REQUIRE(parsed.edges.size() == 3);
}

// ===========================================================================
// GraphML tests
// ===========================================================================

TEST_CASE("write_graphml: weighted graph produces valid XML", "[io][graphml]") {
  auto g = make_test_graph();
  std::ostringstream os;
  write_graphml(os, g);
  std::string output = os.str();

  REQUIRE(output.find("<?xml") != std::string::npos);
  REQUIRE(output.find("<graphml") != std::string::npos);
  REQUIRE(output.find("<graph id=\"G\"") != std::string::npos);
  REQUIRE(output.find("<node id=\"n0\"") != std::string::npos);
  REQUIRE(output.find("<node id=\"n1\"") != std::string::npos);
  REQUIRE(output.find("<node id=\"n2\"") != std::string::npos);
  REQUIRE(output.find("source=\"n0\" target=\"n1\"") != std::string::npos);
  REQUIRE(output.find("1.5") != std::string::npos);
  REQUIRE(output.find("</graphml>") != std::string::npos);
}

TEST_CASE("write_graphml: plain graph has no data elements", "[io][graphml]") {
  auto g = make_plain_graph();
  std::ostringstream os;
  write_graphml(os, g);
  std::string output = os.str();

  REQUIRE(output.find("<node id=\"n0\"/>") != std::string::npos);
  REQUIRE(output.find("<data") == std::string::npos);
}

TEST_CASE("read_graphml: parse nodes and edges", "[io][graphml]") {
  std::istringstream is(R"(<?xml version="1.0" encoding="UTF-8"?>
<graphml xmlns="http://graphml.graphdrawing.org/xmlns">
  <key id="d0" for="node" attr.name="label" attr.type="string"/>
  <key id="d1" for="edge" attr.name="weight" attr.type="string"/>
  <graph id="test" edgedefault="directed">
    <node id="n0">
      <data key="d0">start</data>
    </node>
    <node id="n1">
      <data key="d0">end</data>
    </node>
    <edge id="e0" source="n0" target="n1">
      <data key="d1">3.14</data>
    </edge>
  </graph>
</graphml>
  )");

  auto result = read_graphml(is);
  REQUIRE(result.directed == true);
  REQUIRE(result.id == "test");
  REQUIRE(result.keys.size() == 2);
  REQUIRE(result.nodes.size() == 2);
  REQUIRE(result.edges.size() == 1);

  REQUIRE(result.nodes[0].id == "n0");
  REQUIRE(result.nodes[0].data["d0"] == "start");
  REQUIRE(result.nodes[1].data["d0"] == "end");

  REQUIRE(result.edges[0].source == "n0");
  REQUIRE(result.edges[0].target == "n1");
  REQUIRE(result.edges[0].data["d1"] == "3.14");

  REQUIRE(result.keys[0].name == "label");
  REQUIRE(result.keys[1].name == "weight");
}

TEST_CASE("GraphML roundtrip: write then read", "[io][graphml]") {
  auto g = make_test_graph();
  std::ostringstream oss;
  write_graphml(oss, g);

  std::istringstream iss(oss.str());
  auto parsed = read_graphml(iss);

  REQUIRE(parsed.nodes.size() == 3);
  REQUIRE(parsed.edges.size() == 3);
  REQUIRE(parsed.edges[0].source == "n0");
  REQUIRE(parsed.edges[0].target == "n1");
}

// ===========================================================================
// JSON tests
// ===========================================================================

TEST_CASE("write_json: weighted graph produces valid JSON", "[io][json]") {
  auto g = make_test_graph();
  std::ostringstream os;
  write_json(os, g);
  std::string output = os.str();

  REQUIRE(output.find("\"directed\":") != std::string::npos);
  REQUIRE(output.find("\"nodes\":") != std::string::npos);
  REQUIRE(output.find("\"edges\":") != std::string::npos);
  REQUIRE(output.find("\"id\":") != std::string::npos);
  REQUIRE(output.find("\"source\":") != std::string::npos);
  REQUIRE(output.find("\"target\":") != std::string::npos);
  REQUIRE(output.find("\"label\":") != std::string::npos);
  REQUIRE(output.find("1.5") != std::string::npos);
}

TEST_CASE("write_json: plain graph has no labels", "[io][json]") {
  auto g = make_plain_graph();
  std::ostringstream os;
  write_json(os, g);
  std::string output = os.str();

  REQUIRE(output.find("\"label\"") == std::string::npos);
}

TEST_CASE("write_json: compact mode (indent=0)", "[io][json]") {
  auto g = make_plain_graph();
  std::ostringstream os;
  write_json(os, g, 0);
  std::string output = os.str();

  // Should have no indentation spaces at start of lines
  REQUIRE(output.find("  ") == std::string::npos);
}

TEST_CASE("read_json: parse nodes and edges", "[io][json]") {
  std::istringstream is(R"({
    "directed": true,
    "nodes": [
      {"id": "0", "label": "start"},
      {"id": "1", "label": "end"}
    ],
    "edges": [
      {"source": "0", "target": "1", "weight": "3.14"}
    ]
  })");

  auto result = read_json(is);
  REQUIRE(result.directed == true);
  REQUIRE(result.nodes.size() == 2);
  REQUIRE(result.edges.size() == 1);

  REQUIRE(result.nodes[0].id == "0");
  REQUIRE(result.nodes[0].attrs["label"] == "start");
  REQUIRE(result.nodes[1].id == "1");
  REQUIRE(result.nodes[1].attrs["label"] == "end");

  REQUIRE(result.edges[0].source == "0");
  REQUIRE(result.edges[0].target == "1");
  REQUIRE(result.edges[0].attrs["weight"] == "3.14");
}

TEST_CASE("read_json: numeric ids", "[io][json]") {
  std::istringstream is(R"({
    "directed": false,
    "nodes": [{"id": 0}, {"id": 1}, {"id": 2}],
    "edges": [{"source": 0, "target": 1}, {"source": 1, "target": 2}]
  })");

  auto result = read_json(is);
  REQUIRE(result.directed == false);
  REQUIRE(result.nodes.size() == 3);
  REQUIRE(result.edges.size() == 2);
  REQUIRE(result.nodes[0].id == "0");
  REQUIRE(result.edges[0].source == "0");
  REQUIRE(result.edges[0].target == "1");
}

TEST_CASE("JSON roundtrip: write then read", "[io][json]") {
  auto g = make_test_graph();
  std::ostringstream oss;
  write_json(oss, g);

  std::istringstream iss(oss.str());
  auto parsed = read_json(iss);

  REQUIRE(parsed.directed == true);
  REQUIRE(parsed.nodes.size() == 3);
  REQUIRE(parsed.edges.size() == 3);
}

// ===========================================================================
// Special characters / escaping
// ===========================================================================

TEST_CASE("DOT escapes special characters in labels", "[io][dot]") {
  std::istringstream is(R"(
    digraph G {
      0 [label="hello world"];
      0 -> 1;
    }
  )");
  auto result = read_dot(is);
  REQUIRE(result.vertex_labels[0] == "hello world");
}

TEST_CASE("write_graphml: XML-escapes values", "[io][graphml]") {
  // Build a graph where we use custom attrs with special chars
  auto g = make_plain_graph();
  std::ostringstream os;
  write_graphml(os, g,
    [](const auto& /*g*/, auto uid) -> std::map<std::string, std::string> {
      if (uid == 0) return {{"label", "a < b & c > d"}};
      return {};
    },
    [](const auto& /*g*/, auto /*s*/, auto /*t*/, auto /*uv*/) -> std::map<std::string, std::string> {
      return {};
    });
  std::string output = os.str();

  REQUIRE(output.find("a &lt; b &amp; c &gt; d") != std::string::npos);
}

// ===========================================================================
// DIMACS tests
// ===========================================================================

TEST_CASE("write_dimacs: generic arc list", "[io][dimacs]") {
  auto g = make_test_graph();
  std::ostringstream os;
  write_dimacs(os, g);
  std::string output = os.str();

  REQUIRE(output.find("p sp 3 3") != std::string::npos);
  // 1-indexed endpoints: 0->1 becomes "a 1 2", weight 1.5
  REQUIRE(output.find("a 1 2 1.5") != std::string::npos);
  REQUIRE(output.find("a 1 3 2.5") != std::string::npos);
  REQUIRE(output.find("a 2 3 3.5") != std::string::npos);
}

TEST_CASE("write_dimacs: custom problem type", "[io][dimacs]") {
  auto g = make_plain_graph();
  std::ostringstream os;
  write_dimacs(os, g, "max");
  std::string output = os.str();

  REQUIRE(output.find("p max 3 4") != std::string::npos);
}

TEST_CASE("write_dimacs_max_flow: source/sink descriptors", "[io][dimacs]") {
  auto g = make_test_graph();
  std::ostringstream os;
  write_dimacs_max_flow(os, g, 0u, 2u,
    [](const auto& gr, auto uv) { return edge_value(gr, uv); });
  std::string output = os.str();

  REQUIRE(output.find("p max 3 3") != std::string::npos);
  REQUIRE(output.find("n 1 s") != std::string::npos);
  REQUIRE(output.find("n 3 t") != std::string::npos);
  REQUIRE(output.find("a 1 2 1.5") != std::string::npos);
}

TEST_CASE("read_dimacs: parse max-flow problem", "[io][dimacs]") {
  std::istringstream is(R"(c sample max-flow problem
p max 4 5
n 1 s
n 4 t
a 1 2 10
a 1 3 5
a 2 4 8
a 3 4 7
a 2 3 3
)");

  auto result = read_dimacs(is);
  REQUIRE(result.problem == "max");
  REQUIRE(result.num_vertices == 4);
  REQUIRE(result.num_arcs == 5);
  REQUIRE(result.nodes.size() == 2);
  // ids normalized to 0-indexed
  REQUIRE(result.nodes[0].id == 0);
  REQUIRE(result.nodes[0].designation == "s");
  REQUIRE(result.nodes[1].id == 3);
  REQUIRE(result.nodes[1].designation == "t");
  REQUIRE(result.edges.size() == 5);
  REQUIRE(result.edges[0].source == 0);
  REQUIRE(result.edges[0].target == 1);
  REQUIRE(result.edges[0].weight == "10");
}

TEST_CASE("read_dimacs: edge format (e lines)", "[io][dimacs]") {
  std::istringstream is(R"(c clique format
p edge 3 2
e 1 2
e 2 3
)");

  auto result = read_dimacs(is);
  REQUIRE(result.problem == "edge");
  REQUIRE(result.edges.size() == 2);
  REQUIRE(result.edges[0].source == 0);
  REQUIRE(result.edges[0].target == 1);
  REQUIRE(result.edges[1].source == 1);
  REQUIRE(result.edges[1].target == 2);
  REQUIRE(result.edges[0].weight.empty());
}

TEST_CASE("DIMACS roundtrip: write then read", "[io][dimacs]") {
  auto g = make_test_graph();
  std::ostringstream oss;
  write_dimacs(oss, g);

  std::istringstream iss(oss.str());
  auto parsed = read_dimacs(iss);

  REQUIRE(parsed.num_vertices == 3);
  REQUIRE(parsed.num_arcs == 3);
  REQUIRE(parsed.edges.size() == 3);
  REQUIRE(parsed.edges[0].source == 0);
  REQUIRE(parsed.edges[0].target == 1);
}

// ===========================================================================
// METIS tests
// ===========================================================================

TEST_CASE("write_metis: undirected adjacency", "[io][metis]") {
  auto g = make_plain_graph(); // 0->1, 0->2, 1->2, 2->0
  std::ostringstream os;
  write_metis(os, g);
  std::string output = os.str();

  // 3 vertices; undirected edges {0,1},{0,2},{1,2} => 3 edges
  REQUIRE(output.find("3 3") != std::string::npos);
  // vertex 1 (id 0) is adjacent to 2 and 3 (ids 1,2)
  std::istringstream iss(output);
  std::string line;
  std::getline(iss, line); // comment
  std::getline(iss, line); // header
  std::getline(iss, line); // vertex 1 line
  REQUIRE(line.find("2") != std::string::npos);
  REQUIRE(line.find("3") != std::string::npos);
}

TEST_CASE("write_metis: with edge weights", "[io][metis]") {
  auto g = make_test_graph();
  std::ostringstream os;
  write_metis(os, g, /*with_weights=*/true);
  std::string output = os.str();

  REQUIRE(output.find("001") != std::string::npos); // fmt flag
  REQUIRE(output.find("1.5") != std::string::npos);
}

TEST_CASE("read_metis: unweighted graph", "[io][metis]") {
  std::istringstream is(R"(% sample
4 5
2 3
1 3 4
1 2 4
2 3
)");

  auto result = read_metis(is);
  REQUIRE(result.num_vertices == 4);
  REQUIRE(result.num_edges == 5);
  REQUIRE(result.adjacency.size() == 4);
  // vertex 0 neighbours: 2,3 (file) -> 1,2 (0-indexed)
  REQUIRE(result.adjacency[0].size() == 2);
  REQUIRE(result.adjacency[0][0].neighbor == 1);
  REQUIRE(result.adjacency[0][1].neighbor == 2);
  // vertex 1 neighbours: 1,3,4 -> 0,2,3
  REQUIRE(result.adjacency[1].size() == 3);
  REQUIRE(result.adjacency[1][0].neighbor == 0);
  REQUIRE(result.adjacency[1][2].neighbor == 3);
}

TEST_CASE("read_metis: weighted graph (fmt=001)", "[io][metis]") {
  std::istringstream is(R"(% weighted
3 2 001
2 5 3 7
1 5
1 7
)");

  auto result = read_metis(is);
  REQUIRE(result.num_vertices == 3);
  REQUIRE(result.fmt == 1);
  REQUIRE(result.adjacency[0].size() == 2);
  REQUIRE(result.adjacency[0][0].neighbor == 1);
  REQUIRE(result.adjacency[0][0].weight == "5");
  REQUIRE(result.adjacency[0][1].neighbor == 2);
  REQUIRE(result.adjacency[0][1].weight == "7");
}

TEST_CASE("METIS roundtrip: write then read", "[io][metis]") {
  auto g = make_plain_graph();
  std::ostringstream oss;
  write_metis(oss, g);

  std::istringstream iss(oss.str());
  auto parsed = read_metis(iss);

  REQUIRE(parsed.num_vertices == 3);
  REQUIRE(parsed.num_edges == 3);
  // symmetric: total adjacency entries == 2 * edges
  std::uint64_t total = 0;
  for (const auto& a : parsed.adjacency) total += a.size();
  REQUIRE(total == 6);
}

// ===========================================================================
// Adjacency List Text tests
// ===========================================================================

TEST_CASE("write_adjacency_list_text: structure dump", "[io][adjtext]") {
  auto g = make_plain_graph(); // 0->1, 0->2, 1->2, 2->0
  std::ostringstream os;
  write_adjacency_list_text(os, g);
  std::string output = os.str();

  REQUIRE(output.find("0: 1 2") != std::string::npos);
  REQUIRE(output.find("1: 2") != std::string::npos);
  REQUIRE(output.find("2: 0") != std::string::npos);
}

TEST_CASE("read_adjacency_list_text: parse with colon", "[io][adjtext]") {
  std::istringstream is(R"(0: 1 2
1: 2
2: 0 3
3:
)");

  auto result = read_adjacency_list_text(is);
  REQUIRE(result.vertex_ids.size() == 4);
  REQUIRE(result.edges.size() == 5);
  REQUIRE(result.edges[0].source == "0");
  REQUIRE(result.edges[0].target == "1");
  REQUIRE(result.edges[4].source == "2");
  REQUIRE(result.edges[4].target == "3");
}

TEST_CASE("read_adjacency_list_text: whitespace-only separator", "[io][adjtext]") {
  std::istringstream is(R"(A B C
B C
C A
)");

  auto result = read_adjacency_list_text(is);
  REQUIRE(result.vertex_ids.size() == 3);
  REQUIRE(result.edges.size() == 4);
  REQUIRE(result.edges[0].source == "A");
  REQUIRE(result.edges[0].target == "B");
}

TEST_CASE("Adjacency List Text roundtrip: write then read", "[io][adjtext]") {
  auto g = make_plain_graph();
  std::ostringstream oss;
  write_adjacency_list_text(oss, g);

  std::istringstream iss(oss.str());
  auto parsed = read_adjacency_list_text(iss);

  REQUIRE(parsed.vertex_ids.size() == 3);
  REQUIRE(parsed.edges.size() == 4);
}
