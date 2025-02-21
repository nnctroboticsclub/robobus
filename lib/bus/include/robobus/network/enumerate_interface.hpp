#pragma once

#include "device/device_summary.hpp"
#include "device_id_manager.hpp"
#include "interface/interface.hpp"
#include "interface/interface_summary.hpp"
#include "robobus/context/context.hpp"
#include "robobus/coroutine/generic_awaiter.hpp"
#include "robobus/internal/string_literal.hpp"
#include "robobus/network/address.hpp"
#include "robobus/runtime/lazy_resumer.hpp"
#include "robobus/runtime/runtime_impls.hpp"

#include <robotics/random/random.hpp>

namespace robobus::network {

template <typename T>
class CollectingEncoder {
  mutable std::vector<T> collected_;

 public:
  uint8_t Encode(T const& value) const {
    // If the value is already collected, return the index
    for (auto i = 0u; i < collected_.size(); i++) {
      if (collected_[i] == value) {
        return static_cast<uint8_t>(i);
      }
    }

    // Collect the value and return the index
    collected_.push_back(value);
    return static_cast<uint8_t>(collected_.size() - 1);
  }

  std::optional<T> operator[](uint8_t index) const {
    if (index >= collected_.size()) {
      return std::nullopt;
    }

    return collected_[index];
  }
};

/// @brief Enumerate インタフェース
/// @details Enumerate インタフェースは，デバイスの列挙機能及び，デバイスのメタデータの取得機能を提供する．
/// また， Advertisement Tag を用いる唯一のインタフェースである．
class EnumerateInterface : public IInterface,
                           public CANTxMixin,
                           public SyncRxMixin {
  static inline robotics::logger::Logger logger{"test->robo-bus.enum",
                                                "Device"};

  using CANTxMixin::Send;
  using SyncRxMixin::ReceiveAwait;

  DeviceIDManager device_id_manager_;

  enum class Opcode : uint8_t {
    // 3-way handshake
    kScanUnenumerated = 0x00,
    kAdvertise = 0x01,
    kAssignID = 0x02,

    // Device information
    kQueryDevice = 0x10,
    kDevice = 0x11,

    // Interface information
    kQueryInterface = 0x12,
    kInterface = 0x13,

    // String information
    kQueryString = 0x14,
    kStringResponse = 0x15,
  };

  /// @brief 送信用に収集されたテキストのリスト
  CollectingEncoder<std::string> collected_strings_;

  /// @brief 送信用に収集されたインタフェースサマリのリスト
  CollectingEncoder<InterfaceSummary> collected_interfaces_;

  /// @brief Send 用バッファ (状態変数ではない)
  mutable std::vector<uint8_t> send_buffer_;

  std::string creator_;
  std::string name_;

  std::optional<coroutine::IAwaiterRef<Address>> on_enumerate_finished_;

  void SendDevice(Address dest, DeviceSummary const& summary) const {
    for (auto& intf : summary.interfaces) {
      collected_interfaces_.Encode(intf);
    }

    Send(dest, {
                   static_cast<uint8_t>(Opcode::kDevice),  //
                   static_cast<uint8_t>(summary.interfaces.size()),
                   collected_strings_.Encode(summary.creator),
                   collected_strings_.Encode(summary.name),
               });
  }

  void SendInterface(Address dest, uint8_t interface_index) const {
    auto const& intf_ = collected_interfaces_[interface_index];
    if (!intf_.has_value()) {
      return;
    }

    auto const& intf = *intf_;
    Send(dest, {
                   static_cast<uint8_t>(Opcode::kInterface),  //
                   interface_index,                           //
                   intf.port,                                 //
                   intf.id,                                   //

                   collected_strings_.Encode(intf.name),
               });
  }

  void SendString(Address dest, uint8_t string_id, uint8_t index) const {
    auto& data = send_buffer_;
    data.resize(0);

    data.push_back(static_cast<uint8_t>(Opcode::kStringResponse));
    data.push_back(string_id);
    data.push_back(index);

    auto str_ = collected_strings_[string_id];
    if (!str_.has_value()) {
      return;
    }
    auto str = *str_;
    int length = str.length();

    if (index == 0) {
      while (length > 0 && data.size() < 8) {
        data.push_back(length & 0xFF);
        length >>= 8;
      }
    } else {
      auto i_start = (index - 1) * 4;
      for (auto i = i_start; i < i_start + 4 && i < length; i++) {
        data.push_back(str[i]);
      }
    }

    Send(dest, data);
  }

  Coroutine<std::string> QueryStr(Address dev, uint8_t str) {
    using enum Opcode;

    // logger.Debug("\x1b[4;33mQuerying string %d from %d\x1b[m", str, dev.Get());

    //* Acquire the string length
    Send(dev, {static_cast<uint8_t>(Opcode::kQueryString), str, 0});
    auto [dest, data_length] =
        co_await ReceiveAwait(static_cast<uint8_t>(Opcode::kStringResponse));

    auto length = 0;
    for (size_t i = 3; i < data_length.size(); i++) {
      length |= data_length[i] << (8 * (i - 3));
    }

    // logger.Debug("\x1b[33mString %d has length %d\x1b[m", str, length);

    //* Calculate some
    auto num_chunks = (length + 3) / 4;

    auto buffer = std::vector<uint8_t>{};
    buffer.reserve(length);

    //* Acquire the string data
    for (auto i = 0; i < num_chunks; i++) {
      Send(dev, {static_cast<uint8_t>(Opcode::kQueryString), str,
                 static_cast<unsigned char>(i + 1)});
      auto [remote, data] =
          co_await ReceiveAwait(static_cast<uint8_t>(Opcode::kStringResponse));

      for (size_t j = 3; j < data.size(); j++) {
        buffer.push_back(data[j]);
      }
    }

    auto string = std::string(buffer.begin(), buffer.end());
    // logger.Debug("\x1b[33mReceived string %d from %d ==> %s\x1b[m", str, dev.Get(), string);

    co_return string;
  }

  Coroutine<InterfaceSummary> QueryInterface(Address dev, uint8_t index) {
    using enum Opcode;

    Send(dev, {static_cast<uint8_t>(Opcode::kQueryInterface), index});
    auto [dest, data] =
        co_await ReceiveAwait(static_cast<uint8_t>(Opcode::kInterface));

    co_return InterfaceSummary{
        .port = data[2],
        .id = data[3],
        .name = co_await QueryStr(dev, data[4]),
    };
  }

 public:
  using CANTxMixin::CANTxMixin;

  [[nodiscard]] uint8_t GetID() const final { return 0x00; }
  [[nodiscard]] std::string GetName() const final { return "Enumerate"; }

  void SetCreator(std::string_view const& creator) { creator_ = creator; }
  void SetName(std::string_view const& name) { name_ = name; }

  void HandleRx(Address const& from, CANDataType const& data) final {
    HandleRxMixin(from, data);
  }

  void HandleRxAsync(Address const& from, CANDataType const& data) final {
    using enum device::State;
    using enum Opcode;

    auto command = static_cast<Opcode>(data[0]);

    if (command == kScanUnenumerated && this->device.IsNotInitialized()) {
      auto adv_tag = Address(robotics::system::Random::GetByte() | 0x80);
      this->device.TransitionToIdWaiting(adv_tag);

      Send(from, {static_cast<uint8_t>(kAdvertise)});
    } else if (command == kAdvertise && this->device.IsInitialized()) {
      auto const new_device_id = device_id_manager_.GetAvailableDeviceID();

      Send(from, {static_cast<uint8_t>(kAssignID), new_device_id.Get()});

      logger.Info("Assigning device id %d for tag %d", new_device_id.Get(),
                  from.Get());
    } else if (command == kAssignID && this->device.IsIdWaiting()) {
      auto new_device_id = Address(data[1]);
      this->device.TransitionToInitialized(new_device_id);

      logger.Info("Device initialized with id %d", new_device_id.Get());
      if (on_enumerate_finished_.has_value()) {
        (*on_enumerate_finished_)->Resume(new_device_id);
      }
    }

    switch (command) {
      case kQueryDevice: {
        DeviceSummary summary{
            .creator = creator_,
            .name = name_,
            .interfaces = {},
        };

        for (auto const& [port, intf] : this->device.GetInterfaces()) {
          summary.interfaces.push_back({
              .port = port,
              .id = intf->GetID(),
              .name = intf->GetName(),
          });
        }

        SendDevice(from, summary);
        break;
      }
      case kQueryInterface: {
        auto index = data[1];
        SendInterface(from, index);
        break;
      }
      case kQueryString: {
        auto str_id = data[1];
        auto index = data[2];

        SendString(from, str_id, index);
        break;
      }
      default: {
        break;
      }
    }
  }

  Coroutine<DeviceSummary> Query(Address dev) {
    using enum Opcode;

    Send(dev, {static_cast<uint8_t>(Opcode::kQueryDevice)});
    auto [dest, data] =
        co_await ReceiveAwait(static_cast<uint8_t>(Opcode::kDevice));

    // logger.Debug("Received device information from %d", dest.Get());
    // logger.HexDebug(data.data(), data.size());

    auto num_interfaces = data[1];
    auto interfaces = std::vector<InterfaceSummary>{};
    for (uint8_t index = 0; index < num_interfaces; index++) {
      interfaces.push_back(co_await QueryInterface(dev, index));
    }

    auto creator = co_await QueryStr(dev, data[2]);
    auto name = co_await QueryStr(dev, data[3]);

    co_return DeviceSummary{
        .creator = creator,
        .name = name,
        .interfaces = interfaces,
    };
  }

  template <runtime::RuntimeImpl Runtime, internal::StringLiteral kPath>
  Coroutine<Address> AwaitEnumerated(context::Context<Runtime, kPath>& ctx) {
    using Awaiter = runtime::LazyResumerAwaiter<Address, Runtime>;

    on_enumerate_finished_ = std::make_unique<Awaiter>(ctx.GetLoop());
    co_return co_await** on_enumerate_finished_;
  }

  void FindEnumeratedDevices() const { Send(Address::Broadcast(), {0x00}); }
};
}  // namespace robobus::network