#pragma once

#include <string>
#include <vector>

#include "../interface/interface_summary.hpp"
#include "./device_meta.hpp"

namespace robobus::network {
/// @brief デバイスの概要
struct DeviceSummary {
  DeviceMeta meta;

  /// @brief デバイスが持つインタフェースのリスト
  std::vector<InterfaceSummary> interfaces;
};
}  // namespace robobus::network