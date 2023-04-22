#ifndef JAC_HOLDER_HPP
#define JAC_HOLDER_HPP

/// @file

#include <functional>

#include <jac/macros.hpp>
#include <jac/types.hpp>

namespace jac {

namespace detail {

template <typename T>
class holder_impl {
  private:
    JAC_NO_UNIQ_ADDR T value_;

  public:
    using value_type = T;
    using reference = T&;
    using const_reference = std::add_const_t<T>&;
    using rvalue_reference = T&&;
    using const_rvalue_reference = std::add_const_t<T>&&;
    using pointer = T*;
    using const_pointer = std::add_const_t<T>*;

    static constexpr bool is_array = false;

    constexpr holder_impl() = default;

    template <typename... Args>
    explicit constexpr holder_impl([[maybe_unused]] std::in_place_t in_place, Args&&... args)
        : value_(std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr holder_impl([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> ilist, Args&&... args)
        : value_(ilist, std::forward<Args>(args)...) {}

    constexpr reference get() & noexcept {
        return value_;
    }

    constexpr const_reference get() const& noexcept {
        return value_;
    }

    constexpr rvalue_reference get() && {
        return std::move(value_);
    }

    constexpr const_rvalue_reference get() const&& {
        return std::move(value_);
    }

    constexpr pointer ptr() noexcept {
        return &value_;
    }

    constexpr const_pointer ptr() const noexcept {
        return &value_;
    }

    constexpr void swap(holder_impl& other) {
        using std::swap;
        swap(value_, other.value_);
    }
};

template <typename T>
class holder_impl<T&> {
  private:
    T* value_{nullptr};

  public:
    using value_type = T&;
    using reference = T&;
    using const_reference = T&;
    using rvalue_reference = T&;
    using const_rvalue_reference = T&;
    using pointer = T*;
    using const_pointer = T*;

    static constexpr bool is_array = false;

    constexpr holder_impl() = default;

    template <typename U>
    constexpr holder_impl([[maybe_unused]] std::in_place_t in_place, U& value)
        : value_(&value) {}

    constexpr reference get() const noexcept {
        return *value_;
    }

    constexpr pointer ptr() const noexcept {
        return value_;
    }

    constexpr void swap(holder_impl& other) noexcept {
        std::swap(value_, other.value_);
    }
};

template <typename T>
class holder_impl<T&&> : public holder_impl<T> {
  public:
    using value_type = typename holder_impl<T>::value_type;
    using reference = typename holder_impl<T>::reference;
    using const_reference = typename holder_impl<T>::const_reference;
    using rvalue_reference = typename holder_impl<T>::rvalue_reference;
    using const_rvalue_reference = typename holder_impl<T>::const_rvalue_reference;
    using pointer = typename holder_impl<T>::pointer;
    using const_pointer = typename holder_impl<T>::const_pointer;

    using holder_impl<T>::holder_impl;

    static constexpr bool is_array = false;
};

template <>
class holder_impl<void> : public holder_impl<void_t> {
  public:
    using holder_impl<void_t>::holder_impl;

    using value_type = typename holder_impl<void_t>::value_type;
    using reference = typename holder_impl<void_t>::reference;
    using const_reference = typename holder_impl<void_t>::const_reference;
    using rvalue_reference = typename holder_impl<void_t>::rvalue_reference;
    using const_rvalue_reference = typename holder_impl<void_t>::const_rvalue_reference;
    using pointer = typename holder_impl<void_t>::pointer;
    using const_pointer = typename holder_impl<void_t>::const_pointer;

    static constexpr bool is_array = false;
};

template <typename T, size_t N>
class holder_impl<T[N]> {
  private:
    T value_[N];

    template <typename U, size_t... IDX>
    constexpr holder_impl([[maybe_unused]] std::index_sequence<IDX...> seq, std::initializer_list<U> ilist)
        : value_{ilist.begin()[IDX]...} {}

  public:
    using value_type = T[N];
    using reference = T(&)[N];
    using const_reference = std::add_const_t<T>(&)[N];
    using rvalue_reference = T(&&)[N];
    using const_rvalue_reference = std::add_const_t<T>(&&)[N];
    using pointer = T(*)[N];
    using const_pointer = std::add_const_t<T>(*)[N];

    static constexpr bool is_array = true;

    constexpr holder_impl() = default;

    template <typename... Args>
    constexpr holder_impl([[maybe_unused]] std::in_place_t in_place, Args&&... args)
        : value_{std::forward<Args>(args)...} {}

    template <typename U>
    constexpr holder_impl([[maybe_unused]] std::in_place_t in_place, std::initializer_list<U> ilist)
        : holder_impl(std::make_index_sequence<N>{}, ilist) {}

    constexpr reference get() & noexcept {
        return value_;
    }

    constexpr const_reference get() const& noexcept {
        return value_;
    }

    constexpr rvalue_reference get() && {
        return std::move(value_);
    }

    constexpr const_rvalue_reference get() const&& {
        return std::move(value_);
    }

    constexpr pointer ptr() noexcept {
        return &value_;
    }

    constexpr const_pointer ptr() const noexcept {
        return &value_;
    }

    constexpr void swap(holder_impl& other) noexcept {
        using std::swap;
        for (size_t i = 0; i < N; ++i) {
            swap(value_[i], other.value_[i]);
        }
    }
};

template <typename T, size_t N>
class holder_impl<T(&)[N]> {
  private:
    T(*value_)[N]{nullptr};

  public:
    using value_type = T(&)[N];
    using reference = value_type;
    using const_reference = value_type;
    using rvalue_reference = value_type;
    using const_rvalue_reference = value_type;
    using pointer = T(*)[N];
    using const_pointer = T(*)[N];

    static constexpr bool is_array = false;

    constexpr holder_impl() = default;

    constexpr holder_impl([[maybe_unused]] std::in_place_t in_place, T(&value)[N]) noexcept
        : value_(&value) {}

    constexpr reference get() const noexcept {
        return *value_;
    }

    constexpr pointer ptr() const noexcept {
        return value_;
    }

    constexpr void swap(holder_impl& other) {
        std::swap(value_, other.value_);
    }
};

}

/// @brief A copyable and movable type capable of holding any type
///
/// @details
/// jac::holder is a utility for abstracting away an underlying type for other
/// containers. It can hold references and `void` while minimizing differences
/// in behavior for these types compared to standard value types.
///
/// jac::holder also has an extra template parameter, `Tag` that can be used
/// to create a distinct "holder" type. For example, the following `my_holder`
/// type is a unique holder type that can be differentiated from other holders,
/// such as by using `std::is_same_v` or `std::is_convertible_v`.
/// ```
/// enum class my_holder_tag { };
///
/// template <typename T>
/// using my_holder = jac::holder<T, my_holder_tag>;
/// ```
template <typename T, typename Tag = void>
class holder {
  private:
    detail::holder_impl<T> value_;

