#pragma once

#include <robobus/internal/sematicses.hpp>

#include "../address.hpp"
#include "../p2p_message_id.hpp"
#include "device_feature.hpp"
#include "device_state.hpp"

#include <robobus/monitor/bus.hpp>
#include <robobus/monitor/entry.hpp>

#include <robobus/context/context.hpp>
#include <robobus/coroutine/coroutine.hpp>

namespace robobus::network {
class IInterface;
}

namespace robobus::network::device {
/// @brief デバイスを表すクラス．
/// @details このクラスでは通信機能を持たない．主な責任はデバイス情報の管理である．
class Device : public robobus::internal::NonCopyable<Device> {
  Address self_ = Address(0);
  Address advertise_tag_ = Address(0);
  State state_ = State::kNotInitialized;
  Features features_ = Features();

  std::unordered_map<uint8_t, std::shared_ptr<IInterface>> interfaces;

 public:
  //* インタフェース操作

  bool AddInterface(uint8_t port, std::shared_ptr<IInterface> interface) {
    if (interface == nullptr)
      return false;

    if (interfaces.contains(port))
      return false;

    interfaces[port] = std::move(interface);

    return true;
  }

  [[nodiscard]] auto& GetInterfaces() const { return interfaces; }

  [[nodiscard]] bool HasInterface(uint8_t port) const {
    return interfaces.contains(port);
  }

  [[nodiscard]] std::shared_ptr<IInterface> GetInterface(uint8_t port) const {
    if (interfaces.contains(port))
      return interfaces.at(port);

    return nullptr;
  }

  //* ステートマシン操作用

  void TransitionToInitialized(Address self_id) {
    self_ = self_id;
    state_ = State::kInitialized;
  }

  void TransitionToIdWaiting(Address advertise_tag) {
    this->advertise_tag_ = advertise_tag;
    state_ = State::kIdWaiting;
  }

  //* デバイスに対して編集を加える関数群

  void AddFeature(Feature feature) { features_.Add(feature); }

  //* 状態の取得用関数

  [[nodiscard]] bool IsNotInitialized() const {
    return state_ == State::kNotInitialized;
  }

  [[nodiscard]] bool IsIdWaiting() const { return state_ == State::kIdWaiting; }

  [[nodiscard]] bool IsInitialized() const {
    return state_ == State::kInitialized;
  }

  [[nodiscard]] Address GetSelfId() const {
    using enum State;
    switch (state_) {
      case kNotInitialized:
        return advertise_tag_;
      case kIdWaiting:
        return advertise_tag_;
      case kInitialized:
        return self_;
      default:
        return Address(0);
    }
  }

  //* ユーティリティ

  [[nodiscard]] bool IsToSelf(P2PMessageID const& msg_id) const {
    auto src = msg_id.GetSource();
    if (src == self_)
      return false;

    auto dst = msg_id.GetDestination();

    if (dst == Address::Broadcast())
      return true;

    if (dst == self_)
      return true;

    if (dst == advertise_tag_)
      return true;

    return false;
  }

  /// @brief Robobus ルーチンでデバイスの状態を監視するコルーチン.
  template <robobus::runtime::RuntimeImpl Runtime,
            robobus::internal::StringLiteral kPath>
  Coroutine<void> StartMonitor(Context<Runtime, kPath>&& ctx_) {
    using namespace std::chrono_literals;
    Context<Runtime, kPath> ctx = std::move(ctx_);

    auto entry = ctx.template GetDebugInfo<"Device">();

    while (true) {
      auto string = std::string{};
      string += "ID: ";
      string += std::to_string(self_.Get());
      string += "  ";

      string += "Tag: ";
      string += std::to_string(advertise_tag_.Get());
      string += "  ";

      string += "State: ";
      string += std::to_string(static_cast<int>(state_));
      string += "  ";

      string += "Features: ";
      string += features_.Contains(Feature::kContainer) ? "C" : "";
      string += features_.Contains(Feature::kDevice) ? "D" : "";

      entry.Message(string);

      co_await ctx.Sleep(20ms);
    }
  }
};
}  // namespace robobus::network::device

namespace robobus::network {
/// @brief デバイスを表すクラス．
using Device = device::Device;
}  // namespace robobus::network