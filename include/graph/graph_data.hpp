#pragma once
#include <concepts>
#include <stdexcept>
#include <string>
#include "detail/graph_using.hpp"

namespace graph {

/**
 * @brief Exception class for graph-related errors
 * 
 * Used throughout the graph library to report runtime errors such as:
 * - Invalid vertex or edge IDs
 * - Out-of-bounds access
 * - Constraint violations (e.g., unordered edges, invalid partitions)
 * - Data consistency issues
 */
class graph_error : public std::runtime_error {
public:
  explicit graph_error(const std::string& msg) : std::runtime_error(msg) {}
  explicit graph_error(const char* msg) : std::runtime_error(msg) {}
};

//
// vertex_data
// for(auto&& [uid, u]        : vertexlist(g))
// for(auto&& [uid, u, value] : vertexlist(g, [](vertex_t<G> u) { return ...; } )
//
template <class VId, class V, class VV>
struct vertex_data {
  using id_type     = VId; // e.g. vertex_id_t<G>
  using vertex_type = V;   // e.g. vertex_t<G> (vertex descriptor)
  using value_type  = VV;  // e.g. vertex_value_t<G>

  id_type     id;
  vertex_type vertex;
  value_type  value;
};

// Specializations with VId present
template <class VId, class V>
struct vertex_data<VId, V, void> {
  using id_type     = VId;
  using vertex_type = V;
  using value_type  = void;

  id_type     id;
  vertex_type vertex;
};
template <class VId, class VV>
struct vertex_data<VId, void, VV> {
  using id_type     = VId;
  using vertex_type = void;
  using value_type  = VV;

  id_type    id;
  value_type value;
};
template <class VId>
struct vertex_data<VId, void, void> {
  using id_type     = VId;
  using vertex_type = void;
  using value_type  = void;

  id_type id;
};

// Specializations with VId=void (descriptor-based pattern)
template <class V, class VV>
struct vertex_data<void, V, VV> {
  using id_type     = void;
  using vertex_type = V;
  using value_type  = VV;

  vertex_type vertex;
  value_type  value;
};
template <class V>
struct vertex_data<void, V, void> {
  using id_type     = void;
  using vertex_type = V;
  using value_type  = void;

  vertex_type vertex;
};
template <class VV>
struct vertex_data<void, void, VV> {
  using id_type     = void;
  using vertex_type = void;
  using value_type  = VV;

  value_type value;
};
template <>
struct vertex_data<void, void, void> {
  using id_type     = void;
  using vertex_type = void;
  using value_type  = void;
};

template <class VId, class VV>
using copyable_vertex_t = vertex_data<VId, void, VV>; // {id, value}

//
// edge_data
//
// for(auto&& [target_id, uv]        : incidence(g,u))
// for(auto&& [target_id, uv, value] : incidence(g,u, [](edge_t<G> uv) { return ...; })
//
// for(auto&& [source_id, target_id, uv]        : incidence(g,u))
// for(auto&& [source_id, target_id, uv, value] : incidence(g,u, [](edge_t<G> uv) { return ...; })
//
template <class VId, bool Sourced, class E, class EV>
struct edge_data {
  using source_id_type = VId; // e.g. vertex_id_t<G> when Sourced==true, or void
  using target_id_type = VId; // e.g. vertex_id_t<G>
  using edge_type      = E;   // e.g. edge_t<G> (edge descriptor) or void
  using value_type     = EV;  // e.g. edge_value_t<G> or void

  source_id_type source_id;
  target_id_type target_id;
  edge_type      edge;
  value_type     value;
};

// Sourced=true specializations with VId present
template <class VId, class E>
struct edge_data<VId, true, E, void> {
  using source_id_type = VId;
  using target_id_type = VId;
  using edge_type      = E;
  using value_type     = void;

  source_id_type source_id;
  target_id_type target_id;
  edge_type      edge;
};
template <class VId>
struct edge_data<VId, true, void, void> {
  using source_id_type = VId;
  using target_id_type = VId;
  using edge_type      = void;
  using value_type     = void;

  source_id_type source_id;
  target_id_type target_id;
};
template <class VId, class EV>
struct edge_data<VId, true, void, EV> {
  using source_id_type = VId;
  using target_id_type = VId;
  using edge_type      = void;
  using value_type     = EV;