  public:
    using value_type = typename detail::holder_impl<T>::value_type;
    using reference = typename detail::holder_impl<T>::reference;
    using const_reference = typename detail::holder_impl<T>::const_reference;
    using rvalue_reference = typename detail::holder_impl<T>::rvalue_reference;
    using const_rvalue_reference = typename detail::holder_impl<T>::const_rvalue_reference;
    using pointer = typename detail::holder_impl<T>::pointer;
    using const_pointer = typename detail::holder_impl<T>::const_pointer;
    using tag_type = Tag;

    constexpr holder() = default;

    constexpr holder(const holder&) = default;

    constexpr holder(holder&&) = default;

    template <typename... Args>
    constexpr explicit(sizeof...(Args) == 0) holder(std::in_place_t in_place, Args&&... args)
        : value_(in_place, std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    constexpr holder(std::in_place_t in_place, std::initializer_list<U> ilist, Args&&... args)
        : value_(in_place, ilist, std::forward<Args>(args)...) {}

    template <typename U>
        requires(
            std::is_constructible_v<value_type, typename holder<U, Tag>::const_reference> &&
            !std::is_constructible_v<value_type, holder<U, Tag>&> &&
            !std::is_constructible_v<value_type, const holder<U, Tag>&> &&
            !std::is_constructible_v<value_type, holder<U, Tag>&&> &&
            !std::is_constructible_v<value_type, const holder<U, Tag>&&> &&
            !std::is_convertible_v<holder<U, Tag>&, value_type> &&
            !std::is_convertible_v<const holder<U, Tag>&, value_type> &&
            !std::is_convertible_v<holder<U, Tag>&&, value_type> &&
            !std::is_convertible_v<const holder<U, Tag>&&, value_type>
        )
    constexpr explicit(!std::is_convertible_v<typename holder<U, Tag>::const_reference, value_type>)
    holder(const holder<U, Tag>& other)
        : value_(std::in_place, *other) {}

    template <typename U>
        requires(
            std::is_constructible_v<value_type, typename holder<U, Tag>::rvalue_reference> &&
            !std::is_constructible_v<value_type, holder<U, Tag>&> &&
            !std::is_constructible_v<value_type, const holder<U, Tag>&> &&
            !std::is_constructible_v<value_type, holder<U, Tag>&&> &&
            !std::is_constructible_v<value_type, const holder<U, Tag>&&> &&
            !std::is_convertible_v<holder<U, Tag>&, value_type> &&
            !std::is_convertible_v<const holder<U, Tag>&, value_type> &&
            !std::is_convertible_v<holder<U, Tag>&&, value_type> &&
            !std::is_convertible_v<const holder<U, Tag>&&, value_type>
        )
    constexpr explicit(!std::is_convertible_v<typename holder<U, Tag>::rvalue_reference, value_type>)
    holder(holder<U, Tag>&& other)
        : value_(std::in_place, *std::move(other)) {}

    template <typename U = value_type>
        requires(
            std::is_constructible_v<value_type, U&&> &&
            !std::is_same_v<std::remove_cvref_t<U>, std::in_place_t> &&
            !std::is_same_v<std::remove_cvref_t<U>, holder<T, Tag>>
        )
    constexpr explicit(!std::is_convertible_v<U&&, value_type>) holder(U&& value)
        : value_(std::in_place, std::forward<U>(value)) {}

    template <typename U = value_type>
        requires(
            detail::holder_impl<T>::is_array ||
            std::is_constructible_v<value_type, std::initializer_list<U>>
        )
    constexpr holder(std::initializer_list<U> ilist)
        : value_(std::in_place, ilist) {}

    constexpr ~holder() = default;

    constexpr holder& operator=(const holder&) = default;

    constexpr holder& operator=(holder&&) = default;

    template <typename U = value_type>
        requires(
            !std::is_same_v<std::remove_cvref_t<U>, holder<T, Tag>> &&
            std::is_constructible_v<value_type, U> &&
            std::is_assignable_v<reference, U> &&
            (!std::is_same_v<std::decay_t<U>, value_type> || !std::is_scalar_v<value_type>)
        )
    constexpr holder& operator=(U&& value) {
        value_.get() = std::forward<U>(value);
        return *this;
    }

    template <typename U>
        requires(
            !std::is_constructible_v<value_type, holder<U, Tag>&> &&
            !std::is_constructible_v<value_type, const holder<U, Tag>&> &&
            !std::is_constructible_v<value_type, holder<U, Tag>&&> &&
            !std::is_constructible_v<value_type, const holder<U, Tag>&&> &&
            !std::is_convertible_v<holder<U, Tag>&, value_type> &&
            !std::is_convertible_v<const holder<U, Tag>&, value_type> &&
            !std::is_convertible_v<holder<U, Tag>&&, value_type> &&
            !std::is_convertible_v<const holder<U, Tag>&&, value_type> &&
            !std::is_assignable_v<value_type, holder<U, Tag>&> &&
            !std::is_assignable_v<value_type, const holder<U, Tag>&> &&
            !std::is_assignable_v<value_type, holder<U, Tag>&&> &&
            !std::is_assignable_v<value_type, const holder<U, Tag>&&>
        )
    constexpr holder& operator=(const holder<U, Tag>& other) {
        value_.get() = *other;
        return *this;
    }

    template <typename U>
        requires(
            !std::is_constructible_v<value_type, holder<U, Tag>&> &&
            !std::is_constructible_v<value_type, const holder<U, Tag>&> &&
            !std::is_constructible_v<value_type, holder<U, Tag>&&> &&
            !std::is_constructible_v<value_type, const holder<U, Tag>&&> &&
            !std::is_convertible_v<holder<U, Tag>&, value_type> &&
            !std::is_convertible_v<const holder<U, Tag>&, value_type> &&
            !std::is_convertible_v<holder<U, Tag>&&, value_type> &&
            !std::is_convertible_v<const holder<U, Tag>&&, value_type> &&
            !std::is_assignable_v<value_type, holder<U, Tag>&> &&
            !std::is_assignable_v<value_type, const holder<U, Tag>&> &&
            !std::is_assignable_v<value_type, holder<U, Tag>&&> &&
            !std::is_assignable_v<value_type, const holder<U, Tag>&&>
        )
    constexpr holder& operator=(holder<U, Tag>&& other) {
        value_.get() = *std::move(other);
        return *this;
    }

    constexpr reference value() & noexcept {
        return value_.get();
    }

    constexpr const_reference value() const& noexcept {
        return value_.get();
    }

    constexpr rvalue_reference value() && {
        return std::move(value_).get();
    }

    constexpr const_rvalue_reference value() const&& {
        return std::move(value_).get();
    }

    constexpr reference operator*() & noexcept {
        return value_.get();
    }

    constexpr const_reference operator*() const& noexcept {
        return value_.get();
    }

    constexpr rvalue_reference operator*() && {
        return std::move(value_).get();
    }

    constexpr const_rvalue_reference operator*() const&& {
        return std::move(value_).get();
    }

    constexpr pointer operator->() noexcept {
        return value_.ptr();
    }

    constexpr const_pointer operator->() const noexcept {
        return value_.ptr();
    }

    constexpr void swap(holder& other) {
        value_.swap(other.value_);
    }

    template <typename F>
    constexpr auto transform(F&& f) & {
        return holder<std::invoke_result_t<F, reference>, Tag>(std::invoke(std::forward<F>(f)));
    }

    template <typename F>
    constexpr auto transform(F&& f) const& {
        return holder<std::invoke_result_t<F, const_reference>, Tag>(std::invoke(std::forward<F>(f)));
    }

    template <typename F>
    constexpr auto transform(F&& f) && {
        return holder<std::invoke_result_t<F, rvalue_reference>, Tag>(std::invoke(std::forward<F>(f)));
    }

    template <typename F>
    constexpr auto transform(F&& f) const&& {
        return holder<std::invoke_result_t<F, const_rvalue_reference>, Tag>(std::invoke(std::forward<F>(f)));
    }

    constexpr holder<reference, Tag> as_ref() & noexcept {
        return holder<reference, Tag>(value_.get());
    }

    constexpr holder<const_reference, Tag> as_ref() const& noexcept {
        return holder<const_reference, Tag>(value_.get());
    }

    template <typename U>
    friend constexpr std::compare_three_way_result_t<typename holder<T, Tag>::value_type, typename holder<U, Tag>::value_type>
    operator<=>(const holder<T, Tag>& lhs, const holder<U, Tag>& rhs) {
        return *lhs <=> *rhs;
    }

    template <typename U>
    friend constexpr bool operator==(const holder<T, Tag>& lhs, const holder<U, Tag>& rhs) {
        return *lhs == *rhs;
    }

    friend constexpr void swap(holder<T, Tag>& a, holder<T, Tag>& b) {
        a.swap(b);
    }
};

template <typename T>
holder(T&&) -> holder<T>;

}

template <typename T, typename Tag>
struct std::hash<::jac::holder<T, Tag>> {
  private:
    JAC_NO_UNIQ_ADDR std::hash<typename ::jac::holder<T, Tag>::value_type> inner_;

  public:
    constexpr size_t operator()(const ::jac::holder<T, Tag>& value) const {
        return inner_(value);
    }
};

#endif
