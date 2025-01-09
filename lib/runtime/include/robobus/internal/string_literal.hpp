#pragma once

#include <cstddef>
namespace robobus::internal {
template <size_t N, char... Char>
struct StringLiteral {
  static constexpr char data[N + 1] = {Char..., '\0'};

  template <size_t M, char... Char2>
  constexpr auto operator+(const StringLiteral<M, Char2...>&) const {
    return StringLiteral<N + M, Char..., Char2...>{};
  }

  template <char c>
  constexpr auto operator+() const {
    return StringLiteral<N + 1, Char..., c>{};
  }
};

}  // namespace robobus::internal