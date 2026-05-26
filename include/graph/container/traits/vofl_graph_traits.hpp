#pragma once

#include <vector>
#include <forward_list>

#include <graph/container/dynamic_graph.hpp>

namespace graph::container {

// vofl_graph_traits
//  Vertices: std::vector (contiguous; random access)
//  Edges:    std::forward_list (singly-linked; forward iteration only)
//  Notes: Lightweight singly-linked adjacency for cheap edge insertion.
//  Template parameters: EV (edge value or void), VV (vertex value or void), GV (graph value or void),
//  VId (integral vertex id).
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Bidirectional = false>
struct vofl_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_out_edge<EV, VV, GV, VId, Bidirectional, vofl_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Bidirectional, vofl_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Bidirectional, vofl_graph_traits>;

  using vertices_type = std::vector<vertex_type>;
  using edges_type    = std::forward_list<edge_type>;
};

// Templated type alias for quick vofl_graph definition
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Bidirectional = false>
using vofl_graph = dynamic_graph<EV, VV, GV, VId, Bidirectional, vofl_graph_traits<EV, VV, GV, VId, Bidirectional>>;

} // namespace graph::container
