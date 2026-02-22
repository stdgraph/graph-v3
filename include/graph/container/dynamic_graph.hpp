#pragma once

#include <compare>
#include <concepts>
#include <algorithm>
#include <limits>
#include <map>
#include <stdexcept>
#include <cassert>
#include "graph/graph.hpp"
#include "graph/adj_list/vertex_descriptor_view.hpp"
#include "container_utility.hpp"

// load_vertices(vrng, vvalue_fnc) -> [uid,vval]
//
// load_edges(erng, eproj) -> [uid,vid]
// load_edges(erng, eproj) -> [uid,vid, eval]
//
// load_edges(erng, eproj) -> [uid,vid]
// load_edges(erng, eproj) -> [uid,vid, eval]
//
// load_edges(erng, eproj, vrng, vproj) -> [uid,vid],       [uid,vval]
// load_edges(erng, eproj, vrng, vproj) -> [uid,vid, eval], [uid,vval]
//
// load_edges(initializer_list<[uid,vid]>
// load_edges(initializer_list<[uid,vid,eval]>
//
// [uid,vval]     <-- copyable_vertex<VId,VV>
// [uid,vid]      <-- copyable_edge<VId,void>
// [uid,vid,eval] <-- copyable_edge<VId,EV>
//

namespace graph::container {

// Import types from graph::adj_list for convenience
using adj_list::vertex_descriptor;
using adj_list::edge_descriptor;
using adj_list::vertex_descriptor_view;
using adj_list::edge_descriptor_view;
using adj_list::vertex_descriptor_type;
using adj_list::edge_descriptor_type;
using adj_list::in_edge_tag;

//--------------------------------------------------------------------------------------------------
// dynamic_graph traits forward references
// Note: Include the appropriate traits header for the container combination you need:
//   #include <graph/container/traits/vofl_graph_traits.hpp>  // vector + forward_list
//   #include <graph/container/traits/vol_graph_traits.hpp>   // vector + list
//   #include <graph/container/traits/vov_graph_traits.hpp>   // vector + vector
//   #include <graph/container/traits/vod_graph_traits.hpp>   // vector + deque
//   #include <graph/container/traits/dofl_graph_traits.hpp>  // deque + forward_list
//   #include <graph/container/traits/dol_graph_traits.hpp>   // deque + list
//   #include <graph/container/traits/dov_graph_traits.hpp>   // deque + vector
//   #include <graph/container/traits/dod_graph_traits.hpp>   // deque + deque
//   #include <graph/container/traits/mofl_graph_traits.hpp>  // map + forward_list
//

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct vofl_graph_traits;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct vol_graph_traits;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct vov_graph_traits;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct vod_graph_traits;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct dofl_graph_traits;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct dol_graph_traits;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct dov_graph_traits;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct dod_graph_traits;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional>
struct mofl_graph_traits;


//--------------------------------------------------------------------------------------------------
// dynamic_graph forward references
//

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge;

template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex;

template <class EV            = void,
          class VV            = void,
          class GV            = void,
          class VId           = uint32_t,
          bool Sourced        = false,
          bool Bidirectional  = false,
          class Traits        = vofl_graph_traits<EV, VV, GV, VId, Sourced, Bidirectional>>
class dynamic_graph;

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// dynamic_adjacency_graph type alias
//
// Note: Trait definitions have been moved to separate headers for faster compilation.
// Include the appropriate traits header for your container combination:
//   #include <graph/container/traits/vofl_graph_traits.hpp>  // vector + forward_list
//   #include <graph/container/traits/vol_graph_traits.hpp>   // vector + list
//   #include <graph/container/traits/vov_graph_traits.hpp>   // vector + vector
//   #include <graph/container/traits/vod_graph_traits.hpp>   // vector + deque
//   #include <graph/container/traits/dofl_graph_traits.hpp>  // deque + forward_list
//   #include <graph/container/traits/dol_graph_traits.hpp>   // deque + list
//   #include <graph/container/traits/dov_graph_traits.hpp>   // deque + vector
//   #include <graph/container/traits/dod_graph_traits.hpp>   // deque + deque
//   #include <graph/container/traits/mofl_graph_traits.hpp>  // map + forward_list
//

/**
 * @ingroup graph_containers
 * @brief A templated type alias to simplify definition of a dynamic_graph.
 * 
 * The @c Traits type has all the types and values required for dynamic_graph
 * template arguments, allowing a simpler definition that only takes the @c Traits
 * struct.
 * 
 * @tparam Traits The traits struct/class. (See examples above.)
*/
template <typename Traits>
using dynamic_adjacency_graph = dynamic_graph<typename Traits::edge_value_type,
                                              typename Traits::vertex_value_type,
                                              typename Traits::graph_value_type,
                                              typename Traits::vertex_id_type,
                                              Traits::sourced,
                                              Traits::bidirectional,
                                              Traits>;

//--------------------------------------------------------------------------------------------------
// dynamic_edge
//

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c target_id(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_source for the source id and dynamic_edge_value for an edge value.
 * 
 * Unlike the other composable edge property classes, this class doesn't have an option for not
 * existing. The target id always exists. It could easily be merged into the dynamic_edge class,
 * but was kept separate for design symmetry with the other property classes.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge_target {
public:
  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;

public:
  constexpr dynamic_edge_target(vertex_id_type targ_id) : target_id_(targ_id) {}

  constexpr dynamic_edge_target()                           = default;
  constexpr dynamic_edge_target(const dynamic_edge_target&) = default;
  constexpr dynamic_edge_target(dynamic_edge_target&&)      = default;
  constexpr ~dynamic_edge_target()                          = default;

  constexpr dynamic_edge_target& operator=(const dynamic_edge_target&) = default;
  constexpr dynamic_edge_target& operator=(dynamic_edge_target&&)      = default;

public:
  constexpr vertex_id_type target_id() const { return target_id_; }

private:
  vertex_id_type target_id_ = vertex_id_type();

private:
  // target_id(g,uv) - ADL customization point for CPO
  friend constexpr vertex_id_type target_id(const graph_type& g, const edge_type& uv) noexcept { return uv.target_id_; }

  // target(g,uv) - returns the target vertex
  // friend constexpr vertex_type& target(graph_type& g, edge_type& uv) noexcept {
  //   return begin(vertices(g))[uv.target_id_];
  // }
  // friend constexpr const vertex_type& target(const graph_type& g, const edge_type& uv) noexcept {
  //   return begin(vertices(g))[uv.target_id_];
  // }
};

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c source_id(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_target for the target id and dynamic_edge_value for an edge value.
 * 
 * A specialization for @c Sourced=false exists as an empty class so any call to @c source_id(g,uv) will
 * generate a compile error.
 * 
 * The edge descriptor provides a reliable way to get the source vertex id, so storing it on the edge
 * is optional for most use cases. Howeever, it's useful to support it to provide a fully self-contained
 * edge object that can be used in the context of an independent graph that is effective in its own right.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge_source {
public:
  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;

public:
  constexpr dynamic_edge_source(vertex_id_type source_id) : source_id_(source_id) {}

  constexpr dynamic_edge_source()                           = default;
  constexpr dynamic_edge_source(const dynamic_edge_source&) = default;
  constexpr dynamic_edge_source(dynamic_edge_source&&)      = default;
  constexpr ~dynamic_edge_source()                          = default;

  constexpr dynamic_edge_source& operator=(const dynamic_edge_source&) = default;
  constexpr dynamic_edge_source& operator=(dynamic_edge_source&&)      = default;

public:
  constexpr vertex_id_type source_id() const { return source_id_; }

private:
  vertex_id_type source_id_ = vertex_id_type();

private:
  // source_id(g,uv), source(g)
  // friend constexpr vertex_id_type source_id(const graph_type& g, const edge_type& uv) noexcept { return uv.source_id_; }

  // friend constexpr vertex_type& source(graph_type& g, edge_type& uv) noexcept {
  //   return begin(vertices(g))[uv.source_id_];
  // }
  // friend constexpr const vertex_type& source(const graph_type& g, const edge_type& uv) noexcept {
  //   return begin(vertices(g))[uv.source_id_];
  // }
};

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c source_id(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_target for the target id and dynamic_edge_value for an edge value.
 * 
 * This specialization for @c Sourced=false is a simple placeholder that doesn't define anything.
 * If the user attempts to call to @c source_id(g,uv) or @c source(g,uv) will generate a compile 
 * error so they can resolve the incorrect usage.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced [false] Source id is not stored on the edge. Use of @c source_id(g,uv) or 
 *                 source(g,uv) will generate an error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_edge_source<EV, VV, GV, VId, false, Bidirectional, Traits> {};

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c edge_value(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_target for the target id and dynamic_edge_source for source id.
 * 
 * A specialization for @c EV=void exists as an empty class so any call to @c edge_value(g,uv) will
 * generate a compile error.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge_value {
public:
  using value_type  = EV;
  using graph_type  = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using vertex_type = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using edge_type   = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;

public:
  constexpr dynamic_edge_value(const value_type& value) : value_(value) {}
  constexpr dynamic_edge_value(value_type&& value) : value_(std::move(value)) {}

  constexpr dynamic_edge_value()                          = default;
  constexpr dynamic_edge_value(const dynamic_edge_value&) = default;
  constexpr dynamic_edge_value(dynamic_edge_value&&)      = default;
  constexpr ~dynamic_edge_value()                         = default;

  constexpr dynamic_edge_value& operator=(const dynamic_edge_value&) = default;
  constexpr dynamic_edge_value& operator=(dynamic_edge_value&&)      = default;

public:
  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private:
  value_type value_ = value_type();

#if 0
private: // CPO properties
  friend constexpr value_type&       edge_value(graph_type& g, edge_type& uv) noexcept { return uv.value_; }
  friend constexpr const value_type& edge_value(const graph_type& g, const edge_type& uv) noexcept { return uv.value_; }
#endif
};

/**
 * @ingroup graph_containers
 * @brief Implementation of the @c edge_value(g,uv) property of a @c dynamic_edge in a @c dynamic_graph.
 * 
 * This is one of three composable classes used to define properties for a @c dynamic_edge, the
 * others being dynamic_edge_target for the target id and dynamic_edge_source for source id.
 * 
 * This specialization for @c EV=void is a simple placeholder that doesn't define anything.
 * Any call to @c edge_value(g,uv) will generate a compile error so the user can resolve the
 * incorrect usage.
 * 
 * @tparam EV      [void] A value type is not defined for the edge. Calling @c edge_value(g,uv)
 *                 will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge_value<void, VV, GV, VId, Sourced, Bidirectional, Traits> {};


/**
 * @ingroup graph_containers
 * @brief The edge class for a @c dynamic_graph.
 * 
 * The edge is a composition of a target id, optional source id (@c Sourced) and optional edge value.
 * This implementation supports a source id and edge value, where additional specializations exist
 * for different combinations. The real functionality for properties is implemented in the 
 * @c dynamic_edge_target, @c dynamic_edge_source and @c dynamic_edge_value base classes.
 *
 * The following combinations of EV and Sourced are supported in @c dynamic_edge specializations.
 * The only difference between them are the values taken by the constructors to match the 
 * inclusion/exclusion of the source Id and/or value properties.
 *   *  < @c EV not void, @c Sourced=true >  (this implementation)
 *   *  < @c EV not void, @c Sourced=false > 
 *   *  < @c EV is  void, @c Sourced=true >
 *   *  < @c EV is  void, @c Sourced=false >
 *
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_edge
      : public dynamic_edge_target<EV, VV, GV, VId, Sourced, Bidirectional, Traits>
      , public dynamic_edge_source<EV, VV, GV, VId, Sourced, Bidirectional, Traits>
      , public dynamic_edge_value<EV, VV, GV, VId, Sourced, Bidirectional, Traits> {
public:
  using base_target_type = dynamic_edge_target<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using base_source_type = dynamic_edge_source<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using base_value_type  = dynamic_edge_value<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;

  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type uid, vertex_id_type vid) : base_target_type(vid), base_source_type(uid) {}
  constexpr dynamic_edge(vertex_id_type uid, vertex_id_type vid, const value_type& val)
        : base_target_type(vid), base_source_type(uid), base_value_type(val) {}
  constexpr dynamic_edge(vertex_id_type uid, vertex_id_type vid, value_type&& val)
        : base_target_type(vid), base_source_type(uid), base_value_type(move(val)) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;

  // Comparison operators for ordered/unordered set containers
  // Compare by (source_id, target_id) - edge value is intentionally excluded
  constexpr auto operator<=>(const dynamic_edge& rhs) const noexcept {
    if (auto cmp = base_source_type::source_id() <=> rhs.base_source_type::source_id(); cmp != 0)
      return cmp;
    return base_target_type::target_id() <=> rhs.base_target_type::target_id();
  }
  constexpr bool operator==(const dynamic_edge& rhs) const noexcept {
    return base_source_type::source_id() == rhs.base_source_type::source_id() &&
           base_target_type::target_id() == rhs.base_target_type::target_id();
  }
};

/**
 * @ingroup graph_containers
 * @brief The edge class for a @c dynamic_graph.
 * 
 * The edge is a composition of a target id, optional source id (@c Sourced) and optional edge value.
 * This implementation supports a source id and edge value, where additional specializations exist
 * for different combinations. The real functionality for properties is implemented in the 
 * @c dynamic_edge_target, @c dynamic_edge_source and @c dynamic_edge_value base classes.
 *
 * This is a specialization definition for @c EV=void and @c Sourced=true.
 * 
 * @tparam EV      [void] A value type is not defined for the edge. Calling @c edge_value(g,uv)
 *                 will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced [true] Source id is stored on the edge. Use of @c source_id(g,uv) and
 *                 @c source(g,uv) will return a value without error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_edge<void, VV, GV, VId, true, Bidirectional, Traits>
      : public dynamic_edge_target<void, VV, GV, VId, true, Bidirectional, Traits>
      , public dynamic_edge_source<void, VV, GV, VId, true, Bidirectional, Traits>
      , public dynamic_edge_value<void, VV, GV, VId, true, Bidirectional, Traits> {
public:
  using base_target_type = dynamic_edge_target<void, VV, GV, VId, true, Bidirectional, Traits>;
  using base_source_type = dynamic_edge_source<void, VV, GV, VId, true, Bidirectional, Traits>;
  using base_value_type  = dynamic_edge_value<void, VV, GV, VId, true, Bidirectional, Traits>;

  using vertex_id_type = VId;
  using value_type     = void;
  using graph_type     = dynamic_graph<void, VV, GV, VId, true, Bidirectional, Traits>;
  using vertex_type    = dynamic_vertex<void, VV, GV, VId, true, Bidirectional, Traits>;
  using edge_type      = dynamic_edge<void, VV, GV, VId, true, Bidirectional, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type src_id, vertex_id_type tgt_id)
        : base_target_type(tgt_id), base_source_type(src_id) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;

  // Comparison operators for ordered/unordered set containers
  // Compare by (source_id, target_id)
  constexpr auto operator<=>(const dynamic_edge& rhs) const noexcept {
    if (auto cmp = base_source_type::source_id() <=> rhs.base_source_type::source_id(); cmp != 0)
      return cmp;
    return base_target_type::target_id() <=> rhs.base_target_type::target_id();
  }
  constexpr bool operator==(const dynamic_edge& rhs) const noexcept {
    return base_source_type::source_id() == rhs.base_source_type::source_id() &&
           base_target_type::target_id() == rhs.base_target_type::target_id();
  }
};


/**
 * @ingroup graph_containers
 * @brief The edge class for a @c dynamic_graph.
 * 
 * The edge is a composition of a target id, optional source id (@c Sourced) and optional edge value.
 * This implementation supports a source id and edge value, where additional specializations exist
 * for different combinations. The real functionality for properties is implemented in the 
 * @c dynamic_edge_target, @c dynamic_edge_source and @c dynamic_edge_value base classes.
 *
 * This is a specialization definition for @c EV!=void and @c Sourced=false.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced [false] Source id is not stored on the edge. Use of @c source_id(g,uv) or 
 *                 @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_edge<EV, VV, GV, VId, false, Bidirectional, Traits>
      : public dynamic_edge_target<EV, VV, GV, VId, false, Bidirectional, Traits>
      , public dynamic_edge_source<EV, VV, GV, VId, false, Bidirectional, Traits>
      , public dynamic_edge_value<EV, VV, GV, VId, false, Bidirectional, Traits> {
public:
  using base_target_type = dynamic_edge_target<EV, VV, GV, VId, false, Bidirectional, Traits>;
  using base_source_type = dynamic_edge_source<EV, VV, GV, VId, false, Bidirectional, Traits>;
  using base_value_type  = dynamic_edge_value<EV, VV, GV, VId, false, Bidirectional, Traits>;

  using vertex_id_type = VId;
  using value_type     = EV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, false, Bidirectional, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, false, Bidirectional, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, false, Bidirectional, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type targ_id) : base_target_type(targ_id) {}
  constexpr dynamic_edge(vertex_id_type targ_id, const value_type& val)
        : base_target_type(targ_id), base_value_type(val) {}
  constexpr dynamic_edge(vertex_id_type targ_id, value_type&& val)
        : base_target_type(targ_id), base_value_type(std::move(val)) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;

  // Comparison operators for ordered/unordered set containers
  // Compare by target_id only (source_id not stored when Sourced=false)
  constexpr auto operator<=>(const dynamic_edge& rhs) const noexcept {
    return base_target_type::target_id() <=> rhs.base_target_type::target_id();
  }
  constexpr bool operator==(const dynamic_edge& rhs) const noexcept {
    return base_target_type::target_id() == rhs.base_target_type::target_id();
  }
};

/**
 * @ingroup graph_containers
 * @brief The edge class for a @c dynamic_graph.
 * 
 * The edge is a composition of a target id, optional source id (@c Sourced) and optional edge value.
 * This implementation supports a source id and edge value, where additional specializations exist
 * for different combinations. The real functionality for properties is implemented in the 
 * @c dynamic_edge_target, @c dynamic_edge_source and @c dynamic_edge_value base classes.
 *
 * This is a specialization definition for @c EV=void and @c Sourced=false.
 * 
 * @tparam EV      [void] A value type is not defined for the edge. Calling @c edge_value(g,uv)
 *                 will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced [false] Source id is not stored on the edge. Use of @c source_id(g,uv) or 
 *                 @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class VV, class GV, class VId, bool Bidirectional, class Traits>
class dynamic_edge<void, VV, GV, VId, false, Bidirectional, Traits>
      : public dynamic_edge_target<void, VV, GV, VId, false, Bidirectional, Traits>
      , public dynamic_edge_source<void, VV, GV, VId, false, Bidirectional, Traits>
      , public dynamic_edge_value<void, VV, GV, VId, false, Bidirectional, Traits> {
public:
  using base_target_type = dynamic_edge_target<void, VV, GV, VId, false, Bidirectional, Traits>;
  using base_source_type = dynamic_edge_source<void, VV, GV, VId, false, Bidirectional, Traits>;
  using base_value_type  = dynamic_edge_value<void, VV, GV, VId, false, Bidirectional, Traits>;

  using vertex_id_type = VId;
  using value_type     = void;
  using graph_type     = dynamic_graph<void, VV, GV, VId, false, Bidirectional, Traits>;
  using vertex_type    = dynamic_vertex<void, VV, GV, VId, false, Bidirectional, Traits>;
  using edge_type      = dynamic_edge<void, VV, GV, VId, false, Bidirectional, Traits>;

public:
  constexpr dynamic_edge(vertex_id_type vid) : base_target_type(vid) {}

  constexpr dynamic_edge()                    = default;
  constexpr dynamic_edge(const dynamic_edge&) = default;
  constexpr dynamic_edge(dynamic_edge&&)      = default;
  constexpr ~dynamic_edge()                   = default;

  constexpr dynamic_edge& operator=(const dynamic_edge&) = default;
  constexpr dynamic_edge& operator=(dynamic_edge&&)      = default;

  // Comparison operators for ordered/unordered set containers
  // Compare by target_id only (source_id not stored when Sourced=false)
  constexpr auto operator<=>(const dynamic_edge& rhs) const noexcept {
    return base_target_type::target_id() <=> rhs.base_target_type::target_id();
  }
  constexpr bool operator==(const dynamic_edge& rhs) const noexcept {
    return base_target_type::target_id() == rhs.base_target_type::target_id();
  }
};

//--------------------------------------------------------------------------------------------------
// dynamic_vertex_bidir_base — conditional base class for bidirectional vertex support
//
// When Bidirectional=false (default), this base is an empty class with no storage cost.
// When Bidirectional=true, it stores the reverse (incoming) edge container directly on the vertex.
// The in_edges() member function is automatically discovered by the in_edges CPO via the
// _vertex_member dispatch tier (u.inner_value(g).in_edges()), so no ADL friend is needed.
//
// Bidirectional support requires Sourced=true so that the source_id CPO can correctly
// identify the origin vertex of each incoming edge via the native edge member tier.
//--------------------------------------------------------------------------------------------------

/// @brief Empty base for non-bidirectional vertices (zero storage cost).
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex_bidir_base {
public:
  constexpr dynamic_vertex_bidir_base()                                 = default;
  constexpr dynamic_vertex_bidir_base(const dynamic_vertex_bidir_base&) = default;
  constexpr dynamic_vertex_bidir_base(dynamic_vertex_bidir_base&&)      = default;
  constexpr ~dynamic_vertex_bidir_base()                                = default;

  constexpr dynamic_vertex_bidir_base& operator=(const dynamic_vertex_bidir_base&) = default;
  constexpr dynamic_vertex_bidir_base& operator=(dynamic_vertex_bidir_base&&)      = default;

  /// Accept allocator but ignore it (empty base).
  template <class Alloc>
  constexpr dynamic_vertex_bidir_base(Alloc) {}
};

/// @brief Populated base for bidirectional vertices — stores incoming edges.
template <class EV, class VV, class GV, class VId, bool Sourced, class Traits>
class dynamic_vertex_bidir_base<EV, VV, GV, VId, Sourced, true, Traits> {
  static_assert(Sourced,
                "Bidirectional dynamic_graph requires Sourced=true so that source_id(g, ie) "
                "correctly identifies the origin vertex of each incoming edge.");

public:
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

  constexpr dynamic_vertex_bidir_base()                                 = default;
  constexpr dynamic_vertex_bidir_base(const dynamic_vertex_bidir_base&) = default;
  constexpr dynamic_vertex_bidir_base(dynamic_vertex_bidir_base&&)      = default;
  constexpr ~dynamic_vertex_bidir_base()                                = default;

  constexpr dynamic_vertex_bidir_base& operator=(const dynamic_vertex_bidir_base&) = default;
  constexpr dynamic_vertex_bidir_base& operator=(dynamic_vertex_bidir_base&&)      = default;

  constexpr dynamic_vertex_bidir_base(allocator_type alloc) : in_edges_(alloc) {}

  constexpr edges_type&       in_edges() noexcept { return in_edges_; }
  constexpr const edges_type& in_edges() const noexcept { return in_edges_; }

private:
  edges_type in_edges_;
};

//--------------------------------------------------------------------------------------------------
// dynamic_vertex
//

/**
 * @ingroup graph_containers
 * @brief Base implementation of a vertex that provides access to outgoing edges on the vertex.
 * 
 * Edges are stored in a container on the vertex.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex_base
      : public dynamic_vertex_bidir_base<EV, VV, GV, VId, Sourced, Bidirectional, Traits> {
  using bidir_base = dynamic_vertex_bidir_base<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;

public:
  using vertex_id_type = VId;
  using value_type     = VV;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex_base()                           = default;
  constexpr dynamic_vertex_base(const dynamic_vertex_base&) = default;
  constexpr dynamic_vertex_base(dynamic_vertex_base&&)      = default;
  constexpr ~dynamic_vertex_base()                          = default;

  constexpr dynamic_vertex_base& operator=(const dynamic_vertex_base&) = default;
  constexpr dynamic_vertex_base& operator=(dynamic_vertex_base&&)      = default;

  constexpr dynamic_vertex_base(allocator_type alloc) : bidir_base(alloc), edges_(alloc) {}

public:
  constexpr edges_type&       edges() noexcept { return edges_; }
  constexpr const edges_type& edges() const noexcept { return edges_; }

  constexpr auto begin() noexcept { return edges_.begin(); }
  constexpr auto begin() const noexcept { return edges_.begin(); }
  constexpr auto cbegin() const noexcept { return edges_.begin(); }

  constexpr auto end() noexcept { return edges_.end(); }
  constexpr auto end() const noexcept { return edges_.end(); }
  constexpr auto cend() const noexcept { return edges_.end(); }

private:
  edges_type edges_;

private: // CPO properties
  // Note: edge_value friend function is defined in dynamic_graph_base

  // Helper to extract the vertex type from a descriptor's value_type
  // For vector: value_type is vertex_type directly
  // For map: value_type is pair<const VId, vertex_type>, so we need second_type
  template <typename VT>
  struct extract_vertex_from_value {
    using type = VT; // Default: the value type itself
  };

  template <typename K, typename V>
  struct extract_vertex_from_value<std::pair<K, V>> {
    using type = V; // For pairs, use the second element
  };

  // ADL friend function - CPO will find this via ADL
  template <typename U>
  requires vertex_descriptor_type<U>
  [[nodiscard]] friend constexpr auto edges(graph_type& g, const U& u) noexcept {
    using vertex_iter_t = typename U::iterator_type;
    auto& edges_ref     = u.inner_value(g).edges_;
    using edge_iter_t   = decltype(edges_ref.begin());
    return edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_ref, u);
  }

  template <typename U>
  requires vertex_descriptor_type<U>
  [[nodiscard]] friend constexpr auto edges(const graph_type& g, const U& u) noexcept {
    using vertex_iter_t   = typename U::iterator_type;
    const auto& edges_ref = u.inner_value(g).edges_;
    using edge_iter_t     = decltype(edges_ref.begin());
    return edge_descriptor_view<edge_iter_t, vertex_iter_t>(edges_ref, u);
  }

  // ADL friend functions for in-edges — only available for bidirectional graphs
  template <typename U>
  requires vertex_descriptor_type<U>
  [[nodiscard]] friend constexpr auto in_edges(graph_type& g, const U& u) noexcept
  requires(Bidirectional)
  {
    using vertex_iter_t = typename U::iterator_type;
    auto& in_edges_ref  = u.inner_value(g).in_edges();
    using edge_iter_t   = decltype(in_edges_ref.begin());
    return edge_descriptor_view<edge_iter_t, vertex_iter_t, in_edge_tag>(in_edges_ref, u);
  }

  template <typename U>
  requires vertex_descriptor_type<U>
  [[nodiscard]] friend constexpr auto in_edges(const graph_type& g, const U& u) noexcept
  requires(Bidirectional)
  {
    using vertex_iter_t      = typename U::iterator_type;
    const auto& in_edges_ref = u.inner_value(g).in_edges();
    using edge_iter_t        = decltype(in_edges_ref.begin());
    return edge_descriptor_view<edge_iter_t, vertex_iter_t, in_edge_tag>(in_edges_ref, u);
  }

  // friend constexpr typename edges_type::iterator
  // find_vertex_edge(graph_type& g, vertex_id_type uid, vertex_id_type vid) {
  //   return std::ranges::find(g[uid].edges_,
  //                            [&g, &vid](const edge_type& uv) -> bool { return target_id(g, uv) == vid; });
  // }
  // friend constexpr typename edges_type::const_iterator
  // find_vertex_edge(const graph_type& g, vertex_id_type uid, vertex_id_type vid) {
  //   return std::ranges::find(g[uid].edges_,
  //                            [&g, &vid](const edge_type& uv) -> bool { return target_id(g, uv) == vid; });
  // }
};


/**
 * @ingroup graph_containers
 * @brief Implementation of a vertex class of dynamic_graph that provides access to outgoing edges 
 * and the vertex value.
 * 
 * A specialization of this class exists for VV=void that excludes the vertex value. When that is used,
 * Any attempt to use @c vertex_value(g,u) will generate a compile error.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex : public dynamic_vertex_base<EV, VV, GV, VId, Sourced, Bidirectional, Traits> {
public:
  using base_type      = dynamic_vertex_base<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using vertex_id_type = VId;
  using value_type     = remove_cvref_t<VV>;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using vertex_type    = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using edge_type      = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex(const value_type& value, allocator_type alloc = allocator_type())
        : base_type(alloc), value_(value) {}
  constexpr dynamic_vertex(value_type&& value, allocator_type alloc = allocator_type())
        : base_type(alloc), value_(move(value)) {}
  constexpr dynamic_vertex(allocator_type alloc) : base_type(alloc) {}

  constexpr dynamic_vertex()                      = default;
  constexpr dynamic_vertex(const dynamic_vertex&) = default;
  constexpr dynamic_vertex(dynamic_vertex&&)      = default;
  constexpr ~dynamic_vertex()                     = default;

  constexpr dynamic_vertex& operator=(const dynamic_vertex&) = default;
  constexpr dynamic_vertex& operator=(dynamic_vertex&&)      = default;

public:
  using base_type::edges;

  constexpr value_type&       value() noexcept { return value_; }
  constexpr const value_type& value() const noexcept { return value_; }

private:
  value_type value_ = value_type();

private: // CPO properties
         // friend constexpr value_type&       vertex_value(graph_type& g, vertex_type& u) { return u.value_; }
  // friend constexpr const value_type& vertex_value(const graph_type& g, const vertex_type& u) { return u.value_; }
};


/**
 * @ingroup graph_containers
 * @brief Implementation of a vertex class of dynamic_graph that provides access to outgoing edges.
 * 
 * This is a specialization for @c VV=void that excludes the @c vertex_value(g,u) definition. When it is used,
 * a compile error will be generated so the user can resolve the incorrect usage.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge and 
 *                 calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      [void] No user value is stored on the vertex and calling @c vertex_value(g,u)
 *                 will generate a compile error.
 * @tparam GV      The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
*/
template <class EV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_vertex<EV, void, GV, VId, Sourced, Bidirectional, Traits>
      : public dynamic_vertex_base<EV, void, GV, VId, Sourced, Bidirectional, Traits> {
public:
  using base_type      = dynamic_vertex_base<EV, void, GV, VId, Sourced, Bidirectional, Traits>;
  using vertex_id_type = VId;
  using value_type     = void;
  using graph_type     = dynamic_graph<EV, void, GV, VId, Sourced, Bidirectional, Traits>;
  using vertex_type    = dynamic_vertex<EV, void, GV, VId, Sourced, Bidirectional, Traits>;
  using edge_type      = dynamic_edge<EV, void, GV, VId, Sourced, Bidirectional, Traits>;
  using edges_type     = typename Traits::edges_type;
  using allocator_type = typename edges_type::allocator_type;

public:
  constexpr dynamic_vertex()                      = default;
  constexpr dynamic_vertex(const dynamic_vertex&) = default;
  constexpr dynamic_vertex(dynamic_vertex&&)      = default;
  constexpr ~dynamic_vertex()                     = default;

  constexpr dynamic_vertex& operator=(const dynamic_vertex&) = default;
  constexpr dynamic_vertex& operator=(dynamic_vertex&&)      = default;

  constexpr dynamic_vertex(allocator_type alloc) : base_type(alloc) {}
};

/**-------------------------------------------------------------------------------------------------
 * @ingroup graph_containers
 * @brief dynamic_graph_base defines the core implementation for a graph with a variety 
 * characteristics including edge, vertex and graph value types, whether edges are sourced or not, 
 * the vertex id type, and selection of the containers used for vertices and edges.
 * 
 * This class includes the vertices and functions to access them, and the constructors and 
 * functions to load the vertices and edges into the graph.
 * 
 * dynamic_graph provides the interface that includes or excludes (GV=void) a graph value.
 * dynamic_graph_base has the core implementation for the graph.
 * 
 * No additional space is used if the edge value type (EV) or vertex value type (VV) is void.
 * 
 * The partition function is intended to determine the partition id for a vertex id on constructors.
 * It has been added to assure the same interface as the compressed_graph, but is not implemented
 * at this time.
 * 
 * @tparam EV            The edge value type. If "void" is used no user value is stored on the edge and 
 *                       calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV            The vertex value type. If "void" is used no user value is stored on the vertex
 *                       and calls to @c vertex_value(g,u) will generate a compile error.
 * @tparam GV            The graph value type. If "void" is used no user value is stored on the graph
 *                       and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced       Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                       and @c source(g,uv) will generate a compile error.
 * @tparam VId           The type used for the vertex id.
 * @tparam Traits        Defines the types for vertex and edge containers.
 * @tparam Bidirectional If true, maintains per-vertex reverse adjacency lists so that
 *                       @c in_edges(g,u) is available. Requires @c Sourced=true.
*/
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph_base {

public: // types
  using graph_type   = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using graph_traits = Traits;

  using partition_id_type = VId;
  using partition_vector  = std::vector<VId>;

  using vertex_id_type        = VId;
  using vertex_type           = dynamic_vertex<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using vertices_type         = typename Traits::vertices_type;
  using vertex_allocator_type = typename vertices_type::allocator_type;
  using size_type             = typename vertices_type::size_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;

public: // Construction/Destruction/Assignment
  constexpr dynamic_graph_base()                          = default;
  constexpr dynamic_graph_base(const dynamic_graph_base&) = default;
  constexpr dynamic_graph_base(dynamic_graph_base&&)      = default;
  constexpr ~dynamic_graph_base()                         = default;

  constexpr dynamic_graph_base& operator=(const dynamic_graph_base&) = default;
  constexpr dynamic_graph_base& operator=(dynamic_graph_base&&)      = default;

  /**
   * @brief Construct an empty graph using the allocator passed.
   * 
   * @param alloc Used to allocate vertices and edges.
  */
  dynamic_graph_base(vertex_allocator_type alloc) : vertices_(alloc), partition_(alloc) { terminate_partitions(); }

  /**
   * @brief Construct the graph using edge and vertex ranges.
   * 
   * The value_type of @c erng must be converted to @c copyable_edge_t<G> before it can be
   * added to the graph, which is done using the @c eproj function. If the value_type of
   * @c erng is already copyable_edge_t<G>, @c std::identity can be used instead. @c copyable_edge_t<G>
   * will have a edge value member if an edge value type has been defined for the graph (e.g.
   * @c EV is not @c void).
   * 
   * The value_type of @c vrng must be converted to @c copyable_vertex_t<G> before it can be
   * added to the graph, which is done using the @c vproj function. If the value_type of
   * @c vrng is already copyable_vertex_t<G>, @c std::identity can be used instead. @c copyable_vertex_t<G>
   * will have a vertex value member if a vertex value type has been defined for the graph (e.g.
   * @c VV is not @c void).
   * 
   * @tparam ERng    The edge data range.
   * @tparam VRng    The vertex data range.
   * @tparam EProj   A function type to convert the ERng value_type to copyable_edge_t<G>.
   *                 If ERng value_type is already copyable_vertex_t<G>, identity can be used.
   * @tparam VProj   A projection function type to convert the VRng value_type to copyable_vertex_t<G>
   *                 If VRng value_type is already copyable_vertex_t<G>, identify can be used.
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng                The edge values.
   * @param vrng                The vertex values.
   * @param eproj               The projection function that converts the ERng value_type to copyable_edge_t<G>, 
   *                            or identity() if ERng value_type is already copyable_edge_t<G>.
   * @param vproj               The projection function that converts the ERng value_type to copyable_vertex_t<G>, or 
   *                            identity() if VRng value_type is already copyable_vertex_t<G>.
   * @param partition_start_ids Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
   * @param alloc The allocator used for vertices and edges containers.
  */
  template <class ERng, class VRng, forward_range PartRng, class EProj = identity, class VProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  dynamic_graph_base(ERng&&                erng,
                     VRng&&                vrng,
                     EProj                 eproj,
                     VProj                 vproj,
                     const PartRng&        partition_start_ids = std::vector<VId>(),
                     vertex_allocator_type alloc               = vertex_allocator_type())
        : vertices_(alloc), partition_(alloc) {
    partition_.assign(partition_start_ids.begin(), partition_start_ids.end());
    load_vertices(vrng, vproj);
    // TODO: not all partitions may be created properly when vertex_ids in edges don't include vertices in all partitions
    // canonical order: load_edges(erng, eproj, vertex_count, edge_count_hint)
    load_edges(erng, eproj, vertices_.size(), 0);
    terminate_partitions();
  }

  /**
   * @brief Construct the graph using an edge range.
   * 
   * The value_type of @c erng must be converted to @c copyable_edge_t<G> before it can be
   * added to the graph, which is done using the @c eproj function. If the value_type of
   * @c erng is already copyable_edge_t<G>, @c std::identity can be used instead. @c copyable_edge_t<G>
   * will have a edge value member if an edge value type has been defined for the graph (e.g.
   * @c EV is not @c void).
   * 
   * Edges are scanned to determine the largest vertex id needed.
   * 
   * If vertices have a user-defined value (e.g. VV not void), the value must be default-constructable.
   * 
   * @tparam ERng    The edge data range.
   * @tparam EProj   A projection function type to convert the ERng value_type to copyable_edge_t<G>.
   *                 If ERng value_type is already copyable_vertex_t<G>, identity can be used.
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng                The edge values.
   * @param eproj               The projection function that converts the ERng value_type to copyable_edge_t<G>, 
   *                            or identity() if ERng value_type is already copyable_edge_t<G>.
   * @param partition_start_ids Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
   * @param alloc               The allocator used for vertices and edges containers.
  */
  template <class ERng, forward_range PartRng, class EProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  dynamic_graph_base(ERng&&                erng,
                     EProj                 eproj,
                     const PartRng&        partition_start_ids = std::vector<VId>(),
                     vertex_allocator_type alloc               = vertex_allocator_type())
        : vertices_(alloc), partition_(alloc) {
    partition_.assign(partition_start_ids.begin(), partition_start_ids.end());
    // canonical order: load_edges(erng, eproj, vertex_count, edge_count_hint) with inference (vertex_count==0)
    load_edges(move(erng), eproj, 0, 0);
    terminate_partitions();
  }

  /**
   * @brief Construct the graph using a vertex count and an edge range.
   * 
   * The value_type of @c erng must be converted to @c copyable_edge_t<G> before it can be
   * added to the graph, which is done using the @c eproj function. If the value_type of
   * @c erng is already copyable_edge_t<G>, @c std::identity can be used instead. @c copyable_edge_t<G>
   * will have a edge value member if an edge value type has been defined for the graph (e.g.
   * @c EV is not @c void).
   * 
   * Edges are NOT scanned to determine the largest vertex id needed.
   * 
   * If vertices have a user-defined value (e.g. VV not void), the value must be default-constructable.
   * 
   * @tparam ERng    The edge data range.
   * @tparam EProj   A function type to convert the ERng value_type to copyable_edge_t<G>.
   *                 If ERng value_type is already copyable_vertex_t<G>, identity can be used.
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param vertex_count        The number of vertices to create.
   * @param erng                The edge values.
   * @param eproj               A function that converts the ERng value_type to copyable_edge_t<G>, 
   *                            or identity() if ERng value_type is already copyable_edge_t<G>.
   * @param partition_start_ids Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
   * @param alloc               The allocator used for vertices and edges containers.
  */
  template <class ERng, forward_range PartRng, class EProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  dynamic_graph_base(size_t                vertex_count,
                     ERng&&                erng,
                     EProj                 eproj,
                     const PartRng&        partition_start_ids = std::vector<VId>(),
                     vertex_allocator_type alloc               = vertex_allocator_type())
        : vertices_(alloc), partition_(alloc) {
    partition_.assign(partition_start_ids.begin(), partition_start_ids.end());
    // canonical order: load_edges(erng, eproj, vertex_count, edge_count_hint)
    load_edges(move(erng), eproj, vertex_count, 0);
    terminate_partitions();
  }

  /**
   * @brief Construct the graph using a intializer list of copyable edges.
   * 
   * If vertices have a user-defined value (e.g. VV not void), the value must be default-constructable.
   * 
   * @param il    The initializer list of copyable edge values.
   * @param alloc The allocator to use for the vertices and edges containers.
  */
  dynamic_graph_base(const std::initializer_list<copyable_edge_t<VId, EV>>& il,
                     edge_allocator_type                                    alloc = edge_allocator_type())
        : vertices_(alloc), partition_(alloc) {
    if constexpr (is_associative_container<vertices_type>) {
      // For associative containers, load_edges will auto-insert vertices via operator[]
      // No need to pre-size or compute max vertex ID
      load_edges(il, {}, 0, il.size());
    } else {
      // For sequential containers, compute the required size and pre-allocate
      size_t last_id = 0;
      for (auto&& e : il)
        last_id = max(last_id, static_cast<size_t>(max(e.source_id, e.target_id)));
      resize_vertices(last_id + 1);
      // canonical order: supply vertex_count to avoid inference pass
      load_edges(il, {}, vertices_.size(), il.size());
    }
    terminate_partitions();
  }

public: // Load operations
  /**
   * @brief Load vertices and copy vertex values into the graph.
   * 
   * The id assigned in the returned @c copyable_vertex_t<VId,VV> by @vproj from @vproj is used to look up 
   * the vertex in the internal vertices container and the vertex value is copied to the vertex's value. This 
   * implies the following consequences/behavior:
   *  * Vertex values must be default-constructable
   *  * @c vrng entries don't need to be assigned consecutive vertex id's.
   *  * If the same id is used for different entries of @c vrng (or subsequent calls to @c load_vertices())
   *    then there is a conflict as to which one should be used. In this case, the last one assigned is used.
   * 
   * @tparam VRng        Type of the range of vertices to be loaded.
   * @tparam VProj       Type of projection function to convert a @c VRng value type to a @c copyable_vertex_t.
   *                     If the @c VRng value type is already a copyable_vertex_t then std::identity can be used.
   * @param vrng         The range of vertices to be loaded.
   * @param vproj        The projection function to convert the @c vrng value type to a @c copyable_vertex_t.
   *                     if the @c vrng value type is already a copyable_vertex_t then std::identity() can be used.
   * @param vertex_count The number of vertices to resize the internal vertices container to, if available. This
   *                     can be used to pre-extend the number of vertices beyond those supplied in @c vrng.
  */
  template <class VRng, class VProj = identity>
  void load_vertices(const VRng& vrng, VProj vproj = {}, size_type vertex_count = 0) {
    if constexpr (is_associative_container<vertices_type>) {
      // For associative containers, operator[] auto-inserts vertices
      // No resize needed, use vertex ID directly as key
      for (auto&& v : vrng) {
        if constexpr (is_void_v<VV>) {
          // copyable_vertex_t<VId, void> has only 1 element: {id}
          auto&& projected = vproj(v);
          VId    id        = projected.id;
          (void)vertices_[id]; // ensure vertex exists
        } else {
          auto&& [id, value]    = vproj(v); //copyable_vertex_t<VId, VV>
          vertices_[id].value() = value;
        }
      }
    } else {
      // For sequential containers, pre-size and use index-based access
      if constexpr (sized_range<VRng> && resizable<vertices_type>) {
        vertex_count = std::max(vertex_count, static_cast<size_t>(vertices_.size()));
        resize_vertices(std::max(vertex_count, static_cast<size_t>(std::ranges::size(vrng))));
      }
      for (auto&& v : vrng) {
        if constexpr (is_void_v<VV>) {
          // copyable_vertex_t<VId, void> has only 1 element: {id}
          auto&& projected = vproj(v);
          size_t k         = static_cast<size_t>(projected.id);
          if constexpr (random_access_range<vertices_type>) {
            if (k >= vertices_.size()) [[unlikely]]
              throw std::out_of_range("vertex id in load_vertices exceeds current vertex container size");
          }
          (void)vertices_[k]; // ensure vertex exists
        } else {
          auto&& [id, value] = vproj(v); //copyable_vertex_t<VId, VV>
          size_t k           = static_cast<size_t>(id);
          if constexpr (random_access_range<vertices_type>) {
            if (k >= vertices_.size()) [[unlikely]]
              throw std::out_of_range("vertex id in load_vertices exceeds current vertex container size");
          }
          vertices_[k].value() = value;
        }
      }
    }
  }

  /**
   * @brief Load vertices and move vertex values into the graph.
   * 
   * The id assigned in the returned @c copyable_vertex_t<VId,VV> by @vproj from @vproj is used to look up 
   * the vertex in the internal vertices container and the vertex value is moved to the vertex's value. This 
   * implies the following consequences/behavior:
   *  * Vertex values must be default-constructable
   *  * @c vrng entries don't need to be assigned consecutive vertex id's.
   *  * If the same id is used for different entries of @c vrng (or subsequent calls to @c load_vertices())
   *    then there is a conflict as to which one should be used. In this case, the last one assigned is used.
   * 
   * TODO This is the wrong design for move vs. copy because we're not moving the container, only some of the
   * contents of the container. It would be better to have a policy parameter (type, run-time or both) or to
   * have functions with different names to make it clear. Examples might be @c copy_vertex_values & 
   * @c move_vertex_values.
   * 
   * @tparam VRng        Type of the range of vertices to be loaded.
   * @tparam VProj       Type of projection function to convert a @c VRng value type to a @c copyable_vertex_t.
   *                     If the @c VRng value type is already a copyable_vertex_t then std::identity can be used.
   * @param vrng         The range of vertices to be loaded.
   * @param vproj        The projection function to convert the @c vrng value type to a @c copyable_vertex_t.
   *                     if the @c vrng value type is already a copyable_vertex_t then std::identity() can be used.
   * @param vertex_count The number of vertices to resize the internal vertices container to, if available. This
   *                     can be used to pre-extend the number of vertices beyond those supplied in @c vrng.
  */
  template <class VRng, class VProj = identity>
  void load_vertices(VRng&& vrng, VProj vproj = {}, size_type vertex_count = 0) {
    if constexpr (is_associative_container<vertices_type>) {
      // For associative containers, operator[] auto-inserts vertices
      // No resize needed, use vertex ID directly as key
      for (auto&& v : vrng) {
        if constexpr (is_void_v<VV>) {
          // copyable_vertex_t<VId, void> has only 1 element: {id}
          auto&& projected = vproj(v);
          VId    id        = projected.id;
          (void)vertices_[id]; // ensure vertex exists
        } else {
          auto&& [id, value]    = vproj(v); //copyable_vertex_t<VId, VV>
          vertices_[id].value() = move(value);
        }
      }
    } else {
      // For sequential containers, pre-size and use index-based access
      // Harmonize sizing logic with const& overload (ensure we never shrink and honor explicit vertex_count)
      if constexpr (sized_range<VRng> && resizable<vertices_type>) {
        vertex_count = std::max(vertex_count, static_cast<size_t>(vertices_.size()));
        resize_vertices(std::max(vertex_count, static_cast<size_t>(std::ranges::size(vrng))));
      }
      for (auto&& v : vrng) {
        if constexpr (is_void_v<VV>) {
          // copyable_vertex_t<VId, void> has only 1 element: {id}
          auto&& projected = vproj(v);
          size_t k         = static_cast<size_t>(projected.id);
          if constexpr (random_access_range<vertices_type>) {
            if (k >= vertices_.size()) [[unlikely]]
              throw std::out_of_range("vertex id in load_vertices exceeds current vertex container size");
          }
          (void)vertices_[k]; // ensure vertex exists
        } else {
          auto&& [id, value] = vproj(v); //copyable_vertex_t<VId, VV>
          size_t k           = static_cast<size_t>(id);
          if constexpr (random_access_range<vertices_type>) {
            if (k >= vertices_.size()) [[unlikely]]
              throw std::out_of_range("vertex id in load_vertices exceeds current vertex container size");
          }
          vertices_[k].value() = move(value);
        }
      }
    }
  }


  /**
   * @brief Load edges and copy edge values into the graph.
   * 
   * Edge values are appended to the container for a vertex. No check is made for duplicate entries.
   * 
   * If edge values have been defined for the graph they will be copied into the graph's edge value.
   * 
   * If the @c source_id or @c target_id of the returned @c copyable_edge_t<VId,EV> by @c eproj is
   * larger than the number of vertices an exception will be thrown.
   * 
   * TODO: ERng not a forward_range because CSV reader doesn't conform to be a forward_range
   *
   * @tparam ERng  Range type of source data for edges
   * @tparam EProj Projection function type that converts @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already a @c copyable_edge_t<VId,EV> then @c std::identity can 
   *               be used instead.
   *
   * @param erng            The source range of edge data.
   * @param eproj           The projection function to convert an @c erng value type to a @c copyable_edge_t<VId,EV>.
   *                        If the @c erng value type is already a @c copyable_edge_t<VId,EV> then std::identity()
   *                        can be used instead.
   * @param vertex_count    If larger than the existing number of vertices then the number of vertices will be grown 
   *                        to match @c vertex_count, if applicable. Vertex values need to be movable.
  * @param edge_count_hint Optional hint for number of edges; used to reserve temporary storage
  */
  // Removed const& overload of load_edges to avoid const-qualification of elements that breaks
  // legacy projection lambdas expecting non-const lvalue references. The forwarding reference
  // overload below correctly handles lvalue/rvalue and const/non-const ERng.

  /**
   * @brief Load edges and move edge values into the graph.
   * 
   * Edge values are appended to the container for a vertex. No check is made for duplicate entries.
   * 
   * If edge values have been defined for the graph they will be copied into the graph's edge value.
   * 
   * If the @c source_id or @c target_id of the returned @c copyable_edge_t<VId,EV> by @c eproj is
   * larger than the number of vertices an exception will be thrown.
   * 
   * TODO: ERng not a forward_range because CSV reader doesn't conform to be a forward_range
   * 
   * TODO: This is the wrong design because we're not moving the @c erng into the graph, we're just
   *       moving edge values. Alternative designs include using a policy parameter (type and/or run-time)
   *       or different function names, such as append_edges_copy_values and append_edges_move_values, or
   *       simple copy_edge_values and move_edge_values.
   *
   * @tparam ERng  Range type of source data for edges
   * @tparam EProj Projection function type that converts @c ERng value type to a @c copyable_edge_t<VId,EV>.
   *               If the @c ERng value type is already a @c copyable_edge_t<VId,EV> then @c std::identity can 
   *               be used instead.
   *
   * @param erng            The source range of edge data.
   * @param eproj           The projection function to convert an @c erng value type to a @c copyable_edge_t<VId,EV>.
   *                        If the @c erng value type is already a @c copyable_edge_t<VId,EV> then std::identity()
   *                        can be used instead.
   * @param vertex_count    If larger than the existing number of vertices then the number of vertices will be grown 
   *                        to match @c vertex_count, if applicable. Vertex values need to be movable.
  * @param edge_count_hint Optional hint for number of edges; used to reserve temporary storage when
  *                        inferring vertex count from a single pass over a forward range.
  */
  template <class ERng, class EProj = identity>
  void load_edges(ERng&& erng, EProj eproj = {}, size_type vertex_count = 0, size_type edge_count_hint = 0) {
    using std::move; // ADL safety

    // For associative containers (map/unordered_map), we use a different strategy:
    // - operator[] auto-inserts vertices, so no need to resize or check bounds
    // - We don't need to infer max_id for sizing
    if constexpr (is_associative_container<vertices_type>) {
      // Associative container path: simply iterate and insert
      for (auto&& edge_data : erng) {
        using proj_edge_ref_t = decltype(eproj(edge_data));
        using proj_edge_t     = std::decay_t<proj_edge_ref_t>;
        static_assert(!std::is_reference_v<proj_edge_t>, "proj_edge_t must not be a reference");
        proj_edge_t e = eproj(edge_data); // materialize value

        // operator[] on map will auto-insert default vertex if not present
        // We need to ensure both source and target vertices exist
        (void)vertices_[e.target_id]; // ensure target vertex exists
        auto& vertex_edges = vertices_[e.source_id].edges();
        if constexpr (Sourced) {
          if constexpr (is_void_v<EV>) {
            if constexpr (Bidirectional) {
              auto& rev = vertices_[e.target_id].in_edges();
              emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id));
            }
            emplace_edge(vertex_edges, e.target_id, edge_type(e.source_id, e.target_id));
          } else {
            if constexpr (Bidirectional) {
              auto& rev = vertices_[e.target_id].in_edges();
              emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id, e.value)); // copy
            }
            emplace_edge(vertex_edges, e.target_id, edge_type(e.source_id, e.target_id, std::move(e.value)));
          }
        } else {
          if constexpr (is_void_v<EV>) {
            emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id));
          } else {
            emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id, std::move(e.value)));
          }
        }
        edge_count_ += 1;
      }
    } else {
      // Sequential container path (vector/deque): original logic
      // Optimized strategy: for forward ranges without explicit vertex_count we do a single pass collecting
      // projected edges, then size vertices once, then insert. This avoids projecting twice.
      bool const need_infer = (vertex_count == 0);
      if constexpr (resizable<vertices_type>) {
        if (vertices_.size() < vertex_count) {
          vertices_.resize(vertex_count, vertex_type(vertices_.get_allocator()));
        }
      }

      if constexpr (forward_range<ERng>) {
        if (need_infer) {
          // Single pass: collect edges & determine max id
          size_type max_id           = vertices_.empty() ? 0 : static_cast<size_t>(vertices_.size() - 1);
          using projected_edge_ref_t = decltype(eproj(*std::begin(erng)));
          using projected_edge_type  = std::decay_t<projected_edge_ref_t>;
          static_assert(!std::is_reference_v<projected_edge_type>, "projected_edge_type must not be a reference");
          std::vector<projected_edge_type> projected;
          if (edge_count_hint)
            projected.reserve(edge_count_hint);
          for (auto&& edge_data_scan : erng) {
            projected_edge_type e_scan = eproj(edge_data_scan); // materialize value (avoid storing references)
            max_id                     = std::max(max_id, static_cast<size_t>(e_scan.source_id));
            max_id                     = std::max(max_id, static_cast<size_t>(e_scan.target_id));
            projected.push_back(e_scan); // copy (value semantics)
          }
          // Only resize if we collected edges; empty input should not create vertices
          if (!projected.empty()) {
            if constexpr (resizable<vertices_type>) {
              if (vertices_.size() <= max_id) {
                vertices_.resize(max_id + 1, vertex_type(vertices_.get_allocator()));
              }
            }
          }
          // If adjacency edge container is vector we can reserve per-vertex capacity now.
          using edges_container_example = decltype(vertices_[0].edges());
          if constexpr (requires(edges_container_example c) { c.reserve(0); }) {
            // Compute out-degree counts per vertex id
            std::vector<size_type> degrees(vertices_.size(), size_type{0});
            for (auto const& e : projected) {
              degrees[static_cast<size_t>(e.source_id)]++;
            }
            for (size_t vid = 0; vid < degrees.size(); ++vid) {
              auto& ec = vertices_[vid].edges();
              if constexpr (requires { ec.reserve(degrees[vid]); }) {
                if (degrees[vid])
                  ec.reserve(degrees[vid]);
              }
            }
            // Also reserve reverse edge capacity for bidirectional graphs
            if constexpr (Bidirectional) {
              std::vector<size_type> in_degrees(vertices_.size(), size_type{0});
              for (auto const& e : projected) {
                in_degrees[static_cast<size_t>(e.target_id)]++;
              }
              for (size_t vid = 0; vid < in_degrees.size(); ++vid) {
                auto& rc = vertices_[vid].in_edges();
                if constexpr (requires { rc.reserve(in_degrees[vid]); }) {
                  if (in_degrees[vid])
                    rc.reserve(in_degrees[vid]);
                }
              }
            }
          }
          // Insert from cached list
          for (auto& e : projected) {
            if (static_cast<size_t>(e.source_id) >= vertices_.size())
              throw std::runtime_error("source id exceeds the number of vertices in load_edges");
            if (static_cast<size_t>(e.target_id) >= vertices_.size())
              throw std::runtime_error("target id exceeds the number of vertices in load_edges");
            auto& vertex_edges = vertices_[static_cast<size_t>(e.source_id)].edges();
            if constexpr (Sourced) {
              if constexpr (is_void_v<EV>) {
                if constexpr (Bidirectional) {
                  auto& rev = vertices_[static_cast<size_t>(e.target_id)].in_edges();
                  emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id));
                }
                emplace_edge(vertex_edges, e.target_id, edge_type(e.source_id, e.target_id));
              } else {
                if constexpr (Bidirectional) {
                  auto& rev = vertices_[static_cast<size_t>(e.target_id)].in_edges();
                  emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id, e.value)); // copy
                }
                emplace_edge(vertex_edges, e.target_id, edge_type(e.source_id, e.target_id, std::move(e.value)));
              }
            } else {
              if constexpr (is_void_v<EV>) {
                emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id));
              } else {
                emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id, std::move(e.value)));
              }
            }
            edge_count_ += 1;
          }
          return; // done
        }
      }

      // Fallback path: (1) vertex_count explicitly supplied or (2) non-forward single-pass range.
      for (auto&& edge_data : erng) {
        using proj_edge_ref_t = decltype(eproj(edge_data));
        using proj_edge_t     = std::decay_t<proj_edge_ref_t>;
        static_assert(!std::is_reference_v<proj_edge_t>, "proj_edge_t must not be a reference");
        proj_edge_t e = eproj(edge_data); // materialize value
        if (static_cast<size_t>(e.source_id) >= vertices_.size())
          throw std::runtime_error("source id exceeds the number of vertices in load_edges");
        if (static_cast<size_t>(e.target_id) >= vertices_.size())
          throw std::runtime_error("target id exceeds the number of vertices in load_edges");
        auto& vertex_edges = vertices_[static_cast<size_t>(e.source_id)].edges();
        if constexpr (Sourced) {
          if constexpr (is_void_v<EV>) {
            if constexpr (Bidirectional) {
              auto& rev = vertices_[static_cast<size_t>(e.target_id)].in_edges();
              emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id));
            }
            emplace_edge(vertex_edges, e.target_id, edge_type(e.source_id, e.target_id));
          } else {
            if constexpr (Bidirectional) {
              auto& rev = vertices_[static_cast<size_t>(e.target_id)].in_edges();
              emplace_edge(rev, e.source_id, edge_type(e.source_id, e.target_id, e.value)); // copy
            }
            emplace_edge(vertex_edges, e.target_id, edge_type(e.source_id, e.target_id, std::move(e.value)));
          }
        } else {
          if constexpr (is_void_v<EV>) {
            emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id));
          } else {
            emplace_edge(vertex_edges, e.target_id, edge_type(e.target_id, std::move(e.value)));
          }
        }
        edge_count_ += 1;
      }
    }
  }

  // ---------------------------------------------------------------------------
  // (Removed deprecated legacy parameter order bridge overload)

