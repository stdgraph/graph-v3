/**
 * @file graph_cpo.hpp
 * @brief Graph customization point objects (CPOs)
 * 
 * This file contains the implementation of all graph CPOs following the MSVC
 * standard library style as documented in docs/cpo.md.
 * 
 * All CPOs defined here allow customization through:
 * 1. Member functions (highest priority)
 * 2. ADL-findable free functions (medium priority)
 * 3. Default implementations (lowest priority)
 */

#pragma once

#include <concepts>
#include <ranges>
#include <iterator>
#include "graph/adj_list/vertex_descriptor_view.hpp"
#include "graph/adj_list/edge_descriptor_view.hpp"
#include "graph/adj_list/descriptor.hpp"
#include "graph/adj_list/descriptor_traits.hpp"
#include "graph/edge_list/edge_list_traits.hpp"
#include "graph/detail/cpo_common.hpp"
#include "graph/detail/edge_cpo.hpp"

namespace graph::adj_list {

namespace _cpo_impls {
    
    // Import shared _Choice_t from common header
    using detail::_cpo_impls::_Choice_t;

    // =========================================================================
    // vertices(g) CPO
    // =========================================================================
    
    namespace _vertices {
        enum class _St { _none, _member, _adl, _inner_value_pattern };
        
        // Check for g.vertices() member function
        template<typename G>
        concept _has_member = requires(G& g) {
            { g.vertices() } -> std::ranges::forward_range;
        };
        
        // Check for ADL vertices(g)
        template<typename G>
        concept _has_adl = requires(G& g) {
            { vertices(g) } -> std::ranges::forward_range;
        };
        
        // Check if g is a range with iterators that follow inner value pattern
        template<typename G>
        concept _has_inner_value_pattern = std::ranges::forward_range<G> && 
            requires { typename std::ranges::iterator_t<G>; } &&
            has_inner_value_pattern<std::ranges::iterator_t<G>>;
        
        template<typename G>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G>) {
                return {_St::_member, noexcept(std::declval<G&>().vertices())};
            } else if constexpr (_has_adl<G>) {
                return {_St::_adl, noexcept(vertices(std::declval<G&>()))};
            } else if constexpr (_has_inner_value_pattern<G>) {
                return {_St::_inner_value_pattern, true};
            } else {
                return {_St::_none, false};
            }
        }
        
        // Helper to wrap result in vertex_descriptor_view if not already
        template<typename Result>
        [[nodiscard]] constexpr auto _wrap_if_needed(Result&& result) noexcept {
            using ResultType = std::remove_cvref_t<Result>;
            if constexpr (is_vertex_descriptor_view_v<ResultType>) {
                // Already a vertex_descriptor_view, return as-is
                return std::forward<Result>(result);
            } else {
                // Not a vertex_descriptor_view, wrap it
                return vertex_descriptor_view(std::forward<Result>(result));
            }
        }
        
        class _fn {
        private:
            template<typename G>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>>();
            
        public:
            /**
             * @brief Get range of vertices in graph
             * 
             * IMPORTANT: This CPO MUST always return a vertex_descriptor_view.
             * 
             * Resolution order:
             * 1. If g.vertices() exists -> use it (wrap in descriptor view if needed)
             * 2. If ADL vertices(g) exists -> use it (wrap in descriptor view if needed)
             * 3. If g follows inner value patterns -> return vertex_descriptor_view(g)
             * 
             * If custom g.vertices() or ADL vertices(g) already returns a 
             * vertex_descriptor_view, it's used as-is. Otherwise, the result is 
             * automatically wrapped in vertex_descriptor_view.
             * 
             * @param g Graph container
             * @return vertex_descriptor_view wrapping the vertices
             */
            template<typename G>
            [[nodiscard]] constexpr auto operator()(G&& g) const
                noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                if constexpr (_Choice<_G>._Strategy == _St::_member) {
                    return _wrap_if_needed(g.vertices());
                } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                    return _wrap_if_needed(vertices(g));
                } else if constexpr (_Choice<_G>._Strategy == _St::_inner_value_pattern) {
                    return vertex_descriptor_view(g);
                }
            }
        };
        
        // =====================================================================
        // vertices(g, pid) - partition-specific vertex range
        // =====================================================================
        
        enum class _St_pid { _none, _member, _adl, _default };
        
        // Check for g.vertices(pid) member function
        template<typename G, typename PId>
        concept _has_member_pid = 
            requires(G& g, const PId& pid) {
                { g.vertices(pid) } -> std::ranges::forward_range;
            };
        
        // Check for ADL vertices(g, pid)
        template<typename G, typename PId>
        concept _has_adl_pid = 
            requires(G& g, const PId& pid) {
                { vertices(g, pid) } -> std::ranges::forward_range;
            };
        
        // Check if we can use default implementation (always available)
        template<typename G, typename PId>
        concept _has_default_pid = std::integral<PId>;
        
        template<typename G, typename PId>
        [[nodiscard]] consteval _Choice_t<_St_pid> _Choose_pid() noexcept {
            if constexpr (_has_member_pid<G, PId>) {
                return {_St_pid::_member, 
                        noexcept(std::declval<G&>().vertices(std::declval<const PId&>()))};
            } else if constexpr (_has_adl_pid<G, PId>) {
                return {_St_pid::_adl, 
                        noexcept(vertices(std::declval<G&>(), std::declval<const PId&>()))};
            } else if constexpr (_has_default_pid<G, PId>) {
                return {_St_pid::_default, true}; // Default is noexcept
            } else {
                return {_St_pid::_none, false};
            }
        }
        
        // Combined CPO function with both overloads
        class _fn_combined {
        private:
            template<typename G>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>>();
            
            template<typename G, typename PId>
            static constexpr _Choice_t<_St_pid> _Choice_pid = _Choose_pid<std::remove_cvref_t<G>, std::remove_cvref_t<PId>>();
            
        public:
            /**
             * @brief Get range of vertices in graph
             * 
             * IMPORTANT: This CPO MUST always return a vertex_descriptor_view.
             * 
             * Resolution order:
             * 1. If g.vertices() exists -> use it (wrap in descriptor view if needed)
             * 2. If ADL vertices(g) exists -> use it (wrap in descriptor view if needed)
             * 3. If g follows inner value patterns -> return vertex_descriptor_view(g)
             * 
             * If custom g.vertices() or ADL vertices(g) already returns a 
             * vertex_descriptor_view, it's used as-is. Otherwise, the result is 
             * automatically wrapped in vertex_descriptor_view.
             * 
             * @param g Graph container
             * @return vertex_descriptor_view wrapping the vertices
             */
            template<typename G>
            [[nodiscard]] constexpr auto operator()(G&& g) const
                noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                if constexpr (_Choice<_G>._Strategy == _St::_member) {
                    return _wrap_if_needed(g.vertices());
                } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                    return _wrap_if_needed(vertices(g));
                } else if constexpr (_Choice<_G>._Strategy == _St::_inner_value_pattern) {
                    return vertex_descriptor_view(g);
                }
            }
            
            /**
             * @brief Get range of vertices in a specific partition
             * 
             * IMPORTANT: This CPO MUST always return a vertex_descriptor_view.
             * 
             * Resolution order:
             * 1. If g.vertices(pid) exists -> use it (wrap in descriptor view if needed)
             * 2. If ADL vertices(g, pid) exists -> use it (wrap in descriptor view if needed)
             * 3. Default: returns vertices(g) if pid==0, empty vertex_descriptor_view otherwise
             * 
             * If custom g.vertices(pid) or ADL vertices(g, pid) already returns a 
             * vertex_descriptor_view, it's used as-is. Otherwise, the result is 
             * automatically wrapped in vertex_descriptor_view.
             * 
             * The default implementation assumes single partition (partition 0).
             * For multi-partition graphs, provide custom member function or ADL.
             * 
             * @tparam G Graph type
             * @tparam PId Partition ID type (integral)
             * @param g Graph container
             * @param pid Partition ID
             * @return vertex_descriptor_view wrapping the vertices in the partition
             */
            template<typename G, std::integral PId>
            [[nodiscard]] constexpr auto operator()(G&& g, const PId& pid) const
                noexcept(_Choice_pid<std::remove_cvref_t<G>, std::remove_cvref_t<PId>>._No_throw)
                requires (_Choice_pid<std::remove_cvref_t<G>, std::remove_cvref_t<PId>>._Strategy != _St_pid::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _PId = std::remove_cvref_t<PId>;
                
                if constexpr (_Choice_pid<_G, _PId>._Strategy == _St_pid::_member) {
                    return _wrap_if_needed(g.vertices(pid));
                } else if constexpr (_Choice_pid<_G, _PId>._Strategy == _St_pid::_adl) {
                    return _wrap_if_needed(vertices(g, pid));
                } else if constexpr (_Choice_pid<_G, _PId>._Strategy == _St_pid::_default) {
                    // Default: single partition (partition 0 only)
                    if (pid == 0) {
                        return (*this)(std::forward<G>(g));
                    } else {
                        // Empty range for non-existent partitions
                        // Use default constructor to create empty vertex_descriptor_view
                        using result_type = std::remove_cvref_t<decltype((*this)(std::forward<G>(g)))>;
                        return result_type{};
                    }
                }
            }
        };
    } // namespace _vertices

} // namespace _cpo_impls

// =============================================================================
// vertices(g) and vertices(g, pid) - Public CPO instance and type aliases
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting vertex range from a graph
     * 
     * Usage: 
     *   auto verts = graph::vertices(my_graph);           // All vertices
     *   auto verts = graph::vertices(my_graph, pid);      // Vertices in partition pid
     * 
     * Returns: vertex_descriptor_view
     */
    inline constexpr _cpo_impls::_vertices::_fn_combined vertices{};
} // namespace _cpo_instances

/**
 * @brief Range type returned by vertices(g)
 * 
 * This is always vertex_descriptor_view<Iter> where Iter is the iterator
 * type of the underlying container.
 */
template<typename G>
using vertex_range_t = decltype(vertices(std::declval<G&>()));

/**
 * @brief Iterator type for traversing vertices
 * 
 * Iterator over the vertex_descriptor_view returned by vertices(g).
 */
template<typename G>
using vertex_iterator_t = std::ranges::iterator_t<vertex_range_t<G>>;

/**
 * @brief Vertex descriptor type for graph G
 * 
 * This is the value_type of the vertex range - a vertex_descriptor<Iter>
 * that wraps an iterator into the graph's vertex container.
 */
template<typename G>
using vertex_t = std::ranges::range_value_t<vertex_range_t<G>>;

namespace _cpo_impls {

    // =========================================================================
    // vertex_id(g, u) CPO
    // =========================================================================
    
    namespace _vertex_id {
        enum class _St { _none, _adl_descriptor, _descriptor };
        
        // Check for ADL vertex_id(g, descriptor)
        template<typename G, typename U>
        concept _has_adl_descriptor = 
            is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
            requires(const G& g, const U& u) {
                { vertex_id(g, u) };
            };
        
        // Check if descriptor has vertex_id() member (default)
        template<typename U>
        concept _has_descriptor = is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
            requires(const U& u) {
                { u.vertex_id() };
            };
        
        template<typename G, typename U>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_adl_descriptor<G, U>) {
                return {_St::_adl_descriptor, 
                        noexcept(vertex_id(std::declval<const G&>(), std::declval<const U&>()))};
            } else if constexpr (_has_descriptor<U>) {
                return {_St::_descriptor, 
                        noexcept(std::declval<const U&>().vertex_id())};
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G, typename U>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>, std::remove_cvref_t<U>>();
            
        public:
            /**
             * @brief Get unique ID for a vertex
             * 
             * Resolution order:
             * 1. vertex_id(g, u) - ADL with vertex_descriptor (graph-provided customization)
             * 2. u.vertex_id() - descriptor's default method (fallback)
             * 
             * The ADL path allows graphs to provide custom vertex_id implementations via
             * friend functions. This is useful for graphs like compressed_graph where the
             * descriptor directly stores the ID.
             * 
             * The descriptor default works for standard containers:
             * - Random-access containers (vector, deque): returns the index
             * - Associative containers (map): returns the key
             * - Bidirectional containers: returns the iterator position
             * 
             * @tparam G Graph type
             * @tparam U Vertex descriptor type (constrained to be a vertex_descriptor_type)
             * @param g Graph container
             * @param u Vertex descriptor (must be vertex_t<G> - the vertex descriptor type for the graph)
             * @return Unique identifier for the vertex
             */
            template<typename G, vertex_descriptor_type U>
            [[nodiscard]] constexpr auto operator()(const G& g, const U& u) const
                noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
                -> decltype(auto)
                requires (_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _U = std::remove_cvref_t<U>;
                
                if constexpr (_Choice<_G, _U>._Strategy == _St::_adl_descriptor) {
                    return vertex_id(g, u);
                } else if constexpr (_Choice<_G, _U>._Strategy == _St::_descriptor) {
                    return u.vertex_id();
                }
            }
        };
    } // namespace _vertex_id

} // namespace _cpo_impls

