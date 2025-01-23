#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace robobus::internal {
template <size_t N>
struct StringLiteral {
  static constexpr size_t size = N;
  char data[N + 1];

  consteval StringLiteral(const char (&str)[N + 1]) {
    for (size_t i = 0; i < N; ++i) {
      data[i] = str[i];
    }
    data[N] = '\0';
  }

  constexpr StringLiteral() : data() {}

  template <size_t M>
  constexpr StringLiteral<N + M> operator+(
      const StringLiteral<M>& other) const {
    StringLiteral<N + M> result;
    std::copy(data, data + N, result.data);
    std::copy(other.data, other.data + M + 1, result.data + N);
    return result;
  }

  constexpr StringLiteral<N + 1> operator+(const char ch) const {
    StringLiteral<N + 1> result{};
    std::copy(data, data + N, result.data);
    result.data[N] = ch;
    result.data[N + 1] = '\0';
    return result;
  }
};

template <size_t N>
StringLiteral(const char (&)[N]) -> StringLiteral<N - 1>;

template <typename T>
constexpr bool IsStringLiteral_v = false;

template <size_t N>
constexpr bool IsStringLiteral_v<StringLiteral<N>> = true;

template <typename T>
concept IsStringLiteral = IsStringLiteral_v<T>;

}  // namespace robobus::internal