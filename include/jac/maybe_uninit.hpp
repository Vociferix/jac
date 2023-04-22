#ifndef JAC_MAYBE_UNINIT_HPP
#define JAC_MAYBE_UNINIT_HPP

/// @file

#include <memory>

#include <jac/types.hpp>

namespace jac {

/// @brief A constexpr friendly type wrapper for manual construction and destruction
template <typename T>
class maybe_uninit {
  private:
    union {
        void_t uninit_;
        T value_;
    };

  public:
    using value_type = T;
    using reference = T&;
    using const_reference = std::add_const_t<T>&;
    using rvalue_reference = T&&;
    using const_rvalue_reference = std::add_const_t<T>&&;
    using pointer = T*;
    using const_pointer = std::add_const_t<T>*;

    constexpr maybe_uninit() noexcept : uninit_{} {}

    template <typename... Args>
    constexpr explicit(sizeof...(Args) != 0) maybe_uninit([[maybe_unused]] std::in_place_t in_place, Args&&... args)
        : value_(std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr maybe_uninit([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> ilist, Args&&... args)
        : value_(ilist, std::forward<Args>(args)...) {}

    template <typename U>
        requires(!std::is_same_v<std::remove_cvref_t<U>, maybe_uninit<T>>)
    constexpr explicit(!std::is_convertible_v<U, T>) maybe_uninit(U&& value)
        : value_(std::forward<U>(value)) {}

    template <typename... Args>
    constexpr T& construct(Args&&... args) {
        return *std::construct_at(&value_, std::forward<Args>(args)...);
    }

    template <typename U, typename... Args>
    constexpr T& construct(std::initializer_list<U> ilist, Args&&... args) {
        return *std::construct_at(&value_, ilist, std::forward<Args>(args)...);
    }

    constexpr void destroy() {
        std::destroy_at(&value_);
    }

    constexpr reference value() & noexcept {
        return value_;
    }

    constexpr const_reference value() const& noexcept {
        return value_;
    }

    constexpr rvalue_reference value() && {
        return std::move(value_);
    }

    constexpr const_rvalue_reference value() const&& {
        return std::move(value_);
    }

    constexpr pointer data() noexcept {
        return &value_;
    }

    constexpr const_pointer data() const noexcept {
        return &value_;
    }

    constexpr reference operator*() & noexcept {
        return value_;
    }

    constexpr const_reference operator*() const& noexcept {
        return value_;
    }

    constexpr rvalue_reference operator*() && {
        return std::move(value_);
    }

    constexpr const_rvalue_reference operator*() const&& {
        return std::move(value_);
    }

    constexpr pointer operator->() noexcept {
        return &value_;
    }

    constexpr const_pointer operator->() const noexcept {
        return &value_;
    }
};

}

#endif
