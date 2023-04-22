#ifndef JAC_UTILS_HPP
#define JAC_UTILS_HPP

/// @file

#include <cstddef>

namespace jac {

constexpr size_t hash_combine(size_t x, size_t y) noexcept {
    return x ^ (y + 0x9e3779b9 + (x << 6) + (x >> 2));
}

} // namespace jac

#endif
