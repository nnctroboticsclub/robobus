#pragma once

#include "../runtime/loop.hpp"
#include "robobus/context/context.hpp"
#include "robobus/internal/sematicses.hpp"
#include "robobus/internal/string_literal.hpp"
#include "robobus/runtime/runtime_impls.hpp"

namespace robobus::context {

template <runtime::RuntimeImpl Runtime, internal::StringLiteral kPath>
class Context;

/// @brief コルーチンベースプログラムで用いるコンテキスト
template <runtime::RuntimeImpl Runtime>
class RootContext : public internal::NonCopyable<RootContext<Runtime>> {
 private:
  runtime::Loop<Runtime> loop_{};

 public:
  RootContext() = default;

  [[noreturn]] auto Run() -> void { loop_.Run(); }

  auto GetLoop() -> runtime::Loop<Runtime>& { return loop_; }

  template <internal::StringLiteral tag>
  auto Child() {
    return Context<Runtime, tag>(this);
  }
};
}  // namespace robobus::context
