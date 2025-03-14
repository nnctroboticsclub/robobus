#pragma once

#include <coroutine>
#include "coroutine_awaiter.hpp"
#include "promise.hpp"
#include "robobus/coroutine/generic_awaiter.hpp"

namespace robobus::coroutine {
template <typename ReturnType>
struct CoroutineAwaiter;

//* Coroutine
template <typename ReturnType>
struct Coroutine {
  using coro_handle = std::coroutine_handle<Promise<ReturnType>>;
  using promise_type = Promise<ReturnType>;

  explicit Coroutine(coro_handle handle) : handle(handle) {}

  coro_handle handle;

  auto operator co_await() {
    return CoroutineAwaiter<ReturnType>{handle.promise()};
  }
};

template <typename T, typename RetT>
concept CoroutineLike = requires(T t) {  //
  typename T::coro_handle;

  { t.operator co_await() } -> AwaiterLike<RetT>;
};

}  // namespace robobus::coroutine

namespace robobus {
using coroutine::Coroutine;
}