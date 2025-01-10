#pragma once

#include <memory>

#include "../debug/debug_adapter.hpp"
#include "../runtime/loop.hpp"
#include "robobus/context/context.hpp"
#include "robobus/internal/string_literal.hpp"
#include "robobus/runtime/runtime_impls.hpp"

namespace robobus::context {

template <runtime::RuntimeImpl Runtime, internal::StringLiteral kPath>
class Context;

/// @brief コルーチンベースプログラムで用いるコンテキスト
template <runtime::RuntimeImpl Runtime>
class RootContext {
 private:
  runtime::Loop<Runtime> loop_{};

 public:
  [[noreturn]] auto Run() -> void { loop_.Run(); }

  auto GetLoop() -> runtime::Loop<Runtime>& { return loop_; }

  template <internal::StringLiteral tag>
  auto& Child() {
    return *new Context<Runtime, tag>(this);
  }
};
}  // namespace robobus::context