  source_id_type source_id;
  target_id_type target_id;
  value_type     value;
};

// Sourced=false specializations with VId present
template <class VId, class E, class EV>
struct edge_data<VId, false, E, EV> {
  using source_id_type = void;
  using target_id_type = VId;
  using edge_type      = E;
  using value_type     = EV;

  target_id_type target_id;
  edge_type      edge;
  value_type     value;
};
template <class VId, class E>
struct edge_data<VId, false, E, void> {
  using source_id_type = void;
  using target_id_type = VId;
  using edge_type      = E;
  using value_type     = void;

  target_id_type target_id;
  edge_type      edge;
};

template <class VId, class EV>
struct edge_data<VId, false, void, EV> {
  using source_id_type = void;
  using target_id_type = VId;
  using edge_type      = void;
  using value_type     = EV;

  target_id_type target_id;
  value_type     value;
};
template <class VId>
struct edge_data<VId, false, void, void> {
  using source_id_type = void;
  using target_id_type = VId;
  using edge_type      = void;
  using value_type     = void;

  target_id_type target_id;
};

// Sourced=true specializations with VId=void (descriptor-based pattern)
template <class E, class EV>
struct edge_data<void, true, E, EV> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = E;
  using value_type     = EV;

  edge_type  edge;
  value_type value;
};
template <class E>
struct edge_data<void, true, E, void> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = E;
  using value_type     = void;

  edge_type edge;
};
template <class EV>
struct edge_data<void, true, void, EV> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = void;
  using value_type     = EV;

  value_type value;
};
template <>
struct edge_data<void, true, void, void> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = void;
  using value_type     = void;
};

// Sourced=false specializations with VId=void (descriptor-based pattern)
template <class E, class EV>
struct edge_data<void, false, E, EV> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = E;
  using value_type     = EV;

  edge_type  edge;
  value_type value;
};
template <class E>
struct edge_data<void, false, E, void> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = E;
  using value_type     = void;

  edge_type edge;
};
template <class EV>
struct edge_data<void, false, void, EV> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = void;
  using value_type     = EV;

  value_type value;
};
template <>
struct edge_data<void, false, void, void> {
  using source_id_type = void;
  using target_id_type = void;
  using edge_type      = void;
  using value_type     = void;
};

//
// targeted_edge
// for(auto&& [vid,uv,value] : edges_view(g, u, [](edge_t<G> uv) { return ...; } )
// for(auto&& [vid,uv]       : edges_view(g, u) )
//
//template <class VId, class E, class EV>
//using targeted_edge = edge_data<VId, false, E, EV>; // {target_id, edge, [, value]}

//
// sourced_edge
// for(auto&& [uid,vid,uv,value] : sourced_edges_view(g, u, [](edge_t<G> uv) { return ...; } )
// for(auto&& [uid,vid,uv]       : sourced_edges_view(g, u) )
//
//template <class VId, class V, class E, class EV>
//using sourced_edge = edge_data<VId, true, E, EV>; // {source_id, target_id, edge, [, value]}

//
// edgelist_edge
// for(auto&& [uid,vid,uv,value] : edges_view(g, [](edge_t<G> uv) { return ...; } )
// for(auto&& [uid,vid,uv]       : edges_view(g) )
//
template <class VId, class E, class EV>
using edgelist_edge = edge_data<VId, true, E, EV>; // {source_id, target_id [, edge] [, value]}

//
// copyable_edge_t
//
template <class VId, class EV = void>
using copyable_edge_t = edge_data<VId, true, void, EV>; // {source_id, target_id [, value]}

//
// neighbor_data (for adjacency)
//
template <class VId, bool Sourced, class V, class VV>
struct neighbor_data {
  using source_id_type = VId; // e.g. vertex_id_t<G> when Sourced==true, or void
  using target_id_type = VId; // e.g. vertex_id_t<G>
  using vertex_type    = V;   // e.g. vertex_t<G> (vertex descriptor) or void
  using value_type     = VV;  // e.g. vertex_value_t<G> or void

  source_id_type source_id;
  target_id_type target_id;
  vertex_type    target;
  value_type     value;
};

