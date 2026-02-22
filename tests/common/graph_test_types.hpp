/**
 * @file graph_test_types.hpp
 * @brief Template infrastructure for consolidated CPO testing across container types
 * 
 * This header provides a tag-based type generation system that allows Catch2's
 * TEMPLATE_TEST_CASE to test multiple container types (vov, vod, dov, dod, vol, dol) with
 * multiple value configurations (void, int, string, sourced) in a single test file.
 * 
 * Usage:
 *   TEMPLATE_TEST_CASE("test name", "[tags]", vov_tag, vod_tag, dov_tag, dod_tag, vol_tag, dol_tag) {
 *       using Types = graph_test_types<TestType>;
 *       using Graph_void = typename Types::void_type;
 *       using Graph_int_ev = typename Types::int_ev;
 *       // ... run tests with these types
 *   }
 * 
 * Each tag type provides:
 * - A template alias for the graph_traits
 * - A human-readable name for test output
 * - All 8 standard type configurations via graph_test_types
 */

#ifndef GRAPH_TEST_TYPES_HPP
#define GRAPH_TEST_TYPES_HPP

#include <graph/container/dynamic_graph.hpp>
#include <graph/container/traits/vov_graph_traits.hpp>
#include <graph/container/traits/vod_graph_traits.hpp>
#include <graph/container/traits/dov_graph_traits.hpp>
#include <graph/container/traits/dod_graph_traits.hpp>
#include <graph/container/traits/vol_graph_traits.hpp>
#include <graph/container/traits/dol_graph_traits.hpp>
#include <graph/container/traits/vofl_graph_traits.hpp>
#include <graph/container/traits/dofl_graph_traits.hpp>
#include <graph/container/traits/mofl_graph_traits.hpp>
#include <graph/container/traits/vos_graph_traits.hpp>
#include <graph/container/traits/dos_graph_traits.hpp>
#include <graph/container/traits/mos_graph_traits.hpp>
#include <graph/container/traits/uos_graph_traits.hpp>
#include <graph/container/traits/vous_graph_traits.hpp>
#include <graph/container/traits/dous_graph_traits.hpp>
#include <graph/container/traits/mous_graph_traits.hpp>
#include <graph/container/traits/uous_graph_traits.hpp>
// Map-based vertex containers (mo* = map, uo* = unordered_map)
#include <graph/container/traits/mol_graph_traits.hpp>
#include <graph/container/traits/mov_graph_traits.hpp>
#include <graph/container/traits/mod_graph_traits.hpp>
#include <graph/container/traits/uol_graph_traits.hpp>
#include <graph/container/traits/uov_graph_traits.hpp>
#include <graph/container/traits/uod_graph_traits.hpp>
#include <graph/container/traits/uofl_graph_traits.hpp>
// Edge multiset containers (sorted edges with duplicates allowed)
#include <graph/container/traits/vom_graph_traits.hpp>
#include <graph/container/traits/mom_graph_traits.hpp>
// Edge unordered_map containers (hash-based, deduplicated by target_id)
#include <graph/container/traits/voum_graph_traits.hpp>
#include <string>
namespace graph::test {

// =============================================================================
// Tag types for each random-access container type
// =============================================================================

/**
 * @brief Tag for vector<vertex> + vector<edge> container type
 */
struct vov_tag {
  static constexpr const char* name = "vov";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::vov_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for vector<vertex> + deque<edge> container type
 */
struct vod_tag {
  static constexpr const char* name = "vod";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::vod_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for deque<vertex> + vector<edge> container type
 */
struct dov_tag {
  static constexpr const char* name = "dov";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::dov_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for deque<vertex> + deque<edge> container type
 */
struct dod_tag {
  static constexpr const char* name = "dod";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::dod_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for vector<vertex> + list<edge> container type
 */
struct vol_tag {
  static constexpr const char* name = "vol";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::vol_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for deque<vertex> + list<edge> container type
 */
struct dol_tag {
  static constexpr const char* name = "dol";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::dol_graph_traits<EV, VV, GV, VId, Sourced>;
};

// =============================================================================
// Tag types for forward_list edge containers (reverse insertion order)
// =============================================================================

/**
 * @brief Tag for vector<vertex> + forward_list<edge> container type
 * @note Edges appear in reverse insertion order (push_front semantics)
 */
struct vofl_tag {
  static constexpr const char* name = "vofl";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::vofl_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for deque<vertex> + forward_list<edge> container type
 * @note Edges appear in reverse insertion order (push_front semantics)
 */
struct dofl_tag {
  static constexpr const char* name = "dofl";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::dofl_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for map<vertex> + forward_list<edge> container type
 * @note Edges appear in reverse insertion order (push_front semantics)
 */
struct mofl_tag {
  static constexpr const char* name = "mofl";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::mofl_graph_traits<EV, VV, GV, VId, Sourced>;
};

// =============================================================================
// Tag types for sorted edge containers (edges ordered by target_id)
// =============================================================================

/**
 * @brief Tag for vector<vertex> + set<edge> container type
 * @note Edges are ordered by target_id (sorted set semantics)
 */
struct vos_tag {
  static constexpr const char* name = "vos";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::vos_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for deque<vertex> + set<edge> container type
 * @note Edges are ordered by target_id (sorted set semantics)
 */
struct dos_tag {
  static constexpr const char* name = "dos";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::dos_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for map<vertex> + set<edge> container type
 * @note Edges are ordered by target_id (sorted set semantics)
 */
struct mos_tag {
  static constexpr const char* name = "mos";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::mos_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for unordered_map<vertex> + set<edge> container type (undirected)
 * @note Edges are ordered by target_id (sorted set semantics)
 */
struct uos_tag {
  static constexpr const char* name = "uos";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::uos_graph_traits<EV, VV, GV, VId, Sourced>;
};

// =============================================================================
// Tag types for unordered edge containers (edges in unspecified order)
// =============================================================================

/**
 * @brief Tag for vector<vertex> + unordered_set<edge> container type
 * @note Edge order is unspecified (hash-based container)
 */
struct vous_tag {
  static constexpr const char* name = "vous";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::vous_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for deque<vertex> + unordered_set<edge> container type
 * @note Edge order is unspecified (hash-based container)
 */
struct dous_tag {
  static constexpr const char* name = "dous";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::dous_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for map<vertex> + unordered_set<edge> container type
 * @note Edge order is unspecified (hash-based container)
 */
struct mous_tag {
  static constexpr const char* name = "mous";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::mous_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for unordered_map<vertex> + unordered_set<edge> container type
 * @note Edge order is unspecified (hash-based container)
 */
struct uous_tag {
  static constexpr const char* name = "uous";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::uous_graph_traits<EV, VV, GV, VId, Sourced>;
};

// =============================================================================
// Tag types for map-based vertex containers (sparse vertex IDs)
// Vertices are created on-demand from edge endpoints, not via resize_vertices()
// =============================================================================

/**
 * @brief Tag for map<vertex> + list<edge> container type
 * @note Vertices are sparse (on-demand creation), iterated in sorted order
 */
struct mol_tag {
  static constexpr const char* name = "mol";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::mol_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for map<vertex> + vector<edge> container type
 * @note Vertices are sparse (on-demand creation), iterated in sorted order
 */
struct mov_tag {
  static constexpr const char* name = "mov";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::mov_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for map<vertex> + deque<edge> container type
 * @note Vertices are sparse (on-demand creation), iterated in sorted order
 */
struct mod_tag {
  static constexpr const char* name = "mod";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::mod_graph_traits<EV, VV, GV, VId, Sourced>;
};

// =============================================================================
// Tag types for unordered_map-based vertex containers (sparse vertex IDs)
// Vertices are created on-demand from edge endpoints, not via resize_vertices()
// =============================================================================

/**
 * @brief Tag for unordered_map<vertex> + list<edge> container type
 * @note Vertices are sparse (on-demand creation), iteration order unspecified
 */
struct uol_tag {
  static constexpr const char* name = "uol";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::uol_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for unordered_map<vertex> + vector<edge> container type
 * @note Vertices are sparse (on-demand creation), iteration order unspecified
 */
struct uov_tag {
  static constexpr const char* name = "uov";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::uov_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for unordered_map<vertex> + deque<edge> container type
 * @note Vertices are sparse (on-demand creation), iteration order unspecified
 */
struct uod_tag {
  static constexpr const char* name = "uod";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::uod_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for unordered_map<vertex> + forward_list<edge> container type
 * @note Vertices are sparse (on-demand creation), iteration order unspecified
 * @note Edges appear in reverse insertion order (push_front semantics)
 */
struct uofl_tag {
  static constexpr const char* name = "uofl";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::uofl_graph_traits<EV, VV, GV, VId, Sourced>;
};

// =============================================================================
// Tag types for edge multiset containers (sorted edges with duplicates)
// Allows multiple edges between same vertex pair (parallel edges)
// =============================================================================

/**
 * @brief Tag for vector<vertex> + map<edge> container type
 * @note Edges are sorted by target_id (map key), deduplicated (only one edge per target)
 */
struct vom_tag {
  static constexpr const char* name = "vom";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::vom_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for map<vertex> + map<edge> container type
 * @note Vertices are sparse, edges are sorted by target_id (map key), deduplicated
 */
struct mom_tag {
  static constexpr const char* name = "mom";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::mom_graph_traits<EV, VV, GV, VId, Sourced>;
};

/**
 * @brief Tag for vector<vertex> + unordered_map<edge> container type
 * @note Edges are hash-based, deduplicated (only one edge per target), unordered
 */
struct voum_tag {
  static constexpr const char* name = "voum";

