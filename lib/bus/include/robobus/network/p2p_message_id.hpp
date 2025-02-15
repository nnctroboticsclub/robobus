#pragma once

#include "address.hpp"
#include "message_id.hpp"

namespace robobus::network {
/// @brief 1:1 通信におけるメッセージ ID を表す型．
/// @details メッセージ ID には，送信元，送信先，インタフェース ID が含まれる．
class P2PMessageID : public MessageID {
 public:
  using MessageID::MessageID;

  [[nodiscard]] auto GetSource() const -> Address {
    return Address((Get() >> 16) & 0xFF);
  }

  [[nodiscard]] auto GetDestination() const -> Address {
    return Address((Get() >> 8) & 0xFF);
  }

  [[nodiscard]] auto GetID() const -> uint8_t { return Get() & 0xff; }

  static auto FromParts(Address from, Address dest, uint8_t id) {
    uint32_t raw_msg_id = 0;
    raw_msg_id |= static_cast<int>(MessageKind::kRawP2P) << 24;
    raw_msg_id |= from.Get() << 16;
    raw_msg_id |= dest.Get() << 8;
    raw_msg_id |= id;

    return P2PMessageID(raw_msg_id);
  }
};
}  // namespace robobus::network