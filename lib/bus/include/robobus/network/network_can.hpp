#pragma once

#include "robobus/network/core.hpp"

namespace robobus::network {
/// @brief RobobusNetwork の CAN ラッパー
class RobobusOnCAN : public RobobusNetwork {
  static inline robotics::logger::Logger logger{"test->robo-bus.can", "Device"};
  std::shared_ptr<robotics::network::CANBase> can_;

 public:
  explicit RobobusOnCAN(std::shared_ptr<robotics::network::CANBase> can)
      : RobobusNetwork(), can_(std::move(can)) {
    can_->OnRx([this](uint32_t id, std::vector<uint8_t> const& data) {
      auto msg_id = MessageID(id);

      if (msg_id.GetKind() == MessageKind::kRawP2P) {
        this->HandleP2PMessage(P2PMessageID(id), data);
      } else {
        logger.Error("Unknown message ID: %08lx", id);
      }
    });
  }

  void Send(Address remote_device, uint8_t remote_port,
            robobus::network::CANDataType const& data) {
    auto msg_id = P2PMessageID::FromParts(GET__DEVICE().GetSelfId(),
                                          remote_device, remote_port);

    can_->Send(msg_id.Get(), data);
  }

  auto GetCANTx(uint8_t port) {
    return InterfaceCANTx(can_, GET__DEVICE(), port);
  }

  template <typename Intf, typename... Args>
  auto NewInterface(uint8_t port, Args&&... args) {
    auto interface = std::make_shared<Intf>(GetCANTx(port), GET__DEVICE(),
                                            std::forward<Args>(args)...);
    AddInterface(port, interface);

    return interface;
  }
};
}  // namespace robobus::network