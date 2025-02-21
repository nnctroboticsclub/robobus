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

#include <type_traits>
#include "robobus/coroutine/generic_awaiter.hpp"
#include "robobus/runtime/loop.hpp"
#include "robobus/runtime/runtime_impls.hpp"

namespace robobus::runtime {
/// @brief 遅延再開 Awaiter
/// @tparam T コルーチンの戻り値の型
template <typename T, RuntimeImpl Runtime>
requires std::is_copy_constructible_v<T> class LazyResumerAwaiter
    : public coroutine::IAwaiter<T> {
  Loop<Runtime>& loop;
  std::coroutine_handle<> handle = nullptr;
  std::optional<T> value;

 public:
  explicit LazyResumerAwaiter(Loop<Runtime>& ctx) : loop(ctx) {}

  void Resume(T value) final {
    this->value = value;
    loop.ScheduleResumeInstantly(handle);
  }

  bool await_ready() const final { return false; }

  void await_suspend(std::coroutine_handle<> handle) final {
    this->handle = handle;
  }

  T await_resume() final { return *value; }
};
struct LazyResumer {};
}  // namespace robobus::runtime