// =============================================================================
// vertex_id(g, u) - Public CPO instance and type alias
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting vertex ID from a vertex descriptor
     * 
     * Usage: auto id = graph::vertex_id(my_graph, vertex_descriptor);
     * 
     * Returns: Vertex ID (index for vector, key for map, etc.)
     */
    inline constexpr _cpo_impls::_vertex_id::_fn vertex_id{};
} // namespace _cpo_instances

/**
 * @brief Vertex ID type for graph G
 * 
 * The type of the unique identifier returned by vertex_id(g, u).
 * - For random-access containers (vector, deque): size_t (index)
 * - For associative containers (map): key type
 * - For bidirectional containers: iterator-based ID
 */
template<typename G>
using vertex_id_t = decltype(vertex_id(std::declval<G&>(), std::declval<vertex_t<G>>()));

namespace _cpo_impls {

    // =========================================================================
    // find_vertex(g, uid) CPO
    // =========================================================================
    
    namespace _find_vertex {
        enum class _St { _none, _member, _adl, _associative, _random_access };
        
        // Use the public vertices CPO (already declared above)
        using _cpo_instances::vertices;
        
        // Check for g.find_vertex(uid) member function
        template<typename G, typename VId>
        concept _has_member = requires(G& g, const VId& uid) {
            { g.find_vertex(uid) };
        };
        
        // Check for ADL find_vertex(g, uid)
        template<typename G, typename VId>
        concept _has_adl = requires(G& g, const VId& uid) {
            { find_vertex(g, uid) };
        };
        
        // Check if graph is an associative container with find() member
        // (map, unordered_map, etc. where the key is the vertex ID)
        template<typename G, typename VId>
        concept _has_associative = requires(G& g, const VId& uid) {
            { g.find(uid) } -> std::same_as<typename G::iterator>;
            { g.end() } -> std::same_as<typename G::iterator>;
        };
        
        // Check if vertices(g) is sized (which implies we can use std::ranges::next with offset)
        template<typename G, typename VId>
        concept _has_random_access = 
            std::ranges::sized_range<decltype(vertices(std::declval<G&>()))> &&
            requires(VId uid) {
                std::ranges::next(std::ranges::begin(vertices(std::declval<G&>())), uid);
            };
        
        template<typename G, typename VId>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G, VId>) {
                return {_St::_member, 
                        noexcept(std::declval<G&>().find_vertex(std::declval<const VId&>()))};
            } else if constexpr (_has_adl<G, VId>) {
                return {_St::_adl, 
                        noexcept(find_vertex(std::declval<G&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_associative<G, VId>) {
                constexpr bool no_throw_find = noexcept(std::declval<G&>().find(std::declval<const VId&>()));
                // Note: We directly construct iterator from map iterator
                return {_St::_associative, no_throw_find};
            } else if constexpr (_has_random_access<G, VId>) {
                constexpr bool no_throw_vertices = noexcept(vertices(std::declval<G&>()));
                constexpr bool no_throw_begin = noexcept(std::ranges::begin(vertices(std::declval<G&>())));
                constexpr bool no_throw_next = noexcept(std::ranges::next(std::ranges::begin(vertices(std::declval<G&>())), std::declval<const VId&>()));
                return {_St::_random_access, no_throw_vertices && no_throw_begin && no_throw_next};
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G, typename VId>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>();
            
        public:
            /**
             * @brief Find vertex by ID
             * 
             * Returns an iterator to a vertex descriptor for the given vertex ID. The returned
             * iterator can be compared with vertices(g).end() to check validity, following the
             * standard STL pattern for find operations.
             * 
             * Resolution order:
             * 1. g.find_vertex(uid) - member function (highest priority)
             * 2. find_vertex(g, uid) - ADL
             * 3. g.find(uid) + convert to vertex iterator - associative container default
             * 4. std::ranges::next(begin(vertices(g)), uid) - random access default (lowest priority)
             * 
             * Complexity:
             * - Random-access containers (vector, deque): O(1) via indexed access
             * - Ordered associative (map): O(log n) via find()
             * - Unordered associative (unordered_map): O(1) average via find()
             * 
             * @tparam G Graph type
             * @tparam VId Vertex ID type (typically vertex_id_t<G>)
             * @param g Graph container
             * @param uid Vertex ID to find
             * @return Iterator to the vertex descriptor (typically vertex_descriptor_view<...>::iterator)
             * 
             * @note The returned iterator is lightweight and stores only the vertex position
             *       (index or underlying iterator). It does not hold a reference to any view
             *       object, making it independent of view lifetime.
             * 
             * @example Checking validity and using the result
             * @code
             * auto v_it = find_vertex(g, 5);
             * if (v_it != vertices(g).end()) {
             *     // Valid vertex found
             *     auto v_desc = *v_it;  // Dereference to get vertex_descriptor
             *     auto& e = edges(g, v_desc);
             *     // Process edges...
             * }
             * @endcode
             * 
             * @example Direct usage with CPOs (most common pattern)
             * @code
             * auto v_it = find_vertex(g, uid);
             * if (v_it != vertices(g).end()) {
             *     // CPOs accept iterators directly in many cases
             *     for (auto e : edges(g, *v_it)) {
             *         // Process edge...
             *     }
             * }
             * @endcode
             */
            template<typename G, typename VId>
            [[nodiscard]] constexpr auto operator()(G&& g, const VId& uid) const
                noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>._No_throw)
                -> decltype(auto)
                requires (_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _VId = std::remove_cvref_t<VId>;
                if constexpr (_Choice<_G, _VId>._Strategy == _St::_member) {
                    if constexpr (requires { typename _G::vertex_id_type; }) {
                        return std::forward<G>(g).find_vertex(static_cast<typename _G::vertex_id_type>(uid));
                    } else {
                        return std::forward<G>(g).find_vertex(uid);
                    }
                } else if constexpr (_Choice<_G, _VId>._Strategy == _St::_adl) {
                    if constexpr (requires { typename _G::vertex_id_type; }) {
                        return find_vertex(std::forward<G>(g), static_cast<typename _G::vertex_id_type>(uid));
                    } else {
                        return find_vertex(std::forward<G>(g), uid);
                    }
                } else if constexpr (_Choice<_G, _VId>._Strategy == _St::_associative) {
                    // For associative containers, use find() directly
                    // Construct vertex_descriptor_view iterator directly from map iterator
                    auto map_iter = [&g, &uid]() {
                        if constexpr (requires { typename _G::vertex_id_type; }) {
                            return g.find(static_cast<typename _G::vertex_id_type>(uid));
                        } else {
                            return g.find(uid);
                        }
                    }();
                    using container_iterator = decltype(map_iter);
                    using view_type = vertex_descriptor_view<container_iterator>;
                    using view_iterator = typename view_type::iterator;
                    return view_iterator{map_iter};
                } else if constexpr (_Choice<_G, _VId>._Strategy == _St::_random_access) {
                    using diff_type = std::iter_difference_t<decltype(std::ranges::begin(vertices(std::forward<G>(g))))>;
                    return std::ranges::next(std::ranges::begin(vertices(std::forward<G>(g))), static_cast<diff_type>(uid));
                }
            }
        };
    } // namespace _find_vertex

} // namespace _cpo_impls

// =============================================================================
// find_vertex(g, uid) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for finding a vertex by its ID
     * 
     * Returns an iterator to the vertex descriptor that can be compared with
     * vertices(g).end() to check if the vertex was found. The iterator is lightweight
     * and independent of any view lifetime.
     * 
     * @example Basic usage with validity check
     * @code
     * auto v_it = graph::find_vertex(g, vertex_id);
     * if (v_it != graph::vertices(g).end()) {
     *     // Vertex exists - safe to use
     *     auto edges_range = graph::edges(g, *v_it);
     * }
     * @endcode
     * 
     * @see _cpo_impls::_find_vertex::_fn::operator() for detailed documentation
     */
    inline constexpr _cpo_impls::_find_vertex::_fn find_vertex{};
} // namespace _cpo_instances

namespace _cpo_impls {

    // =========================================================================
    // edges(g, u) and edges(g, uid) CPO
    // =========================================================================
    
    namespace _edges {
        // Use the public CPO instances (already declared above)
        using _cpo_instances::find_vertex;
        
        // Strategy enum for edges(g, u) - vertex descriptor version
        enum class _St_u { _none, _member, _adl, _edge_value_pattern };
        
        // Check for g.edges(u) member function - vertex descriptor
        template<typename G, typename U>
        concept _has_member_u = requires(G& g, const U& u) {
            { g.edges(u) } -> std::ranges::forward_range;
        };
        
        // Check for ADL edges(g, u) - vertex descriptor
        template<typename G, typename U>
        concept _has_adl_u = requires(G& g, const U& u) {
            { edges(g, u) } -> std::ranges::forward_range;
        };
        
        // Check if the vertex descriptor's inner value is a forward range
        // and its elements satisfy edge_value_type
        template<typename G, typename U>
        concept _has_edge_value_pattern = 
            is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
            requires(G& g, const U& u) {
                { u.inner_value(g) } -> std::ranges::forward_range;
            } &&
            requires(G& g, const U& u) {
                typename std::ranges::range_value_t<decltype(u.inner_value(g))>;
                requires edge_value_type<std::ranges::range_value_t<decltype(u.inner_value(g))>>;
            };
        
        template<typename G, typename U>
        [[nodiscard]] consteval _Choice_t<_St_u> _Choose_u() noexcept {
            if constexpr (_has_member_u<G, U>) {
                return {_St_u::_member, noexcept(std::declval<G&>().edges(std::declval<const U&>()))};
            } else if constexpr (_has_adl_u<G, U>) {
                return {_St_u::_adl, noexcept(edges(std::declval<G&>(), std::declval<const U&>()))};
            } else if constexpr (_has_edge_value_pattern<G, U>) {
                return {_St_u::_edge_value_pattern, noexcept(std::declval<const U&>().inner_value(std::declval<G&>()))};
            } else {
                return {_St_u::_none, false};
            }
        }
        
        // Strategy enum for edges(g, uid) - vertex ID version
        enum class _St_uid { _none, _member, _adl, _default };
        
        // Check for g.edges(uid) member function - vertex ID
        template<typename G, typename VId>
        concept _has_member_uid = requires(G& g, const VId& uid) {
            { g.edges(uid) } -> std::ranges::forward_range;
        };
        
        // Check for ADL edges(g, uid) - vertex ID
        template<typename G, typename VId>
        concept _has_adl_uid = requires(G& g, const VId& uid) {
            { edges(g, uid) } -> std::ranges::forward_range;
        };
        
        // Check if we can use default implementation: edges(g, *find_vertex(g, uid))
        // We check that find_vertex works and returns something dereferenceable to a vertex descriptor
        // and that edges(g, vertex_descriptor) is callable via member or ADL (not recursively via uid)
        template<typename G, typename VId>
        concept _has_default_uid = requires(G& g, const VId& uid) {
            { find_vertex(g, uid) } -> std::input_iterator;
            requires vertex_descriptor_type<decltype(*find_vertex(g, uid))>;
        } && (_has_member_u<G, decltype(*find_vertex(std::declval<G&>(), std::declval<const VId&>()))> ||
              _has_adl_u<G, decltype(*find_vertex(std::declval<G&>(), std::declval<const VId&>()))> ||
              _has_edge_value_pattern<G, decltype(*find_vertex(std::declval<G&>(), std::declval<const VId&>()))>);
        
