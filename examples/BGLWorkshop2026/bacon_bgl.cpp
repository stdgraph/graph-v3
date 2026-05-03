// bacon_bgl.cpp — Bacon number examples using Boost Graph Library (BGL).
//
// bacon1_bgl: mirrors bacon1 from bacon_gv3.cpp.
//   - boost::adjacency_list<vecS, vecS, undirectedS> — actors only, integral vertex ids
//   - boost::breadth_first_search with DepthRecorder visitor
//   - Bacon number = BFS depth (direct actor↔actor graph, no bipartite step)
//
// bacon2_bgl: mirrors bacon2 from bacon_gv3.cpp.
//   - boost::adjacency_list (undirectedS + vertex bundle) — bipartite movie↔actor graph
//   - boost::breadth_first_search with a custom visitor for BFS depths
//   - manual DOT output (same visual style as bacon_gv3)
//   - Bacon number = depth / 2 (bipartite path: actor → movie → actor)
//
// Example: dot -Tjpeg output/bacon2_bgl.dot -o output/bacon2_bgl.jpg

#include "graphs/imdb-graph.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>

#include "graph/io/dot.hpp" // for graph::io::detail::dot_escape

#include <algorithm>
#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using std::cout;
using std::string;
using std::vector;
using namespace std::string_literals;

// ── bacon1_bgl ───────────────────────────────────────────────────────────────
// Actors-only graph with integral vertex ids; direct BFS gives Bacon numbers.
// Uses the same hardcoded costars adjacency list as bacon1 in bacon_gv3.cpp.
namespace bacon1_bgl {

using vid_t   = std::size_t;
// Undirected actor-actor graph; each co-star pair stored once.
using graph_t = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using edge_desc_t = boost::graph_traits<graph_t>::edge_descriptor;

// BGL visitor: depth[v] = depth[u] + 1 on each tree edge.
class DepthRecorder : public boost::default_bfs_visitor {
public:
  explicit DepthRecorder(vector<vid_t>& depth) : depth_(depth) {}
  void tree_edge(edge_desc_t e, const graph_t& g) const {
    depth_[boost::target(e, g)] = depth_[boost::source(e, g)] + 1;
  }
private:
  vector<vid_t>& depth_;
};

void run() {
  constexpr vid_t infinity = std::numeric_limits<vid_t>::max();

  // Same costars adjacency list as bacon1 in bacon_gv3.cpp.
  // costars[actor_id] = list of co-star actor ids (actors only, no movies).
  const vector<vector<vid_t>> costars{
      {1, 5, 6}, {7, 10, 0, 5, 12}, {4, 3, 11}, {2, 11}, {8, 9, 2, 12}, {0, 1},
      {7, 0},    {6, 1, 10},        {4, 9},     {4, 8},  {7, 1},        {2, 3},
      {1, 4}};

  // Build undirected BGL graph from costars.
  const vid_t n = static_cast<vid_t>(actors.size());
  graph_t g(n);
  for (vid_t u = 0; u < n; ++u)
    for (vid_t v : costars[u])
      if (u < v) // add each undirected edge once
        boost::add_edge(u, v, g);

  // BFS from Kevin Bacon.
  const vid_t   seed = static_cast<vid_t>(std::ranges::find(actors, "Kevin Bacon") - actors.begin());
  vector<vid_t> bacon_number(n, infinity);
  bacon_number[seed] = 0;
  boost::breadth_first_search(g, seed, boost::visitor(DepthRecorder{bacon_number}));

  for (vid_t uid = 0; uid < n; ++uid) {
    if (bacon_number[uid] == infinity)
      cout << actors[uid] << " has Bacon number infinity\n";
    else
      cout << actors[uid] << " has Bacon number " << bacon_number[uid] << '\n';
  }
}

} // namespace bacon1_bgl