private:
  constexpr void terminate_partitions() {
    // Partitions are only meaningful for sequential containers with numeric IDs
    // For associative containers (map/unordered_map), partition functionality is not supported
    if constexpr (is_associative_container<vertices_type>) {
      // No partition support for associative containers - just initialize with a sentinel
      // Note: partition_ may not be properly typed for string keys, so we skip validation
      return;
    } else {
      if (partition_.empty()) {
        partition_.push_back(0);
      } else {
        bool valid = (partition_[0] == 0);
        for (size_t i = 1; valid && i < partition_.size(); ++i) {
          if (!(partition_[i - 1] < partition_[i]))
            valid = false; // enforce strictly increasing
        }
        if (!valid) {
          throw std::invalid_argument("partition_start_ids must start with 0 and be strictly increasing");
        }
      }
      if (!partition_.empty() && partition_.back() > static_cast<partition_id_type>(vertices_.size())) {
        throw std::invalid_argument("partition_start_ids contain id greater than vertex count");
      }
      partition_.push_back(static_cast<partition_id_type>(vertices_.size()));
    }
  }

public: // Properties
  constexpr auto begin() noexcept { return vertices_.begin(); }
  constexpr auto begin() const noexcept { return vertices_.begin(); }
  constexpr auto cbegin() const noexcept { return vertices_.begin(); }

  constexpr auto end() noexcept { return vertices_.end(); }
  constexpr auto end() const noexcept { return vertices_.end(); }
  constexpr auto cend() const noexcept { return vertices_.end(); }

  constexpr auto size() const noexcept { return vertices_.size(); }

  // Returns reference to vertex at index/key i
  // Throws std::out_of_range if vertex doesn't exist
  // Note: For maps, at() returns mapped_type& (vertex), not pair
  constexpr vertex_type&       operator[](size_type i) { return vertices_.at(i); }
  constexpr const vertex_type& operator[](size_type i) const { return vertices_.at(i); }

