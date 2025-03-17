#pragma once

#include <cstring>

#include <logger/logger.hpp>

#include "../debug/debug_info.hpp"
#include "../internal/sematicses.hpp"
#include "../runtime/sleep.hpp"
#include "robobus/internal/string_literal.hpp"
#include "robobus/runtime/runtime_impls.hpp"
#include "root_context.hpp"

namespace robobus::context {
template <runtime::RuntimeImpl Runtime>
class RootContext;

template <internal::StringLiteral parent_path, internal::StringLiteral tag>
constexpr auto ConcatPath = parent_path + '.' + tag;

/// @brief コルーチンベースプログラムで用いるコンテキスト
template <runtime::RuntimeImpl Runtime, internal::StringLiteral kPath>
class Context : public internal::NonCopyable<Context<Runtime, kPath>> {
 private:
  RootContext<Runtime>* root_ctx_ = nullptr;
  robotics::logger::Logger* logger_ = nullptr;

 public:
  Context(Context<Runtime, kPath>&& other) noexcept
      : root_ctx_(other.root_ctx_), logger_(other.logger_) {
    other.root_ctx_ = nullptr;
    other.logger_ = nullptr;
  }

  explicit Context(RootContext<Runtime>* root) : root_ctx_(root) {}

  ~Context() = default;

  constexpr char const* ContextId() { return kPath.data; }

  constexpr RootContext<Runtime>* Root() { return root_ctx_; }

  constexpr runtime::Loop<Runtime>& GetLoop() { return Root()->GetLoop(); }

  template <internal::StringLiteral tag>
  auto Child() {
    return Context<Runtime, ConcatPath<kPath, tag>>(Root());
  }

  auto Sleep(std::chrono::milliseconds duration) {
    return runtime::Sleep(GetLoop(), duration);
  }

  template <internal::StringLiteral kName>
  auto GetDebugInfo() -> debug::DebugInfo<Runtime, ConcatPath<kPath, kName>> {
    return debug::DebugInfo<Runtime, ConcatPath<kPath, kName>>();
  }
};

}  // namespace robobus::context

namespace robobus {
using context::Context;
}  // namespace robobus
