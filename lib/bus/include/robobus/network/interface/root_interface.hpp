#pragma once

#include <robobus/coroutine/coroutine.hpp>
#include <robobus/network/interface/interface.hpp>

namespace robobus::network::interface::root {
using ::robobus::Coroutine;
using ::robobus::network::Address;
using ::robobus::network::CANDataType;
using ::robobus::network::CANTxMixin;
using ::robobus::network::Device;
using ::robobus::network::IInterface;
using ::robobus::network::InterfaceCANTx;
using ::robobus::network::SyncRxMixin;

static inline robotics::logger::Logger logger{"root.if.rb.nw",
                                              "\x1b[42mRoot Intf\x1b[m"};

struct Detail {
  uint8_t dev = 0;
  uint8_t port = 0;

  auto GetCANTx(robotics::network::CANRef can, Device& dev) {
    return InterfaceCANTx{can, dev, port};
  }

  template <typename Intf, typename... Args>
  auto MakeInterface(InterfaceCANTx&& tx, Device& dev, Args&&... args) {
    return std::make_shared<Intf>(std::move(tx), dev,
                                  std::forward<Args>(args)...);
  }
};

enum class Opcodes {
  /// @brief インタフェースの概要を取得する
  kLookup = 0x00,

  /// @brief インタフェースの所有権概要
  kIntfOwnershipSummary = 0x01,

  /// @brief インタフェースの所有権詳細を取得する
  kExplainOwnership = 0x02,

  /// @brief 所有権詳細
  kIntfOwnershipInfo = 0x03
};

class RootInterface : public IInterface, public CANTxMixin, public SyncRxMixin {
  using IntfEPs = std::vector<Detail>;

  void SendSummary(Address const& dest, uint8_t intf_id, IntfEPs& eps) {
    CANDataType data;
    data.emplace_back(static_cast<uint8_t>(Opcodes::kIntfOwnershipSummary));
    data.emplace_back(intf_id);
    data.emplace_back(eps.size());
    Send(dest, data);
  }

  void SendDetail(Address const& dest, uint8_t intf_id, Detail const& detail) {
    CANDataType data;
    data.emplace_back(static_cast<uint8_t>(Opcodes::kIntfOwnershipInfo));
    data.emplace_back(intf_id);
    data.emplace_back(detail.dev);
    data.emplace_back(detail.port);
    Send(dest, data);
  }

  Coroutine<uint8_t> QuerySummary(Address remote, uint8_t interface_id) {
    Send(remote, {static_cast<uint8_t>(Opcodes::kLookup), interface_id});
    auto [from, payload] = co_await ReceiveAwait(
        static_cast<uint8_t>(Opcodes::kIntfOwnershipSummary));

    auto ret = payload[2];

    logger.Info("I%d: %d", interface_id, ret);

    co_return ret;
  }
  Coroutine<Detail> QueryInfo(Address remote, uint8_t interface_id,
                              uint8_t index) {
    Send(remote, {static_cast<uint8_t>(Opcodes::kExplainOwnership),
                  interface_id, index});
    auto [from, payload] = co_await ReceiveAwait(
        static_cast<uint8_t>(Opcodes::kIntfOwnershipSummary));

    Detail ret;
    ret.dev = payload[2];
    ret.port = payload[3];

    logger.Info("I%d i%d -> d%d p%d", interface_id, index, ret.dev, ret.port);

    co_return ret;
  }

  Coroutine<IntfEPs> Query(Address remote, uint8_t intf_id) {
    auto intf_count = co_await QuerySummary(remote, intf_id);

    IntfEPs ret;
    ret.resize(intf_count);

    for (uint8_t i = 0; i < intf_count; i++) {
      ret[i] = co_await QueryInfo(remote, intf_id, i);
    }

    co_return ret;
  }

 public:
  RootInterface(InterfaceCANTx can_tx, Device& device)
      : CANTxMixin(can_tx, device), device_(device) {}

  using CANTxMixin::CANTxMixin;
  using CANTxMixin::Send;
  void HandleRx(Address const& from, CANDataType const& data) final {
    SyncRxMixin::HandleRxMixin(from, data);
  }

  [[nodiscard]] uint8_t GetID() const final { return 0x01; }
  [[nodiscard]] std::string GetName() const final { return "Root"; }

  void HandleRxAsync(Address const& from, CANDataType const& data) final {
    if (data.size() < 1) {
      logger.Error("Received empty data frame");
      return;
    }

    auto opcode = static_cast<Opcodes>(data[0]);
    using enum Opcodes;
    switch (opcode) {
      case kLookup: {
        auto intf_id = data[1];

        auto eps = self_ownership_table.contains(intf_id)
                       ? self_ownership_table[intf_id]
                       : IntfEPs{};

        auto self_interfaces = device_.GetInterfaces();
        for (auto& [port, intf] : self_interfaces) {
          if (intf->GetID() == intf_id) {
            eps.emplace_back(
                Detail{.dev = device_.GetSelfId().Get(), .port = port});
          }
        }

        SendSummary(from, intf_id, eps);

        break;
      }
      case kExplainOwnership: {
        auto intf_id = data[1];

        auto eps = self_ownership_table.contains(intf_id)
                       ? self_ownership_table[intf_id]
                       : IntfEPs{};

        auto self_interfaces = device_.GetInterfaces();
        for (auto& [port, intf] : self_interfaces) {
          if (intf->GetID() == intf_id) {
            eps.emplace_back(
                Detail{.dev = device_.GetSelfId().Get(), .port = port});
          }
        }

        auto index = data[2];
        SendDetail(from, intf_id, self_ownership_table[intf_id][index]);
        break;
      }

      case kIntfOwnershipSummary:
      case kIntfOwnershipInfo:
        logger.Error("HandleRxAsync called with data packet");
        break;

      default:
        logger.Error("Unknown command: %02x", opcode);
        break;
    }
  }

  template <typename Interface, typename... Args>
  Coroutine<std::optional<std::shared_ptr<Interface>>> Query(
      robotics::network::CANRef can, Args... args) {
    uint8_t interface_id = ((Interface*)nullptr)->GetID();

    auto a = co_await Query(Address::Root(), interface_id);
    if (a.empty()) {
      co_return std::nullopt;
    }

    auto detail = a[0];

    co_return detail.template MakeInterface<Interface>(
        detail.GetCANTx(can, device_), device_, std::forward(args)...);
  }

  auto RegisterInterface(uint8_t intf_id, uint8_t dev_id, uint8_t port) {
    if (!self_ownership_table.contains(intf_id)) {
      self_ownership_table[intf_id] = IntfEPs{};
    }

    self_ownership_table[intf_id].emplace_back(
        Detail{.dev = dev_id, .port = port});

    logger.Info("Intf registered: I%02x D%02x/p%02x", intf_id, dev_id, port);
  }
  template <typename Interface>
  auto RegisterInterface(uint8_t dev_id, uint8_t port) {
    uint8_t interface_id = ((Interface*)nullptr)->GetID();
    RegisterInterface(interface_id, dev_id, port);
  }

 private:
  Device& device_;
  std::unordered_map<uint8_t, IntfEPs> self_ownership_table = {};
};

}  // namespace robobus::network::interface::root

namespace robobus::network {
using interface::root::RootInterface;
}