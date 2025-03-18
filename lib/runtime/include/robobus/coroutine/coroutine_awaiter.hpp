#pragma once

#include <coroutine>

#include "logger/logger.hpp"
#include "promise.hpp"
#include "robobus/coroutine/generic_awaiter.hpp"

namespace robobus::coroutine {

robotics::logger::Logger logger{"coro.rb", "RB/Coro"};

template <typename ReturnType>
struct CoroutineAwaiter;

//* CoroutineAwaiter
template <std::copy_constructible ReturnType>
struct CoroutineAwaiter<ReturnType> {
  explicit CoroutineAwaiter(Promise<ReturnType>& promise) : promise(promise) {
    waitee_handle =
        std::coroutine_handle<Promise<ReturnType>>::from_promise(promise);
  }

  bool await_ready() const {
    // printf("\x1b[41m  \x1b[0m%p %p.done? --> %d\n", this,
    //        waitee_handle.address(), promise.IsFinished());
    return promise.IsFinished();
  }

  void await_suspend(std::coroutine_handle<> waiter_handle) {
    // printf("\x1b[41m  \x1b[0m%p __GRAPH__ %p --| Ca_%p |--> %p\n", this,
    //        waiter_handle.address(), this, waitee_handle.address());
    promise.AddOnReturnCallback([this, waiter_handle]() {
      // printf("\x1b[41m  \x1b[0m%p __GRAPH__ %p --| Ca_%p - resumed |--> %p\n",
      //        this, waitee_handle.address(), this, waiter_handle.address());
      waiter_handle.resume();
    });
  }

  auto await_resume() const {
    if (promise.IsFinished()) {
      // printf("\x1b[41m  \x1b[0m%p Await_resume with fulfilled promise\n", this);
      return promise.get_return_value().value();
    } else {
      // printf("\x1b[41m  \x1b[0m%p Await_resume with pending promise\n", this);
      while (true)
        ;
    }
  }

 private:
  Promise<ReturnType>& promise;
  std::coroutine_handle<> waitee_handle;
};

template <>
struct CoroutineAwaiter<void> {
  explicit CoroutineAwaiter(Promise<void>& promise) : promise(promise) {
    waitee_handle = std::coroutine_handle<Promise<void>>::from_promise(promise);
  }

  bool await_ready() const {
    // printf("\x1b[41m  \x1b[0m%p %p.done? --> %d\n", this,
    //        waitee_handle.address(), promise.IsFinished());
    return promise.IsFinished();
  }
  void await_suspend(std::coroutine_handle<> waiter_handle) {
    // printf("\x1b[41m  \x1b[0m%p __GRAPH__ %p --| Ca_%p |--> %p\n", this,
    //        waiter_handle.address(), this, waitee_handle.address());
    promise.AddOnReturnCallback([this, waiter_handle]() {
      // printf("\x1b[41m  \x1b[0m%p __GRAPH__ %p --| Ca_%p - resumed |--> %p\n",
      //        this, waitee_handle.address(), this, waiter_handle.address());
      waiter_handle.resume();
    });
  }
  auto await_resume() const { return; }

 private:
  Promise<void>& promise;
  std::coroutine_handle<> waitee_handle;
};

static_assert(AwaiterLike<CoroutineAwaiter<void>, void>,
              "CoroutineAwaiter<void> must be awaiter");
}  // namespace robobus::coroutine