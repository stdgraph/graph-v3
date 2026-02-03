/**
 * @file edge_cpo.hpp
 * @brief Shared edge customization point objects (CPOs) for both adjacency lists and edge lists
 * 
 * This file contains CPO implementations for edge operations that must work identically
 * for both adjacency list graphs and edge list graphs:
 * - target_id(g, uv) - Get target vertex ID from an edge
 * - source_id(g, uv) - Get source vertex ID from an edge  
 * - edge_value(g, uv) - Get user-defined edge value/properties
 * 
 * These CPOs are shared because they operate on edges regardless of whether the edges
 * come from an adjacency list or an edge list. The resolution order supports multiple
 * edge representations including descriptors, info structs, and tuple-like types.
 */

#pragma once

#include <concepts>
#include <ranges>
#include <tuple>
#include "graph/detail/cpo_common.hpp"
#include "graph/adj_list/descriptor_traits.hpp"
#include "graph/edge_list/edge_list_traits.hpp"

namespace graph {

namespace _cpo_impls {

    using detail::_cpo_impls::_Choice_t;

    // =========================================================================
    // target_id(g, uv) CPO
    // =========================================================================
    
    namespace _target_id {
        enum class _St { 
            _none, 
            _native_edge_member, 
            _adl_descriptor, 
            _adj_list_descriptor,   // Tier 4: adj_list::edge_descriptor
            _edge_list_descriptor,  // Tier 5: edge_list::edge_descriptor
            _edge_info_member,      // Tier 6: edge_info data member
            _tuple_like             // Tier 7: tuple/pair
        };
        
        // Check if the underlying native edge type has target_id() member - highest priority
        // This checks uv.value()->target_id() where value() returns iterator to native edge
        template<typename G, typename E>
        concept _has_native_edge_member = adj_list::is_edge_descriptor_v<std::remove_cvref_t<E>> &&
            requires(G& g, const E& uv) {
                { (*uv.value()).target_id() };
            };
        
        // Check for ADL target_id(g, descriptor) - medium priority custom override
        // This allows customization by providing a free function that takes the descriptor
        // Accepts either edge_descriptor or underlying edge value type
        template<typename G, typename E>
        concept _has_adl_descriptor = 
            requires(const G& g, const E& uv) {
                { target_id(g, uv) };
            };
        
        // Check if adj_list descriptor has target_id() member (Tier 4) 
        // Note: edge_descriptor.target_id() requires the edge container, not the graph
        // underlying_value() gives us the vertex for vov or the edge container for raw adjacency lists
        // The edge_descriptor.target_id() handles extracting the target ID correctly in both cases
        template<typename G, typename E>
        concept _has_adj_list_descriptor = adj_list::is_edge_descriptor_v<std::remove_cvref_t<E>> &&
            requires(G& g, const E& uv) {
                { uv.source().underlying_value(g) };
                { uv.target_id(uv.source().underlying_value(g)) };
            };
        
        // Tier 5: Check if edge_list descriptor has target_id() member
        template<typename UV>
        concept _has_edge_list_descriptor = edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
            requires(const UV& uv) {
                { uv.target_id() };
            };
        
        // Tier 6: Check for edge_info-style direct data member access
        // Must NOT be a descriptor type (to avoid ambiguity with method calls)
        template<typename UV>
        concept _has_edge_info_member = 
            !adj_list::is_edge_descriptor_v<std::remove_cvref_t<UV>> &&
            !edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
            requires(const UV& uv) {
                uv.target_id;  // data member, not method
            } &&
            !requires(const UV& uv) {
                uv.target_id();  // exclude if it's callable (i.e., a method)
            };
        
