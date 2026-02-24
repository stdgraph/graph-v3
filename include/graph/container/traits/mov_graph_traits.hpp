#pragma once

#include <cstdint>
#include <map>
#include <vector>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_out_edge;

template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_graph;

// mov_graph_traits
//  Vertices: std::map (associative; key-based lookup; bidirectional iteration)
//  Edges:    std::vector (contiguous; best for random access & cache locality)
//  Notes: Supports sparse, non-contiguous vertex IDs with key-based access.
//         Unlike sequential containers (vector/deque), vertices must be explicitly
//         created - no auto-extension when edges reference undefined vertices.
//         Vertex IDs can be any ordered type (int, string, custom struct with operator<).
//         The std::vector edge container provides random access iteration.
//  Template parameters: EV (edge value or void), VV (vertex value or void), GV (graph value or void),
//  VId (vertex id - any ordered type with operator<).
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Bidirectional = false>
struct mov_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_out_edge<EV, VV, GV, VId, Bidirectional, mov_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Bidirectional, mov_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Bidirectional, mov_graph_traits>;

  using vertices_type = std::map<VId, vertex_type>;
  using edges_type    = std::vector<edge_type>;
};

} // namespace graph::container