        template<typename G, typename VId>
        [[nodiscard]] consteval _Choice_t<_St_uid> _Choose_uid() noexcept {
            if constexpr (_has_member_uid<G, VId>) {
                return {_St_uid::_member, noexcept(std::declval<G&>().edges(std::declval<const VId&>()))};
            } else if constexpr (_has_adl_uid<G, VId>) {
                return {_St_uid::_adl, noexcept(edges(std::declval<G&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_default_uid<G, VId>) {
                // Default is not noexcept as it depends on find_vertex and edges(g,u)
                return {_St_uid::_default, false};
            } else {
                return {_St_uid::_none, false};
            }
        }
        
        // Helper to wrap result in edge_descriptor_view if not already
        template<typename Result, typename U>
        [[nodiscard]] constexpr auto _wrap_if_needed(Result&& result, const U& source_vertex) noexcept {
            using ResultType = std::remove_cvref_t<Result>;
            if constexpr (is_edge_descriptor_view_v<ResultType>) {
                // Already an edge_descriptor_view, return as-is
                return std::forward<Result>(result);
            } else if constexpr (std::ranges::range<ResultType>) {
                // Range (including subrange) - construct directly with iterators
                using VertexIterType = typename U::iterator_type;
                using EdgeIterType = std::ranges::iterator_t<ResultType>;
                return edge_descriptor_view<EdgeIterType, VertexIterType>(
                    std::ranges::begin(result), std::ranges::end(result), source_vertex);
            } else {
                // Not a range, assume it's a container - use deduction guide
                return edge_descriptor_view(std::forward<Result>(result), source_vertex);
            }
        }
        
        class _fn {
        private:
            template<typename G, typename U>
            static constexpr _Choice_t<_St_u> _Choice_u = _Choose_u<std::remove_cvref_t<G>, std::remove_cvref_t<U>>();
            
            template<typename G, typename VId>
            static constexpr _Choice_t<_St_uid> _Choice_uid = _Choose_uid<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>();
            
        public:
            /**
             * @brief Get range of outgoing edges from a vertex (by descriptor)
             * 
             * IMPORTANT: This CPO MUST always return an edge_descriptor_view.
             * 
             * Resolution order:
             * 1. If g.edges(u) exists -> use it (wrap in descriptor view if needed)
             * 2. If ADL edges(g, u) exists -> use it (wrap in descriptor view if needed)
             * 3. If u.inner_value(g) is a forward range of edge_value_type elements 
             *    -> return edge_descriptor_view(u.inner_value(g), u)
             * 
             * If custom g.edges(u) or ADL edges(g, u) already returns an 
             * edge_descriptor_view, it's used as-is. Otherwise, the result is 
             * automatically wrapped in edge_descriptor_view.
             * 
             * The default implementation works automatically for common patterns:
             * - Simple edges: vector<int> (target IDs)
             * - Pair edges: vector<pair<int, Weight>> (target + property)
             * - Tuple edges: vector<tuple<int, ...>> (target + properties)
             * - Custom edges: structs/classes with custom extraction
             * 
             * @tparam G Graph type
             * @tparam U Vertex descriptor type (constrained to be a vertex_descriptor_type)
             * @param g Graph container
             * @param u Vertex descriptor (must be vertex_t<G> - the vertex descriptor type for the graph)
             * @return edge_descriptor_view wrapping the edges
             */
            template<typename G, vertex_descriptor_type U>
            [[nodiscard]] constexpr auto operator()(G&& g, const U& u) const
                noexcept(_Choice_u<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
                requires (_Choice_u<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._Strategy != _St_u::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _U = std::remove_cvref_t<U>;
                if constexpr (_Choice_u<_G, _U>._Strategy == _St_u::_member) {
                    return _wrap_if_needed(g.edges(u), u);
                } else if constexpr (_Choice_u<_G, _U>._Strategy == _St_u::_adl) {
                    return _wrap_if_needed(edges(g, u), u);
                } else if constexpr (_Choice_u<_G, _U>._Strategy == _St_u::_edge_value_pattern) {
                    return edge_descriptor_view(u.inner_value(g), u);
                }
            }
            
            /**
             * @brief Get range of outgoing edges from a vertex (by ID)
             * 
             * IMPORTANT: This CPO MUST always return an edge_descriptor_view.
             * 
             * This is a convenience function that combines find_vertex + edges(g,u).
             * 
             * Resolution order:
             * 1. g.edges(uid) - Member function (highest priority)
             * 2. edges(g, uid) - ADL (medium priority)
             * 3. edges(g, *find_vertex(g, uid)) - Default (lowest priority)
             * 
             * The default implementation:
             * - Uses find_vertex(g, uid) to get the vertex iterator
             * - Dereferences to get the vertex descriptor
             * - Calls edges(g, u) with the vertex descriptor
             * 
             * Complexity matches find_vertex:
             * - Random-access containers (vector, deque): O(1)
             * - Ordered associative (map): O(log n)
             * - Unordered associative (unordered_map): O(1) average
             * 
             * @tparam G Graph type
             * @tparam VId Vertex ID type (typically vertex_id_t<G>)
             * @param g Graph container
             * @param uid Vertex ID
             * @return edge_descriptor_view wrapping the edges
             */
            template<typename G, typename VId>
                requires (!vertex_descriptor_type<VId>)
            [[nodiscard]] constexpr auto operator()(G&& g, const VId& uid) const
                noexcept(_Choice_uid<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>._No_throw)
                requires (_Choice_uid<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>._Strategy != _St_uid::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _VId = std::remove_cvref_t<VId>;
                
                if constexpr (_Choice_uid<_G, _VId>._Strategy == _St_uid::_member) {
                    // Member function: g.edges(uid)
                    // Wrap result but we don't have the vertex descriptor yet
                    // The member function must handle this appropriately
                    auto result = [&g, &uid]() {
                        if constexpr (requires { typename _G::vertex_id_type; }) {
                            return g.edges(static_cast<typename _G::vertex_id_type>(uid));
                        } else {
                            return g.edges(uid);
                        }
                    }();
                    // For member function, we need to get the vertex descriptor to wrap properly
                    auto v = *find_vertex(g, uid);
                    return _wrap_if_needed(std::move(result), v);
                } else if constexpr (_Choice_uid<_G, _VId>._Strategy == _St_uid::_adl) {
                    // ADL: edges(g, uid)
                    auto result = [&g, &uid]() {
                        if constexpr (requires { typename _G::vertex_id_type; }) {
                            return edges(g, static_cast<typename _G::vertex_id_type>(uid));
                        } else {
                            return edges(g, static_cast<vertex_id_t<_G>>(uid));
                        }
                    }();
                    auto v = *find_vertex(g, static_cast<vertex_id_t<_G>>(uid));
                    return _wrap_if_needed(std::move(result), v);
                } else if constexpr (_Choice_uid<_G, _VId>._Strategy == _St_uid::_default) {
                    // Default: find vertex then call edges(g, u)
                    auto v = *find_vertex(std::forward<G>(g), static_cast<vertex_id_t<_G>>(uid));
                    return (*this)(std::forward<G>(g), v);
                }
            }
        };
    } // namespace _edges

} // namespace _cpo_impls

// =============================================================================
// edges(g, u) - Public CPO instance and type aliases
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting outgoing edges from a vertex
     * 
     * Usage: auto vertex_edges = graph::edges(my_graph, vertex_descriptor);
     * 
     * Returns: edge_descriptor_view
     */
    inline constexpr _cpo_impls::_edges::_fn edges{};
} // namespace _cpo_instances

/**
 * @brief Range type returned by edges(g, u)
 * 
 * This is always edge_descriptor_view<EdgeIter, VertexIter> where EdgeIter
 * is the iterator type of the underlying edge container and VertexIter is
 * the iterator type of the vertex container.
 */
template<typename G>
using vertex_edge_range_t = decltype(edges(std::declval<G&>(), std::declval<vertex_t<G>>()));

/**
 * @brief Iterator type for traversing edges from a vertex
 * 
 * Iterator over the edge_descriptor_view returned by edges(g, u).
 */
template<typename G>
using vertex_edge_iterator_t = std::ranges::iterator_t<vertex_edge_range_t<G>>;

/**
 * @brief Edge descriptor type for graph G
 * 
 * This is the value_type of the edge range - an edge_descriptor<EdgeIter, VertexIter>
 * that wraps an edge and maintains its source vertex.
 */
template<typename G>
using edge_t = std::ranges::range_value_t<vertex_edge_range_t<G>>;

namespace _cpo_impls {

} // namespace _cpo_impls

namespace _cpo_impls {

    // =========================================================================
    // target(g, uv) CPO
    // =========================================================================
    
    namespace _target {
        enum class _St { _none, _member, _adl, _default };
        
        // Use the public CPO instances from graph namespace (shared edge CPOs)
        using _cpo_instances::find_vertex;
        using graph::target_id;
        
        // Check for g.target(uv) member function
        // Accepts either edge_descriptor or underlying edge value type
        template<typename G, typename E>
        concept _has_member = 
            requires(G& g, const E& uv) {
                { g.target(uv) };
            };
        
        // Check for ADL target(g, uv)
        // Accepts either edge_descriptor or underlying edge value type
        template<typename G, typename E>
        concept _has_adl = 
            requires(G& g, const E& uv) {
                { target(g, uv) };
            };
        
        // Check if we can use default implementation: find_vertex(g, target_id(g, uv))
        template<typename G, typename E>
        concept _has_default = 
            is_edge_descriptor_v<std::remove_cvref_t<E>> &&
            requires(G& g, const E& uv) {
                { target_id(g, uv) };
                { find_vertex(g, target_id(g, uv)) };
            };
        
        template<typename G, typename E>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G, E>) {
                return {_St::_member, 
                        noexcept(std::declval<G&>().target(std::declval<const E&>()))};
            } else if constexpr (_has_adl<G, E>) {
                return {_St::_adl, 
                        noexcept(target(std::declval<G&>(), std::declval<const E&>()))};
            } else if constexpr (_has_default<G, E>) {
                // Default is not noexcept as it depends on target_id and find_vertex
                return {_St::_default, false};
            } else {
                return {_St::_none, false};
            }
        }
        
        // Helper to convert result to vertex descriptor if needed
        // Supports two cases:
        // 1. Result is already a vertex_descriptor -> return as-is
        // 2. Result is an iterator (to vertex_descriptor_view) -> dereference to get vertex_descriptor
        //
        // Note: Custom implementations should return either:
        // - A vertex_descriptor directly (vertex_t<G>)
        // - An iterator from vertices(g) (vertex_iterator_t<G>)
        template<typename G, typename Result>
        [[nodiscard]] constexpr auto _to_vertex_descriptor(G&&, Result&& result) noexcept {
            using ResultType = std::remove_cvref_t<Result>;
            if constexpr (is_vertex_descriptor_v<ResultType>) {
                // Already a vertex_descriptor, return as-is
                return std::forward<Result>(result);
            } else {
                // Assume it's an iterator - dereference to get vertex_descriptor
                return *std::forward<Result>(result);
            }
        }
        
        class _fn {
        private:
            template<typename G, typename E>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>, std::remove_cvref_t<E>>();
            
        public:
            /**
             * @brief Get target vertex descriptor from an edge
             * 
             * Resolution order:
             * 1. g.target(uv) - Graph member function (highest priority)
             *    - May return vertex_descriptor or vertex_iterator (auto-converted)
             * 2. target(g, uv) - ADL (medium priority)
             *    - May return vertex_descriptor or vertex_iterator (auto-converted)
             * 3. *find_vertex(g, target_id(g, uv)) - Default (lowest priority)
             * 
             * Custom implementations (member/ADL) can return:
             * - vertex_descriptor directly (vertex_t<G>) - used as-is
             * - vertex_iterator (iterator to vertices) - dereferenced to get descriptor
             * 
             * The default implementation:
             * - Uses target_id(g, uv) to get the target vertex ID
             * - Uses find_vertex(g, target_id) to get the vertex iterator
             * - Dereferences the iterator to get the vertex descriptor
             * 
             * This is a convenience function that combines target_id and find_vertex.
             * For random-access graphs, this is O(1).
             * For associative graphs, this is O(log n) or O(1) average.
             * 
             * @tparam G Graph type
             * @tparam E Edge descriptor type (constrained to be an edge_descriptor_type)
             * @param g Graph container
             * @param uv Edge descriptor (must be edge_t<G> - the edge descriptor type for the graph)
             * @return Vertex descriptor for the target vertex (vertex_t<G>)
             */
            template<typename G, edge_descriptor_type E>
            [[nodiscard]] constexpr auto operator()(G&& g, const E& uv) const
                noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._No_throw)
                -> decltype(auto)
                requires (_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _E = std::remove_cvref_t<E>;
                
                if constexpr (_Choice<_G, _E>._Strategy == _St::_member) {
                    // Graph member function - may return vertex_descriptor or iterator
                    return _to_vertex_descriptor(g, g.target(uv));
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl) {
                    // ADL - may return vertex_descriptor or iterator
                    return _to_vertex_descriptor(g, target(g, uv));
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_default) {
                    // Default: use target_id + find_vertex
                    return *find_vertex(std::forward<G>(g), target_id(g, uv));
                }
            }
        };
    } // namespace _target

} // namespace _cpo_impls

// =============================================================================
// target(g, uv) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting target vertex descriptor from an edge
     * 
     * Usage: auto target_vertex = graph::target(my_graph, edge_descriptor);
     * 
     * Returns: Vertex descriptor (vertex_iterator_t<G>) for the target vertex
     */
    inline constexpr _cpo_impls::_target::_fn target{};
} // namespace _cpo_instances

namespace _cpo_impls {

