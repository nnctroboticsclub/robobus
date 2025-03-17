#pragma once

#include <optional>

#include <robobus/coroutine/generic_awaiter.hpp>
#include <robobus/internal/sematicses.hpp>

#include <logger/logger.hpp>

namespace robobus::coroutine {
/// @brief Resume が呼ばれたらそのコンテキストでコルーチンを再開するためのAwaiter
/// @details この Awaiter は主に， Await する側と Resume する側を分離することが目的
/// これにより，コールバック関数 + ステートマシンのような構成のプログラムを
/// 純粋なコルーチンで実装することができる．また，このクラスは Loop を使用しないため，
/// Loop& を必要としない
template <typename T>
class ImmediacyAwaiter final
    : public robobus::coroutine::IAwaiter<T>,
      public robobus::internal::NonCopyable<ImmediacyAwaiter<T>> {
  static inline robotics::logger::Logger logger{"coroutine.imm-await",
                                                "ImmediacyAwaiter"};

  static constexpr bool kVerbose = false;

 public:
  ~ImmediacyAwaiter() final = default;

  void Resume(T value) final {
    if (not waiter_available) {
      logger.Error("\x1b[2;35m(%p)\x1b[m %p: Resumed without awaiter.", this,
                   handle);
      asm("bkpt #1");
    }

    if (kVerbose)
      logger.Trace("\x1b[2;35m(%p)\x1b[m %p: Resume", this, handle);

    auto handle_ = handle;

    this->waiter_available = false;
    this->return_value = value;
    this->handle = nullptr;

    handle_.resume();
  }

  bool await_ready() const final { return waiter_available; }

  void await_suspend(std::coroutine_handle<> handle) final {
    this->waiter_available = true;
    this->handle = handle;

    if (kVerbose)
      logger.Trace("\x1b[2;35m(%p)\x1b[m %p: Suspended", this, handle);
  }

  T await_resume() final {
    // we dont need check if return_value has any value since this awaiter
    // must be resumed by call the ImmediacyAwaiter::Resume()
    if (kVerbose)
      logger.Trace("\x1b[2;35m(%p)\x1b[m %p: Returning value", this, handle);
    return *return_value;
  }

 public:
  bool waiter_available = false;
  std::coroutine_handle<> handle = nullptr;
  std::optional<T> return_value = std::nullopt;
};
}  // namespace robobus::coroutine