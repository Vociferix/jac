#ifndef JAC_OPTION_HPP
#define JAC_OPTION_HPP

/// @file

#include <exception>
#include <functional>
#include <optional>

#include <jac/holder.hpp>
#include <jac/macros.hpp>
#include <jac/types.hpp>
#include <jac/utils.hpp>

namespace jac {

namespace detail {

template <typename T>
class option_impl;

template <typename T>
class option_impl {
  private:
    std::optional<T> value_{std::nullopt};

  public:
    using value_type = typename holder<T>::value_type;
    using reference = typename holder<T>::reference;
    using const_reference = typename holder<T>::const_reference;
    using rvalue_reference = typename holder<T>::rvalue_reference;
    using const_rvalue_reference = typename holder<T>::const_rvalue_reference;
    using pointer = typename holder<T>::pointer;
    using const_pointer = typename holder<T>::const_pointer;

    template <typename... Args>
    explicit constexpr option_impl(std::in_place_t in_place, Args&&... args)
        : value_(in_place, std::forward<Args>(args)...) {}

    constexpr bool has_value() const noexcept { return value_.has_value(); }

    constexpr void reset() { value_.reset(); }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        return value_.emplace(std::forward<Args>(args)...);
    }

    constexpr pointer ptr() noexcept { return value_.operator->(); }

    constexpr const_pointer ptr() const noexcept { return value_.operator->(); }

    constexpr reference get() & noexcept { return *value_; }

    constexpr const_reference get() const& noexcept { return *value_; }

    constexpr rvalue_reference get() && { return *std::move(value_); }

    constexpr const_rvalue_reference get() const&& {
        return *std::move(value_);
    }

    constexpr void swap(option_impl& other) { value_.swap(other.value_); }
};

template <typename T>
class option_impl<T&> {
  private:
    T* value_{nullptr};

  public:
    using value_type = typename holder<T&>::value_type;
    using reference = typename holder<T&>::reference;
    using const_reference = typename holder<T&>::const_reference;
    using rvalue_reference = typename holder<T&>::rvalue_reference;
    using const_rvalue_reference = typename holder<T&>::const_rvalue_reference;
    using pointer = typename holder<T&>::pointer;
    using const_pointer = typename holder<T&>::const_pointer;

    template <typename U>
    explicit constexpr option_impl(U* ptr) : value_(static_cast<T*>(ptr)) {}

    template <typename U>
    explicit constexpr option_impl([[maybe_unused]] std::in_place_t in_place,
                                   U& arg)
        : value_(static_cast<T*>(&arg)) {}

    constexpr bool has_value() const noexcept { return value_ != nullptr; }

    constexpr void reset() { value_ = nullptr; }

    template <typename U>
    constexpr reference emplace(U& arg) {
        return value_ = static_cast<T*>(&arg);
    }

    constexpr pointer ptr() const noexcept { return value_; }

    constexpr reference get() const noexcept { return *value_; }

    constexpr void swap(option_impl& other) noexcept {
        std::swap(value_, other.value_);
    }
};

template <typename T>
class option_impl<T&&> : public option_impl<T> {
  public:
    using value_type = typename option_impl<T>::value_type;
    using reference = typename option_impl<T>::reference;
    using const_reference = typename option_impl<T>::const_reference;
    using rvalue_reference = typename option_impl<T>::rvalue_reference;
    using const_rvalue_reference =
        typename option_impl<T>::const_rvalue_reference;
    using pointer = typename option_impl<T>::pointer;
    using const_pointer = typename option_impl<T>::const_pointer;

    using option_impl<T>::option_impl;
};

template <>
class option_impl<void_t> {
  private:
    bool has_value_{false};
    JAC_NO_UNIQ_ADDR void_t value_;

  public:
    using value_type = typename holder<void_t>::value_type;
    using reference = typename holder<void_t>::reference;
    using const_reference = typename holder<void_t>::const_reference;
    using rvalue_reference = typename holder<void_t>::rvalue_reference;
    using const_rvalue_reference =
        typename holder<void_t>::const_rvalue_reference;
    using pointer = typename holder<void_t>::pointer;
    using const_pointer = typename holder<void_t>::const_pointer;

    template <typename... Args>
    explicit constexpr option_impl([[maybe_unused]] std::in_place_t in_place,
                                   [[maybe_unused]] Args&&... args)
        : has_value_(true) {}