    // =========================================================================
    // num_vertices(g) CPO
    // =========================================================================
    
    namespace _num_vertices {
        enum class _St { _none, _member, _adl, _ranges };
        
        // Check for g.num_vertices() member function
        template<typename G>
        concept _has_member = 
            requires(const G& g) {
                { g.num_vertices() } -> std::integral;
            };
        
        // Check for ADL num_vertices(g)
        template<typename G>
        concept _has_adl = 
            requires(const G& g) {
                { num_vertices(g) } -> std::integral;
            };
        
        // Check if graph is a sized_range (default)
        template<typename G>
        concept _has_ranges = std::ranges::sized_range<G>;
        
        template<typename G>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G>) {
                return {_St::_member, 
                        noexcept(std::declval<const G&>().num_vertices())};
            } else if constexpr (_has_adl<G>) {
                return {_St::_adl, 
                        noexcept(num_vertices(std::declval<const G&>()))};
            } else if constexpr (_has_ranges<G>) {
                return {_St::_ranges, 
                        noexcept(std::ranges::size(std::declval<const G&>()))};
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>>();
            
        public:
            /**
             * @brief Get the number of vertices in the graph
             * 
             * Resolution order (three-tier approach):
             * 1. g.num_vertices() - Member function (highest priority)
             * 2. num_vertices(g) - ADL (medium priority)
             * 3. std::ranges::size(g) - Ranges default (lowest priority)
             * 
             * The default implementation works for any sized_range (vector, deque, map, etc.)
             * Custom graph types can override by providing a member function or ADL function.
             * 
             * @tparam G Graph type
             * @param g Graph container
             * @return Number of vertices in the graph
             */
            template<typename G>
            [[nodiscard]] constexpr auto operator()(const G& g) const
                noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                
                if constexpr (_Choice<_G>._Strategy == _St::_member) {
                    return g.num_vertices();
                } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                    return num_vertices(g);
                } else if constexpr (_Choice<_G>._Strategy == _St::_ranges) {
                    return std::ranges::size(g);
                }
            }
        };
        
        // =====================================================================
        // num_vertices(g, pid) - partition-specific vertex count
        // =====================================================================
        
        enum class _St_pid { _none, _member, _adl, _default };
        
        // Check for g.num_vertices(pid) member function
        template<typename G, typename PId>
        concept _has_member_pid = 
            requires(const G& g, const PId& pid) {
                { g.num_vertices(pid) } -> std::integral;
            };
        
        // Check for ADL num_vertices(g, pid)
        template<typename G, typename PId>
        concept _has_adl_pid = 
            requires(const G& g, const PId& pid) {
                { num_vertices(g, pid) } -> std::integral;
            };
        
        // Check if we can use default implementation (always available)
        template<typename G, typename PId>
        concept _has_default_pid = std::integral<PId>;
        
        template<typename G, typename PId>
        [[nodiscard]] consteval _Choice_t<_St_pid> _Choose_pid() noexcept {
            if constexpr (_has_member_pid<G, PId>) {
                return {_St_pid::_member, 
                        noexcept(std::declval<const G&>().num_vertices(std::declval<const PId&>()))};
            } else if constexpr (_has_adl_pid<G, PId>) {
                return {_St_pid::_adl, 
                        noexcept(num_vertices(std::declval<const G&>(), std::declval<const PId&>()))};
            } else if constexpr (_has_default_pid<G, PId>) {
                return {_St_pid::_default, true}; // Default is noexcept
            } else {
                return {_St_pid::_none, false};
            }
        }
        
        // Combined CPO function with both overloads
        class _fn_combined {
        private:
            template<typename G>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>>();
            
            template<typename G, typename PId>
            static constexpr _Choice_t<_St_pid> _Choice_pid = _Choose_pid<std::remove_cvref_t<G>, std::remove_cvref_t<PId>>();
            
        public:
            /**
             * @brief Get the number of vertices in the graph
             * 
             * Resolution order (three-tier approach):
             * 1. g.num_vertices() - Member function (highest priority)
             * 2. num_vertices(g) - ADL (medium priority)
             * 3. std::ranges::size(g) - Ranges default (lowest priority)
             * 
             * The default implementation works for any sized_range (vector, deque, map, etc.)
             * Custom graph types can override by providing a member function or ADL function.
             * 
             * @tparam G Graph type
             * @param g Graph container
             * @return Number of vertices in the graph
             */
            template<typename G>
            [[nodiscard]] constexpr auto operator()(const G& g) const
                noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                
                if constexpr (_Choice<_G>._Strategy == _St::_member) {
                    return g.num_vertices();
                } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                    return num_vertices(g);
                } else if constexpr (_Choice<_G>._Strategy == _St::_ranges) {
                    return std::ranges::size(g);
                }
            }
            
            /**
             * @brief Get the number of vertices in a specific partition
             * 
             * Resolution order:
             * 1. g.num_vertices(pid) - Member function (highest priority)
             * 2. num_vertices(g, pid) - ADL (medium priority)
             * 3. Default: returns num_vertices(g) if pid==0, 0 otherwise (lowest priority)
             * 
             * The default implementation assumes single partition (partition 0).
             * For multi-partition graphs, provide custom member function or ADL.
             * 
             * @tparam G Graph type
             * @tparam PId Partition ID type (integral)
             * @param g Graph container
             * @param pid Partition ID
             * @return Number of vertices in the partition
             */
            template<typename G, std::integral PId>
            [[nodiscard]] constexpr auto operator()(const G& g, const PId& pid) const
                noexcept(_Choice_pid<std::remove_cvref_t<G>, std::remove_cvref_t<PId>>._No_throw)
                requires (_Choice_pid<std::remove_cvref_t<G>, std::remove_cvref_t<PId>>._Strategy != _St_pid::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _PId = std::remove_cvref_t<PId>;
                
                if constexpr (_Choice_pid<_G, _PId>._Strategy == _St_pid::_member) {
                    return g.num_vertices(pid);
                } else if constexpr (_Choice_pid<_G, _PId>._Strategy == _St_pid::_adl) {
                    return num_vertices(g, pid);
                } else if constexpr (_Choice_pid<_G, _PId>._Strategy == _St_pid::_default) {
                    // Default: single partition (partition 0 only)
                    if (pid == 0) {
                        return (*this)(g);
                    } else {
                        // No vertices in non-existent partitions
                        return static_cast<decltype((*this)(g))>(0);
                    }
                }
            }
        };
    } // namespace _num_vertices

} // namespace _cpo_impls

// =============================================================================
// num_vertices(g) and num_vertices(g, pid) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting the number of vertices in the graph
     * 
     * Usage: 
     *   auto count = graph::num_vertices(my_graph);        // All vertices
     *   auto count = graph::num_vertices(my_graph, pid);   // Vertices in partition pid
     * 
     * Returns: Number of vertices (size_t or similar integral type)
     */
    inline constexpr _cpo_impls::_num_vertices::_fn_combined num_vertices{};
} // namespace _cpo_instances

namespace _cpo_impls {

    // =========================================================================
    // num_edges(g) and num_edges(g, u) CPO
    // =========================================================================
    
    namespace _num_edges {
        // Strategy enum for num_edges(g) - graph version
        enum class _St { _none, _member, _adl, _default };
        
        // Check for g.num_edges() member function
        template<typename G>
        concept _has_member = 
            requires(const G& g) {
                { g.num_edges() } -> std::integral;
            };
        
        // Check for ADL num_edges(g)
        template<typename G>
        concept _has_adl = 
            requires(const G& g) {
                { num_edges(g) } -> std::integral;
            };
        
        // Check if we can iterate vertices and their edges
        template<typename G>
        concept _has_default = requires(G& g) {
            { vertices(g) };
            requires std::ranges::forward_range<decltype(vertices(g))>;
        };
        
        template<typename G>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G>) {
                return {_St::_member, 
                        noexcept(std::declval<const G&>().num_edges())};
            } else if constexpr (_has_adl<G>) {
                return {_St::_adl, 
                        noexcept(num_edges(std::declval<const G&>()))};
            } else if constexpr (_has_default<G>) {
                // Default implementation is not noexcept as it may allocate
                return {_St::_default, false};
            } else {
                return {_St::_none, false};
            }
        }
        
        // Strategy enum for num_edges(g, u) - vertex descriptor version
        enum class _St_u { _none, _member, _adl, _default };
        
        // Check for g.num_edges(u) member function - vertex descriptor
        template<typename G, typename U>
        concept _has_member_u = requires(G& g, const U& u) {
            { g.num_edges(u) } -> std::integral;
        };
        
        // Check for ADL num_edges(g, u) - vertex descriptor
        template<typename G, typename U>
        concept _has_adl_u = requires(G& g, const U& u) {
            { num_edges(g, u) } -> std::integral;
        };
        
        // Check if we can use default: std::ranges::size(edges(g, u))
        template<typename G, typename U>
        concept _has_default_u = requires(G& g, const U& u) {
            { edges(g, u) } -> std::ranges::sized_range;
        };
        
        template<typename G, typename U>
        [[nodiscard]] consteval _Choice_t<_St_u> _Choose_u() noexcept {
            if constexpr (_has_member_u<G, U>) {
                return {_St_u::_member, noexcept(std::declval<G&>().num_edges(std::declval<const U&>()))};
            } else if constexpr (_has_adl_u<G, U>) {
                return {_St_u::_adl, noexcept(num_edges(std::declval<G&>(), std::declval<const U&>()))};
            } else if constexpr (_has_default_u<G, U>) {
                return {_St_u::_default, noexcept(std::ranges::size(edges(std::declval<G&>(), std::declval<const U&>())))};
            } else {
                return {_St_u::_none, false};
            }
        }
        
        // Strategy enum for num_edges(g, uid) - vertex ID version
        enum class _St_uid { _none, _member, _adl, _default };
        
        // Check for g.num_edges(uid) member function - vertex ID
        template<typename G, typename VId>
        concept _has_member_uid = requires(G& g, const VId& uid) {
            { g.num_edges(uid) } -> std::integral;
        };
        
        // Check for ADL num_edges(g, uid) - vertex ID
        template<typename G, typename VId>
        concept _has_adl_uid = requires(G& g, const VId& uid) {
            { num_edges(g, uid) } -> std::integral;
        };
        
        // Check if we can use default implementation: num_edges(g, *find_vertex(g, uid))
        template<typename G, typename VId>
        concept _has_default_uid = requires(G& g, const VId& uid) {
            { find_vertex(g, uid) } -> std::input_iterator;
            requires vertex_descriptor_type<decltype(*find_vertex(g, uid))>;
            requires _has_default_u<G, decltype(*find_vertex(g, uid))>;
        };
        
        template<typename G, typename VId>
        [[nodiscard]] consteval _Choice_t<_St_uid> _Choose_uid() noexcept {
            if constexpr (_has_member_uid<G, VId>) {
                return {_St_uid::_member, noexcept(std::declval<G&>().num_edges(std::declval<const VId&>()))};
            } else if constexpr (_has_adl_uid<G, VId>) {
                return {_St_uid::_adl, noexcept(num_edges(std::declval<G&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_default_uid<G, VId>) {
                return {_St_uid::_default, false};
            } else {
                return {_St_uid::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>>();
            
            template<typename G, typename U>
            static constexpr _Choice_t<_St_u> _Choice_u = _Choose_u<std::remove_cvref_t<G>, std::remove_cvref_t<U>>();
            
            template<typename G, typename VId>
            static constexpr _Choice_t<_St_uid> _Choice_uid = _Choose_uid<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>();
            
        public:
            /**
             * @brief Get the total number of edges in the graph
             * 
             * Resolution order:
             * 1. g.num_edges() - Member function (highest priority)
             * 2. num_edges(g) - ADL (medium priority)
             * 3. Default implementation (lowest priority) - Iterates through all vertices
             *    and sums the number of edges for each vertex
             * 
             * The default implementation:
             * - Iterates through all vertices using vertices(g)
             * - For each vertex u, gets edges(g, u)
             * - If the edges range is sized, uses std::ranges::size()
             * - Otherwise, uses std::ranges::distance() on the underlying range
             * 
             * For directed graphs, this counts each edge once.
             * For undirected graphs, this counts each edge twice (once per endpoint).
             * 
             * @tparam G Graph type
             * @param g Graph container
             * @return Total number of edges in the graph
             */
            template<typename G>
            [[nodiscard]] constexpr auto operator()(G&& g) const
                noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                
                if constexpr (_Choice<_G>._Strategy == _St::_member) {
                    return g.num_edges();
                } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                    return num_edges(g);
                } else if constexpr (_Choice<_G>._Strategy == _St::_default) {
                    // Default implementation: iterate through vertices and sum edge counts
                    std::size_t count = 0;
                    
                    for (auto u : vertices(g)) {
                        auto edge_range = edges(g, u);
                        
                        // Try to use sized_range if available, otherwise use distance
                        if constexpr (std::ranges::sized_range<decltype(edge_range)>) {
                            count += std::ranges::size(edge_range);
                        } else {
                            // Use the underlying range for distance calculation
                            // edge_descriptor_view exposes the underlying container via begin()/end()
                            count += static_cast<std::size_t>(std::ranges::distance(edge_range.begin(), edge_range.end()));
                        }
                    }
                    
                    return count;
                }
            }
            
            /**
             * @brief Get the number of outgoing edges from a specific vertex (descriptor version)
             * 
             * Resolution order:
             * 1. g.num_edges(u) - Member function (highest priority)
             * 2. num_edges(g, u) - ADL (medium priority)
             * 3. Default implementation (lowest priority) - std::ranges::size(edges(g, u))
             * 
             * @tparam G Graph type
             * @tparam U Vertex descriptor type
             * @param g Graph container
             * @param u Vertex descriptor
             * @return Number of outgoing edges from vertex u
             */
            template<typename G, vertex_descriptor_type U>
            [[nodiscard]] constexpr auto operator()(G&& g, const U& u) const
                noexcept(_Choice_u<G, U>._No_throw)
                requires (_Choice_u<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._Strategy != _St_u::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _U = std::remove_cvref_t<U>;
                
                if constexpr (_Choice_u<_G, _U>._Strategy == _St_u::_member) {
                    return g.num_edges(u);
                } else if constexpr (_Choice_u<_G, _U>._Strategy == _St_u::_adl) {
                    return num_edges(g, u);
                } else if constexpr (_Choice_u<_G, _U>._Strategy == _St_u::_default) {
                    return std::ranges::size(edges(std::forward<G>(g), u));
                }
            }
            
            /**
             * @brief Get the number of outgoing edges from a specific vertex (ID version)
             * 
             * Resolution order:
             * 1. g.num_edges(uid) - Member function (highest priority)
             * 2. num_edges(g, uid) - ADL (medium priority)
             * 3. Default implementation (lowest priority) - num_edges(g, *find_vertex(g, uid))
             * 
             * @tparam G Graph type
             * @tparam VId Vertex ID type
             * @param g Graph container
             * @param uid Vertex ID
             * @return Number of outgoing edges from vertex uid
             */
            template<typename G, typename VId>
                requires (!vertex_descriptor_type<VId>)
            [[nodiscard]] constexpr auto operator()(G&& g, const VId& uid) const
                noexcept(_Choice_uid<G, VId>._No_throw)
                requires (_Choice_uid<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>._Strategy != _St_uid::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _VId = std::remove_cvref_t<VId>;
                
                if constexpr (_Choice_uid<_G, _VId>._Strategy == _St_uid::_member) {
                    return g.num_edges(uid);
                } else if constexpr (_Choice_uid<_G, _VId>._Strategy == _St_uid::_adl) {
                    return num_edges(g, uid);
                } else if constexpr (_Choice_uid<_G, _VId>._Strategy == _St_uid::_default) {
                    // Default: find vertex then call num_edges(g, u)
                    auto v = *find_vertex(std::forward<G>(g), uid);
                    return (*this)(std::forward<G>(g), v);
                }
            }
        };
    } // namespace _num_edges
} // namespace _cpo_impls

// =============================================================================
// num_edges(g) and num_edges(g, u) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting the number of edges in the graph
     * 
     * Usage: 
     * - auto total = graph::num_edges(my_graph);        // Total edges in graph
     * - auto count = graph::num_edges(my_graph, u);     // Edges from vertex u
     * - auto count = graph::num_edges(my_graph, uid);   // Edges from vertex uid
     * 
     * Returns: Number of edges (size_t)
     */
    inline constexpr _cpo_impls::_num_edges::_fn num_edges{};
} // namespace _cpo_instances

