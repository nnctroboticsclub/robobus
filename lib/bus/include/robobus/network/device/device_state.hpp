#pragma once

namespace robobus::network::device {
/// @brief デバイスの状態を表す列挙型
enum class State {
  kNotInitialized,
  kIdWaiting,
  kInitialized,
};
}  // namespace robobus::network::device