public: // Operations
  void reserve_vertices(size_type count) {
    if constexpr (reservable<vertices_type>) // reserve if we can; otherwise ignored
      vertices_.reserve(count);
  }
  void reserve_edges(size_type count) {
    // ignored for this graph; may be meaningful for another data structure like CSR
  }

  void resize_vertices(size_type count) {
    if constexpr (resizable<vertices_type>) // resize if we can; otherwise ignored
      vertices_.resize(count);
  }
  void resize_edges(size_type count) {
    // ignored for this graph; may be meaningful for another data structure like CSR
  }

  void clear() noexcept {
    vertices_.clear();
    partition_.clear();
    partition_.push_back(0);
    edge_count_ = 0;
  }

  /**
   * @brief Check if a vertex with the given id exists in the graph.
   * 
   * For sequential containers (vector/deque), checks if id < size().
   * For associative containers (map/unordered_map), uses contains() or find().
   * 
   * @param id The vertex id to check for.
   * @return true if a vertex with this id exists, false otherwise.
   * @note Complexity: O(1) for sequential containers, O(log n) for map, O(1) average for unordered_map.
   * @note This method never modifies the container (unlike operator[] on maps).
   */
  [[nodiscard]] constexpr bool contains_vertex(const vertex_id_type& id) const noexcept {
    if constexpr (is_associative_container<vertices_type>) {
      if constexpr (requires { vertices_.contains(id); }) {
        return vertices_.contains(id); // C++20 contains()
      } else {
        return vertices_.find(id) != vertices_.end();
      }
    } else {
      // Sequential container: check bounds
      if constexpr (std::is_signed_v<vertex_id_type>) {
        if (id < 0)
          return false;
      }
      return static_cast<size_t>(id) < vertices_.size();
    }
  }

  /**
   * @brief Find a vertex by id, returning an iterator to the underlying container.
   * 
   * For sequential containers (vector/deque), returns begin() + id if valid, end() otherwise.
   * For associative containers (map/unordered_map), uses find().
   * 
   * This is a direct container access method, distinct from the find_vertex CPO which
   * returns vertex descriptors. Use this when you need raw container iterators.
   * 
   * @param id The vertex id to find.
   * @return Iterator to the vertex if found, end() otherwise.
   * @note Complexity: O(1) for sequential containers, O(log n) for map, O(1) average for unordered_map.
   * @note This method never modifies the container (unlike operator[] on maps).
   */
  [[nodiscard]] constexpr auto try_find_vertex(const vertex_id_type& id) noexcept {
    if constexpr (is_associative_container<vertices_type>) {
      return vertices_.find(id);
    } else {
      // Sequential container: check bounds and return end() equivalent (vertices_.size()) or the index
      if constexpr (std::is_signed_v<vertex_id_type>) {
        if (id < 0)
          return vertices_.end();
      }
      if (static_cast<size_t>(id) >= vertices_.size()) {
        return vertices_.end();
      }
      return vertices_.begin() + static_cast<size_t>(id);
    }
  }

  [[nodiscard]] constexpr auto try_find_vertex(const vertex_id_type& id) const noexcept {
    if constexpr (is_associative_container<vertices_type>) {
      return vertices_.find(id);
    } else {
      // Sequential container: check bounds and return end() equivalent (vertices_.size()) or the index
      if constexpr (std::is_signed_v<vertex_id_type>) {
        if (id < 0)
          return vertices_.end();
      }
      if (static_cast<size_t>(id) >= vertices_.size()) {
        return vertices_.end();
      }
      return vertices_.begin() + static_cast<size_t>(id);
    }
  }

  /**
   * @brief Access a vertex by id, throwing if not found.
   * 
   * For sequential containers (vector/deque), checks bounds and throws std::out_of_range.
   * For associative containers (map/unordered_map), uses at() which throws std::out_of_range.
   * 
   * @param id The vertex id to access.
   * @return Reference to the vertex.
   * @throws std::out_of_range if the vertex does not exist.
   * @note Complexity: O(1) for sequential containers, O(log n) for map, O(1) average for unordered_map.
   * @note This method never modifies the container (unlike operator[] on maps).
   */
  [[nodiscard]] constexpr vertex_type& vertex_at(const vertex_id_type& id) {
    if constexpr (is_associative_container<vertices_type>) {
      return vertices_.at(id);
    } else {
      // Sequential container: use at() for bounds checking
      return vertices_.at(static_cast<size_t>(id));
    }
  }

  [[nodiscard]] constexpr const vertex_type& vertex_at(const vertex_id_type& id) const {
    if constexpr (is_associative_container<vertices_type>) {
      return vertices_.at(id);
    } else {
      // Sequential container: use at() for bounds checking
      return vertices_.at(static_cast<size_t>(id));
    }
  }

