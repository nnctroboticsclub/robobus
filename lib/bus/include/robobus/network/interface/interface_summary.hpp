#pragma once

#include <cstdint>

#include <string>

namespace robobus::network {

/// @brief Interface の概要
struct InterfaceSummary {
  /// @brief ポート番号
  uint8_t port;

  /// @brief インタフェースの ID
  uint8_t id;

  /// @brief インタフェースの名前
  std::string name;

  bool operator==(InterfaceSummary const& rhs) const {
    return port == rhs.port && id == rhs.id && name == rhs.name;
  }
};

}  // namespace robobus::network