namespace _cpo_impls {
    // =========================================================================
    // degree(g, u) and degree(g, uid) CPO
    // =========================================================================
    
    namespace _degree {
        // Strategy enum for degree(g, u) - vertex descriptor version
        enum class _St_u { _none, _member, _adl, _default };
        
        // Check for g.degree(u) member function - vertex descriptor
        template<typename G, typename U>
        concept _has_member_u = requires(G& g, const U& u) {
            { g.degree(u) } -> std::integral;
        };
        
        // Check for ADL degree(g, u) - vertex descriptor
        template<typename G, typename U>
        concept _has_adl_u = requires(G& g, const U& u) {
            { degree(g, u) } -> std::integral;
        };
        
        // Check if we can use default: count edges via size() or distance()
        template<typename G, typename U>
        concept _has_default_u = requires(G& g, const U& u) {
            { edges(g, u) } -> std::ranges::forward_range;
        };
        
        template<typename G, typename U>
        [[nodiscard]] consteval _Choice_t<_St_u> _Choose_u() noexcept {
            if constexpr (_has_member_u<G, U>) {
                return {_St_u::_member, noexcept(std::declval<G&>().degree(std::declval<const U&>()))};
            } else if constexpr (_has_adl_u<G, U>) {
                return {_St_u::_adl, noexcept(degree(std::declval<G&>(), std::declval<const U&>()))};
            } else if constexpr (_has_default_u<G, U>) {
                // Default implementation uses size() or distance() depending on the range type
                // Both operations typically don't throw, so we return true
                return {_St_u::_default, true};
            } else {
                return {_St_u::_none, false};
            }
        }
        
        // Strategy enum for degree(g, uid) - vertex ID version
        enum class _St_uid { _none, _member, _adl, _default };
        
        // Check for g.degree(uid) member function - vertex ID
        template<typename G, typename VId>
        concept _has_member_uid = requires(G& g, const VId& uid) {
            { g.degree(uid) } -> std::integral;
        };
        
        // Check for ADL degree(g, uid) - vertex ID
        template<typename G, typename VId>
        concept _has_adl_uid = requires(G& g, const VId& uid) {
            { degree(g, uid) } -> std::integral;
        };
        
        // Check if we can use default implementation: degree(g, *find_vertex(g, uid))
        template<typename G, typename VId>
        concept _has_default_uid = requires(G& g, const VId& uid) {
            { find_vertex(g, uid) } -> std::input_iterator;
            requires vertex_descriptor_type<decltype(*find_vertex(g, uid))>;
            requires _has_default_u<G, decltype(*find_vertex(g, uid))>;
        };
        
