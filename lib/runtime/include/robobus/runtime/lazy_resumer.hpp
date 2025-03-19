#pragma once

/// @file robobus/runtime/lazy_resumer.hpp
/// @note 遅延再開
/// @details
/// 　遅延再開機能は Loop のスタックを用いてコルーチンの再開をサポートする．
/// 　これにより，コルーチンの再開時にスタックオーバーフローを回避することができる．
/// また， ISR コンテキストから Resume した際，ISR コンテキストからすぐ抜けることができる．
/// これによってリアルタイム性を担保することができる．
/// 　ただし，通信などのパース処理については LazyResumer を使用することは推奨されない．
/// これは，パース処理は軽量に済まし，ディスパッチに LazyResumer を採用するほうが合理的であるためである．
/// ただし，リアルタイム性が必要な通信においてはその限りではない．
/// @note この機能は，Loop の ScheduleResumeInstantly によって Resume される．

#include <coroutine>
#include <functional>
#include <type_traits>
#include "robobus/coroutine/generic_awaiter.hpp"
#include "robobus/runtime/loop.hpp"
#include "robobus/runtime/runtime_impls.hpp"

namespace robobus::runtime {
/// @brief 遅延再開 Awaiter
/// @tparam T コルーチンの戻り値の型
template <typename T>
requires std::is_copy_constructible_v<T> class LazyResumerAwaiter
    : public coroutine::IAwaiter<T> {

  bool waiter_available = false;
  std::coroutine_handle<> handle = nullptr;
  std::optional<T> value = std::nullopt;
  std::function<void(std::coroutine_handle<>)> instant_resumer = nullptr;

 public:
  ~LazyResumerAwaiter() = default;

  template <RuntimeImpl Runtime>
  void AssociateLoop(Loop<Runtime>& loop) {
    instant_resumer = [&loop](std::coroutine_handle<> handle) {
      loop.ScheduleResumeInstantly(handle);
    };
  }

  template <RuntimeImpl Runtime>
  void Resume(Loop<Runtime>& loop, T value) {
    if (not waiter_available) {
      printf("\x1b[2;35m(%p)\x1b[m %p: Resumed without awaiter.", this, handle);
      while (true)
        ;
    }

    this->value = value;
    loop.ScheduleResumeInstantly(handle);
    this->waiter_available = false;
    this->handle = nullptr;
  }

  void Resume(T value) final {
    if (not waiter_available) {
      printf("\x1b[2;35m(%p)\x1b[m %p: Resumed without awaiter.", this, handle);
      while (true)
        ;
    }

    if (not instant_resumer) {
      printf("\x1b[2;35m(%p)\x1b[m %p: Instant resumer is not set.", this,
             handle);
      while (true)
        ;
    }

    this->value = value;
    instant_resumer(handle);
    printf("\x1b[2;35m(%p)\x1b[m %p: Resumed.", this, handle.address());

    this->waiter_available = false;
    this->handle = nullptr;
  }

  bool await_ready() const final { return waiter_available; }

  void await_suspend(std::coroutine_handle<> handle) final {
    printf("\x1b[2;35m(%p)\x1b[m %p: Suspended", this, handle.address());
    waiter_available = true;
    this->handle = handle;
  }

  T await_resume() final { return *value; }
};
}  // namespace robobus::runtime