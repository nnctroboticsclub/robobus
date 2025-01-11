#pragma once

#include <string_view>

#include "robobus/internal/string_literal.hpp"
#include "robobus/runtime/runtime_impls.hpp"

namespace robobus::context {
template <runtime::RuntimeImpl Clock>
class SharedContext;
}

namespace robobus::debug {

template <runtime::RuntimeImpl Runtime, internal::StringLiteral kTag>
class DebugInfo {
 public:
  auto Message(std::string_view message) -> void {
    Runtime::DebugAdapter::Message(kTag.data, message);
  }
};

}  // namespace robobus::debug
