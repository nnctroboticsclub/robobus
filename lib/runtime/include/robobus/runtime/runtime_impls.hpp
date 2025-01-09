#pragma once

#include <chrono>
#include <string_view>
#include <utility>

namespace robobus::runtime {

template <typename T>
concept RuntimeImpl = requires {
  T::DebugAdapter::Message(std::declval<std::string_view>(),
                           std::declval<std::string_view>());
}
&&std::chrono::is_clock_v<typename T::Clock>;
}  // namespace robobus::runtime