    constexpr bool has_value() const noexcept { return has_value_; }

    constexpr void reset() noexcept { has_value_ = false; }

    template <typename... Args>
    constexpr reference emplace([[maybe_unused]] Args&&... args) {
        has_value_ = true;
        return value_;
    }

    constexpr pointer ptr() noexcept { return &value_; }

    constexpr const_pointer ptr() const noexcept { return &value_; }

    constexpr reference get() & noexcept { return value_; }

    constexpr const_reference get() const& noexcept { return value_; }

    constexpr rvalue_reference get() && noexcept { return std::move(value_); }

    constexpr const_rvalue_reference get() const&& noexcept {
        return std::move(value_);
    }

    constexpr void swap([[maybe_unused]] option_impl& other) noexcept {}
};

template <>
class option_impl<void> : public option_impl<void_t> {
  public:
    using value_type = typename option_impl<void_t>::value_type;
    using reference = typename option_impl<void_t>::reference;
    using const_reference = typename option_impl<void_t>::const_reference;
    using rvalue_reference = typename option_impl<void_t>::rvalue_reference;
    using const_rvalue_reference =
        typename option_impl<void_t>::const_rvalue_reference;
    using pointer = typename option_impl<void_t>::pointer;
    using const_pointer = typename option_impl<void_t>::const_pointer;

    using option_impl<void_t>::option_impl;
};

} // namespace detail

class bad_option_access : public std::exception {
  public:
    bad_option_access() = default;
    bad_option_access(const bad_option_access&) = default;
    bad_option_access(bad_option_access&&) = default;
    ~bad_option_access() override = default;
    bad_option_access& operator=(const bad_option_access&) = default;
    bad_option_access& operator=(bad_option_access&&) = default;

    const char* what() const noexcept override {
        return "bad access of null jac::option";
    }
};

/// @brief A nullable wrapper around a type
///
/// @details
/// `option` is modeled after `std::optional`, but `option` supports references
/// and `void`. When the wrapped type is a reference, `option` functions as a
/// lightweight smart pointer and interoperates with raw pointers. When the
/// wrapped type is `void`, `jac::void_t` is used as the value type, and
/// `option` essentially functions as a `bool`.
template <typename T>
class option {
  private:
    detail::option_impl<T> impl_;

  public:
    using value_type = typename detail::option_impl<T>::value_type;
    using reference = typename detail::option_impl<T>::reference;
    using const_reference = typename detail::option_impl<T>::const_reference;
    using rvalue_reference = typename detail::option_impl<T>::rvalue_reference;
    using const_rvalue_reference =
        typename detail::option_impl<T>::const_rvalue_reference;
    using pointer = typename detail::option_impl<T>::pointer;
    using const_pointer = typename detail::option_impl<T>::const_pointer;

    constexpr option() = default;

    constexpr option(const option&) = default;

    constexpr option(option&&) = default;

    constexpr option([[maybe_unused]] null_t null) : option() {}

    constexpr option([[maybe_unused]] std::nullopt_t null) : option() {}

    constexpr option([[maybe_unused]] std::nullptr_t null)
        requires(std::is_lvalue_reference_v<T>)
        : option() {}