        template<typename G, typename VId>
        [[nodiscard]] consteval _Choice_t<_St_uid> _Choose_uid() noexcept {
            if constexpr (_has_member_uid<G, VId>) {
                return {_St_uid::_member, noexcept(std::declval<G&>().degree(std::declval<const VId&>()))};
            } else if constexpr (_has_adl_uid<G, VId>) {
                return {_St_uid::_adl, noexcept(degree(std::declval<G&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_default_uid<G, VId>) {
                return {_St_uid::_default, false};
            } else {
                return {_St_uid::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G, typename U>
            static constexpr _Choice_t<_St_u> _Choice_u = _Choose_u<std::remove_cvref_t<G>, std::remove_cvref_t<U>>();
            
            template<typename G, typename VId>
            static constexpr _Choice_t<_St_uid> _Choice_uid = _Choose_uid<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>();
            
        public:
            // degree(g, u) - vertex descriptor version
            template<typename G, vertex_descriptor_type U>
            [[nodiscard]] constexpr auto operator()(G&& g, const U& u) const
                noexcept(_Choice_u<G, U>._No_throw)
                requires (_Choice_u<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._Strategy != _St_u::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _U = std::remove_cvref_t<U>;
                
                if constexpr (_Choice_u<_G, _U>._Strategy == _St_u::_member) {
                    return g.degree(u);
                } else if constexpr (_Choice_u<_G, _U>._Strategy == _St_u::_adl) {
                    return degree(g, u);
                } else if constexpr (_Choice_u<_G, _U>._Strategy == _St_u::_default) {
                    auto edge_range = edges(std::forward<G>(g), u);
                    if constexpr (std::ranges::sized_range<decltype(edge_range)>) {
                        return std::ranges::size(edge_range);
                    } else {
                        return std::ranges::distance(edge_range);
                    }
                }
            }
            
            // degree(g, uid) - vertex ID version
            template<typename G, typename VId>
                requires (!vertex_descriptor_type<VId>)
            [[nodiscard]] constexpr auto operator()(G&& g, const VId& uid) const
                noexcept(_Choice_uid<G, VId>._No_throw)
                requires (_Choice_uid<std::remove_cvref_t<G>, std::remove_cvref_t<VId>>._Strategy != _St_uid::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _VId = std::remove_cvref_t<VId>;
                
                if constexpr (_Choice_uid<_G, _VId>._Strategy == _St_uid::_member) {
                    if constexpr (requires { typename _G::vertex_id_type; }) {
                        return g.degree(static_cast<typename _G::vertex_id_type>(uid));
                    } else {
                        return g.degree(uid);
                    }
                } else if constexpr (_Choice_uid<_G, _VId>._Strategy == _St_uid::_adl) {
                    if constexpr (requires { typename _G::vertex_id_type; }) {
                        return degree(g, static_cast<typename _G::vertex_id_type>(uid));
                    } else {
                        return degree(g, uid);
                    }
                } else if constexpr (_Choice_uid<_G, _VId>._Strategy == _St_uid::_default) {
                    // Default: find vertex then call degree(g, u)
                    auto v = *find_vertex(std::forward<G>(g), static_cast<vertex_id_t<_G>>(uid));
                    return (*this)(std::forward<G>(g), v);
                }
            }
        };
    } // namespace _degree
} // namespace _cpo_impls

// =============================================================================
// degree(g, u) and degree(g, uid) - Public CPO instances
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting the degree (number of outgoing edges) of a vertex
     * 
     * Usage: 
     *   auto deg = graph::degree(my_graph, vertex_descriptor);
     *   auto deg = graph::degree(my_graph, vertex_id);
     * 
     * Returns: Number of outgoing edges from the vertex (integral type)
     */
    inline constexpr _cpo_impls::_degree::_fn degree{};
} // namespace _cpo_instances

namespace _cpo_impls {
    // =========================================================================
    // find_vertex_edge(g, u, v) and find_vertex_edge(g, u, vid) and 
    // find_vertex_edge(g, uid, vid) CPO
    // =========================================================================
    
    namespace _find_vertex_edge {
        // Strategy enum for find_vertex_edge(g, u, v) - both descriptors
        enum class _St_uu { _none, _member, _adl, _default };
        
        // Check for g.find_vertex_edge(u, v) member function
        template<typename G, typename U, typename V>
        concept _has_member_uu = requires(G& g, const U& u, const V& v) {
            { g.find_vertex_edge(u, v) };
        };
        
        // Check for ADL find_vertex_edge(g, u, v)
        template<typename G, typename U, typename V>
        concept _has_adl_uu = requires(G& g, const U& u, const V& v) {
            { find_vertex_edge(g, u, v) };
        };
        
        // Check if we can use default: iterate edges(g,u) and find matching target
        template<typename G, typename U, typename V>
        concept _has_default_uu = requires(G& g, const U& u, const V& v) {
            { edges(g, u) } -> std::ranges::input_range;
            { target_id(g, *std::ranges::begin(edges(g, u))) };
            { vertex_id(g, v) };
        };
        
        template<typename G, typename U, typename V>
        [[nodiscard]] consteval _Choice_t<_St_uu> _Choose_uu() noexcept {
            if constexpr (_has_member_uu<G, U, V>) {
                return {_St_uu::_member, noexcept(std::declval<G&>().find_vertex_edge(std::declval<const U&>(), std::declval<const V&>()))};
            } else if constexpr (_has_adl_uu<G, U, V>) {
                return {_St_uu::_adl, noexcept(find_vertex_edge(std::declval<G&>(), std::declval<const U&>(), std::declval<const V&>()))};
            } else if constexpr (_has_default_uu<G, U, V>) {
                return {_St_uu::_default, false};
            } else {
                return {_St_uu::_none, false};
            }
        }
        
        // Strategy enum for find_vertex_edge(g, u, vid) - descriptor + ID
        enum class _St_uid { _none, _member, _adl, _default };
        
        // Check for g.find_vertex_edge(u, vid) member function
        template<typename G, typename U, typename VId>
        concept _has_member_uid = requires(G& g, const U& u, const VId& vid) {
            { g.find_vertex_edge(u, vid) };
        };
        
        // Check for ADL find_vertex_edge(g, u, vid)
        template<typename G, typename U, typename VId>
        concept _has_adl_uid = requires(G& g, const U& u, const VId& vid) {
            { find_vertex_edge(g, u, vid) };
        };
        
        // Check if we can use default: iterate edges(g,u) and find matching target_id
        template<typename G, typename U, typename VId>
        concept _has_default_uid = requires(G& g, const U& u, const VId& vid) {
            { edges(g, u) } -> std::ranges::input_range;
            { target_id(g, *std::ranges::begin(edges(g, u))) };
        };
        
        template<typename G, typename U, typename VId>
        [[nodiscard]] consteval _Choice_t<_St_uid> _Choose_uid() noexcept {
            if constexpr (_has_member_uid<G, U, VId>) {
                return {_St_uid::_member, noexcept(std::declval<G&>().find_vertex_edge(std::declval<const U&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_adl_uid<G, U, VId>) {
                return {_St_uid::_adl, noexcept(find_vertex_edge(std::declval<G&>(), std::declval<const U&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_default_uid<G, U, VId>) {
                return {_St_uid::_default, false};
            } else {
                return {_St_uid::_none, false};
            }
        }
        
        // Strategy enum for find_vertex_edge(g, uid, vid) - both IDs
        enum class _St_uidvid { _none, _member, _adl, _default };
        
        // Check for g.find_vertex_edge(uid, vid) member function
        template<typename G, typename UId, typename VId>
        concept _has_member_uidvid = requires(G& g, const UId& uid, const VId& vid) {
            { g.find_vertex_edge(uid, vid) };
        };
        
        // Check for ADL find_vertex_edge(g, uid, vid)
        template<typename G, typename UId, typename VId>
        concept _has_adl_uidvid = requires(G& g, const UId& uid, const VId& vid) {
            { find_vertex_edge(g, uid, vid) };
        };
        
        // Check if we can use default: find_vertex_edge(g, *find_vertex(g,uid), vid)
        template<typename G, typename UId, typename VId>
        concept _has_default_uidvid = requires(G& g, const UId& uid, const VId& vid) {
            { find_vertex(g, uid) } -> std::input_iterator;
            requires vertex_descriptor_type<decltype(*find_vertex(g, uid))>;
            requires _has_default_uid<G, decltype(*find_vertex(g, uid)), VId>;
        };
        
        template<typename G, typename UId, typename VId>
        [[nodiscard]] consteval _Choice_t<_St_uidvid> _Choose_uidvid() noexcept {
            if constexpr (_has_member_uidvid<G, UId, VId>) {
                return {_St_uidvid::_member, noexcept(std::declval<G&>().find_vertex_edge(std::declval<const UId&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_adl_uidvid<G, UId, VId>) {
                return {_St_uidvid::_adl, noexcept(find_vertex_edge(std::declval<G&>(), std::declval<const UId&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_default_uidvid<G, UId, VId>) {
                return {_St_uidvid::_default, false};
            } else {
                return {_St_uidvid::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G, typename U, typename V>
            static constexpr _Choice_t<_St_uu> _Choice_uu = _Choose_uu<std::remove_cvref_t<G>, std::remove_cvref_t<U>, std::remove_cvref_t<V>>();
            
            template<typename G, typename U, typename VId>
            static constexpr _Choice_t<_St_uid> _Choice_uid = _Choose_uid<std::remove_cvref_t<G>, std::remove_cvref_t<U>, std::remove_cvref_t<VId>>();
            
            template<typename G, typename UId, typename VId>
            static constexpr _Choice_t<_St_uidvid> _Choice_uidvid = _Choose_uidvid<std::remove_cvref_t<G>, std::remove_cvref_t<UId>, std::remove_cvref_t<VId>>();
            
        public:
            // find_vertex_edge(g, u, v) - both vertex descriptors
            template<typename G, vertex_descriptor_type U, vertex_descriptor_type V>
            [[nodiscard]] constexpr auto operator()(G&& g, const U& u, const V& v) const
                noexcept(_Choice_uu<G, U, V>._No_throw)
                requires (_Choice_uu<std::remove_cvref_t<G>, std::remove_cvref_t<U>, std::remove_cvref_t<V>>._Strategy != _St_uu::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _U = std::remove_cvref_t<U>;
                using _V = std::remove_cvref_t<V>;
                
                if constexpr (_Choice_uu<_G, _U, _V>._Strategy == _St_uu::_member) {
                    return g.find_vertex_edge(u, v);
                } else if constexpr (_Choice_uu<_G, _U, _V>._Strategy == _St_uu::_adl) {
                    return find_vertex_edge(g, u, v);
                } else if constexpr (_Choice_uu<_G, _U, _V>._Strategy == _St_uu::_default) {
                    // Default: iterate edges(g,u) and find edge with matching target
                    auto target_vid = vertex_id(std::forward<G>(g), v);
                    auto edge_range = edges(std::forward<G>(g), u);
                    auto it = std::ranges::find_if(edge_range, [&](const auto& e) {
                        return static_cast<vertex_id_t<_G>>(target_id(std::forward<G>(g), e)) == static_cast<vertex_id_t<_G>>(target_vid);
                    });
                    // Not found - return end as an edge descriptor
                    // This mimics std::find behavior where end() is returned when not found
                    return *it;
                }
            }
            
            // find_vertex_edge(g, u, vid) - descriptor + vertex ID
            template<typename G, vertex_descriptor_type U, typename VId>
                requires (!vertex_descriptor_type<VId>)
            [[nodiscard]] constexpr auto operator()(G&& g, const U& u, const VId& vid) const
                noexcept(_Choice_uid<G, U, VId>._No_throw)
                requires (_Choice_uid<std::remove_cvref_t<G>, std::remove_cvref_t<U>, std::remove_cvref_t<VId>>._Strategy != _St_uid::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _U = std::remove_cvref_t<U>;
                using _VId = std::remove_cvref_t<VId>;
                
                if constexpr (_Choice_uid<_G, _U, _VId>._Strategy == _St_uid::_member) {
                    return g.find_vertex_edge(u, vid);
                } else if constexpr (_Choice_uid<_G, _U, _VId>._Strategy == _St_uid::_adl) {
                    return find_vertex_edge(g, u, vid);
                } else if constexpr (_Choice_uid<_G, _U, _VId>._Strategy == _St_uid::_default) {
                    // Default: iterate edges(g,u) and find edge with matching target_id
                    auto edge_range = edges(std::forward<G>(g), u);
                    auto it = std::ranges::find_if(edge_range, [&](const auto& e) {
                        return static_cast<vertex_id_t<_G>>(target_id(std::forward<G>(g), e)) == static_cast<vertex_id_t<_G>>(vid);
                    });
                    // Not found - return end as an edge descriptor
                    return *it;
                }
            }
            
            // find_vertex_edge(g, uid, vid) - both vertex IDs
            template<typename G, typename UId, typename VId>
                requires (!vertex_descriptor_type<UId>) && (!vertex_descriptor_type<VId>)
            [[nodiscard]] constexpr auto operator()(G&& g, const UId& uid, const VId& vid) const
                noexcept(_Choice_uidvid<G, UId, VId>._No_throw)
                requires (_Choice_uidvid<std::remove_cvref_t<G>, std::remove_cvref_t<UId>, std::remove_cvref_t<VId>>._Strategy != _St_uidvid::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _UId = std::remove_cvref_t<UId>;
                using _VId = std::remove_cvref_t<VId>;
                
                if constexpr (_Choice_uidvid<_G, _UId, _VId>._Strategy == _St_uidvid::_member) {
                    if constexpr (requires { typename _G::vertex_id_type; }) {
                        return g.find_vertex_edge(static_cast<typename _G::vertex_id_type>(uid), static_cast<typename _G::vertex_id_type>(vid));
                    } else {
                        return g.find_vertex_edge(uid, vid);
                    }
                } else if constexpr (_Choice_uidvid<_G, _UId, _VId>._Strategy == _St_uidvid::_adl) {
                    if constexpr (requires { typename _G::vertex_id_type; }) {
                        return find_vertex_edge(g, static_cast<typename _G::vertex_id_type>(uid), static_cast<typename _G::vertex_id_type>(vid));
                    } else {
                        return find_vertex_edge(g, uid, vid);
                    }
                } else if constexpr (_Choice_uidvid<_G, _UId, _VId>._Strategy == _St_uidvid::_default) {
                    // Default: find source vertex then call find_vertex_edge(g, u, vid)
                    auto u = *find_vertex(std::forward<G>(g), uid);
                    return (*this)(std::forward<G>(g), u, vid);
                }
            }
        };
    } // namespace _find_vertex_edge
} // namespace _cpo_impls

// =============================================================================
// find_vertex_edge(g, u, v/vid) and find_vertex_edge(g, uid, vid) - Public CPO instances
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for finding an edge from source vertex u to target vertex v
     * 
     * Usage: 
     *   auto e = graph::find_vertex_edge(my_graph, u_descriptor, v_descriptor);
     *   auto e = graph::find_vertex_edge(my_graph, u_descriptor, target_id);
     *   auto e = graph::find_vertex_edge(my_graph, source_id, target_id);
     * 
     * Returns: Edge descriptor if found, or end iterator/sentinel if not found
     */
    inline constexpr _cpo_impls::_find_vertex_edge::_fn find_vertex_edge{};
} // namespace _cpo_instances

namespace _cpo_impls {
    // =========================================================================
    // contains_edge(g, u, v) and contains_edge(g, uid, vid) CPO
    // =========================================================================
    
    namespace _contains_edge {
        // Strategy enum for contains_edge(g, u, v)
        enum class _St_uv { _none, _member, _adl, _default };
        
        // Check for g.contains_edge(u, v) member function
        template<typename G, typename U, typename V>
        concept _has_member_uv = requires(G& g, const U& u, const V& v) {
            { g.contains_edge(u, v) } -> std::convertible_to<bool>;
        };
        
        // Check for ADL contains_edge(g, u, v)
        template<typename G, typename U, typename V>
        concept _has_adl_uv = requires(G& g, const U& u, const V& v) {
            { contains_edge(g, u, v) } -> std::convertible_to<bool>;
        };
        
        // Check if we can use default: iterate edges and compare target_id
        template<typename G, typename U, typename V>
        concept _has_default_uv = requires(G& g, const U& u, const V& v) {
            { edges(g, u) } -> std::ranges::input_range;
            { target_id(g, *std::ranges::begin(edges(g, u))) };
            { vertex_id(g, v) };
        };
        
        template<typename G, typename U, typename V>
        [[nodiscard]] consteval _Choice_t<_St_uv> _Choose_uv() noexcept {
            if constexpr (_has_member_uv<G, U, V>) {
                return {_St_uv::_member, noexcept(std::declval<G&>().contains_edge(std::declval<const U&>(), std::declval<const V&>()))};
            } else if constexpr (_has_adl_uv<G, U, V>) {
                return {_St_uv::_adl, noexcept(contains_edge(std::declval<G&>(), std::declval<const U&>(), std::declval<const V&>()))};
            } else if constexpr (_has_default_uv<G, U, V>) {
                return {_St_uv::_default, false};
            } else {
                return {_St_uv::_none, false};
            }
        }
        
        // Strategy enum for contains_edge(g, uid, vid)
        enum class _St_uidvid { _none, _member, _adl, _default };
        
        // Check for g.contains_edge(uid, vid) member function
        template<typename G, typename UId, typename VId>
        concept _has_member_uidvid = requires(G& g, const UId& uid, const VId& vid) {
            { g.contains_edge(uid, vid) } -> std::convertible_to<bool>;
        };
        
        // Check for ADL contains_edge(g, uid, vid)
        template<typename G, typename UId, typename VId>
        concept _has_adl_uidvid = requires(G& g, const UId& uid, const VId& vid) {
            { contains_edge(g, uid, vid) } -> std::convertible_to<bool>;
        };
        
        // Check if we can use default: find_vertex then iterate edges and compare target_id
        template<typename G, typename UId, typename VId>
        concept _has_default_uidvid = requires(G& g, const UId& uid, const VId& vid) {
            { find_vertex(g, uid) };
            { edges(g, *find_vertex(g, uid)) } -> std::ranges::input_range;
            { target_id(g, *std::ranges::begin(edges(g, *find_vertex(g, uid)))) };
        };
        
        template<typename G, typename UId, typename VId>
        [[nodiscard]] consteval _Choice_t<_St_uidvid> _Choose_uidvid() noexcept {
            if constexpr (_has_member_uidvid<G, UId, VId>) {
                return {_St_uidvid::_member, noexcept(std::declval<G&>().contains_edge(std::declval<const UId&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_adl_uidvid<G, UId, VId>) {
                return {_St_uidvid::_adl, noexcept(contains_edge(std::declval<G&>(), std::declval<const UId&>(), std::declval<const VId&>()))};
            } else if constexpr (_has_default_uidvid<G, UId, VId>) {
                return {_St_uidvid::_default, false};
            } else {
                return {_St_uidvid::_none, false};
            }
        }
        
        // Template variable for compile-time strategy selection
        template<typename G, typename U, typename V>
        inline constexpr _Choice_t<_St_uv> _Choice_uv = _Choose_uv<G, U, V>();
        
        template<typename G, typename UId, typename VId>
        inline constexpr _Choice_t<_St_uidvid> _Choice_uidvid = _Choose_uidvid<G, UId, VId>();
        
        struct _fn {
            // contains_edge(g, u, v) - both vertex descriptors
            template<typename G, vertex_descriptor_type U, vertex_descriptor_type V>
            [[nodiscard]] constexpr bool operator()(G&& g, const U& u, const V& v) const
                noexcept(_Choice_uv<G, U, V>._No_throw)
                requires (_Choice_uv<std::remove_cvref_t<G>, std::remove_cvref_t<U>, std::remove_cvref_t<V>>._Strategy != _St_uv::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _U = std::remove_cvref_t<U>;
                using _V = std::remove_cvref_t<V>;
                
                if constexpr (_Choice_uv<_G, _U, _V>._Strategy == _St_uv::_member) {
                    return g.contains_edge(u, v);
                } else if constexpr (_Choice_uv<_G, _U, _V>._Strategy == _St_uv::_adl) {
                    return contains_edge(g, u, v);
                } else if constexpr (_Choice_uv<_G, _U, _V>._Strategy == _St_uv::_default) {
                    // Default: iterate edges and check if target matches
                    auto target_vid = vertex_id(std::forward<G>(g), v);
                    auto edge_range = edges(std::forward<G>(g), u);
                    auto it = std::ranges::find_if(edge_range, [&](const auto& e) {
                        return static_cast<vertex_id_t<_G>>(target_id(std::forward<G>(g), e)) == static_cast<vertex_id_t<_G>>(target_vid);
                    });
                    return it != std::ranges::end(edge_range);
                }
            }
            
            // contains_edge(g, uid, vid) - both vertex IDs
            template<typename G, typename UId, typename VId>
                requires (!vertex_descriptor_type<UId>) && (!vertex_descriptor_type<VId>)
            [[nodiscard]] constexpr bool operator()(G&& g, const UId& uid, const VId& vid) const
                noexcept(_Choice_uidvid<G, UId, VId>._No_throw)
                requires (_Choice_uidvid<std::remove_cvref_t<G>, std::remove_cvref_t<UId>, std::remove_cvref_t<VId>>._Strategy != _St_uidvid::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _UId = std::remove_cvref_t<UId>;
                using _VId = std::remove_cvref_t<VId>;
                
                if constexpr (_Choice_uidvid<_G, _UId, _VId>._Strategy == _St_uidvid::_member) {
                    if constexpr (requires { typename _G::vertex_id_type; }) {
                        return g.contains_edge(static_cast<typename _G::vertex_id_type>(uid), static_cast<typename _G::vertex_id_type>(vid));
                    } else {
                        return g.contains_edge(uid, vid);
                    }
                } else if constexpr (_Choice_uidvid<_G, _UId, _VId>._Strategy == _St_uidvid::_adl) {
                    if constexpr (requires { typename _G::vertex_id_type; }) {
                        return contains_edge(g, static_cast<typename _G::vertex_id_type>(uid), static_cast<typename _G::vertex_id_type>(vid));
                    } else {
                        return contains_edge(g, uid, vid);
                    }
                } else if constexpr (_Choice_uidvid<_G, _UId, _VId>._Strategy == _St_uidvid::_default) {
                    // Default: find source vertex then iterate edges and check target
                    auto u = *find_vertex(std::forward<G>(g), uid);
                    auto edge_range = edges(std::forward<G>(g), u);
                    auto it = std::ranges::find_if(edge_range, [&](const auto& e) {
                        return target_id(std::forward<G>(g), e) == static_cast<decltype(target_id(std::forward<G>(g), e))>(vid);
                    });
                    return it != std::ranges::end(edge_range);
                }
            }
        };
    } // namespace _contains_edge

} // namespace _cpo_impls

// =============================================================================
// contains_edge(g, u, v) and contains_edge(g, uid, vid) - Public CPO instances
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for checking if an edge exists from source vertex u to target vertex v
     * 
     * Usage: 
     *   bool exists = graph::contains_edge(my_graph, u_descriptor, v_descriptor);
     *   bool exists = graph::contains_edge(my_graph, source_id, target_id);
     * 
     * Returns: true if edge exists, false otherwise
     */
    inline constexpr _cpo_impls::_contains_edge::_fn contains_edge{};
} // namespace _cpo_instances

namespace _cpo_impls {

    // =========================================================================
    // has_edge(g) CPO
    // =========================================================================
    
    namespace _has_edge {
        enum class _St { _none, _member, _adl, _default };
        
        // Check for g.has_edge() member function
        template<typename G>
        concept _has_member = 
            requires(const G& g) {
                { g.has_edge() } -> std::convertible_to<bool>;
            };
        
        // Check for ADL has_edge(g)
        template<typename G>
        concept _has_adl = 
            requires(const G& g) {
                { has_edge(g) } -> std::convertible_to<bool>;
            };
        
        // Check if we can iterate vertices and their edges (default)
        template<typename G>
        concept _has_default = requires(G& g) {
            { vertices(g) };
            requires std::ranges::forward_range<decltype(vertices(g))>;
        };
        
        template<typename G>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G>) {
                return {_St::_member, 
                        noexcept(std::declval<const G&>().has_edge())};
            } else if constexpr (_has_adl<G>) {
                return {_St::_adl, 
                        noexcept(has_edge(std::declval<const G&>()))};
            } else if constexpr (_has_default<G>) {
                // Default implementation is not noexcept as it may allocate
                return {_St::_default, false};
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>>();
            
        public:
            /**
             * @brief Check if the graph has any edges
             * 
             * Resolution order:
             * 1. g.has_edge() - Member function (highest priority)
             * 2. has_edge(g) - ADL (medium priority)
             * 3. Default implementation (lowest priority) - Iterates through vertices
             *    and checks if any vertex has a non-empty edges range
             * 
             * The default implementation:
             * - Iterates through all vertices using vertices(g)
             * - For each vertex u, gets edges(g, u)
             * - Returns true when the first non-empty edge range is found
             * - Returns false if all vertices have empty edge ranges
             * 
             * @tparam G Graph type
             * @param g Graph container
             * @return true if the graph has at least one edge, false otherwise
             */
            template<typename G>
            [[nodiscard]] constexpr bool operator()(G&& g) const
                noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                
                if constexpr (_Choice<_G>._Strategy == _St::_member) {
                    return g.has_edge();
                } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                    return has_edge(g);
                } else if constexpr (_Choice<_G>._Strategy == _St::_default) {
                    // Default implementation: check if any vertex has edges
                    auto vertex_range = vertices(std::forward<G>(g));
                    auto it = std::ranges::find_if(vertex_range, [&](const auto& u) {
                        auto edge_range = edges(std::forward<G>(g), u);
                        return !std::ranges::empty(edge_range);
                    });
                    return it != std::ranges::end(vertex_range);
                }
            }
        };
    } // namespace _has_edge

} // namespace _cpo_impls

// =============================================================================
// has_edge(g) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for checking if the graph has any edges
     * 
     * Usage: 
     *   bool has_edges = graph::has_edge(my_graph);
     * 
     * Returns: true if graph has at least one edge, false otherwise
     */
    inline constexpr _cpo_impls::_has_edge::_fn has_edge{};
} // namespace _cpo_instances

namespace _cpo_impls {

    // =========================================================================
    // vertex_value(g, u) CPO
    // =========================================================================
    
    namespace _vertex_value {
        enum class _St { _none, _member, _adl, _default };
        
        // Check for g.vertex_value(u) member function
        // Note: Uses G (not G&) to preserve const qualification
        template<typename G, typename U>
        concept _has_member = requires(G g, const U& u) {
            { g.vertex_value(u) };
        };
        
        // Check for ADL vertex_value(g, u)
        // Note: Uses G (not G&) to preserve const qualification
        template<typename G, typename U>
        concept _has_adl = requires(G g, const U& u) {
            { vertex_value(g, u) };
        };
        
        // Check if we can use default: u.inner_value(g)
        // Note: Uses G (not G&) to preserve const qualification
        template<typename G, typename U>
        concept _has_default = 
            is_vertex_descriptor_v<std::remove_cvref_t<U>> &&
            requires(G g, const U& u) {
                { u.inner_value(g) };
            };
        
        template<typename G, typename U>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G, U>) {
                return {_St::_member, 
                        noexcept(std::declval<G>().vertex_value(std::declval<const U&>()))};
            } else if constexpr (_has_adl<G, U>) {
                return {_St::_adl, 
                        noexcept(vertex_value(std::declval<G>(), std::declval<const U&>()))};
            } else if constexpr (_has_default<G, U>) {
                return {_St::_default, 
                        noexcept(std::declval<const U&>().inner_value(std::declval<G>()))};
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G, typename U>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>, std::remove_cvref_t<U>>();
            
        public:
            /**
             * @brief Get the user-defined value associated with a vertex
             * 
             * Resolution order:
             * 1. g.vertex_value(u) - Member function (highest priority)
             * 2. vertex_value(g, u) - ADL (high priority)
             * 3. u.inner_value(g) - Default using descriptor's inner_value (lowest priority)
             * 
             * The default implementation:
             * - Uses u.inner_value(g) which returns the actual vertex data
             * - For random-access containers (vector): returns container[index]
             * - For associative containers (map): returns the .second value
             * - For bidirectional containers: returns the dereferenced value
             * 
             * This provides access to user-defined vertex properties/data stored in the graph.
             * 
             * @tparam G Graph type
             * @tparam U Vertex descriptor type (constrained to be a vertex_descriptor_type)
             * @param g Graph container
             * @param u Vertex descriptor
             * @return Reference to the vertex value/data
             */
            template<typename G, vertex_descriptor_type U>
            [[nodiscard]] constexpr decltype(auto) operator()(G&& g, const U& u) const
                noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _U = std::remove_cvref_t<U>;
                
                if constexpr (_Choice<_G, _U>._Strategy == _St::_member) {
                    return g.vertex_value(u);
                } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl) {
                    return vertex_value(g, u);
                } else if constexpr (_Choice<_G, _U>._Strategy == _St::_default) {
                    return u.inner_value(std::forward<G>(g));
                }
            }
        };
    } // namespace _vertex_value

} // namespace _cpo_impls

// =============================================================================
// vertex_value(g, u) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting the user-defined value associated with a vertex
     * 
     * Usage: 
     *   auto& value = graph::vertex_value(my_graph, vertex_descriptor);
     * 
     * Returns: Reference to the vertex value/data
     */
    inline constexpr _cpo_impls::_vertex_value::_fn vertex_value{};
} // namespace _cpo_instances

namespace _cpo_impls {

    // =========================================================================
    // graph_value(g) CPO
    // =========================================================================
    
    namespace _graph_value {
        enum class _St { _none, _member, _adl };
        
        // Check for g.graph_value() member function
        template<typename G>
        concept _has_member = requires(G&& g) {
            { std::forward<G>(g).graph_value() };
        };
        
        // Check for ADL graph_value(g)
        template<typename G>
        concept _has_adl = requires(G&& g) {
            { graph_value(std::forward<G>(g)) };
        };
        
        template<typename G>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G>) {
                return {_St::_member, 
                        noexcept(std::declval<G>().graph_value())};
            } else if constexpr (_has_adl<G>) {
                return {_St::_adl, 
                        noexcept(graph_value(std::declval<G>()))};
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>>();
            
        public:
            /**
             * @brief Get the user-defined value associated with the graph
             * 
             * Resolution order:
             * 1. g.graph_value() - Member function (highest priority)
             * 2. graph_value(g) - ADL (lowest priority)
             * 
             * There is no default implementation. If neither member nor ADL is found,
             * a compile-time error will occur due to the requires constraint.
             * 
             * This provides access to user-defined graph-level properties/metadata
             * stored in the graph container (e.g., name, creation date, statistics).
             * 
             * @tparam G Graph type
             * @param g Graph container
             * @return Reference to the graph value/properties (or by-value if custom implementation returns by-value)
             */
            template<typename G>
            [[nodiscard]] constexpr decltype(auto) operator()(G&& g) const
                noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                
                if constexpr (_Choice<_G>._Strategy == _St::_member) {
                    return std::forward<G>(g).graph_value();
                } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                    return graph_value(std::forward<G>(g));
                }
            }
        };
    } // namespace _graph_value

} // namespace _cpo_impls

// =============================================================================
// graph_value(g) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting the user-defined value associated with the graph
     * 
     * Usage: 
     *   auto& value = graph::graph_value(my_graph);
     * 
     * Returns: Reference to the graph value/metadata (or by-value if custom)
     * 
     * Note: No default implementation - requires either member function or ADL
     */
    inline constexpr _cpo_impls::_graph_value::_fn graph_value{};
} // namespace _cpo_instances

namespace _cpo_impls {

    // =========================================================================
    // source(g, uv) CPO - Get source vertex descriptor from an edge
    // =========================================================================
    
    namespace _source {
        enum class _St { _none, _member, _adl, _descriptor, _default };
        
        // Use the public CPO instances from graph namespace (shared edge CPOs)
        using _cpo_instances::find_vertex;
        using graph::source_id;
        
        // Check for g.source(uv) member function
        // Accepts either edge_descriptor or underlying edge value type
        template<typename G, typename E>
        concept _has_member = 
            requires(G& g, const E& uv) {
                { g.source(uv) };
            };
        
        // Check for ADL source(g, uv)
        // Accepts either edge_descriptor or underlying edge value type
        template<typename G, typename E>
        concept _has_adl = 
            requires(G& g, const E& uv) {
                { source(g, uv) };
            };
        
        // Check if edge descriptor has source() member
        template<typename E>
        concept _has_descriptor = is_edge_descriptor_v<std::remove_cvref_t<E>> &&
            requires(const E& uv) {
                { uv.source() };
            };
        
        // Check if default implementation is available
        // Requires source_id(g, uv) and find_vertex(g, id) to work
        template<typename G, typename E>
        concept _has_default = 
            requires(G& g, const E& uv) {
                { source_id(g, uv) } -> std::convertible_to<decltype(source_id(g, uv))>;
                { find_vertex(g, source_id(g, uv)) };
            };
        
        template<typename G, typename E>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G, E>) {
                return {_St::_member, 
                        noexcept(std::declval<G&>().source(std::declval<const E&>()))};
            } else if constexpr (_has_adl<G, E>) {
                return {_St::_adl, 
                        noexcept(source(std::declval<G&>(), std::declval<const E&>()))};
            } else if constexpr (_has_descriptor<E>) {
                return {_St::_descriptor,
                        noexcept(std::declval<const E&>().source())};
            } else if constexpr (_has_default<G, E>) {
                return {_St::_default,
                        noexcept(find_vertex(std::declval<G&>(), source_id(std::declval<G&>(), std::declval<const E&>())))};
            } else {
                return {_St::_none, false};
            }
        }
        
        // Helper to convert result to vertex descriptor if needed
        // Supports two cases:
        // 1. Result is already a vertex_descriptor -> return as-is
        // 2. Result is an iterator (to vertex_descriptor_view) -> dereference to get vertex_descriptor
        template<typename G, typename Result>
        [[nodiscard]] constexpr auto _to_vertex_descriptor(G&&, Result&& result) noexcept {
            using ResultType = std::remove_cvref_t<Result>;
            if constexpr (is_vertex_descriptor_v<ResultType>) {
                return std::forward<Result>(result);
            } else {
                return *std::forward<Result>(result);
            }
        }
        
        class _fn {
        private:
            template<typename G, typename E>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>, std::remove_cvref_t<E>>();
            
        public:
            /**
             * @brief Get source vertex descriptor from an edge
             * 
             * Resolution order:
             * 1. g.source(uv) - Graph member function (highest priority)
             *    - May return vertex_descriptor or vertex_iterator (auto-converted)
             * 2. source(g, uv) - ADL (high priority)
             *    - May return vertex_descriptor or vertex_iterator (auto-converted)
             * 3. uv.source() - Edge descriptor's source() member (medium priority)
             *    - Returns the stored source vertex descriptor directly
             * 4. *find_vertex(g, source_id(g, uv)) - Default (lowest priority)
             *    - Uses source_id to get ID, then find_vertex to locate vertex
             * 
             * Custom implementations (member/ADL) can return:
             * - vertex_descriptor directly (vertex_t<G>) - used as-is
             * - vertex_iterator (iterator to vertices) - dereferenced to get descriptor
             * 
             * The descriptor implementation (tier 3) works for any edge_descriptor that
             * stores its source vertex descriptor, returning it directly without lookup.
             * 
             * The default implementation (tier 4) works for any graph that supports:
             * - source_id(g, uv) to get the source vertex ID
             * - find_vertex(g, id) to find a vertex by ID
             * 
             * @tparam G Graph type
             * @tparam E Edge descriptor type (constrained to be an edge_descriptor_type)
             * @param g Graph container
             * @param uv Edge descriptor (must be edge_t<G> - the edge descriptor type for the graph)
             * @return Vertex descriptor for the source vertex (vertex_t<G>)
             */
            template<typename G, edge_descriptor_type E>
            [[nodiscard]] constexpr auto operator()(G&& g, const E& uv) const
                noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._No_throw)
                -> decltype(auto)
                requires (_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _E = std::remove_cvref_t<E>;
                
                if constexpr (_Choice<_G, _E>._Strategy == _St::_member) {
                    // Graph member function - may return vertex_descriptor or iterator
                    return _to_vertex_descriptor(g, g.source(uv));
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl) {
                    // ADL - may return vertex_descriptor or iterator
                    return _to_vertex_descriptor(g, source(g, uv));
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_descriptor) {
                    // Edge descriptor's source() returns vertex_descriptor directly
                    return uv.source();
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_default) {
                    // Default: use source_id + find_vertex
                    return *find_vertex(std::forward<G>(g), source_id(std::forward<G>(g), uv));
                }
            }
        };
    } // namespace _source

} // namespace _cpo_impls

// =============================================================================
// source(g, uv) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting source vertex descriptor from an edge
     * 
     * Usage: auto source_vertex = graph::source(my_graph, edge_descriptor);
     * 
     * Returns: Vertex descriptor (vertex_iterator_t<G>) for the source vertex
     * 
     * Note: No default implementation. Must be provided by graph types that support
     *       sourced edges (e.g., bidirectional graphs, edge lists).
     */
    inline constexpr _cpo_impls::_source::_fn source{};
} // namespace _cpo_instances

namespace _cpo_impls {

    // =========================================================================
    // partition_id(g, u) CPO - Get partition ID for a vertex
    // =========================================================================
    
    namespace _partition_id {
        enum class _St { _none, _member, _adl, _default };
        
        // Check for g.partition_id(u) member function
        // Note: Uses G (not G&) to preserve const qualification
        template<typename G, typename U>
        concept _has_member = 
            requires(G g, const U& u) {
                { g.partition_id(u) };
            };
        
        // Check for ADL partition_id(g, u)
        // Note: Uses G (not G&) to preserve const qualification
        template<typename G, typename U>
        concept _has_adl = 
            requires(G g, const U& u) {
                { partition_id(g, u) };
            };
        
        // Default always available for vertex descriptors
        template<typename U>
        concept _has_default = is_vertex_descriptor_v<std::remove_cvref_t<U>>;
        
        template<typename G, typename U>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G, U>) {
                return {_St::_member, 
                        noexcept(std::declval<G>().partition_id(std::declval<const U&>()))};
            } else if constexpr (_has_adl<G, U>) {
                return {_St::_adl, 
                        noexcept(partition_id(std::declval<G>(), std::declval<const U&>()))};
            } else if constexpr (_has_default<U>) {
                return {_St::_default, true}; // Default is noexcept
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G, typename U>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>, std::remove_cvref_t<U>>();
            
        public:
            /**
             * @brief Get partition ID for a vertex
             * 
             * Resolution order:
             * 1. g.partition_id(u) - Member function (highest priority)
             * 2. partition_id(g, u) - ADL (medium priority)
             * 3. Default: returns 0 (lowest priority)
             * 
             * The default implementation assumes a single partition where all vertices
             * belong to partition 0. This is suitable for:
             * - Non-partitioned graphs
             * - Graphs without explicit partition information
             * - Single-process/single-node graph algorithms
             * 
             * Custom implementations can provide:
             * - Multi-partition support for distributed graphs
             * - Graph coloring/partitioning algorithms
             * - NUMA-aware partitioning
             * - Load-balancing partitions
             * 
             * @tparam G Graph type
             * @tparam U Vertex descriptor type (constrained to be a vertex_descriptor_type)
             * @param g Graph container
             * @param u Vertex descriptor (must be vertex_t<G>)
             * @return Partition ID (integral type, default 0)
             */
            template<typename G, vertex_descriptor_type U>
            [[nodiscard]] constexpr auto operator()(G&& g, const U& u) const
                noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._No_throw)
                -> decltype(auto)
                requires (_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<U>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _U = std::remove_cvref_t<U>;
                
                if constexpr (_Choice<_G, _U>._Strategy == _St::_member) {
                    return g.partition_id(u);
                } else if constexpr (_Choice<_G, _U>._Strategy == _St::_adl) {
                    return partition_id(g, u);
                } else if constexpr (_Choice<_G, _U>._Strategy == _St::_default) {
                    // Default: single partition, all vertices in partition 0
                    return 0;
                }
            }
        };
    } // namespace _partition_id

} // namespace _cpo_impls

// =============================================================================
// partition_id(g, u) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting partition ID for a vertex
     * 
     * Usage: auto pid = graph::partition_id(my_graph, vertex_descriptor);
     * 
     * Returns: Partition ID (default 0 for single-partition graphs)
     */
    inline constexpr _cpo_impls::_partition_id::_fn partition_id{};
} // namespace _cpo_instances

