#pragma once

#include <cstddef>
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

  template <size_t M>
  StringLiteral<N + M> operator+(const StringLiteral<M>& rhs) const {
    char new_data[N + M + 1];
    for (size_t i = 0; i < N; ++i) {
      new_data[i] = data[i];
    }
    for (size_t i = 0; i < M; ++i) {
      new_data[N + i] = rhs.data[i];
    }
    new_data[N + M] = '\0';
    return StringLiteral<N + M>(new_data);
  }

  StringLiteral<N + 1> operator+(char ch) {
    char new_data[N + 2];
    for (size_t i = 0; i < N; ++i) {
      new_data[i] = data[i];
    }
    new_data[N] = ch;
    new_data[N + 1] = '\0';
    return StringLiteral<N + 1>(new_data);
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