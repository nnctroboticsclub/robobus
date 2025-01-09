#pragma once

#include <memory>

#include "../debug/debug_adapter.hpp"
#include "../runtime/loop.hpp"
#include "robobus/context/context.hpp"
#include "robobus/runtime/runtime_impls.hpp"

namespace robobus::context {

template <runtime::RuntimeImpl Runtime>
class Context;

/// @brief コルーチンベースプログラムで用いるコンテキスト
template <runtime::RuntimeImpl Runtime>
class RootContext {
 private:
  runtime::Loop<Runtime> loop_{};

 public:
  [[noreturn]] auto Run() -> void { loop_.Run(); }

  auto GetLoop() -> runtime::Loop<Runtime>& { return loop_; }

  inline auto AddTask(std::coroutine_handle<> coroutine) -> void {
    loop_.AddTask(coroutine);
  }

  Context<Runtime>& Child(std::string tag) {
    return *new Context<Runtime>(this, tag);
  }
};
}  // namespace robobus::context
