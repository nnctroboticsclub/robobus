#pragma once

#include <string>
#include <string_view>

#include <chrono>
#include <utility>
#include "robobus/runtime/runtime_impls.hpp"

namespace robobus::context {
template <runtime::RuntimeImpl Clock>
class SharedContext;
}

namespace robobus::debug {

template <runtime::RuntimeImpl Runtime>
class DebugInfo {
 public:
  explicit DebugInfo(context::SharedContext<Runtime>* ctx, std::string tag)
      : ctx_(ctx), tag_(std::move(tag)) {}

  auto Message(std::string_view message) -> void {
    auto adapter = ctx_.Root().lock()->GetDebugAdapter();
    if (adapter.has_value()) {
      adapter.value()->Message(tag_, message);
    }
  }

 private:
  context::SharedContext<Runtime> ctx_;
  std::string tag_;
};

}  // namespace robobus::debug
