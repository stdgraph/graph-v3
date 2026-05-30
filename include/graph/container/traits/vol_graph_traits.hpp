#pragma once

#include <vector>
#include <list>

#include <graph/container/dynamic_graph.hpp>

namespace graph::container {

// vol_graph_traits
//  Vertices: std::vector
//  Edges:    std::list (bidirectional). Parameter semantics mirror vofl_graph_traits.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Bidirectional = false>
struct vol_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_out_edge<EV, VV, GV, VId, Bidirectional, vol_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Bidirectional, vol_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Bidirectional, vol_graph_traits>;

  using vertices_type = std::vector<vertex_type>;
  using edges_type    = std::list<edge_type>;
};

// Templated type alias for quick vol_graph definition
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Bidirectional = false>
using vol_graph = dynamic_graph<EV, VV, GV, VId, Bidirectional, vol_graph_traits<EV, VV, GV, VId, Bidirectional>>;

} // namespace graph::container
