#pragma once

#include "device/device.hpp"
#include "interface/interface.hpp"

namespace robobus::network {
class RobobusNetwork {
  static inline robotics::logger::Logger logger{"test->robo-bus.nw", "Device"};

  Device device_self_;

 public:
  explicit RobobusNetwork() = default;

  void AddFeature(device::Feature feature) { device_self_.AddFeature(feature); }

  bool AddInterface(uint8_t port, std::shared_ptr<IInterface> interface) {
    return device_self_.AddInterface(port, std::move(interface));
  }

  [[nodiscard]] auto& GetInterfaces() const {
    return device_self_.GetInterfaces();
  }

  void HandleP2PMessage(P2PMessageID msg_id,
                        std::vector<uint8_t> const& data) const {
    if (!device_self_.IsToSelf(msg_id)) {
      return;
    }

    auto interface_id = msg_id.GetID();

    for (auto const& [port, interface] : device_self_.GetInterfaces()) {
      if (port == interface_id) {
        interface->HandleRx(msg_id.GetSource(), data);
        return;
      }
    }

    logger.Error("Unknown interface %d", interface_id);
  }

  auto& GET__DEVICE() { return device_self_; }
};
}  // namespace robobus::network