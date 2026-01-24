/**
 * @file edge_descriptor.hpp
 * @brief Edge descriptor template for graph edges
 */

#pragma once

#include "vertex_descriptor.hpp"
#include <compare>
#include <functional>
#include <numeric>

namespace graph::adj_list {

/**
 * @brief Descriptor for edges in a graph
 * 
 * Provides a lightweight, type-safe handle to edges stored in various container types.
 * Maintains both the edge location and the source vertex descriptor.
 * 
 * @tparam EdgeIter Iterator type of the underlying edge container
 * @tparam VertexIter Iterator type of the vertex container
 */
template<edge_iterator EdgeIter, vertex_iterator VertexIter>
class edge_descriptor {
public:
    using edge_iterator_type = EdgeIter;
    using vertex_iterator_type = VertexIter;
    using vertex_desc = vertex_descriptor<VertexIter>;
    
    // Conditional storage type for edge: size_t for random access, iterator for forward
    using edge_storage_type = std::conditional_t<
        std::random_access_iterator<EdgeIter>,
        std::size_t,
        EdgeIter
    >;
    
    // Default constructor
    constexpr edge_descriptor() noexcept 
        requires std::default_initializable<edge_storage_type> && 
                 std::default_initializable<vertex_desc>
        : edge_storage_{}, source_{} {}
    
    // Constructor from edge storage and source vertex descriptor
    constexpr edge_descriptor(edge_storage_type edge_val, vertex_desc source) noexcept
        : edge_storage_(edge_val), source_(source) {}
    
    /**
     * @brief Get the underlying edge storage value (index or iterator)
     * @return The stored edge index (for random access) or iterator (for forward)
     */
    [[nodiscard]] constexpr edge_storage_type value() const noexcept {
        return edge_storage_;
    }
    
    /**
     * @brief Get the source vertex descriptor
     * @return The vertex descriptor representing the source of this edge
     */
    [[nodiscard]] constexpr vertex_desc source() const noexcept {
        return source_;
    }
    
    /**
     * @brief Get the source vertex ID
     * @return The vertex ID of the source vertex
     * 
     * Extracts the ID from the stored source vertex descriptor.
     * 
     * @note Current implementation returns by value (auto) which is suitable for
     * integral vertex IDs. When non-trivial vertex ID types (e.g., std::string)
     * are supported, this method should:
     * 1. Change return type to decltype(auto) for reference semantics
     * 2. This delegates to vertex_descriptor::vertex_id() which will also need updating
     * See descriptor.md "Lambda Reference Binding Issues" section for details.
     */
    [[nodiscard]] constexpr auto source_id() const noexcept {
        return source_.vertex_id();
    }
    
    /**
     * @brief Get the target vertex ID from the edge data
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return The target vertex identifier extracted from the edge
     * 
     * For random access iterators, uses the stored index to access the container.
     * For forward/bidirectional iterators, dereferences the stored iterator.
     * 
     * The vertex_data parameter varies by graph structure:
     * - vov-style: vertex object with .edges() method
     * - map-based: std::pair<VId, vertex_type> where .second is the vertex
     * - raw adjacency: the edge container directly
     * 
     * Edge data extraction:
     * - Simple integral types: returns the value directly as target ID
     * - Pair-like types: returns .first as target ID
     * - Tuple-like types: returns std::get<0> as target ID
     * 
     * @note Current implementation returns by value (auto) which is suitable for
     * integral vertex IDs. When non-trivial vertex ID types (e.g., std::string)
     * are supported, this method should:
     * 1. Change return type to decltype(auto) for reference semantics
     * 2. Replace lambda-based extraction with direct if constexpr branches
     * 3. Wrap return expressions in parentheses: return (edge_val.first);
     * See descriptor.md "Lambda Reference Binding Issues" section for details.
     */
    template<typename VertexData>
    [[nodiscard]] constexpr auto target_id(const VertexData& vertex_data) const noexcept {
        using edge_value_type = typename std::iterator_traits<EdgeIter>::value_type;
        
        // Extract the actual edge container from vertex_data:
        // 1. If vertex_data has .edges() method, use that (for vov-style vertex)
        // 2. If vertex_data is pair-like (from map iterator dereference):
        //    a. If .second has .edges(), use .second.edges() (for map + vector/list edges)
        //    b. Otherwise use .second directly (for simpler edge storage)
        // 3. Otherwise use vertex_data as-is (raw edge container)
        const auto& edge_container = [&]() -> decltype(auto) {
            if constexpr (requires { vertex_data.edges(); }) {
                return vertex_data.edges();
            } else if constexpr (requires { vertex_data.first; vertex_data.second; }) {
                // For map-based vertices, .second is the vertex
                // Check if the vertex has .edges() method
                if constexpr (requires { vertex_data.second.edges(); }) {
                    return vertex_data.second.edges();
                } else {
                    return vertex_data.second;
                }
            } else {
                return vertex_data;
            }
        }();
        
        // Get the edge value from container (for random access) or iterator (for forward)
        const auto& edge_val = [&]() -> decltype(auto) {
            if constexpr (std::random_access_iterator<EdgeIter>) {
                return edge_container[edge_storage_];
            } else {
                return *edge_storage_;
            }
        }();
        
        // Extract target ID from edge value based on its type
        if constexpr (std::integral<edge_value_type>) {
            // Simple type: the value itself is the target ID
            return edge_val;
        }
        else if constexpr (requires { edge_val.second.target_id(); }) {
            // Map-based edge container: edge_val is pair<const VId, edge_type>
            // Access the edge through .second, then call target_id()
            return edge_val.second.target_id();
        }
        else if constexpr (requires { edge_val.target_id(); }) {
            // Edge object with target_id() member (e.g., dynamic_edge)
            return edge_val.target_id();
        }
        else if constexpr (requires { edge_val.first; }) {
            // Pair-like: .first is the target ID
            return edge_val.first;
        }
        else if constexpr (requires { std::get<0>(edge_val); }) {
            // Tuple-like: first element is the target ID
            return std::get<0>(edge_val);
        }
        else {
            // Fallback: assume the value itself is the target
            return edge_val;
        }
    }
    
