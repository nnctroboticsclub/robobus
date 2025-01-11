#pragma once

#include <cstring>
#include <optional>

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
  RootContext<Runtime>* root_ctx_;
  robotics::logger::Logger* logger_;

 public:
  explicit Context(RootContext<Runtime>* root) : root_ctx_(root) {}

  ~Context() = default;

  constexpr char const* ContextId() { return kPath.data; }

  constexpr RootContext<Runtime>* Root() { return root_ctx_; }

  constexpr runtime::Loop<Runtime>& GetLoop() { return Root()->GetLoop(); }

  template <internal::StringLiteral tag>
  auto& Child() {
    return *new Context<Runtime, ConcatPath<kPath, tag>>(Root());
  }

  auto Sleep(std::chrono::milliseconds duration) {
    return runtime::Sleep(GetLoop(), duration);
  }

  template <internal::StringLiteral kName>
  auto GetDebugInfo() {
    return debug::DebugInfo<Runtime, ConcatPath<kPath, kName>>();
  }

  auto& Logger() {
    if (!logger_) {
      auto cid_cstr = new char[decltype(kPath)::size + 1];
      std::strcpy(cid_cstr, kPath.data);

      logger_ = new robotics::logger::Logger(cid_cstr, cid_cstr);
    }

    return *logger_;
  }
};

}  // namespace robobus::context