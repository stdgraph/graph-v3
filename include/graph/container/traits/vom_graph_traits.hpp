#pragma once

#include <vector>
#include <map>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_graph;

// vom_graph_traits
//  Vertices: std::vector (contiguous; random access by vertex ID)
//  Edges:    std::map<VId, edge_type> (ordered map keyed by target vertex ID)
//
//  Key characteristics:
//  - O(log n) edge lookup by target vertex ID using map::find(vid)
//  - Edges automatically deduplicated (only one edge per target vertex)
//  - Edges stored in sorted order by target_id
//  - Bidirectional iterators for edges (not random access)
//  - Edge insertion/deletion O(log n)
//  - Requires operator<=> on VId (for map ordering)
//
//  Use cases:
//  - Graphs with unique edges (no parallel edges to same target)
//  - Fast edge lookup by target: find_vertex_edge(g, u, v) in O(log n)
//  - Memory overhead: ~8-16 bytes per edge (pair<const VId, edge_type>)
//
//  Parameter semantics mirror vofl_graph_traits.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false>
struct vom_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced = Sourced;

  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, vom_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, vom_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, vom_graph_traits>;

  using vertices_type = std::vector<vertex_type>;
  using edges_type    = std::map<VId, edge_type>; // Map keyed by target vertex ID
};

} // namespace graph::container
