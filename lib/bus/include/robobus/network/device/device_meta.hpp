#pragma once

#include <string>

namespace robobus::network {
/// @brief デバイスの実行にかかわらないメタ情報
struct DeviceMeta {
  /// @brief デバイスの作成者
  std::string creator;

  /// @brief デバイスの名前
  std::string name;
};
}  // namespace robobus::network