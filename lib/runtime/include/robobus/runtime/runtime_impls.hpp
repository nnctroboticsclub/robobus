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

struct DummyRuntime {
  struct Clock {
    using duration = std::chrono::milliseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<Clock>;
    static constexpr bool is_steady = true;

    static time_point now() { return time_point{}; }
  };

  struct DebugAdapter {
    static void Message(std::string_view, std::string_view) {}
  };
};
static_assert(RuntimeImpl<DummyRuntime>, "Invalid RuntimeImpl");

}  // namespace robobus::runtime