        // Tier 7: Check for tuple-like edge (pair, tuple)
        // Must NOT be any descriptor type or have edge_info members
        template<typename UV>
        concept _is_tuple_like_edge = 
            !adj_list::is_edge_descriptor_v<std::remove_cvref_t<UV>> &&
            !edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
            !_has_edge_info_member<UV> &&
            requires {
                std::tuple_size<std::remove_cvref_t<UV>>::value;
            } &&
            requires(const UV& uv) {
                { std::get<0>(uv) };
                { std::get<1>(uv) };
            };
        
        template<typename G, typename E>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_native_edge_member<G, E>) {
                return {_St::_native_edge_member,
                        noexcept((*std::declval<const E&>().value()).target_id())};
            } else if constexpr (_has_adl_descriptor<G, E>) {
                return {_St::_adl_descriptor, 
                        noexcept(target_id(std::declval<const G&>(), std::declval<const E&>()))};
            } else if constexpr (_has_adj_list_descriptor<G, E>) {
                // Default to false (safe) since we have conditional logic
                return {_St::_adj_list_descriptor, false};
            } else if constexpr (_has_edge_list_descriptor<E>) {
                return {_St::_edge_list_descriptor,
                        noexcept(std::declval<const E&>().target_id())};
            } else if constexpr (_has_edge_info_member<E>) {
                return {_St::_edge_info_member,
                        noexcept(std::declval<const E&>().target_id)};
            } else if constexpr (_is_tuple_like_edge<E>) {
                return {_St::_tuple_like,
                        noexcept(std::get<1>(std::declval<const E&>()))};
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G, typename E>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>, std::remove_cvref_t<E>>();
            
        public:
            /**
             * @brief Get target vertex ID from an edge
             * 
             * Resolution order (seven-tier approach):
             * 1. (*uv.value()).target_id() - Native edge member function (highest priority)
             * 2. target_id(g, uv) - ADL with edge_descriptor
             * 3. uv.target_id(uv.source().underlying_value(g)) - adj_list::edge_descriptor (Tier 4)
             * 4. uv.target_id() - edge_list::edge_descriptor member (Tier 5)
             * 5. uv.target_id - edge_info data member (Tier 6)
             * 6. std::get<1>(uv) - tuple-like edge (Tier 7, lowest priority)
             * 
             * Where:
             * - uv must be edge_t<G> (the edge descriptor type for graph G) for tiers 1-4
             * - For tiers 5-7, uv can be edge_list descriptors, edge_info structs, or tuple-like types
             * - The native edge member function is called if the underlying edge type has target_id()
             * - ADL allows customization by providing a free function
             * 
             * Tiers 4-7 support:
             * - adj_list edge descriptors (existing adjacency list edges)
             * - edge_list descriptors (new edge list support)
             * - edge_info structs with direct data members
             * - tuple/pair representations (source, target, [value])
             * 
             * Edge data extraction (tier 4 default implementation):
             * - Simple integral type (int): Returns the value itself (the target ID)
             * - Pair<target, property>: Returns .first (the target ID)
             * - Tuple<target, prop1, ...>: Returns std::get<0> (the target ID)
             * - Custom struct/type: User provides custom extraction via member function or ADL
             * 
             * @tparam G Graph type
             * @tparam E Edge descriptor or edge type
             * @param g Graph container
             * @param uv Edge descriptor or edge
             * @return Target vertex identifier
             */
            template<typename G, typename E>
            [[nodiscard]] constexpr auto operator()(G& g, const E& uv) const
                noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._No_throw)
                -> decltype(auto)
                requires (_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _E = std::remove_cvref_t<E>;
                
                if constexpr (_Choice<_G, _E>._Strategy == _St::_native_edge_member) {
                    // Call target_id() member on the underlying native edge
                    return (*uv.value()).target_id();
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl_descriptor) {
                    return target_id(g, uv);
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adj_list_descriptor) {
                    // Default: use edge_descriptor.target_id() with vertex from underlying_value
                    // For vov: underlying_value gives vertex, edge_descriptor extracts from it
                    // For raw adjacency lists: underlying_value gives edge container directly
                    return uv.target_id(uv.source().underlying_value(g));
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_edge_list_descriptor) {
                    return uv.target_id();
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_edge_info_member) {
                    return uv.target_id;
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_tuple_like) {
                    return std::get<1>(uv);
                }
            }
        };
    } // namespace _target_id

    // =========================================================================
    // source_id(g, uv) CPO
    // =========================================================================
    
    namespace _source_id {
        enum class _St { 
            _none, 
            _native_edge_member, 
            _member, 
            _adl, 
            _adj_list_descriptor,   // Tier 4: adj_list::edge_descriptor
            _edge_list_descriptor,  // Tier 5: edge_list::edge_descriptor
            _edge_info_member,      // Tier 6: edge_info data member
            _tuple_like             // Tier 7: tuple/pair
        };
        
        // Check if the underlying native edge type has source_id() member - highest priority
        // This checks uv.value()->source_id() where value() returns iterator to native edge
        template<typename G, typename E>
        concept _has_native_edge_member = adj_list::is_edge_descriptor_v<std::remove_cvref_t<E>> &&
            requires(G& g, const E& uv) {
                { (*uv.value()).source_id() };
            };
        
        // Check for g.source_id(uv) member function
        // Note: Uses G (not G&) to preserve const qualification
        template<typename G, typename E>
        concept _has_member = requires(G g, const E& uv) {
            { g.source_id(uv) };
        };
        
        // Check for ADL source_id(g, uv)
        // Note: Uses G (not G&) to preserve const qualification
        template<typename G, typename E>
        concept _has_adl = requires(G g, const E& uv) {
            { source_id(g, uv) };
        };
        
        // Check if adj_list edge descriptor has source_id() member (Tier 4)
        template<typename E>
        concept _has_adj_list_descriptor = adj_list::is_edge_descriptor_v<std::remove_cvref_t<E>> &&
            requires(const E& uv) {
                { uv.source_id() };
            };
        
        // Tier 5: Check if edge_list descriptor has source_id() member
        template<typename UV>
        concept _has_edge_list_descriptor = edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
            requires(const UV& uv) {
                { uv.source_id() };
            };
        
        // Tier 6: Check for edge_info-style direct data member access
        // Must NOT be a descriptor type (to avoid ambiguity with method calls)
        template<typename UV>
        concept _has_edge_info_member = 
            !adj_list::is_edge_descriptor_v<std::remove_cvref_t<UV>> &&
            !edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
            requires(const UV& uv) {
                uv.source_id;  // data member, not method
            } &&
            !requires(const UV& uv) {
                uv.source_id();  // exclude if it's callable (i.e., a method)
            };
        
        // Tier 7: Check for tuple-like edge (pair, tuple)
        // Must NOT be any descriptor type or have edge_info members
        template<typename UV>
        concept _is_tuple_like_edge = 
            !adj_list::is_edge_descriptor_v<std::remove_cvref_t<UV>> &&
            !edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
            !_has_edge_info_member<UV> &&
            requires {
                std::tuple_size<std::remove_cvref_t<UV>>::value;
            } &&
            requires(const UV& uv) {
                { std::get<0>(uv) };
                { std::get<1>(uv) };
            };
        
        template<typename G, typename E>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_native_edge_member<G, E>) {
                return {_St::_native_edge_member,
                        noexcept((*std::declval<const E&>().value()).source_id())};
            } else if constexpr (_has_member<G, E>) {
                return {_St::_member, 
                        noexcept(std::declval<G>().source_id(std::declval<const E&>()))};
            } else if constexpr (_has_adl<G, E>) {
                return {_St::_adl, 
                        noexcept(source_id(std::declval<G>(), std::declval<const E&>()))};
            } else if constexpr (_has_adj_list_descriptor<E>) {
                return {_St::_adj_list_descriptor,
                        noexcept(std::declval<const E&>().source_id())};
            } else if constexpr (_has_edge_list_descriptor<E>) {
                return {_St::_edge_list_descriptor,
                        noexcept(std::declval<const E&>().source_id())};
            } else if constexpr (_has_edge_info_member<E>) {
                return {_St::_edge_info_member,
                        noexcept(std::declval<const E&>().source_id)};
            } else if constexpr (_is_tuple_like_edge<E>) {
                return {_St::_tuple_like,
                        noexcept(std::get<0>(std::declval<const E&>()))};
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G, typename E>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>, std::remove_cvref_t<E>>();
            
        public:
            /**
             * @brief Get the source vertex ID for an edge
             * 
             * Resolution order (seven-tier approach):
             * 1. (*uv.value()).source_id() - Native edge member function (highest priority)
             * 2. g.source_id(uv) - Graph member function
             * 3. source_id(g, uv) - ADL
             * 4. uv.source_id() - adj_list::edge_descriptor member (Tier 4)
             * 5. uv.source_id() - edge_list::edge_descriptor member (Tier 5)
             * 6. uv.source_id - edge_info data member (Tier 6)
             * 7. std::get<0>(uv) - tuple-like edge (Tier 7, lowest priority)
             * 
             * Where:
             * - uv must be edge_t<G> (the edge descriptor type for graph G)
             * - The native edge member function is called if the underlying edge type has source_id()
             * - ADL allows customization by providing a free function that takes the descriptor
             * - Tier 4-7 provide fallback implementations for various edge representations
             * 
             * Tiers 4-7 support:
             * - adj_list edge descriptors (existing adjacency list edges)
             * - edge_list descriptors (new edge list support)
             * - edge_info structs with direct data members
             * - tuple/pair representations (source, target, [value])
             * 
             * @tparam G Graph type
             * @tparam E Edge descriptor or edge type
             * @param g Graph container
             * @param uv Edge descriptor or edge
             * @return Source vertex ID (type depends on graph's vertex_id_t)
             */
            template<typename G, typename E>
            [[nodiscard]] constexpr auto operator()(G&& g, const E& uv) const
                noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _E = std::remove_cvref_t<E>;
                
                if constexpr (_Choice<_G, _E>._Strategy == _St::_native_edge_member) {
                    return (*uv.value()).source_id();
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_member) {
                    return g.source_id(uv);
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl) {
                    return source_id(g, uv);
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adj_list_descriptor) {
                    return uv.source_id();
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_edge_list_descriptor) {
                    return uv.source_id();
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_edge_info_member) {
                    return uv.source_id;
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_tuple_like) {
                    return std::get<0>(uv);
                }
            }
        };
    } // namespace _source_id

    // =========================================================================
    // edge_value(g, uv) CPO
    // =========================================================================
    
    namespace _edge_value {
        enum class _St { 
            _none, 
            _member, 
            _adl, 
            _value_fn, 
            _adj_list_descriptor,   // Tier 4: adj_list::edge_descriptor
            _edge_list_descriptor,  // Tier 5: edge_list::edge_descriptor
            _edge_info_member,      // Tier 6: edge_info data member
            _tuple_like             // Tier 7: tuple (3+ elements)
        };
        
        // Check for g.edge_value(uv) member function
        // Note: Uses G (not G&) to preserve const qualification
        template<typename G, typename E>
        concept _has_member = requires(G g, const E& uv) {
            { g.edge_value(uv) };
        };
        
        // Check for ADL edge_value(g, uv)
        // Note: Uses G (not G&) to preserve const qualification
        template<typename G, typename E>
        concept _has_adl = requires(G g, const E& uv) {
            { edge_value(g, uv) };
        };
        
        // Check for uv.value() member function (only for non-descriptor edge types)
        // Note: Uses E (not E&) to preserve const qualification
        template<typename G, typename E>
        concept _has_value_fn = 
            (!adj_list::is_edge_descriptor_v<std::remove_cvref_t<E>>) &&
            requires(const E& uv) {
                { uv.value() };
            };
        
        // Check if we can use adj_list descriptor default: uv.inner_value(v) where v = uv.source().underlying_value(g)
        // Note: Uses G (not G&) to preserve const qualification
        // underlying_value() gives us the vertex for vov or the edge container for raw adjacency lists
        // The edge_descriptor.inner_value() handles extracting properties correctly in both cases
        template<typename G, typename E>
        concept _has_adj_list_descriptor = 
            adj_list::is_edge_descriptor_v<std::remove_cvref_t<E>> &&
            requires(G g, const E& uv) {
                { uv.source().underlying_value(g) };
                { uv.inner_value(uv.source().underlying_value(g)) };
            };
        
        // Tier 5: Check if edge_list descriptor has value() member
        template<typename UV>
        concept _has_edge_list_descriptor = edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
            requires(const UV& uv) {
                { uv.value() };
            };
        
        // Tier 6: Check for edge_info-style direct data member access
        // Must NOT be a descriptor type (to avoid ambiguity with method calls)
        template<typename UV>
        concept _has_edge_info_member = 
            !adj_list::is_edge_descriptor_v<std::remove_cvref_t<UV>> &&
            !edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
            requires(const UV& uv) {
                uv.value;  // data member, not method
            } &&
            !requires(const UV& uv) {
                uv.value();  // exclude if it's callable (i.e., a method)
            };
        
        // Tier 7: Check for tuple-like edge (pair, tuple)
        // Must NOT be any descriptor type or have edge_info members
        // Note: For edge values, this would be std::get<2> (third element)
        template<typename UV>
        concept _is_tuple_like_edge = 
            !adj_list::is_edge_descriptor_v<std::remove_cvref_t<UV>> &&
            !edge_list::is_edge_list_descriptor_v<std::remove_cvref_t<UV>> &&
            !_has_edge_info_member<UV> &&
            requires {
                std::tuple_size<std::remove_cvref_t<UV>>::value;
                requires std::tuple_size<std::remove_cvref_t<UV>>::value >= 3;
            } &&
            requires(const UV& uv) {
                { std::get<0>(uv) };
                { std::get<1>(uv) };
                { std::get<2>(uv) };  // edge value is third element
            };
        
        template<typename G, typename E>
        [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
            if constexpr (_has_member<G, E>) {
                return {_St::_member, 
                        noexcept(std::declval<G>().edge_value(std::declval<const E&>()))};
            } else if constexpr (_has_adl<G, E>) {
                return {_St::_adl, 
                        noexcept(edge_value(std::declval<G>(), std::declval<const E&>()))};
            } else if constexpr (_has_value_fn<G, E>) {
                return {_St::_value_fn, 
                        noexcept(std::declval<const E&>().value())};
            } else if constexpr (_has_adj_list_descriptor<G, E>) {
                // Default to false (safe) since we have conditional logic
                return {_St::_adj_list_descriptor, false};
            } else if constexpr (_has_edge_list_descriptor<E>) {
                return {_St::_edge_list_descriptor,
                        noexcept(std::declval<const E&>().value())};
            } else if constexpr (_has_edge_info_member<E>) {
                return {_St::_edge_info_member,
                        noexcept(std::declval<const E&>().value)};
            } else if constexpr (_is_tuple_like_edge<E>) {
                return {_St::_tuple_like,
                        noexcept(std::get<2>(std::declval<const E&>()))};
            } else {
                return {_St::_none, false};
            }
        }
        
        class _fn {
        private:
            template<typename G, typename E>
            static constexpr _Choice_t<_St> _Choice = _Choose<std::remove_cvref_t<G>, std::remove_cvref_t<E>>();
            
        public:
            /**
             * @brief Get the user-defined value associated with an edge
             * 
             * Resolution order (seven-tier approach):
             * 1. g.edge_value(uv) - Member function (highest priority)
             * 2. edge_value(g, uv) - ADL
             * 3. uv.value() - Member function on edge
             * 4. uv.inner_value(edges) - adj_list::edge_descriptor (Tier 4)
             * 5. uv.value() - edge_list::edge_descriptor member (Tier 5)
             * 6. uv.value - edge_info data member (Tier 6)
             * 7. std::get<2>(uv) - tuple-like edge (Tier 7, lowest priority)
             * 
             * Where:
             * - uv must be edge_t<G> (the edge descriptor type for graph G) for tiers 1-4
             * - For tiers 5-7, uv can be edge_list descriptors, edge_info structs, or tuple-like types
             * 
             * Tiers 4-7 support:
             * - adj_list edge descriptors (existing adjacency list edges)
             * - edge_list descriptors (new edge list support)
             * - edge_info structs with direct data members
             * - tuple/triple representations (source, target, value)
             * 
             * The tier 4 default implementation:
             * - Uses uv.inner_value(edges) where edges = uv.source().underlying_value(g)
             * - For simple edges (int): returns the value itself (the target ID)
             * - For pair edges (target, weight): returns .second (the weight/property)
             * - For tuple edges (target, prop1, prop2, ...): returns tuple of properties [1, N)
             * - For custom edge types: returns the whole edge value
             * 
             * This provides access to user-defined edge properties/weights stored in the graph.
             * 
             * @tparam G Graph type
             * @tparam E Edge descriptor or edge type
             * @param g Graph container
             * @param uv Edge descriptor or edge
             * @return Reference to the edge value/properties (or by-value if custom implementation returns by-value)
             */
            template<typename G, typename E>
            [[nodiscard]] constexpr decltype(auto) operator()(G&& g, E&& uv) const
                noexcept(_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._No_throw)
                requires (_Choice<std::remove_cvref_t<G>, std::remove_cvref_t<E>>._Strategy != _St::_none)
            {
                using _G = std::remove_cvref_t<G>;
                using _E = std::remove_cvref_t<E>;
                
                if constexpr (_Choice<_G, _E>._Strategy == _St::_member) {
                    return g.edge_value(std::forward<E>(uv));
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adl) {
                    return edge_value(g, std::forward<E>(uv));
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_value_fn) {
                    return std::forward<E>(uv).value();
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_adj_list_descriptor) {
                    // Get vertex from underlying_value - works for both vov and raw adjacency lists
                    // For vov: gives vertex, edge_descriptor extracts properties from it
                    // For raw adjacency lists: gives edge container directly
                    return std::forward<E>(uv).inner_value(
                        std::forward<E>(uv).source().underlying_value(std::forward<G>(g))
                    );
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_edge_list_descriptor) {
                    return uv.value();
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_edge_info_member) {
                    return uv.value;
                } else if constexpr (_Choice<_G, _E>._Strategy == _St::_tuple_like) {
                    return std::get<2>(uv);
                }
            }
        };
    } // namespace _edge_value

} // namespace _cpo_impls

// =============================================================================
// Public CPO instances - in graph namespace for shared use
// =============================================================================

inline namespace _cpo_instances {
    /**
     * @brief CPO for getting target vertex ID from an edge
     * 
     * Usage:
     *   auto tid = graph::target_id(g, uv);
     * 
     * Works with both adjacency lists and edge lists.
     */
    inline constexpr _cpo_impls::_target_id::_fn target_id{};
    
    /**
     * @brief CPO for getting source vertex ID from an edge
     * 
     * Usage:
     *   auto sid = graph::source_id(g, uv);
     * 
     * Works with both adjacency lists and edge lists.
     */
    inline constexpr _cpo_impls::_source_id::_fn source_id{};
    
    /**
     * @brief CPO for getting user-defined edge value/properties
     * 
     * Usage:
     *   auto& value = graph::edge_value(g, uv);
     * 
     * Works with both adjacency lists and edge lists.
     */
    inline constexpr _cpo_impls::_edge_value::_fn edge_value{};
    
} // namespace _cpo_instances

} // namespace graph