namespace _cpo_impls {

    // =========================================================================
    // num_partitions(g) CPO - Get number of partitions in the graph
    // =========================================================================
    
    namespace _num_partitions {
        enum class _St { _none, _member, _adl, _default };
        
        // Check for g.num_partitions() member function
        template<typename G>
        concept _has_member = 
            requires(const G& g) {
                { g.num_partitions() } -> std::integral;
            };
        
        // Check for ADL num_partitions(g)
        template<typename G>
        concept _has_adl = 
            requires(const G& g) {
                { num_partitions(g) } -> std::integral;
            };
        
        // Default always available - returns 1 (single partition)
        template<typename G>
        concept _has_default = true;
        
        template<typename G>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G>) {
                return {_St::_member, 
                        noexcept(std::declval<const G&>().num_partitions())};
            } else if constexpr (_has_adl<G>) {
                return {_St::_adl, 
                        noexcept(num_partitions(std::declval<const G&>()))};
            } else if constexpr (_has_default<G>) {
                return {_St::_default, true}; // Default is noexcept
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>>();
            
        public:
            /**
             * @brief Get the number of partitions in the graph
             * 
             * Resolution order:
             * 1. g.num_partitions() - Member function (highest priority)
             * 2. num_partitions(g) - ADL (medium priority)
             * 3. Default: returns 1 (lowest priority)
             * 
             * The default implementation assumes a single partition where all vertices
             * belong to partition 0. This is suitable for:
             * - Non-partitioned graphs
             * - Graphs without explicit partition information
             * - Single-process/single-node graph algorithms
             * 
             * Custom implementations can provide:
             * - Multi-partition support for distributed graphs
             * - Dynamic partition count based on graph structure
             * - Graph coloring results
             * - NUMA-aware partition counts
             * 
             * Relationship to partition_id(g, u):
             * - partition_id returns 0 by default (single partition)
             * - num_partitions returns 1 by default (one partition exists)
             * - For multi-partition graphs: partition_id values should be in range [0, num_partitions)
             * 
             * @tparam G Graph type
             * @param g Graph container
             * @return Number of partitions in the graph (integral type, default 1)
             */
            template<typename G>
            [[nodiscard]] constexpr auto operator()(const G& g) const
                noexcept(_Choice<std::remove_cvref_t<G>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                
                if constexpr (_Choice<_G>._Strategy == _St::_member) {
                    return g.num_partitions();
                } else if constexpr (_Choice<_G>._Strategy == _St::_adl) {
                    return num_partitions(g);
                } else if constexpr (_Choice<_G>._Strategy == _St::_default) {
                    // Default: single partition
                    return 1;
                }
            }
        };
    } // namespace _num_partitions

} // namespace _cpo_impls

// =============================================================================
// num_partitions(g) - Public CPO instance
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting the number of partitions in the graph
     * 
     * Usage: auto count = graph::num_partitions(my_graph);
     * 
     * Returns: Number of partitions (default 1 for single-partition graphs)
     */
    inline constexpr _cpo_impls::_num_partitions::_fn num_partitions{};
    
    // Import shared edge CPOs from graph::detail
    using graph::target_id;
    using graph::source_id;
    using graph::edge_value;
} // namespace _cpo_instances

} // namespace graph::adj_list
