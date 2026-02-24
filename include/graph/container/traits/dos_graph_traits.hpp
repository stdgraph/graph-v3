#pragma once

#include <deque>
#include <set>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_out_edge;

template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_graph;

// dos_graph_traits
//  Vertices: std::deque (stable references on push_back/push_front; random access by index)
//  Edges:    std::set (ordered; automatic deduplication by target_id/source_id)
//
//  Key characteristics:
//  - Vertices have stable references/pointers on push_back/push_front (unlike vector)
//  - Edges are automatically deduplicated (no parallel edges with same endpoints)
//  - Edges are stored in sorted order (by target_id)
//  - O(log n) edge insertion, lookup, and deletion
//  - Bidirectional iterators for edges (no random access to edges)
//  - Requires operator<=> on dynamic_out_edge (implemented in dynamic_graph.hpp)
//
//  Parameter semantics mirror vofl_graph_traits.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Bidirectional = false>
struct dos_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_out_edge<EV, VV, GV, VId, Bidirectional, dos_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Bidirectional, dos_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Bidirectional, dos_graph_traits>;

  using vertices_type = std::deque<vertex_type>;
  using edges_type    = std::set<edge_type>;
};

} // namespace graph::container