    template <typename... Args>
    constexpr explicit(sizeof...(Args) == 0)
        option(std::in_place_t in_place, Args&&... args)
        : impl_(in_place, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr option(std::in_place_t in_place,
                     std::initializer_list<U> ilist,
                     Args&&... args)
        : impl_(in_place, ilist, std::forward<Args>(args)...) {}

    template <typename U>
        requires(std::is_constructible_v<value_type,
                                         typename option<U>::const_reference> &&
                 !std::is_constructible_v<value_type, option<U>&> &&
                 !std::is_constructible_v<value_type, const option<U>&> &&
                 !std::is_constructible_v<value_type, option<U> &&> &&
                 !std::is_constructible_v<value_type, const option<U> &&> &&
                 !std::is_convertible_v<option<U>&, value_type> &&
                 !std::is_convertible_v<const option<U>&, value_type> &&
                 !std::is_convertible_v<option<U> &&, value_type> &&
                 !std::is_convertible_v<const option<U> &&, value_type>)
    constexpr explicit(
        !std::is_convertible_v<typename option<U>::const_reference, value_type>)
        option(const option<U>& other) {
        if (other.has_value()) { impl_.emplace(*other); }
    }

    template <typename U>
        requires(
            std::is_constructible_v<value_type,
                                    typename option<U>::rvalue_reference> &&
            !std::is_constructible_v<value_type, option<U>&> &&
            !std::is_constructible_v<value_type, const option<U>&> &&
            !std::is_constructible_v<value_type, option<U> &&> &&
            !std::is_constructible_v<value_type, const option<U> &&> &&
            !std::is_convertible_v<option<U>&, value_type> &&
            !std::is_convertible_v<const option<U>&, value_type> &&
            !std::is_convertible_v<option<U> &&, value_type> &&
            !std::is_convertible_v<const option<U> &&, value_type>)
    constexpr explicit(
        !std::is_convertible_v<typename option<U>::rvalue_reference,
                               value_type>) option(option<U>&& other) {
        if (other.has_value()) { impl_.emplace(*std::move(other)); }
    }

    template <typename U = value_type>
        requires(std::is_constructible_v<value_type, U &&> &&
                 !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
                 !std::is_same_v<std::remove_cvref_t<U>, option<T>> &&
                 (!std::is_lvalue_reference_v<T> || !std::is_pointer_v<U>))
    constexpr explicit(!std::is_convertible_v<U&&, value_type>)
        option(U&& value)
        : impl_(std::in_place, std::forward<U>(value)) {}

    template <typename U = std::remove_reference_t<T>>
        requires(std::is_lvalue_reference_v<U>)
    constexpr explicit(!std::is_convertible_v<U*, T*>) option(U* ptr)
        : impl_(ptr) {}

    constexpr ~option() = default;

    constexpr option& operator=(const option&) = default;

    constexpr option& operator=(option&&) = default;

    constexpr option& operator=([[maybe_unused]] null_t null) {
        impl_.reset();
        return *this;
    }

    template <typename U = value_type>
        requires(!std::is_same_v<std::remove_cvref_t<U>, option<T>> &&
                 std::is_constructible_v<value_type, U> &&
                 std::is_assignable_v<reference, U> &&
                 (!std::is_same_v<std::decay_t<U>, value_type> ||
                  !std::is_scalar_v<value_type>))
    constexpr option& operator=(U&& value) {
        impl_.emplace(std::forward<U>(value));
        return *this;
    }

    template <typename U>
        requires(!std::is_constructible_v<value_type, option<U>&> &&
                 !std::is_constructible_v<value_type, const option<U>&> &&
                 !std::is_constructible_v<value_type, option<U> &&> &&
                 !std::is_constructible_v<value_type, const option<U> &&> &&
                 !std::is_convertible_v<option<U>&, value_type> &&
                 !std::is_convertible_v<const option<U>&, value_type> &&
                 !std::is_convertible_v<option<U> &&, value_type> &&
                 !std::is_convertible_v<const option<U> &&, value_type> &&
                 !std::is_assignable_v<value_type, option<U>&> &&
                 !std::is_assignable_v<value_type, const option<U>&> &&
                 !std::is_assignable_v<value_type, option<U> &&> &&
                 !std::is_assignable_v<value_type, const option<U> &&>)
    constexpr option& operator=(const option<U>& other) {
        if (other.has_value()) {
            impl_.emplace(*other);
        } else {
            impl_.reset();
        }
        return *this;
    }

    template <typename U>
        requires(!std::is_constructible_v<value_type, option<U>&> &&
                 !std::is_constructible_v<value_type, const option<U>&> &&
                 !std::is_constructible_v<value_type, option<U> &&> &&
                 !std::is_constructible_v<value_type, const option<U> &&> &&
                 !std::is_convertible_v<option<U>&, value_type> &&
                 !std::is_convertible_v<const option<U>&, value_type> &&
                 !std::is_convertible_v<option<U> &&, value_type> &&
                 !std::is_convertible_v<const option<U> &&, value_type> &&
                 !std::is_assignable_v<value_type, option<U>&> &&
                 !std::is_assignable_v<value_type, const option<U>&> &&
                 !std::is_assignable_v<value_type, option<U> &&> &&
                 !std::is_assignable_v<value_type, const option<U> &&>)
    constexpr option& operator=(option<U>&& other) {
        if (other.has_value()) {
            impl_.emplace(*std::move(other));
        } else {
            impl_.reset();
        }
        return *this;
    }

    explicit constexpr operator pointer() const noexcept
        requires(std::is_lvalue_reference_v<T>)
    {
        return impl_.ptr();
    }

    constexpr bool has_value() const noexcept { return impl_.has_value(); }

    constexpr operator bool() const noexcept { return impl_.has_value(); }

    constexpr reference operator*() & noexcept { return impl_.get(); }

    constexpr const_reference operator*() const& noexcept {
        return impl_.get();
    }

    constexpr rvalue_reference operator*() && { return std::move(impl_).get(); }

    constexpr const_rvalue_reference operator*() const&& {
        return std::move(impl_).get();
    }

    constexpr pointer operator->() noexcept { return impl_.ptr(); }

    constexpr const_pointer operator->() const noexcept { return impl_.ptr(); }

    constexpr void reset() { impl_.reset(); }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        return impl_.emplace(std::forward<Args>(args)...);
    }

