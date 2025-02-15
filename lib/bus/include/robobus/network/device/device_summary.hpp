#pragma once

#include <string>
#include <vector>

#include "../interface/interface_summary.hpp"

namespace robobus::network {
/// @brief デバイスの概要
struct DeviceSummary {
  /// @brief デバイスの作成者
  std::string creator;

  /// @brief デバイスの名前
  std::string name;

  /// @brief デバイスが持つインタフェースのリスト
  std::vector<InterfaceSummary> interfaces;
};
}  // namespace robobus::network