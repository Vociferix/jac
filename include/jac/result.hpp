#ifndef JAC_RESULT_HPP
#define JAC_RESULT_HPP

/// @file

#include <exception>
#include <functional>
#include <system_error>
#include <variant>

#include <jac/holder.hpp>
#include <jac/macros.hpp>
#include <jac/types.hpp>
#include <jac/utils.hpp>

namespace jac {

enum class error_tag { };

/// @brief Representation of an error value
template <typename E>
using error = holder<E, error_tag>;

template <typename E, typename... Args>
constexpr error<E> make_error(Args&&... args) {
    return error<E>(std::in_place, std::forward<Args>(args)...);
}

template <typename E, typename U, typename... Args>
constexpr error<E> make_error(std::initializer_list<U> ilist, Args&&... args) {
    return error<E>(std::in_place, ilist, std::forward<Args>(args)...);
}

struct in_place_error_t { };

static inline constexpr in_place_error_t in_place_error{};

class bad_result_access : public std::exception {
  public:
    bad_result_access() = default;
    bad_result_access(const bad_result_access&) = default;
    bad_result_access(bad_result_access&&) = default;
    ~bad_result_access() override = default;
    bad_result_access& operator=(const bad_result_access&) = default;
    bad_result_access& operator=(bad_result_access&&) = default;

    const char* what() const noexcept override {
        return "bad access of jac::result with error value";
    }
};

/// @brief Type safe union of a success value and an error value
///
/// jac::result is modelled after `std::expected`, but it works with references
/// and `void` as both the success or error types.
template <typename T, typename E = std::error_code>
class result {
  private:
    std::variant<holder<T>, holder<E>, std::monostate> value_;

  public:
    using value_type = typename holder<T>::value_type;
    using reference = typename holder<T>::reference;
    using const_reference = typename holder<T>::const_reference;
    using rvalue_reference = typename holder<T>::rvalue_reference;
    using const_rvalue_reference = typename holder<T>::const_rvalue_reference;
    using pointer = typename holder<T>::pointer;
    using const_pointer = typename holder<T>::const_pointer;

    using error_type = typename holder<E>::value_type;
    using error_reference = typename holder<E>::reference;
    using error_const_reference = typename holder<E>::const_reference;
    using error_rvalue_reference = typename holder<E>::rvalue_reference;
    using error_const_rvalue_reference = typename holder<E>::const_rvalue_reference;
    using error_pointer = typename holder<E>::pointer;
    using error_const_pointer = typename holder<E>::const_pointer;

    constexpr result() = default;

    constexpr result(const result&) = default;

    constexpr result(result&&) = default;