private: // Member Variables
  vertices_type    vertices_;
  partition_vector partition_; // partition_[n] holds the first vertex id for each partition n
                               // holds +1 extra terminating partition
  size_t edge_count_ = 0;      // total number of edges in the graph

private: // CPO properties
  friend constexpr vertices_type&       vertices(dynamic_graph_base& g) { return g.vertices_; }
  friend constexpr const vertices_type& vertices(const dynamic_graph_base& g) { return g.vertices_; }

  friend constexpr auto num_vertices(const dynamic_graph_base& g) { return g.vertices_.size(); }

  friend constexpr auto num_edges(const dynamic_graph_base& g) { return g.edge_count_; }
  friend constexpr bool has_edge(const dynamic_graph_base& g) { return g.edge_count_ > 0; }

  /**
   * @brief Find a vertex by id, returning a vertex descriptor view iterator for use with CPOs.
   *
   * This is the ADL customization point for the find_vertex CPO. It returns an iterator to a vertex
   * descriptor that can be compared with vertices(g).end() to check if the vertex was found.
   *
   * For sequential containers (vector/deque), returns iterator if valid, end() otherwise.
   * For associative containers (map/unordered_map), uses find().
   *
   * @param g The graph to search.
   * @param id The vertex id to find.
   * @return Vertex descriptor view iterator.
   * @note Complexity: O(1) for sequential containers, O(log n) for map, O(1) average for unordered_map.
   * @note This function never modifies the container (unlike operator[] on maps).
   */
  friend constexpr auto find_vertex(dynamic_graph_base& g, const vertex_id_type& id) noexcept {
    using container_iter = typename vertices_type::iterator;
    using view_type      = vertex_descriptor_view<container_iter>;
    using view_iterator  = typename view_type::iterator;
    using storage_type   = typename view_type::storage_type;

    if constexpr (is_associative_container<vertices_type>) {
      // For associative containers, storage is the iterator
      return view_iterator{g.vertices_.find(id)};
    } else {
      // Sequential container: storage is index (size_t)
      // Check bounds and return end() equivalent (vertices_.size()) or the index
      if constexpr (std::is_signed_v<vertex_id_type>) {
        if (id < 0)
          return view_iterator{static_cast<storage_type>(g.vertices_.size())};
      }
      if (static_cast<size_t>(id) >= g.vertices_.size()) {
        return view_iterator{static_cast<storage_type>(g.vertices_.size())};
      }
      return view_iterator{static_cast<storage_type>(id)};
    }
  }

  friend constexpr auto find_vertex(const dynamic_graph_base& g, const vertex_id_type& id) noexcept {
    using container_iter = typename vertices_type::const_iterator;
    using view_type      = vertex_descriptor_view<container_iter>;
    using view_iterator  = typename view_type::iterator;
    using storage_type   = typename view_type::storage_type;

    if constexpr (is_associative_container<vertices_type>) {
      // For associative containers, storage is the iterator
      return view_iterator{g.vertices_.find(id)};
    } else {
      // Sequential container: storage is index (size_t)
      if constexpr (std::is_signed_v<vertex_id_type>) {
        if (id < 0)
          return view_iterator{static_cast<storage_type>(g.vertices_.size())};
      }
      if (static_cast<size_t>(id) >= g.vertices_.size()) {
        return view_iterator{static_cast<storage_type>(g.vertices_.size())};
      }
      return view_iterator{static_cast<storage_type>(id)};
    }
  }

  /**
   * @brief Get the user-defined value associated with a vertex
   * 
   * Returns a reference to the user-defined value stored for the given vertex.
   * For compressed_graph, extracts the vertex ID from the descriptor and uses
   * it to directly access the vertex value storage.
   * 
   * @param g The graph (forwarding reference for const preservation)
   * @param u The vertex descriptor
   * @return Reference to the vertex value (const if g is const)
   * @note Complexity: O(1) - direct array access by vertex ID
   * @note This is the ADL customization point for the vertex_value(g, u) CPO
   * @note Only available when VV is not void
  */
  template <typename G, typename U>
  requires std::derived_from<std::remove_cvref_t<G>, dynamic_graph_base> && vertex_descriptor_type<U> &&
           (!std::is_void_v<VV>)
  [[nodiscard]] friend constexpr decltype(auto) vertex_value(G&& g, U&& u) noexcept {
    return std::forward<U>(u).inner_value(std::forward<G>(g).vertices_).value();
  }

  /**
   * @brief Get the user-defined value associated with an edge
   * 
   * Returns a reference to the user-defined value stored for the given edge.
   * Navigates through the edge descriptor to access the edge's value.
   * 
   * @param g The graph (forwarding reference for const preservation)
   * @param uv The edge descriptor
   * @return Reference to the edge value (const if g is const)
   * @note Complexity: O(1) - direct access through descriptor
   * @note This is the ADL customization point for the edge_value(g, uv) CPO
   * @note Only available when EV is not void
  */
  template <typename G, typename E>
  requires std::derived_from<std::remove_cvref_t<G>, dynamic_graph_base> && edge_descriptor_type<E> &&
           (!std::is_void_v<EV>)
  [[nodiscard]] friend constexpr decltype(auto) edge_value(G&& g, E&& uv) noexcept {
    // Direction-aware edge container access:
    // For in-edge descriptors, navigate .in_edges(); for out-edges, navigate .edges()
    auto&& edge_container = [&]() -> decltype(auto) {
      if constexpr (std::remove_cvref_t<E>::is_in_edge) {
        return (std::forward<E>(uv).source().inner_value(std::forward<G>(g).vertices_).in_edges());
      } else {
        return (std::forward<E>(uv).source().inner_value(std::forward<G>(g).vertices_).edges());
      }
    }();

    // Get the edge object by chaining calls to avoid dangling reference warnings
    // For map-based containers, edge_obj is pair<const VId, edge_type>, so access .second
    if constexpr (requires {
                    std::forward<E>(uv).inner_value(edge_container).second.value();
                  }) {
      return std::forward<E>(uv).inner_value(edge_container).second.value();
    } else {
      return std::forward<E>(uv).inner_value(edge_container).value();
    }
  }

  // =========================================================================
  // in_edges(g, u) — incoming edge CPO
  // =========================================================================
  // Bidirectional in_edges are stored directly on each vertex via dynamic_vertex_bidir_base.
  // The in_edges CPO discovers them through the _vertex_member dispatch tier which calls
  // u.inner_value(g).in_edges() and wraps automatically in edge_descriptor_view.
  // No ADL friend function is needed here.

  // friend constexpr vertex_id_type vertex_id(const dynamic_graph_base& g, typename vertices_type::const_iterator ui) {
  //   return static_cast<vertex_id_type>(ui - g.vertices_.begin());
  // }

  // friend constexpr edges_type& edges(graph_type& g, const vertex_id_type uid) { //
  //   return g.vertices_[uid].edges();
  // }
  // friend constexpr const edges_type& edges(const graph_type& g, const vertex_id_type uid) {
  //   return g.vertices_[uid].edges();
  // }

  // friend constexpr auto num_partitions(const dynamic_graph_base& g) {
  //   return static_cast<partition_id_type>(g.partition_.size());
  // }

  // friend constexpr auto partition_id(const dynamic_graph_base& g, vertex_id_type uid) {
  //   auto it = std::upper_bound(g.partition_.begin(), g.partition_.end(), uid);
  //   return static_cast<partition_id_type>(it - g.partition_.begin() - 1);
  // }

  // friend constexpr auto num_vertices(const dynamic_graph_base& g, partition_id_type pid) {
  //   if (static_cast<size_t>(pid) >= g.partition_.size() - 1)
  //     throw std::out_of_range("partition id out of range in num_vertices");
  //   return g.partition_[pid + 1] - g.partition_[pid];
  // }

  // friend constexpr auto vertices(const dynamic_graph_base& g, partition_id_type pid) {
  //   if (static_cast<size_t>(pid) >= g.partition_.size() - 1)
  //     throw std::out_of_range("partition id out of range in partition vertices view");
  //   return subrange(g.vertices_.begin() + g.partition_[pid], g.vertices_.begin() + g.partition_[pid + 1]);
  // }
};

