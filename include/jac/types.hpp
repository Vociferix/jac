#ifndef JAC_TYPES_HPP
#define JAC_TYPES_HPP

/// @file

#include <optional>
#include <variant>

namespace jac {

/// @brief Monostate type useful as a standin for `void`
struct void_t {
    constexpr void_t() = default;

    constexpr void_t(const void_t&) = default;

    constexpr void_t(void_t&&) = default;

    template <typename... Args>
    constexpr explicit void_t([[maybe_unused]] Args&&... args)
        : void_t() {}

    template <typename U, typename... Args>
    constexpr explicit void_t([[maybe_unused]] std::initializer_list<U> ilist, [[maybe_unused]] Args&&... args)
        : void_t() {}

    constexpr ~void_t() = default;

    constexpr void_t& operator=(const void_t&) = default;

    constexpr void_t& operator=(void_t&&) = default;

    constexpr operator std::monostate() const noexcept {
        return std::monostate();
    }

    template <typename T>
    constexpr explicit operator T() const {
        return T{};
    }

    constexpr operator void() const { } // NOLINT
};

/// @brief Monostate object representing an instance of `void`
static inline constexpr void_t void_v{};

/// @brief Type representing nothing, or none
struct null_t {
    constexpr null_t() noexcept = default;

    template <typename T>
    constexpr operator T*() const noexcept { return nullptr; }

    constexpr operator std::nullopt_t() const noexcept { return std::nullopt; }
};

/// @brief An instance of nothing, or none
static inline constexpr null_t null{};

}

#endif
