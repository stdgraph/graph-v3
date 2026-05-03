#pragma once
#include "graph/graph.hpp"
#include "graph/views/vertexlist.hpp"
#include "graph/views/incidence.hpp"
#include "graph/views/dfs.hpp"
#include <fstream>
#include <cassert>
#include <vector>

enum struct directedness : int8_t {
  directed,   // a single edge joins 2 vertices
  directed2,  // 2 edges join 2 vertices, each with different directions; needed for graphviz
  undirected, // one or more edges exist between vertices with no direction
  bidirected  // a single edge between vertices with direction both ways
};

/// Outputs a graphviz .gv file for the routes graph.
///
/// dot -Tpdf -O routes.gv
/// neato -Tpng -O routes.gv
template <class G>
void output_routes_graphviz(G&               g,
                            std::string_view filename,
                            const directedness dir,
                            std::string_view bgcolor = {}) {
  using namespace graph;
  using namespace std::literals;
  std::ofstream of{std::string(filename)};
  assert(of.is_open());

  std::string_view arrows;
  switch (dir) {
  case directedness::bidirected: arrows = "dir=both,arrowhead=vee,arrowtail=vee"; break;
  case directedness::directed:   arrows = "dir=forward,arrowhead=vee"; break;
  case directedness::directed2:  arrows = "dir=forward,arrowhead=vee"; break;
  case directedness::undirected: arrows = "dir=none"; break;
  }

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     << "  splines = curved\n"
     << "  node[shape=oval]\n"
     << "  edge[" << arrows << ", fontcolor=blue]\n";
  if (!bgcolor.empty())
    of << "  bgcolor=" << bgcolor << "\n";

  for (auto&& [uid, u] : views::vertexlist(g)) {
    of << "  " << uid << " [label=\"" << vertex_value(g, u) << " [" << uid << "]\"]\n";
    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      std::string_view arw =
            (dir == directedness::directed2 && vid < uid) ? "dir=back,arrowhead=vee," : ""sv;
      of << "   " << uid << " -> " << vid << " [" << arw << "xlabel=\"" << edge_value(g, uv) << " km\"]\n";
    }
    of << "\n";
  }
  of << "}\n";
}

/// Outputs a graphviz .gv file showing the adjacency-list structure.
template <class G>
void output_routes_graphviz_adjlist(const G&         g,
                                    std::string_view filename,
                                    std::string_view bgcolor = {}) {
  using namespace graph;
  using namespace std::literals;
  std::ofstream of{std::string(filename)};
  assert(of.is_open());

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     << "  graph[rankdir=LR]\n"
     << "  edge[arrowhead=vee]\n";
  if (!bgcolor.empty())
    of << "  bgcolor=" << bgcolor << "\n";

  for (auto&& [uid, u] : views::vertexlist(g)) {
    of << "  " << uid << " [shape=Mrecord, label=\"{<f0>" << uid << "|<f1>" << vertex_value(g, u) << "}\"]\n";
    std::string from = std::to_string(uid);
    for (auto&& [vid, uv] : views::incidence(g, uid)) {
      std::string to = "e"s + std::to_string(uid) + "_"s + std::to_string(vid);
      of << "    " << to << " [shape=record, label=\"{<f0>" << vid << "|<f1>" << edge_value(g, uv) << "km}\"]\n";
      of << "    " << from << " -> " << to;
      from = to;
    }
    of << "\n";
  }
  of << "}\n";
}

/// Outputs a graphviz .gv file showing the DFS tree from a seed vertex.
///
/// In v3, edges_dfs yields [uv] (edge descriptor only).
/// Source ID is obtained via uv.source_id(); target ID via graph::target_id(g, uv).
template <class G>
void output_routes_graphviz_dfs_vertices(G&                    g,
                                         std::string_view      filename,
                                         graph::vertex_id_t<G> seed,
                                         std::string_view      bgcolor = {}) {
  using namespace graph;
  using namespace graph::views;
  using namespace std::literals;
  std::ofstream of{std::string(filename)};
  assert(of.is_open());

  std::vector<bool> visited(graph::num_vertices(g), false);

  of << "digraph routes {\n"
     << "  overlap = scalexy\n"
     << "  node[shape=oval]\n"
     << "  edge[arrowhead=vee]\n";
  if (!bgcolor.empty())
    of << "  bgcolor=" << bgcolor << "\n";

  // Output seed vertex label
  of << "  " << seed << " [label=\"" << graph::vertex_value(g, *graph::find_vertex(g, seed))
     << " [" << seed << "]\"]\n";
  visited[seed] = true;

  // edges_dfs yields tree edges in DFS order.
  // uv.source_id() = source vertex ID (available from edge_descriptor::source_id())
  // target_id(g, uv) = target vertex ID (via rr_adaptor's target_id CPO)
  for (auto&& [uv] : graph::views::edges_dfs(g, seed)) {
    auto uid = uv.source_id();
    auto vid = graph::target_id(g, uv);

    if (!visited[vid]) {
      of << "  " << vid << " [label=\""
         << graph::vertex_value(g, *graph::find_vertex(g, static_cast<graph::vertex_id_t<G>>(vid)))
         << " [" << vid << "]\"]\n";
      visited[vid] = true;
    }
    of << "  " << uid << " -> " << vid << "\n";
  }
  of << "}\n";
}