/**
 * @ingroup graph_containers
 * @brief dynamic_graph defines a graph with a variety characteristics including edge, vertex 
 * and graph value types, whether edges are sourced or not, the vertex id type, and selection 
 * of the containers used for vertices and edges.
 * 
 * dynamic_graph provides the interface that includes or excludes (GV=void) a graph value.
 * dynamic_graph_base has the core implementation for the graph.
 * 
 * No additional space is used if the edge value type (EV), vertex value type (VV) or graph
 * value type (GV) is void.
 * 
 * Only random access vertex container types (e.g. @c vector<V> & @c deque<V>) are supported at the 
 * moment for vertex containers in @c Traits. It is the goal to support bidirectional container types 
 * (e.g. map<VId,V> & unordered_map<VId,V>) in the future.
 * 
 * Forward list containers are supported for edge containers. @c vector<E>, @c deque<E>, @c list<E>
 * and @c forward_list<E> have been tested. Additional work is required to support set<E> and map<VId,E>.
 * 
 * Only integral VId types are supported at the moment. It is the goal to support any type that can be
 * used as a key to find a vertex in the vertices container.
 * 
 * The partition function is intended to determine the partition id for a vertex id on constructors.
 * It has been added to assure the same interface as the compressed_graph, but is not implemented
 * at this time.
 * 
 * @tparam EV      @showinitializer =void The edge value type. If "void" is used no user value is stored on the edge
 *                 and calls to @c edge_value(g,uv) will generate a compile error.
 * @tparam VV      @showinitializer =void The vertex value type. If "void" is used no user value is stored on the vertex
 *                 and calls to @c vertex_value(g,u) will generate a compile error. VV must be default-constructable.
 * @tparam GV      @showinitializer =void The graph value type. If "void" is used no user value is stored on the graph
 *                 and calls to @c graph_value(g) will generate a compile error.
 * @tparam Sourced @showinitializer =false Is a source vertex id stored on the edge? If false, calls to @c source_id(g,uv)
 *                 and @c source(g,uv) will generate a compile error.
 * @tparam VId     @showinitializer =uint32_t Vertex id type
 * @tparam Traits  @showinitializer =vofl_graph_traits<EV,VV,GV,Sourced,VId> Defines the types for vertex and edge containers.
 * @tparam Bidirectional @showinitializer =false If true, maintains reverse adjacency lists for in_edges(g,u).
*/
template <class EV, class VV, class GV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph : public dynamic_graph_base<EV, VV, GV, VId, Sourced, Bidirectional, Traits> {

public: // Types & Constants
  using base_type      = dynamic_graph_base<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using graph_type     = dynamic_graph<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;
  using graph_traits   = Traits;
  using vertex_id_type = VId;
  using value_type     = GV;
  using allocator_type = typename Traits::vertices_type::allocator_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, GV, VId, Sourced, Bidirectional, Traits>;

  constexpr inline const static bool sourced = Sourced;

private: // Members
  [[no_unique_address]] GV graph_value_{};

public: // Construction/Destruction/Assignment
  dynamic_graph()                     = default;
  dynamic_graph(const dynamic_graph&) = default;
  dynamic_graph(dynamic_graph&&)      = default;
  ~dynamic_graph()                    = default;

  dynamic_graph& operator=(const dynamic_graph&) = default;
  dynamic_graph& operator=(dynamic_graph&&)      = default;

  /**
   * @brief Construct a dynamic_graph with a graph value.
   * 
   * @param gv Graph value.
   * @param alloc Used to allocate vertices and edges.
  */
  dynamic_graph(const GV& gv, allocator_type alloc = allocator_type()) : base_type(alloc), graph_value_(gv) {}

  /**
   * @brief Construct a dynamic_graph with a graph value (move version).
   * 
   * @param gv Graph value.
   * @param alloc Used to allocate vertices and edges.
  */
  dynamic_graph(GV&& gv, allocator_type alloc = allocator_type()) : base_type(alloc), graph_value_(std::move(gv)) {}

  /**
   * @brief Construct a dynamic_graph with an empty collection of vertices.
   * 
   * @param alloc Used to allocate vertices and edges.
  */
  dynamic_graph(allocator_type alloc) : base_type(alloc) {}

public: // Graph value accessors
  /**
   * @brief Returns the user-defined value for the graph.
   * 
   * @return The graph value.
  */
  constexpr GV& graph_value() noexcept { return graph_value_; }

  /**
   * @brief Returns the user-defined value for the graph.
   * 
   * @return The graph value.
  */
  constexpr const GV& graph_value() const noexcept { return graph_value_; }

  /**
   * @brief Constructs the graph from a range of edge data and range of vertex data.
   * 
   * The vertices container is pre-extended to accommodate the largest vertex id, if applicable.
   * 
   * @tparam ERng    The range type of edge data to load into the graph.
   * @tparam VRng    The range type of vertex data to load into the graph.
   * @tparam EProj   The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   * @tparam VProj   The projection function type to convert the @c VRng value type to a @c copyable_vertex_t<VId,VV>.
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng                The edge data range used to create new edges.
   * @param vrng                The vertex data range used to define the number of vertices and define vertex values.
   * @param eproj               The edge projection function.
   * @param vproj               The vertex projection function.
   * @param partition_start_ids Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
   * @param alloc               The allocator used for vertices and edges containers.
  */
  template <class ERng, class VRng, forward_range PartRng, class EProj = identity, class VProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  dynamic_graph(const ERng&    erng,
                const VRng&    vrng,
                EProj          eproj               = {},
                VProj          vproj               = {},
                const PartRng& partition_start_ids = std::vector<VId>(),
                allocator_type alloc               = allocator_type())
        : base_type(erng, vrng, eproj, vproj, partition_start_ids, alloc) {}

  /**
   * @brief Construct the graph given the maximum vertex id and edge data range.
   * 
   * @tparam ERng    The range type of edge data to load into the graph.
   * @tparam EProj   The projection function type.
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param max_vertex_id       The maximum vertex id used by edges.
   * @param erng                The edge data range used to create new edges.
   * @param eproj               The edge projection function.
   * @param partition_start_ids Range of starting vertex ids for each partition.
   * @param alloc               The allocator used for the vertices and edges containers.
  */
  template <class ERng, forward_range PartRng, class EProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  dynamic_graph(vertex_id_type max_vertex_id,
                ERng&          erng,
                EProj          eproj               = {},
                const PartRng& partition_start_ids = std::vector<VId>(),
                allocator_type alloc               = allocator_type())
        : base_type((max_vertex_id == std::numeric_limits<vertex_id_type>::max()
                           ? throw std::invalid_argument("max_vertex_id would overflow vertex count")
                           : static_cast<vertex_id_type>(max_vertex_id + 1)),
                    erng,
                    eproj,
                    partition_start_ids,
                    alloc) {}

  /**
   * @brief Construct the graph using an initializer list of copyable edges.
   * 
   * @param il    The initializer list of copyable edge values.
   * @param alloc The allocator to use for the vertices and edges containers.
  */
  dynamic_graph(const std::initializer_list<copyable_edge_t<VId, EV>>& il,
                edge_allocator_type                                    alloc = edge_allocator_type())
        : base_type(il, alloc) {}

  /**
   * @brief Construct the graph with a graph value and an initializer list of copyable edges.
   * 
   * @param gv    The graph value.
   * @param il    The initializer list of copyable edge values.
   * @param alloc The allocator to use for the vertices and edges containers.
  */
  dynamic_graph(const GV&                                              gv,
                const std::initializer_list<copyable_edge_t<VId, EV>>& il,
                edge_allocator_type                                    alloc = edge_allocator_type())
        : base_type(il, alloc), graph_value_(gv) {}

  /**
   * @brief Construct the graph with a graph value (move) and an initializer list of copyable edges.
   * 
   * @param gv    The graph value (moved).
   * @param il    The initializer list of copyable edge values.
   * @param alloc The allocator to use for the vertices and edges containers.
  */
  dynamic_graph(GV&&                                                   gv,
                const std::initializer_list<copyable_edge_t<VId, EV>>& il,
                edge_allocator_type                                    alloc = edge_allocator_type())
        : base_type(il, alloc), graph_value_(std::move(gv)) {}
};

