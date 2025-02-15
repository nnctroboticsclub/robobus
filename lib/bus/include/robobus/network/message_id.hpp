#pragma once

#include <cstdint>
namespace robobus::network {
/// @brief CAN 上でのデータ種別を識別するための列挙型
/// @details このデータはメッセージ ID に埋め込められ，データ幅は 3 bit である．
enum class MessageKind : uint8_t {
  kEmergency = 0,
  kRawP2P = 1,
  kStream = 2,
  kMultiCast = 3,

  // 4 --- 7: Reserved

  kMax
};

static_assert(static_cast<int>(MessageKind::kMax) <= 8, "Too many kinds");

/// @brief Robobus プロトコルにおける CAN から受信されるメッセージ ID
class MessageID {
  uint32_t id_;

 public:
  explicit MessageID(uint32_t id) : id_(id) {}

  [[nodiscard]] auto Get() const -> uint32_t { return id_; }

  [[nodiscard]] MessageKind GetKind() const {
    return static_cast<MessageKind>((id_ >> 24) & 7);
  }
};
};  // namespace robobus::network