    /**
     * @brief Get the underlying container value (the actual edge data)
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return Reference to the edge data from the container
     * 
     * For random access iterators, accesses container[index].
     * For forward/bidirectional iterators, dereferences the stored iterator.
     */
    template<typename VertexData>
    [[nodiscard]] constexpr decltype(auto) underlying_value(VertexData& vertex_data) const noexcept {
        // Extract the actual edge container from vertex_data (see target_id for details)
        if constexpr (requires { vertex_data.edges(); }) {
            auto& edge_container = vertex_data.edges();
            if constexpr (std::random_access_iterator<EdgeIter>) {
                return (edge_container[edge_storage_]);
            } else {
                return (*edge_storage_);
            }
        } else if constexpr (requires { vertex_data.first; vertex_data.second; }) {
            if constexpr (requires { vertex_data.second.edges(); }) {
                auto& edge_container = vertex_data.second.edges();
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            } else {
                auto& edge_container = vertex_data.second;
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
        } else {
            if constexpr (std::random_access_iterator<EdgeIter>) {
                return (vertex_data[edge_storage_]);
            } else {
                return (*edge_storage_);
            }
        }
    }
    
    /**
     * @brief Get the underlying container value (const version)
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return Const reference to the edge data from the container
     */
    template<typename VertexData>
    [[nodiscard]] constexpr decltype(auto) underlying_value(const VertexData& vertex_data) const noexcept {
        // Extract the actual edge container from vertex_data (see target_id for details)
        if constexpr (requires { vertex_data.edges(); }) {
            const auto& edge_container = vertex_data.edges();
            if constexpr (std::random_access_iterator<EdgeIter>) {
                return (edge_container[edge_storage_]);
            } else {
                return (*edge_storage_);
            }
        } else if constexpr (requires { vertex_data.first; vertex_data.second; }) {
            if constexpr (requires { vertex_data.second.edges(); }) {
                const auto& edge_container = vertex_data.second.edges();
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            } else {
                const auto& edge_container = vertex_data.second;
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
        } else {
            if constexpr (std::random_access_iterator<EdgeIter>) {
                return (vertex_data[edge_storage_]);
            } else {
                return (*edge_storage_);
            }
        }
    }
    
    /**
     * @brief Get the inner/property value (excluding the target ID)
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return Reference to the edge properties (excluding target vertex ID)
     * 
     * Behavior based on edge data type:
     * - Simple integral type (int): Returns the value itself (since it's just the target ID)
     * - Pair<target, property>: Returns .second (the property part)
     * - Tuple<target, prop1, prop2, ...>: Returns reference to tuple of remaining elements (excluding first)
     * - Struct/Custom type: Returns the whole value (user manages which fields are properties)
     * 
     * For tuples with 3+ elements, this creates a tuple of references to elements [1, N).
     */
    template<typename VertexData>
    [[nodiscard]] constexpr decltype(auto) inner_value(VertexData& vertex_data) const noexcept {
        using edge_value_type = typename std::iterator_traits<EdgeIter>::value_type;
        
        // Extract the actual edge container from vertex_data (see target_id for details)
        if constexpr (requires { vertex_data.edges(); }) {
            auto& edge_container = vertex_data.edges();
            // Simple type: just the target ID, return it (no separate property)
            if constexpr (std::integral<edge_value_type>) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
            // Pair-like: return .second (the property part)
            else if constexpr (requires { std::declval<edge_value_type>().first; std::declval<edge_value_type>().second; }) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_].second);
                } else {
                    return ((*edge_storage_).second);
                }
            }
            // Tuple-like: return tuple of references to elements [1, N)
            else if constexpr (requires { std::tuple_size<edge_value_type>::value; }) {
                constexpr size_t tuple_size = std::tuple_size<edge_value_type>::value;
                
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    auto& edge_val = edge_container[edge_storage_];
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_val));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_val));
                    }
                    else {
                        // 3+ elements: return a tuple of references to elements [1, N)
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_val)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                } else {
                    auto& edge_value = *edge_storage_;
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_value));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_value));
                    }
                    else {
                        // 3+ elements: return a tuple of references
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_value)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                }
            }
            // Custom struct/type: return the whole value
            else {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
        } else if constexpr (requires { vertex_data.first; vertex_data.second; }) {
            // Map pair case - .second is the vertex, get its edges
            auto& edge_container = vertex_data.second;
            // Simple type: just the target ID, return it (no separate property)
            if constexpr (std::integral<edge_value_type>) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
            // Pair-like: return .second (the property part)
            else if constexpr (requires { std::declval<edge_value_type>().first; std::declval<edge_value_type>().second; }) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_].second);
                } else {
                    return ((*edge_storage_).second);
                }
            }
            // Tuple-like: return tuple of references to elements [1, N)
            else if constexpr (requires { std::tuple_size<edge_value_type>::value; }) {
                constexpr size_t tuple_size = std::tuple_size<edge_value_type>::value;
                
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    auto& edge_val = edge_container[edge_storage_];
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_val));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_val));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_val)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                } else {
                    auto& edge_value = *edge_storage_;
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_value));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_value));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_value)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                }
            }
            // Custom struct/type: return the whole value
            else {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
        } else {
            // Raw edge container - no extraction needed
            // Simple type: just the target ID
            if constexpr (std::integral<edge_value_type>) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (vertex_data[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
            // Pair-like: return .second (the property part)
            else if constexpr (requires { std::declval<edge_value_type>().first; std::declval<edge_value_type>().second; }) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (vertex_data[edge_storage_].second);
                } else {
                    return ((*edge_storage_).second);
                }
            }
            // Tuple-like: return tuple of references to elements [1, N)
            else if constexpr (requires { std::tuple_size<edge_value_type>::value; }) {
                constexpr size_t tuple_size = std::tuple_size<edge_value_type>::value;
                
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    auto& edge_val = vertex_data[edge_storage_];
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_val));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_val));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_val)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                } else {
                    auto& edge_value = *edge_storage_;
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_value));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_value));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_value)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                }
            }
            // Custom struct/type: return the whole value
            else {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (vertex_data[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
        }
    }
    
    /**
     * @brief Get the inner/property value (const version)
     * @param vertex_data The vertex/edge data structure passed from the CPO
     * @return Const reference to the edge properties
     */
    template<typename VertexData>
    [[nodiscard]] constexpr decltype(auto) inner_value(const VertexData& vertex_data) const noexcept {
        using edge_value_type = typename std::iterator_traits<EdgeIter>::value_type;
        
        // Extract the actual edge container from vertex_data (see target_id for details)
        if constexpr (requires { vertex_data.edges(); }) {
            const auto& edge_container = vertex_data.edges();
            // Simple type: just the target ID, return it (no separate property)
            if constexpr (std::integral<edge_value_type>) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
            // Pair-like: return .second (the property part)
            else if constexpr (requires { std::declval<edge_value_type>().first; std::declval<edge_value_type>().second; }) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_].second);
                } else {
                    return ((*edge_storage_).second);
                }
            }
            // Tuple-like: return tuple of references to elements [1, N)
            else if constexpr (requires { std::tuple_size<edge_value_type>::value; }) {
                constexpr size_t tuple_size = std::tuple_size<edge_value_type>::value;
                
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    const auto& edge_val = edge_container[edge_storage_];
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_val));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_val));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_val)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                } else {
                    const auto& edge_value = *edge_storage_;
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_value));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_value));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_value)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                }
            }
            // Custom struct/type: return the whole value
            else {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
        } else if constexpr (requires { vertex_data.first; vertex_data.second; }) {
            // Map pair case - .second is the vertex, get its edges
            const auto& edge_container = vertex_data.second;
            // Simple type: just the target ID, return it (no separate property)
            if constexpr (std::integral<edge_value_type>) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
            // Pair-like: return .second (the property part)
            else if constexpr (requires { std::declval<edge_value_type>().first; std::declval<edge_value_type>().second; }) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_].second);
                } else {
                    return ((*edge_storage_).second);
                }
            }
            // Tuple-like: return tuple of references to elements [1, N)
            else if constexpr (requires { std::tuple_size<edge_value_type>::value; }) {
                constexpr size_t tuple_size = std::tuple_size<edge_value_type>::value;
                
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    const auto& edge_val = edge_container[edge_storage_];
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_val));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_val));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_val)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                } else {
                    const auto& edge_value = *edge_storage_;
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_value));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_value));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_value)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                }
            }
            // Custom struct/type: return the whole value
            else {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (edge_container[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
        } else {
            // Raw edge container - no extraction needed
            // Simple type: just the target ID
            if constexpr (std::integral<edge_value_type>) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (vertex_data[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
            // Pair-like: return .second (the property part)
            else if constexpr (requires { std::declval<edge_value_type>().first; std::declval<edge_value_type>().second; }) {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (vertex_data[edge_storage_].second);
                } else {
                    return ((*edge_storage_).second);
                }
            }
            // Tuple-like: return tuple of references to elements [1, N)
            else if constexpr (requires { std::tuple_size<edge_value_type>::value; }) {
                constexpr size_t tuple_size = std::tuple_size<edge_value_type>::value;
                
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    const auto& edge_val = vertex_data[edge_storage_];
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_val));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_val));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_val)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                } else {
                    const auto& edge_value = *edge_storage_;
                    if constexpr (tuple_size == 1) {
                        return (std::get<0>(edge_value));
                    }
                    else if constexpr (tuple_size == 2) {
                        return (std::get<1>(edge_value));
                    }
                    else {
                        return [&]<size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                            return std::forward_as_tuple(std::get<Is + 1>(edge_value)...);
                        }(std::make_index_sequence<tuple_size - 1>{});
                    }
                }
            }
            // Custom struct/type: return the whole value
            else {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return (vertex_data[edge_storage_]);
                } else {
                    return (*edge_storage_);
                }
            }
        }
    }
    
    // Pre-increment: advances edge position, keeps source unchanged
    constexpr edge_descriptor& operator++() noexcept {
        ++edge_storage_;
        return *this;
    }
    
    // Post-increment
    constexpr edge_descriptor operator++(int) noexcept {
        edge_descriptor tmp = *this;
        ++edge_storage_;
        return tmp;
    }
    
    // Comparison operators
    [[nodiscard]] auto operator<=>(const edge_descriptor&) const noexcept = default;
    [[nodiscard]] bool operator==(const edge_descriptor&) const noexcept = default;
    
