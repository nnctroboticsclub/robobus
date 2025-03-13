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

 public:
  ~ImmediacyAwaiter() final = default;

  void Reset() {
    // logger.Trace("\x1b[2;35m(%p)\x1b[m Resetted awaiter", this);
    return_value = std::nullopt;
    this->waiter_available = false;
  }

  void Resume(T value) final {
    // logger.Trace("\x1b[2;35m(%p)\x1b[m %p: Resume", this, handle);
    this->return_value = value;
    this->handle.resume();
  }

  bool IsWaiterAvailable() const { return waiter_available; }

  bool await_ready() const final { return return_value.has_value(); }

  void await_suspend(std::coroutine_handle<> handle) final {
    this->handle = handle;
    this->waiter_available = true;

    // logger.Trace("\x1b[2;35m(%p)\x1b[m %p: Suspended", this, handle);
  }

  T await_resume() final {
    // we dont need check if return_value has any value since this awaiter
    // must be resumed by call the ImmediacyAwaiter::Resume()
    // logger.Trace("\x1b[2;35m(%p)\x1b[m %p: Returning value", this, handle);
    return *return_value;
  }

 public:
  bool waiter_available = false;
  std::coroutine_handle<> handle = nullptr;
  std::optional<T> return_value = std::nullopt;
};
}  // namespace robobus::coroutine