/**
 * @ingroup graph_containers
 * @brief Specialization of dynamic_graph for GV=void (no graph value).
 * 
 * This specialization excludes the graph_value_ member and graph_value() accessors.
 * Any attempt to use graph_value(g) will generate a compile error.
 * 
 * @tparam EV      The edge value type. If "void" is used no user value is stored on the edge.
 * @tparam VV      The vertex value type. If "void" is used no user value is stored on the vertex.
 * @tparam GV      [void] No user value is stored on the graph and calling graph_value(g) 
 *                 will generate a compile error.
 * @tparam Sourced Is a source vertex id stored on the edge?
 * @tparam VId     Vertex id type
 * @tparam Traits  Defines the types for vertex and edge containers.
 * @tparam Bidirectional If true, maintains reverse adjacency lists for in_edges(g,u).
*/
template <class EV, class VV, class VId, bool Sourced, bool Bidirectional, class Traits>
class dynamic_graph<EV, VV, void, VId, Sourced, Bidirectional, Traits>
      : public dynamic_graph_base<EV, VV, void, VId, Sourced, Bidirectional, Traits> {

public: // Types & Constants
  using base_type      = dynamic_graph_base<EV, VV, void, VId, Sourced, Bidirectional, Traits>;
  using graph_type     = dynamic_graph<EV, VV, void, VId, Sourced, Bidirectional, Traits>;
  using graph_traits   = Traits;
  using vertex_id_type = VId;
  using value_type     = void;
  using allocator_type = typename Traits::vertices_type::allocator_type;

  using edges_type          = typename Traits::edges_type;
  using edge_allocator_type = typename edges_type::allocator_type;
  using edge_type           = dynamic_edge<EV, VV, void, VId, Sourced, Bidirectional, Traits>;

  constexpr inline const static bool sourced = Sourced;

public: // Construction/Destruction/Assignment
  dynamic_graph()                     = default;
  dynamic_graph(const dynamic_graph&) = default;
  dynamic_graph(dynamic_graph&&)      = default;
  ~dynamic_graph()                    = default;

  dynamic_graph& operator=(const dynamic_graph&) = default;
  dynamic_graph& operator=(dynamic_graph&&)      = default;

  /**
   * @brief Construct a dynamic_graph with an empty collection of vertices.
   * 
   * @param alloc Used to allocate vertices and edges.
  */
  dynamic_graph(allocator_type alloc) : base_type(alloc) {}

  /**
   * @brief Constructs the graph from a range of edge data and range of vertex data.
   * 
   * The vertices container is pre-extended to accommodate the largest vertex id, if applicable.
   * 
   * @tparam ERng    The range type of edge data to load into the graph.
   * @tparam VRng    The range type of vertex data to load into the graph.
   * @tparam EProj   The projection function type to convert the @c ERng value type to a @c copyable_edge_t<VId,EV>.
   * @tparam VProj   The projection function type to convert the @c VRng value type to a @c copyable_vertex_t<VId,VV>.
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param erng                The edge data range used to create new edges.
   * @param vrng                The vertex data range used to define the number of vertices and define vertex values.
   * @param eproj               The edge projection function.
   * @param vproj               The vertex projection function.
   * @param partition_start_ids Range of starting vertex ids for each partition. If empty, all vertices are in partition 0.
   * @param alloc               The allocator used for vertices and edges containers.
  */
  template <class ERng, class VRng, forward_range PartRng, class EProj = identity, class VProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  dynamic_graph(const ERng&    erng,
                const VRng&    vrng,
                EProj          eproj               = {},
                VProj          vproj               = {},
                const PartRng& partition_start_ids = std::vector<VId>(),
                allocator_type alloc               = allocator_type())
        : base_type(erng, vrng, eproj, vproj, partition_start_ids, alloc) {}

  /**
   * @brief Construct the graph given the maximum vertex id and edge data range.
   * 
   * @tparam ERng    The range type of edge data to load into the graph.
   * @tparam EProj   The projection function type.
   * @tparam PartRng Range of starting vertex Ids for each partition
   * 
   * @param max_vertex_id       The maximum vertex id used by edges.
   * @param erng                The edge data range used to create new edges.
   * @param eproj               The edge projection function.
   * @param partition_start_ids Range of starting vertex ids for each partition.
   * @param alloc               The allocator used for the vertices and edges containers.
  */
  template <class ERng, forward_range PartRng, class EProj = identity>
  requires convertible_to<range_value_t<PartRng>, VId>
  dynamic_graph(vertex_id_type max_vertex_id,
                ERng&          erng,
                EProj          eproj               = {},
                const PartRng& partition_start_ids = std::vector<VId>(),
                allocator_type alloc               = allocator_type())
        : base_type((max_vertex_id == std::numeric_limits<vertex_id_type>::max()
                           ? throw std::invalid_argument("max_vertex_id would overflow vertex count")
                           : static_cast<vertex_id_type>(max_vertex_id + 1)),
                    erng,
                    eproj,
                    partition_start_ids,
                    alloc) {}

  /**
   * @brief Construct the graph using an initializer list of copyable edges.
   * 
   * @param il    The initializer list of copyable edge values.
   * @param alloc The allocator to use for the vertices and edges containers.
  */
  dynamic_graph(const std::initializer_list<copyable_edge_t<VId, EV>>& il,
                edge_allocator_type                                    alloc = edge_allocator_type())
        : base_type(il, alloc) {}

  // Note: No graph_value() accessors - GV is void
};

} // namespace graph::container