private:
    edge_storage_type edge_storage_;
    vertex_desc source_;
};

} // namespace graph::adj_list

// Hash specialization for std::unordered containers
namespace std {
    template<graph::adj_list::edge_iterator EdgeIter, graph::adj_list::vertex_iterator VertexIter>
    struct hash<graph::adj_list::edge_descriptor<EdgeIter, VertexIter>> {
        [[nodiscard]] size_t operator()(const graph::adj_list::edge_descriptor<EdgeIter, VertexIter>& ed) const noexcept {
            // Combine hash of edge storage and source vertex
            size_t h1 = [&ed]() {
                if constexpr (std::random_access_iterator<EdgeIter>) {
                    return std::hash<std::size_t>{}(ed.value());
                } else {
                    // NOTE: For iterator-based edges, this uses the address of the referenced element.
                    // This assumes the iterator is dereferenceable (not end). Improving this
                    // requires a stable edge identity beyond the iterator itself.
                    return std::hash<std::size_t>{}(
                        reinterpret_cast<std::size_t>(&(*ed.value()))
                    );
                }
            }();
            
            size_t h2 = std::hash<graph::adj_list::vertex_descriptor<VertexIter>>{}(ed.source());
            
            // Combine hashes using a simple mixing function
            return h1 ^ (h2 << 1);
        }
    };
} // namespace std
