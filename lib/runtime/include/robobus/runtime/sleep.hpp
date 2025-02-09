#pragma once

#include "loop.hpp"
#include "robobus/runtime/runtime_impls.hpp"

namespace robobus::runtime {
/// @brief Sleep を行うための awaiter
template <runtime::RuntimeImpl Runtime>
struct SleepAwaiter {
  Loop<Runtime>& ctx;
  Runtime::Clock::duration duration;

  explicit SleepAwaiter(Loop<Runtime>& ctx,
                        typename Runtime::Clock::duration duration)
      : ctx(ctx), duration(duration) {}

  // Sleep が実行済かどうか
  // 今回は常に false を返す
  //  (Sleep した後は true
  //  になる．しかし，その時コルーチンは廃棄されているので)．
  bool await_ready() const { return false; }

  void await_suspend(std::coroutine_handle<> handle) {
    ctx.RequestResumeAt(ctx.time.Now() + duration, handle);
  }

  void await_resume() const { return; }
};

/// @brief Robobus Loop を用いて Sleep を行う
/// @tparam Runtime Runtime の型
/// @param ctx Loop のコンテキスト
/// @param duration Sleep する時間
/// @return Awaiter
template <RuntimeImpl Runtime>
auto Sleep(Loop<Runtime>& ctx, typename Runtime::Clock::duration duration) {
  return SleepAwaiter(ctx, duration);
}
}  // namespace robobus::runtime