/**
 * @file cpo_common.hpp
 * @brief Common types and utilities for customization point objects (CPOs)
 * 
 * This file contains shared infrastructure used by all graph CPOs,
 * including the _Choice_t pattern struct that caches strategy selection
 * and noexcept specifications at compile time.
 */

#pragma once

namespace graph::detail {

namespace _cpo_impls {
    /**
     * @brief Shared choice struct for all graph CPOs
     * 
     * Used to cache both the strategy (which customization path) and
     * the noexcept specification at compile time. This struct is used
     * in the MSVC-style CPO pattern to avoid repeated trait evaluation.
     * 
     * @tparam _Ty Strategy enum type (typically an enum class)
     * 
     * Usage:
     * @code
     * namespace _my_cpo {
     *     enum class _St { _none, _member, _adl, _default };
     *     
     *     template<typename T>
     *     [[nodiscard]] consteval _Choice_t<_St> _Choose() noexcept {
     *         if constexpr (...) {
     *             return {_St::_member, noexcept(...)};
     *         }
     *         // ...
     *     }
     *     
     *     class _fn {
     *     private:
     *         template<typename T>
     *         static constexpr _Choice_t<_St> _Choice = _Choose<T>();
     *     public:
     *         // ...
     *     };
     * }
     * @endcode
     */
    template<typename _Ty>
    struct _Choice_t {
        _Ty _Strategy = _Ty{};   ///< The selected strategy enum value
        bool _No_throw = false;  ///< Whether the selected path is noexcept
    };

} // namespace _cpo_impls

} // namespace graph::detail
