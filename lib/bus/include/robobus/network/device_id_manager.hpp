#pragma once

#include <memory>

#include <robobus/internal/sematicses.hpp>

#include "address.hpp"

namespace robobus::network {
/// @brief デバイス ID の管理クラス．主に Root デバイスで用いられる．
class DeviceIDManager : public internal::NonCopyable<DeviceIDManager> {
  class Impl {
    Address last_available_device_id_ = Address(2);

   public:
    Address GetAvailableDeviceID() {
      auto ret = last_available_device_id_;
      last_available_device_id_ = Address(ret.Get() + 1);
      return ret;
    }
  };

  std::shared_ptr<Impl> impl = std::make_shared<Impl>();

 public:
  [[nodiscard]] Address GetAvailableDeviceID() const {
    return impl->GetAvailableDeviceID();
  }
};
}  // namespace robobus::network