// Sourced=false specializations with VId present
template <class VId, class V, class VV>
struct neighbor_data<VId, false, V, VV> {
  using source_id_type = void;
  using target_id_type = VId;
  using vertex_type    = V;
  using value_type     = VV;

  target_id_type target_id;
  vertex_type    target;
  value_type     value;
};

template <class VId, class V>
struct neighbor_data<VId, false, V, void> {
  using source_id_type = void;
  using target_id_type = VId;
  using vertex_type    = V;
  using value_type     = void;

  target_id_type target_id;
  vertex_type    target;
};

template <class VId, class VV>
struct neighbor_data<VId, false, void, VV> {
  using source_id_type = void;
  using target_id_type = VId;
  using vertex_type    = void;
  using value_type     = VV;

  target_id_type target_id;
  value_type     value;
};

template <class VId>
struct neighbor_data<VId, false, void, void> {
  using source_id_type = void;
  using target_id_type = VId;
  using vertex_type    = void;
  using value_type     = void;

  target_id_type target_id;
};

// Sourced=true specializations with VId present
template <class VId, class V>
struct neighbor_data<VId, true, V, void> {
  using source_id_type = VId;
  using target_id_type = VId;
  using vertex_type    = V;
  using value_type     = void;

  source_id_type source_id;
  target_id_type target_id;
  vertex_type    target;
};

template <class VId, class VV>
struct neighbor_data<VId, true, void, VV> {
  using source_id_type = VId;
  using target_id_type = VId;
  using vertex_type    = void;
  using value_type     = VV;

  source_id_type source_id;
  target_id_type target_id;
  value_type     value;
};

template <class VId>
struct neighbor_data<VId, true, void, void> {
  using source_id_type = VId;
  using target_id_type = VId;
  using vertex_type    = void;
  using value_type     = void;

  source_id_type source_id;
  target_id_type target_id;
};

// Sourced=false specializations with VId=void (descriptor-based pattern)
template <class V, class VV>
struct neighbor_data<void, false, V, VV> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = V;
  using value_type     = VV;

  vertex_type vertex;
  value_type  value;
};

template <class V>
struct neighbor_data<void, false, V, void> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = V;
  using value_type     = void;

  vertex_type vertex;
};

template <class VV>
struct neighbor_data<void, false, void, VV> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = void;
  using value_type     = VV;

  value_type value;
};

template <>
struct neighbor_data<void, false, void, void> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = void;
  using value_type     = void;
};

// Sourced=true specializations with VId=void (descriptor-based pattern)
template <class V, class VV>
struct neighbor_data<void, true, V, VV> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = V;
  using value_type     = VV;

  vertex_type vertex;
  value_type  value;
};

template <class V>
struct neighbor_data<void, true, V, void> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = V;
  using value_type     = void;

  vertex_type vertex;
};

template <class VV>
struct neighbor_data<void, true, void, VV> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = void;
  using value_type     = VV;

  value_type value;
};

template <>
struct neighbor_data<void, true, void, void> {
  using source_id_type = void;
  using target_id_type = void;
  using vertex_type    = void;
  using value_type     = void;
};

//
// copyable_edge_t
//
template <class VId, class VV>                                  // For exposition only
using copyable_neighbor_t = neighbor_data<VId, true, void, VV>; // {source_id, target_id [, value]}

//
// view concepts
//
template <class T, class VId, class VV = void> // For exposition only
concept copyable_vertex = std::convertible_to<T, copyable_vertex_t<VId, VV>>;

template <class T, class VId, class EV = void> // For exposition only
concept copyable_edge = std::convertible_to<T, copyable_edge_t<VId, EV>>;

template <class T, class VId, class EV = void> // For exposition only
concept copyable_neighbor = std::convertible_to<T, copyable_neighbor_t<VId, EV>>;

//
// is_sourced<G>
//
template <class T>
inline constexpr bool is_sourced_v = false;
template <class VId, class V, class VV>
inline constexpr bool is_sourced_v<edge_data<VId, true, V, VV>> = true;
template <class VId, class V, class VV>
inline constexpr bool is_sourced_v<neighbor_data<VId, true, V, VV>> = true;

} // namespace graph
