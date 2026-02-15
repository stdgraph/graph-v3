#pragma once

#include <vector>
#include <set>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_graph;

// vos_graph_traits
//  Vertices: std::vector (contiguous; random access by vertex ID)
//  Edges:    std::set (ordered; automatic deduplication by target_id/source_id)
//
//  Key characteristics:
//  - Edges are automatically deduplicated (no parallel edges with same endpoints)
//  - Edges are stored in sorted order (by source_id if Sourced, then target_id)
//  - O(log n) edge insertion, lookup, and deletion
//  - Bidirectional iterators (no random access to edges)
//  - Requires operator<=> on dynamic_edge (implemented in dynamic_graph.hpp)
//
//  Parameter semantics mirror vofl_graph_traits.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false>
struct vos_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, vos_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, vos_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, vos_graph_traits>;

  using vertices_type = std::vector<vertex_type>;
  using edges_type    = std::set<edge_type>;
};

} // namespace graph::container