    template <typename... Args>
    constexpr explicit(sizeof...(Args) == 0) result(std::in_place_t in_place, Args&&... args)
        : value_(std::in_place_index<0>, in_place, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr result(std::in_place_t in_place, std::initializer_list<U> ilist, Args&&... args)
        : value_(std::in_place_index<0>, in_place, ilist, std::forward<Args>(args)...) {}

    template <typename... Args>
    constexpr explicit(sizeof...(Args) == 0) result([[maybe_unused]] in_place_error_t in_place, Args&&... args)
        : value_(std::in_place_index<1>, std::in_place, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr result([[maybe_unused]] in_place_error_t in_place, std::initializer_list<U> ilist, Args&&... args)
        : value_(std::in_place_index<1>, std::in_place, ilist, std::forward<Args>(args)...) {}

    template <typename U, typename V>
        requires(
            std::is_constructible_v<value_type, typename result<U, V>::const_reference> &&
            std::is_constructible_v<error_type, typename result<U, V>::error_const_reference> &&
            !std::is_constructible_v<value_type, result<U, V>&> &&
            !std::is_constructible_v<value_type, const result<U, V>&> &&
            !std::is_constructible_v<value_type, result<U, V>&&> &&
            !std::is_constructible_v<value_type, const result<U, V>&&> &&
            !std::is_convertible_v<result<U, V>&, value_type> &&
            !std::is_convertible_v<const result<U, V>&, value_type> &&
            !std::is_convertible_v<result<U, V>&&, value_type> &&
            !std::is_convertible_v<const result<U, V>&&, value_type>
        )
    constexpr explicit(!std::is_convertible_v<typename result<U, V>::const_reference, value_type> || !std::is_convertible_v<typename result<U, V>::error_const_reference, error_type>)
    result(const result<U, V>& other)
        : value_(std::in_place_index<2>) {
        if (other.has_value()) {
            value_.template emplace<0>(std::in_place, *other);
        } else {
            value_.template emplace<1>(std::in_place, other.error());
        }
    }

    template <typename U, typename V>
        requires(
            std::is_constructible_v<value_type, typename result<U, V>::rvalue_reference> &&
            std::is_constructible_v<error_type, typename result<U, V>::error_rvalue_reference> &&
            !std::is_constructible_v<value_type, result<U, V>&> &&
            !std::is_constructible_v<value_type, const result<U, V>&> &&
            !std::is_constructible_v<value_type, result<U, V>&&> &&
            !std::is_constructible_v<value_type, const result<U, V>&&> &&
            !std::is_convertible_v<result<U, V>&, value_type> &&
            !std::is_convertible_v<const result<U, V>&, value_type> &&
            !std::is_convertible_v<result<U, V>&&, value_type> &&
            !std::is_convertible_v<const result<U, V>&&, value_type>
        )
    constexpr explicit(!std::is_convertible_v<typename result<U, V>::rvalue_reference, value_type> || !std::is_convertible_v<typename result<U, V>::error_rvalue_reference, error_type>)
    result(result<U, V>&& other)
        : value_(std::in_place_index<2>) {
        if (other.has_value()) {
            value_.template emplace<0>(std::in_place, *std::move(other));
        } else {
            value_.template emplace<1>(std::in_place, std::move(other).error());
        }
    }

    template <typename U = T>
        requires(
            std::is_constructible_v<value_type, U&&> &&
            !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
            !std::is_same_v<std::remove_cvref_t<U>, in_place_error_t> &&
            !std::is_same_v<std::remove_cvref_t<U>, result<T, E>> &&
            !std::is_same_v<std::remove_cvref_t<U>, error<E>>
        )
    constexpr explicit(!std::is_convertible_v<U&&, value_type>) result(U&& value)
        : value_(std::in_place_index<0>, std::in_place, std::forward<U>(value)) {}

    template <typename V = E>
        requires(
            std::is_constructible_v<error_type, typename error<V>::const_reference> &&
            !std::is_constructible_v<value_type, error<V>&> &&
            !std::is_constructible_v<value_type, const error<V>&> &&
            !std::is_constructible_v<value_type, error<V>&&> &&
            !std::is_constructible_v<value_type, const error<V>&&> &&
            !std::is_convertible_v<error<V>&, value_type> &&
            !std::is_convertible_v<const error<V>&, value_type> &&
            !std::is_convertible_v<error<V>&&, value_type> &&
            !std::is_convertible_v<const error<V>&&, value_type>
        )
    constexpr explicit(!std::is_convertible_v<typename error<V>::const_reference, error_type>)
    result(const error<V>& err)
        : value_(std::in_place_index<1>, std::in_place, *err) {}

    template <typename V = E>
        requires(
            std::is_constructible_v<error_type, typename error<V>::rvalue_reference> &&
            !std::is_constructible_v<value_type, error<V>&> &&
            !std::is_constructible_v<value_type, const error<V>&> &&
            !std::is_constructible_v<value_type, error<V>&&> &&
            !std::is_constructible_v<value_type, const error<V>&&> &&
            !std::is_convertible_v<error<V>&, value_type> &&
            !std::is_convertible_v<const error<V>&, value_type> &&
            !std::is_convertible_v<error<V>&&, value_type> &&
            !std::is_convertible_v<const error<V>&&, value_type>
        )
    constexpr explicit(!std::is_convertible_v<typename error<V>::rvalue_reference, error_type>)
    result(error<V>&& err)
        : value_(std::in_place_index<1>, std::in_place, *std::move(err)) {}

    constexpr ~result() = default;

    constexpr result& operator=(const result&) = default;

    constexpr result& operator=(result&&) = default;

    template <typename U = value_type>
        requires(
            !std::is_same_v<std::remove_cvref_t<U>, result<T, E>> &&
            std::is_constructible_v<value_type, U> &&
            std::is_assignable_v<value_type, U> &&
            (!std::is_same_v<std::decay_t<U>, value_type> || !std::is_scalar_v<value_type>)
        )
    constexpr result& operator=(U&& value) {
        if (value_.index() == 0) {
            *std::get_if<0>(value_) = std::forward<U>(value);
        } else {
            value_.template emplace<0>(std::in_place, std::forward<U>(value));
        }
        return *this;
    }

    template <typename V = E>
        requires(
            std::is_constructible_v<error_type, typename error<V>::const_referemce> &&
            std::is_assignable_v<error_type, typename error<V>::const_reference> &&
            !std::is_constructible_v<value_type, error<V>&> &&
            !std::is_constructible_v<value_type, const error<V>&> &&
            !std::is_constructible_v<value_type, error<V>&&> &&
            !std::is_constructible_v<value_type, const error<V>&&> &&
            !std::is_convertible_v<error<V>&, value_type> &&
            !std::is_convertible_v<const error<V>&, value_type> &&
            !std::is_convertible_v<error<V>&&, value_type> &&
            !std::is_convertible_v<const error<V>&&, value_type> &&
            !std::is_assignable_v<value_type, error<V>&> &&
            !std::is_assignable_v<value_type, const error<V>&> &&
            !std::is_assignable_v<value_type, error<V>&&> &&
            !std::is_assignable_v<value_type, const error<V>&&>
        )
    constexpr result& operator=(const error<V>& err) {
        if (value_.index() == 1) {
            *std::get_if<1>(value_) = *err;
        } else {
            value_.template emplace<1>(std::in_place, *err);
        }
        return *this;
    }

    template <typename V = E>
        requires(
            std::is_constructible_v<error_type, typename error<V>::rvalue_reference> &&
            std::is_assignable_v<error_type, typename error<V>::rvalue_reference> &&
            !std::is_constructible_v<value_type, error<V>&> &&
            !std::is_constructible_v<value_type, const error<V>&> &&
            !std::is_constructible_v<value_type, error<V>&&> &&
            !std::is_constructible_v<value_type, const error<V>&&> &&
            !std::is_convertible_v<error<V>&, value_type> &&
            !std::is_convertible_v<const error<V>&, value_type> &&
            !std::is_convertible_v<error<V>&&, value_type> &&
            !std::is_convertible_v<const error<V>&&, value_type> &&
            !std::is_assignable_v<value_type, error<V>&> &&
            !std::is_assignable_v<value_type, const error<V>&> &&
            !std::is_assignable_v<value_type, error<V>&&> &&
            !std::is_assignable_v<value_type, const error<V>&&>
        )
    constexpr result& operator=(error<V>&& err) {
        if (value_.index() == 1) {
            *std::get_if<1>(value_) = *std::move(err);
        } else {
            value_.template emplace<1>(std::in_place, *std::move(err));
        }
        return *this;
    }

    template <typename U, typename V>
        requires(
            !std::is_constructible_v<value_type, result<U, V>&> &&
            !std::is_constructible_v<value_type, const result<U, V>&> &&
            !std::is_constructible_v<value_type, result<U, V>&&> &&
            !std::is_constructible_v<value_type, const result<U, V>&&> &&
            !std::is_convertible_v<result<U, V>&, value_type> &&
            !std::is_convertible_v<const result<U, V>&, value_type> &&
            !std::is_convertible_v<result<U, V>&&, value_type> &&
            !std::is_convertible_v<const result<U, V>&&, value_type> &&
            !std::is_assignable_v<value_type, result<U, V>&> &&
            !std::is_assignable_v<value_type, const result<U, V>&> &&
            !std::is_assignable_v<value_type, result<U, V>&&> &&
            !std::is_assignable_v<value_type, const result<U, V>&&>
        )
    constexpr result& operator=(const result<U, V>& other) {
        if (other.has_value()) {
            if (value_.index() == 0) {
                *std::get_if<0>(value_) = *other;
            } else {
                value_.template emplace<0>(std::in_place, *other);
            }
        } else {
            if (value_.index() == 1) {
                *std::get_if<1>(value_) = other.error();
            } else {
                value_.template emplace<1>(std::in_place, other.error());
            }
        }
        return *this;
    }

    template <typename U, typename V>
        requires(
            !std::is_constructible_v<value_type, result<U, V>&> &&
            !std::is_constructible_v<value_type, const result<U, V>&> &&
            !std::is_constructible_v<value_type, result<U, V>&&> &&
            !std::is_constructible_v<value_type, const result<U, V>&&> &&
            !std::is_convertible_v<result<U, V>&, value_type> &&
            !std::is_convertible_v<const result<U, V>&, value_type> &&
            !std::is_convertible_v<result<U, V>&&, value_type> &&
            !std::is_convertible_v<const result<U, V>&&, value_type> &&
            !std::is_assignable_v<value_type, result<U, V>&> &&
            !std::is_assignable_v<value_type, const result<U, V>&> &&
            !std::is_assignable_v<value_type, result<U, V>&&> &&
            !std::is_assignable_v<value_type, const result<U, V>&&>
        )
    constexpr result& operator=(result<U, V>&& other) {
        if (other.has_value()) {
            if (value_.index() == 0) {
                *std::get_if<0>(value_) = *std::move(other);
            } else {
                value_.template emplace<0>(std::in_place, *std::move(other));
            }
        } else {
            if (value_.index() == 1) {
                *std::get_if<1>(value_) = std::move(other).error();
            } else {
                value_.template emplace<1>(std::in_place, std::move(other).error());
            }
        }
        return *this;
    }

    constexpr bool has_value() const noexcept {
        return value_.index() == 0;
    }

    constexpr operator bool() const noexcept {
        return value_.index() == 0;
    }

    constexpr reference value() & {
        auto ptr = &*std::get_if<0>(value_);
        if (ptr == nullptr) {
            throw bad_result_access();
        }
        return *ptr;
    }

    constexpr const_reference value() const& {
        auto ptr = &*std::get_if<0>(value_);
        if (ptr == nullptr) {
            throw bad_result_access();
        }
        return *ptr;
    }

    constexpr rvalue_reference value() && {
        auto ptr = &*std::get_if<0>(value_);
        if (ptr == nullptr) {
            throw bad_result_access();
        }
        return std::move(*ptr);
    }

    constexpr const_rvalue_reference value() const&& {
        auto ptr = &*std::get_if<0>(value_);
        if (ptr == nullptr) {
            throw bad_result_access();
        }
        return std::move(*ptr);
    }

    constexpr error_reference error() & noexcept {
        return **std::get_if<1>(value_);
    }

    constexpr error_const_reference error() const& noexcept {
        return **std::get_if<1>(value_);
    }

    constexpr error_rvalue_reference error() && {
        return *std::move(*std::get_if<1>(value_));
    }

    constexpr error_const_rvalue_reference error() const&& {
        return *std::move(*std::get_if<1>(value_));
    }

    constexpr reference operator*() & noexcept {
        return **std::get_if<0>(value_);
    }

    constexpr const_reference operator*() const& noexcept {
        return **std::get_if<0>(value_);
    }

    constexpr rvalue_reference operator*() && {
        return *std::move(*std::get_if<0>(value_));
    }

    constexpr const_rvalue_reference operator*() const&& {
        return *std::move(*std::get_if<0>(value_));
    }

    constexpr pointer operator->() noexcept {
        return &**std::get_if<0>(value_);
    }

    constexpr const_pointer operator->() const noexcept {
        return &**std::get_if<0>(value_);
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) const& {
        if (has_value()) {
            return **this;
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) && {
        if (has_value()) {
            return *std::move(*this);
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    template <typename U>
    constexpr error_type error_or(U&& default_error) const& {
        if (!has_value()) {
            return error();
        } else {
            return static_cast<error_type>(std::forward<U>(default_error));
        }
    }

    template <typename U>
    constexpr error_type error_or(U&& default_error) && {
        if (!has_value()) {
            return std::move(*this).error();
        } else {
            return static_cast<error_type>(std::forward<U>(default_error));
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) & {
        if (has_value()) {
            return std::invoke(std::forward<F>(f), **this);
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F, reference>>(in_place_error, error());
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const& {
        if (has_value()) {
            return std::invoke(std::forward<F>(f), **this);
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F, const_reference>>(in_place_error, error());
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) && {
        if (has_value()) {
            return std::invoke(std::forward<F>(f), *std::move(*this));
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F, rvalue_reference>>(in_place_error, std::move(*this).error());
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const&& {
        if (has_value()) {
            return std::invoke(std::forward<F>(f), *std::move(*this));
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F, const_rvalue_reference>>(in_place_error, std::move(*this).error());
        }
    }

    template <typename F>
    constexpr auto or_else(F&& f) & {
        if (has_value()) {
            return std::remove_cvref_t<std::invoke_result_t<F, error_reference>>(**this);
        } else {
            return std::invoke(std::forward<F>(f), error());
        }
    }

    template <typename F>
    constexpr auto or_else(F&& f) const& {
        if (has_value()) {
            return std::remove_cvref_t<std::invoke_result_t<F, error_const_reference>>(**this);
        } else {
            return std::invoke(std::forward<F>(f), error());
        }
    }

    template <typename F>
    constexpr auto or_else(F&& f) && {
        if (has_value()) {
            return std::remove_cvref_t<std::invoke_result_t<F, error_rvalue_reference>>(*std::move(*this));
        } else {
            return std::invoke(std::forward<F>(f), std::move(*this).error());
        }
    }

    template <typename F>
    constexpr auto or_else(F&& f) const&& {
        if (has_value()) {
            return std::remove_cvref_t<std::invoke_result_t<F, error_const_rvalue_reference>>(*std::move(*this));
        } else {
            return std::invoke(std::forward<F>(f), std::move(*this).error());
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) & {
        using ret_t = std::invoke_result_t<F, reference>;
        if (has_value()) {
            return result<ret_t, E>(std::invoke(std::forward<F>(f), **this));
        } else {
            return result<ret_t, E>(in_place_error, error());
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const& {
        using ret_t = std::invoke_result_t<F, const_reference>;
        if (has_value()) {
            return result<ret_t, E>(std::invoke(std::forward<F>(f), **this));
        } else {
            return result<ret_t, E>(in_place_error, error());
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) && {
        using ret_t = std::invoke_result_t<F, rvalue_reference>;
        if (has_value()) {
            return result<ret_t, E>(std::invoke(std::forward<F>(f), *std::move(*this)));
        } else {
            return result<ret_t, E>(in_place_error, std::move(*this).error());
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const&& {
        using ret_t = std::invoke_result_t<F, const_rvalue_reference>;
        if (has_value()) {
            return result<ret_t, E>(std::invoke(std::forward<F>(f), *std::move(*this)));
        } else {
            return result<ret_t, E>(in_place_error, std::move(*this).error());
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) & {
        using ret_t = std::invoke_result_t<F, error_reference>;
        if (has_value()) {
            return result<T, ret_t>(**this);
        } else {
            return result<T, ret_t>(in_place_error, std::invoke(std::forward<F>(f), error()));
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) const& {
        using ret_t = std::invoke_result_t<F, error_const_reference>;
        if (has_value()) {
            return result<T, ret_t>(**this);
        } else {
            return result<T, ret_t>(in_place_error, std::invoke(std::forward<F>(f), error()));
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) && {
        using ret_t = std::invoke_result_t<F, error_rvalue_reference>;
        if (has_value()) {
            return result<T, ret_t>(*std::move(*this));
        } else {
            return result<T, ret_t>(in_place_error, std::invoke(std::forward<F>(f), std::move(*this).error()));
        }
    }

    template <typename F>
    constexpr auto transform_error(F&& f) const&& {
        using ret_t = std::invoke_result_t<F, error_const_rvalue_reference>;
        if (has_value()) {
            return result<T, ret_t>(*std::move(*this));
        } else {
            return result<T, ret_t>(in_place_error, std::invoke(std::forward<F>(f), std::move(*this).error()));
        }
    }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        return value_.template emplace<0>(std::forward<Args>(args)...);
    }

    template <typename U, typename... Args>
    constexpr reference emplace(std::initializer_list<U> ilist, Args&&... args) {
        return value_.template emplace<0>(ilist, std::forward<Args>(args)...);
    }

    template <typename... Args>
    constexpr error_reference emplace_error(Args&&... args) {
        return value_.template emplace<1>(std::forward<Args>(args)...);
    }

    template <typename U, typename... Args>
    constexpr error_reference emplace_error(std::initializer_list<U> ilist, Args&&... args) {
        return value_.template emplace<1>(ilist, std::forward<Args>(args)...);
    }

    constexpr void swap(result& other) {
        value_.swap(other);
    }

    constexpr result<reference, error_reference> as_ref() noexcept {
        if (has_value()) {
            return result<reference, error_reference>(**this);
        } else {
            return result<reference, error_reference>(in_place_error, error());
        }
    }

    constexpr result<const_reference, error_const_reference> as_ref() const noexcept {
        if (has_value()) {
            return result<const_reference, error_const_reference>(**this);
        } else {
            return result<const_reference, error_const_reference>(in_place_error, error());
        }
    }
};

template <typename T, typename E>
constexpr void swap(result<T, E>& a, result<T, E>& b) {
    a.swap(b);
}

template <typename T1, typename E1, typename T2, typename E2>
constexpr bool operator==(const result<T1, E1>& lhs, const result<T2, E2>& rhs) {
    if (lhs.has_value()) {
        return rhs.has_value() && *lhs == *rhs;
    } else {
        return !rhs.has_value() && lhs.error() == rhs.error();
    }
}

template <typename T, typename E, typename U>
constexpr bool operator==(const result<T, E>& lhs, const U& rhs) {
    return lhs.has_value() && *lhs == rhs;
}

template <typename U, typename T, typename E>
constexpr bool operator==(const U& lhs, const result<T, E>& rhs) {
    return rhs.has_value() && lhs == *rhs;
}

template <typename T, typename E, typename V>
constexpr bool operator==(const result<T, E>& lhs, const error<V>& rhs) {
    return !lhs.has_value() && lhs.error() == *rhs;
}

template <typename V, typename T, typename E>
constexpr bool operator==(const error<V>& lhs, const result<T, E>& rhs) {
    return !rhs.has_value() && *lhs == rhs.error();
}

}

template <typename E>
struct std::hash<::jac::error<E>> {
  private:
    JAC_NO_UNIQ_ADDR std::hash<typename ::jac::error<E>::value_type> inner_;

  public:
    constexpr size_t operator()(const ::jac::error<E>& err) const {
        return inner_(*err);
    }
};

template <typename T, typename E>
struct std::hash<::jac::result<T, E>> {
  private:
    JAC_NO_UNIQ_ADDR std::hash<typename ::jac::result<T, E>::value_type> val_hasher_;
    JAC_NO_UNIQ_ADDR std::hash<typename ::jac::result<T, E>::error_type> err_hasher_;
    JAC_NO_UNIQ_ADDR std::hash<bool> bool_hasher_;

  public:
    constexpr size_t operator()(const ::jac::result<T, E>& res) const {
        if (res.has_value()) {
            return ::jac::hash_combine(bool_hasher_(true), val_hasher_(*res));
        } else {
            return ::jac::hash_combine(bool_hasher_(false), val_hasher_(res.error()));
        }
    }
};

#endif
