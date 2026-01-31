#pragma once

#include <type_traits>
#include <concepts>
#include <utility>

namespace graph::edge_list {

namespace detail {
    // Empty type for void value optimization
    struct empty_value {
        constexpr auto operator<=>(const empty_value&) const noexcept = default;
    };
}

/**
 * @brief Lightweight edge descriptor for edge lists
 * 
 * This descriptor stores source and target vertex IDs and optionally an edge value.
 * When EV is void, the value member is optimized away using [[no_unique_address]].
 * 
 * @tparam VId Vertex ID type
 * @tparam EV Edge value type (void for edges without values)
 */
template<typename VId, typename EV = void>
struct edge_descriptor {
    using vertex_id_type = VId;
    using edge_value_type = EV;
    
    VId source_id_;
    VId target_id_;
    [[no_unique_address]] std::conditional_t<std::is_void_v<EV>, 
        detail::empty_value, EV> value_;
    
    // Default constructor
    constexpr edge_descriptor() = default;
    
    // Constructor without value (for void EV)
    constexpr edge_descriptor(VId src, VId tgt) 
        requires std::is_void_v<EV>
        : source_id_(src), target_id_(tgt), value_() {}
    
    // Constructor with value (for non-void EV) - use template to avoid instantiation with void
    template<typename E = EV>
        requires (!std::is_void_v<E>)
    constexpr edge_descriptor(VId src, VId tgt, E val) 
        : source_id_(src), target_id_(tgt), value_(std::move(val)) {}
    
    // Copy constructor
    constexpr edge_descriptor(const edge_descriptor&) = default;
    constexpr edge_descriptor& operator=(const edge_descriptor&) = default;
    
    // Move constructor
    constexpr edge_descriptor(edge_descriptor&&) noexcept = default;
    constexpr edge_descriptor& operator=(edge_descriptor&&) noexcept = default;
    
    // Accessors - return by const reference to avoid copying non-trivial types
    [[nodiscard]] constexpr const VId& source_id() const noexcept { 
        return source_id_; 
    }
    
    [[nodiscard]] constexpr const VId& target_id() const noexcept { 
        return target_id_; 
    }
    
    // Value accessors (only for non-void EV) - use template to avoid forming reference to void
    template<typename E = EV>
        requires (!std::is_void_v<E>)
    [[nodiscard]] constexpr const E& value() const noexcept { 
        return value_; 
    }
    
    template<typename E = EV>
        requires (!std::is_void_v<E>)
    [[nodiscard]] constexpr E& value() noexcept { 
        return value_; 
    }
    
    // Comparison operators
    constexpr bool operator==(const edge_descriptor&) const noexcept = default;
    constexpr auto operator<=>(const edge_descriptor&) const noexcept = default;
};

// Deduction guides
template<typename VId>
edge_descriptor(VId, VId) -> edge_descriptor<VId, void>;

template<typename VId, typename EV>
edge_descriptor(VId, VId, EV) -> edge_descriptor<VId, EV>;

} // namespace graph::edge_list
