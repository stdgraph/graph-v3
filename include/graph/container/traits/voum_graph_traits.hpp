#pragma once

#include <vector>
#include <unordered_map>

namespace graph::container {

// Forward declarations
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_out_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph;

// voum_graph_traits
//  Vertices: std::vector (contiguous; random access by vertex ID)
//  Edges:    std::unordered_map<VId, edge_type> (hash map keyed by target vertex ID)
//
//  Key characteristics:
//  - O(1) average edge lookup by target vertex ID using unordered_map::find(vid)
//  - Edges automatically deduplicated (only one edge per target vertex)
//  - Edges stored in unordered fashion (hash-bucket order, not sorted)
//  - Forward iterators only (no bidirectional or random access)
//  - Edge insertion/deletion O(1) average, O(n) worst case
//  - Requires std::hash<VId> and operator== on VId (for hash map)
//
//  Compared to vom_graph_traits (std::map edges):
//  - vom:  O(log n) operations, sorted order, bidirectional iterators
//  - voum: O(1) average operations, unordered, forward iterators only
//
//  Use cases:
//  - Graphs with unique edges (no parallel edges to same target)
//  - Fast edge lookup by target: find_vertex_edge(g, u, v) in O(1) average
//  - Memory overhead: ~8-16 bytes per edge (pair<const VId, edge_type>)
//
//  Parameter semantics mirror vom_graph_traits.
template <class EV = void, class VV = void, class GV = void, class VId = uint32_t, bool Sourced = false, bool Bidirectional = false>
struct voum_graph_traits {
  using edge_value_type         = EV;
  using vertex_value_type       = VV;
  using graph_value_type        = GV;
  using vertex_id_type          = VId;
  static constexpr bool sourced       = Sourced;
  static constexpr bool bidirectional = Bidirectional;

  using edge_type   = dynamic_out_edge<EV, VV, GV, VId, Sourced, Bidirectional, voum_graph_traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, voum_graph_traits>;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, voum_graph_traits>;

  using vertices_type = std::vector<vertex_type>;
  using edges_type    = std::unordered_map<VId, edge_type>; // Hash map keyed by target vertex ID
};

} // namespace graph::container
