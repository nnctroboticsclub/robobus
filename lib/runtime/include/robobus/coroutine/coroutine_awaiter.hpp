#pragma once

#include <coroutine>

#include "promise.hpp"
#include "robobus/coroutine/generic_awaiter.hpp"

namespace robobus::coroutine {

template <typename ReturnType>
struct CoroutineAwaiter;

//* CoroutineAwaiter
template <std::move_constructible ReturnType>
struct CoroutineAwaiter<ReturnType> {
  explicit CoroutineAwaiter(Promise<ReturnType>& promise) : promise(promise) {}

  bool await_ready() const { return (bool)promise.get_return_value(); }

  void await_suspend(std::coroutine_handle<> handle) {
    promise.AddOnReturnCallback([handle]() { handle.resume(); });
  }

  auto await_resume() const {
    if (promise.get_return_value()) {
      return std::move(promise.get_return_value().value());
    } else {
      return ReturnType{};
    }
  }

 private:
  Promise<ReturnType>& promise;
};

template <>
struct CoroutineAwaiter<void> {
  explicit CoroutineAwaiter(Promise<void>& promise) : promise(promise) {}

  bool await_ready() const { return promise.get_return_value(); }
  void await_suspend(std::coroutine_handle<> handle) {
    promise.AddOnReturnCallback([handle]() { handle.resume(); });
  }
  auto await_resume() const { return; }

 private:
  Promise<void>& promise;
};

static_assert(AwaiterLike<CoroutineAwaiter<void>, void>,
              "CoroutineAwaiter<void> must be awaiter");
}  // namespace robobus::coroutine