namespace bacon2_bgl {

// ── Graph types ───────────────────────────────────────────────────────────────

enum class vertex_kind { movie, actor };

struct VertexProps {
  string      name;
  vertex_kind kind;
};

// undirectedS: each edge stored once, traversable in both directions.
// vecS for vertex/edge containers: vertex_descriptor is a plain size_t.
using graph_t       = boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, VertexProps>;
using vertex_desc_t = boost::graph_traits<graph_t>::vertex_descriptor;
using edge_desc_t   = boost::graph_traits<graph_t>::edge_descriptor;

// ── load() ────────────────────────────────────────────────────────────────────

struct load_result {
  graph_t                                  g;
  std::unordered_map<string, vertex_desc_t> id_map; // name → vertex descriptor
};

// Build the bipartite movie↔actor graph from the global imdb-graph.hpp data.
load_result load() {
  graph_t                                  g;
  std::unordered_map<string, vertex_desc_t> id_map;

  for (const auto& title : movies) {
    auto vd   = boost::add_vertex(VertexProps{title, vertex_kind::movie}, g);
    id_map[title] = vd;
  }
  for (const auto& name : actors) {
    auto vd  = boost::add_vertex(VertexProps{name, vertex_kind::actor}, g);
    id_map[name] = vd;
  }
  // Add one undirected edge per movies_actors tuple (no reverse needed).
  for (const auto& [movie, actor] : movies_actors)
    boost::add_edge(id_map.at(movie), id_map.at(actor), g);

  return {std::move(g), std::move(id_map)};
}

// ── eval() — BFS depth ────────────────────────────────────────────────────────

struct eval_result {
  graph_t                                  g;
  std::unordered_map<string, vertex_desc_t> id_map;
  vector<std::size_t>                      depth; // indexed by vertex_descriptor (vecS → size_t)
};

// BGL visitor: records BFS tree depth by propagating parent depth + 1.
// tree_edge() is called exactly once per discovered vertex (when the edge
// that first reaches it is relaxed).
class DepthRecorder : public boost::default_bfs_visitor {
public:
  explicit DepthRecorder(vector<std::size_t>& depth) : depth_(depth) {}

  void tree_edge(edge_desc_t e, const graph_t& g) const {
    const vertex_desc_t u = boost::source(e, g);
    const vertex_desc_t v = boost::target(e, g);
    depth_[v] = depth_[u] + 1;
  }

private:
  vector<std::size_t>& depth_;
};

// Build the graph and run BFS from "Kevin Bacon".
// Depths are bipartite distances: actor Bacon number = depth / 2.
eval_result eval() {
  auto [g, id_map] = load();

  constexpr std::size_t inf = std::numeric_limits<std::size_t>::max();
  vector<std::size_t>   depth(boost::num_vertices(g), inf);

  vertex_desc_t source = id_map.at("Kevin Bacon");
  depth[source]        = 0;

  boost::breadth_first_search(g, source, boost::visitor(DepthRecorder{depth}));

  return {std::move(g), std::move(id_map), std::move(depth)};
}

// ── run() — print Bacon numbers ───────────────────────────────────────────────

void run() {
  constexpr std::size_t inf = std::numeric_limits<std::size_t>::max();
  auto [g, id_map, depth]   = eval();

  for (const string& actor : actors) {
    std::size_t d = depth[id_map.at(actor)];
    if (d == inf)
      cout << actor << " has Bacon number infinity\n";
    else
      cout << actor << " has Bacon number " << (d / 2) << '\n';
  }
}

// ── dot() — write Graphviz DOT ────────────────────────────────────────────────
// Same visual style as bacon_gv3: actors = blue ellipses with Bacon number,
// movies = yellow boxes.  Because the BGL graph is undirected, boost::edges()
// returns each edge exactly once — no deduplication set needed.

void dot(const string& filename = BGLWS_OUTPUT_DIR "/bacon2_bgl.dot") {
  constexpr std::size_t inf = std::numeric_limits<std::size_t>::max();
  auto [g, id_map, depth]   = eval();

  std::ofstream out(filename);
  if (!out)
    throw std::runtime_error("Cannot open: " + filename);

  out << "graph BaconNumbers_BGL {\n";

  // Vertices
  for (auto [vit, vend] = boost::vertices(g); vit != vend; ++vit) {
    const vertex_desc_t vd    = *vit;
    const VertexProps&  props = g[vd];
    const string        esc   = graph::io::detail::dot_escape(props.name);

    if (props.kind == vertex_kind::actor) {
      const string bn = (depth[vd] == inf) ? "inf"s : std::to_string(depth[vd] / 2);
      out << "  " << std::quoted(props.name)
          << std::format(" [shape=ellipse, style=filled, fillcolor=lightblue, label=\"{}\\n({})\"]",
                         esc, bn)
          << ";\n";
    } else {
      out << "  " << std::quoted(props.name)
          << std::format(" [shape=box, style=filled, fillcolor=lightyellow, label=\"{}\"]", esc)
          << ";\n";
    }
  }

  // Edges — undirectedS guarantees each edge appears once in boost::edges().
  for (auto [eit, eend] = boost::edges(g); eit != eend; ++eit) {
    const string& sn = g[boost::source(*eit, g)].name;
    const string& tn = g[boost::target(*eit, g)].name;
    out << "  " << std::quoted(sn) << " -- " << std::quoted(tn) << ";\n";
  }

  out << "}\n";
  cout << "Wrote " << filename << '\n';
}

} // namespace bacon2_bgl

// ── main ──────────────────────────────────────────────────────────────────────

int main() {
  cout << "\n--- bacon1_bgl ---\n";
  bacon1_bgl::run();

  cout << "\n--- bacon2_bgl ---\n";
  bacon2_bgl::run();

  cout << "\n--- bacon2_bgl dot ---\n";
  bacon2_bgl::dot();
}