  template <typename EV, typename VV, typename GV, typename VId, bool Sourced>
  using traits = graph::container::voum_graph_traits<EV, VV, GV, VId, Sourced>;
};

// =============================================================================
// Type generator: produces all 8 type configurations from a tag
// =============================================================================

/**
 * @brief Generates all standard graph type configurations from a container tag
 * 
 * @tparam Tag One of vov_tag, vod_tag, dov_tag, dod_tag
 * 
 * Provides the 8 standard type aliases:
 * - void_type:     EV=void, VV=void, GV=void, Sourced=false
 * - int_ev:        EV=int,  VV=void, GV=void, Sourced=false
 * - int_vv:        EV=void, VV=int,  GV=void, Sourced=false
 * - all_int:       EV=int,  VV=int,  GV=int,  Sourced=false
 * - string_type:   EV/VV/GV=string,           Sourced=false
 * - sourced_void:  EV=void, VV=void, GV=void, Sourced=true
 * - sourced_int:   EV=int,  VV=void, GV=void, Sourced=true
 * - sourced_all:   EV=int,  VV=int,  GV=int,  Sourced=true
 */
template <typename Tag>
struct graph_test_types {
  using VId = uint32_t;

  // Non-sourced configurations
  using void_type = graph::container::
        dynamic_graph<void, void, void, VId, false, false, typename Tag::template traits<void, void, void, VId, false>>;

