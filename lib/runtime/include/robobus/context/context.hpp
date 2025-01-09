#pragma once

#include <cstring>
#include <optional>

#include <logger/logger.hpp>

#include "../debug/debug_info.hpp"
#include "../internal/sematicses.hpp"
#include "../runtime/sleep.hpp"
#include "robobus/runtime/runtime_impls.hpp"
#include "root_context.hpp"

namespace robobus::context {
template <runtime::RuntimeImpl Runtime>
class RootContext;

/// @brief コルーチンベースプログラムで用いるコンテキスト
template <runtime::RuntimeImpl Runtime, >
class Context : public internal::NonCopyable<Context<Runtime>> {
 private:
  std::string tag_;
  std::string path_;

  RootContext<Runtime>* root_ctx_;
  std::optional<robotics::logger::Logger> logger_;

 public:
  explicit Context(RootContext<Runtime>* root, std::string_view tag,
                   std::string_view parent_path)
      : tag_(tag),
        path_(std::string(parent_path) + "." + std::string(tag)),
        root_ctx_(root) {}

  explicit Context(RootContext<Runtime>* root, std::string_view tag)
      : tag_(tag), path_(tag), root_ctx_(root) {}

  ~Context() = default;

  auto ContextId() { return path_; }

  RootContext<Runtime>* Root() { return root_ctx_; }

  runtime::Loop<Runtime>& GetLoop() { return Root()->GetLoop(); }

  Context<Runtime>* Child(std::string tag) {
    return new Context<Runtime>(Root(), tag, ContextId());
  }

  inline auto AddTask(std::coroutine_handle<> coroutine) {
    Root().Getloop().AddTask(coroutine);
  }

  auto Sleep(std::chrono::milliseconds duration) {
    return runtime::Sleep(GetLoop(), duration);
  }

  auto GetDebugInfo(std::string const& tag) -> debug::DebugInfo<Runtime> {
    return debug::DebugInfo<Runtime>(*this, ContextId() + "." + tag);
  }

  auto Logger() -> robotics::logger::Logger& {
    if (!logger_.has_value()) {
      auto cid = ContextId();

      auto cid_cstr = new char[cid.size() + 1];
      std::strcpy(cid_cstr, cid.c_str());

      logger_ = robotics::logger::Logger(tag_.c_str(), cid_cstr);
    }

    return logger_.value();
  }
};

}  // namespace robobus::context