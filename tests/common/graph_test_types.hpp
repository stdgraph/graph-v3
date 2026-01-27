/**
 * @file graph_test_types.hpp
 * @brief Template infrastructure for consolidated CPO testing across container types
 * 
 * This header provides a tag-based type generation system that allows Catch2's
 * TEMPLATE_TEST_CASE to test multiple container types (vov, vod, dov, dod) with
 * multiple value configurations (void, int, string, sourced) in a single test file.
 * 
 * Usage:
 *   TEMPLATE_TEST_CASE("test name", "[tags]", vov_tag, vod_tag, dov_tag, dod_tag) {
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
    using void_type = graph::container::dynamic_graph<
        void, void, void, VId, false,
        typename Tag::template traits<void, void, void, VId, false>>;
    
    using int_ev = graph::container::dynamic_graph<
        int, void, void, VId, false,
        typename Tag::template traits<int, void, void, VId, false>>;
    
    using int_vv = graph::container::dynamic_graph<
        void, int, void, VId, false,
        typename Tag::template traits<void, int, void, VId, false>>;
    
    using all_int = graph::container::dynamic_graph<
        int, int, int, VId, false,
        typename Tag::template traits<int, int, int, VId, false>>;
    
    using string_type = graph::container::dynamic_graph<
        std::string, std::string, std::string, VId, false,
        typename Tag::template traits<std::string, std::string, std::string, VId, false>>;
    
    // Sourced configurations (for source_id/source CPO tests)
    using sourced_void = graph::container::dynamic_graph<
        void, void, void, VId, true,
        typename Tag::template traits<void, void, void, VId, true>>;
    
    using sourced_int = graph::container::dynamic_graph<
        int, void, void, VId, true,
        typename Tag::template traits<int, void, void, VId, true>>;
    
    using sourced_all = graph::container::dynamic_graph<
        int, int, int, VId, true,
        typename Tag::template traits<int, int, int, VId, true>>;
    
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