  using int_ev = graph::container::
        dynamic_graph<int, void, void, VId, false, false, typename Tag::template traits<int, void, void, VId, false>>;

  using int_vv = graph::container::
        dynamic_graph<void, int, void, VId, false, false, typename Tag::template traits<void, int, void, VId, false>>;

  using all_int = graph::container::
        dynamic_graph<int, int, int, VId, false, false, typename Tag::template traits<int, int, int, VId, false>>;

  using string_type = graph::container::dynamic_graph<
        std::string,
        std::string,
        std::string,
        VId,
        false, false,
        typename Tag::template traits<std::string, std::string, std::string, VId, false>>;

  // Sourced configurations (for source_id/source CPO tests)
  using sourced_void = graph::container::
        dynamic_graph<void, void, void, VId, true, false, typename Tag::template traits<void, void, void, VId, true>>;

  using sourced_int = graph::container::
        dynamic_graph<int, void, void, VId, true, false, typename Tag::template traits<int, void, void, VId, true>>;

  using sourced_all = graph::container::
        dynamic_graph<int, int, int, VId, true, false, typename Tag::template traits<int, int, int, VId, true>>;

  // Container name for test output
  static constexpr const char* name = Tag::name;
};

// =============================================================================
// Helper to get container name at runtime (for DYNAMIC_SECTION)
// =============================================================================

template <typename Tag>
constexpr const char* container_name() {
  return Tag::name;
}

// =============================================================================
// Convenience aliases for Catch2 TEMPLATE_TEST_CASE
// =============================================================================

// Random-access containers (support num_edges(g,u), sized_range edges)
using random_access_container_tags = std::tuple<vov_tag, vod_tag, dov_tag, dod_tag>;

} // namespace graph::test

#endif // GRAPH_TEST_TYPES_HPP