    constexpr reference value() & {
        if (!impl_.has_value()) { throw bad_option_access(); }
        return impl_.get();
    }

    constexpr const_reference value() const& {
        if (!impl_.has_value()) { throw bad_option_access(); }
        return impl_.get();
    }

    constexpr rvalue_reference value() && {
        if (!impl_.has_value()) { throw bad_option_access(); }
        return std::move(impl_).get();
    }

    constexpr const_rvalue_reference value() const&& {
        if (!impl_.has_value()) { throw bad_option_access(); }
        return std::move(impl_).get();
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) const& {
        if (impl_.has_value()) {
            return impl_.get();
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    template <typename U>
    constexpr value_type value_or(U&& default_value) && {
        if (impl_.has_value()) {
            return std::move(impl_).get();
        } else {
            return static_cast<value_type>(std::forward<U>(default_value));
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) & {
        if (impl_.has_value()) {
            return std::invoke(std::forward<F>(f), impl_.get());
        } else {
            return std::remove_cvref_t<std::invoke_result_t<F, reference>>();
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const& {
        if (impl_.has_value()) {
            return std::invoke(std::forward<F>(f), impl_.get());
        } else {
            return std::remove_cvref_t<
                std::invoke_result_t<F, const_reference>>();
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) && {
        if (impl_.has_value()) {
            return std::invoke(std::forward<F>(f), std::move(impl_).get());
        } else {
            return std::remove_cvref_t<
                std::invoke_result_t<F, rvalue_reference>>();
        }
    }

    template <typename F>
    constexpr auto and_then(F&& f) const&& {
        if (impl_.has_value()) {
            return std::invoke(std::forward<F>(f), std::move(impl_).get());
        } else {
            return std::remove_cvref_t<
                std::invoke_result_t<F, const_rvalue_reference>>();
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) & {
        using ret_t = std::invoke_result_t<F, reference>;
        if (impl_.has_value()) {
            return option<ret_t>(std::invoke(std::forward<F>(f), impl_.get()));
        } else {
            return option<ret_t>();
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const& {
        using ret_t = std::invoke_result_t<F, const_reference>;
        if (impl_.has_value()) {
            return option<ret_t>(std::invoke(std::forward<F>(f), impl_.get()));
        } else {
            return option<ret_t>();
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) && {
        using ret_t = std::invoke_result_t<F, rvalue_reference>;
        if (impl_.has_value()) {
            return option<ret_t>(
                std::invoke(std::forward<F>(f), std::move(impl_).get()));
        } else {
            return opiont<ret_t>();
        }
    }

    template <typename F>
    constexpr auto transform(F&& f) const&& {
        using ret_t = std::invoke_result_t<F, const_rvalue_reference>;
        if (impl_.has_value()) {
            return option<ret_t>(
                std::invoke(std::forward<F>(f), std::move(impl_).get()));
        } else {
            return option<ret_t>();
        }
    }

    template <typename F>
    constexpr option or_else(F&& f) const& {
        if (impl_.has_value()) {
            return *this;
        } else {
            return std::invoke(std::forward<F>(f));
        }
    }

    template <typename F>
    constexpr option or_else(F&& f) && {
        if (impl_.has_value()) {
            return std::move(*this);
        } else {
            return std::invoke(std::forward<F>(f));
        }
    }

    constexpr void swap(option& other) { impl_.swap(other.impl_); }

    constexpr option<reference> as_ref() noexcept {
        if (has_value()) {
            return option<reference>(**this);
        } else {
            return option<reference>();
        }
    }

    constexpr option<const_reference> as_ref() const noexcept {
        if (has_value()) {
            return option<const_reference>(**this);
        } else {
            return option<const_reference>();
        }
    }
};

template <typename T>
option(T&&) -> option<T>;

template <typename T>
constexpr void swap(option<T>& a, option<T>& b) {
    a.swap(b);
}

template <typename T, typename U>
constexpr std::compare_three_way_result_t<T, U> operator<=>(
    const option<T>& lhs,
    const option<U>& rhs) {
    if (lhs.has_value() && rhs.has_value()) {
        return *lhs <=> *rhs;
    } else {
        return lhs.has_value() <=> rhs.has_value();
    }
}

template <typename T>
constexpr std::strong_ordering operator<=>(const option<T>& lhs,
                                           [[maybe_unused]] null_t rhs) {
    return lhs.has_value() <=> false;
}

template <typename T>
constexpr std::strong_ordering operator<=>([[maybe_unused]] null_t lhs,
                                           const option<T>& rhs) {
    return false <=> rhs.has_value();
}

template <typename T>
constexpr std::strong_ordering operator<=>(
    const option<T>& lhs,
    [[maybe_unused]] std::nullopt_t rhs) {
    return lhs.has_value() <=> false;
}

template <typename T>
constexpr std::strong_ordering operator<=>([[maybe_unused]] std::nullopt_t lhs,
                                           const option<T>& rhs) {
    return false <=> rhs.has_value();
}

template <typename T>
    requires(std::is_lvalue_reference_v<T>)
constexpr std::strong_ordering operator<=>(
    const option<T>& lhs,
    [[maybe_unused]] std::nullptr_t rhs) {
    return lhs.has_value() <=> false;
}

template <typename T>
    requires(std::is_lvalue_reference_v<T>)
constexpr std::strong_ordering operator<=>([[maybe_unused]] std::nullptr_t lhs,
                                           const option<T>& rhs) {
    return false <=> rhs.has_value();
}

template <typename T, typename U>
    requires(!std::is_lvalue_reference_v<T> || !std::is_pointer_v<U>)
constexpr std::compare_three_way_result_t<T, U> operator<=>(
    const option<T>& lhs,
    const U& rhs) {
    if (lhs.has_value()) {
        return *lhs <=> rhs;
    } else {
        return false <=> true;
    }
}

template <typename T, typename U>
    requires(!std::is_lvalue_reference_v<U> || !std::is_pointer_v<T>)
constexpr std::compare_three_way_result_t<U, T> operator<=>(
    const T& lhs,
    const option<U>& rhs) {
    if (rhs.has_value()) {
        return lhs <=> *rhs;
    } else {
        return true <=> false;
    }
}

template <typename T, typename U>
    requires(std::is_lvalue_reference_v<T>)
constexpr std::compare_three_way_result_t<const T*, U> operator<=>(
    const option<T>& lhs,
    const U* rhs) {
    return lhs.operator->() <=> rhs;
}

template <typename T, typename U>
    requires(std::is_lvalue_reference_v<U>)
constexpr std::compare_three_way_result_t<T, const U*> operator<=>(
    const T* lhs,
    const option<U>& rhs) {
    return lhs <=> rhs.operator->();
}

template <typename T, typename... Args>
constexpr option<T> make_option(Args&&... args) {
    return option<T>(std::in_place, std::forward<Args>(args)...);
}

template <typename T, typename U, typename... Args>
constexpr option<T> make_option(std::initializer_list<U> ilist,
                                Args&&... args) {
    return option<T>(std::in_place, ilist, std::forward<Args>(args)...);
}

} // namespace jac

template <typename T>
struct std::hash<::jac::option<T>> {
  private:
    JAC_NO_UNIQ_ADDR std::hash<T> inner_;
    JAC_NO_UNIQ_ADDR std::hash<bool> bool_hasher_;

  public:
    constexpr size_t operator()(const ::jac::option<T>& value) const {
        auto has_val = value.has_value();
        size_t h = bool_hasher_(has_val);
        if (has_val) { h = ::jac::hash_combine(h, inner_(value)); }
        return h;
    }
};

#endif