// Hash specializations for std::unordered containers
namespace std {

/**
 * @brief Hash specialization for dynamic_edge with Sourced = true
 * 
 * Hashes based on source_id and target_id only (edge value excluded, works for any EV)
 */
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
struct hash<graph::container::dynamic_edge<EV, VV, GV, VId, true, Bidirectional, Traits>> {
  [[nodiscard]] size_t
  operator()(const graph::container::dynamic_edge<EV, VV, GV, VId, true, Bidirectional, Traits>& edge) const noexcept {
    size_t h1 = std::hash<VId>{}(edge.source_id());
    size_t h2 = std::hash<VId>{}(edge.target_id());
    return h1 ^ (h2 << 1);
  }
};

/**
 * @brief Hash specialization for dynamic_edge with Sourced = false
 * 
 * Hashes based on target_id only (no source_id available, works for any EV)
 */
template <class EV, class VV, class GV, class VId, bool Bidirectional, class Traits>
struct hash<graph::container::dynamic_edge<EV, VV, GV, VId, false, Bidirectional, Traits>> {
  [[nodiscard]] size_t
  operator()(const graph::container::dynamic_edge<EV, VV, GV, VId, false, Bidirectional, Traits>& edge) const noexcept {
    return std::hash<VId>{}(edge.target_id());
  }
};

} // namespace std
