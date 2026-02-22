#pragma once

#include <deque>
#include <vector>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph;

// dov_graph_traits
//  Vertices: std::deque (stable iterators)
//  Edges:    std::vector (random access)
//  Parameter semantics mirror vofl_graph_traits.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false, bool Bidirectional = false>
struct dov_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced       = Sourced;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, dov_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, dov_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, dov_graph_traits>;

  using vertices_type = std::deque<vertex_type>;
  using edges_type    = std::vector<edge_type>;
};

} // namespace graph::container
