#pragma once

#include <cstdint>

#include <coroutine>
#include <memory>
#include <optional>

#include <robobus/internal/sematicses.hpp>
#include <robotics/network/can_base.hpp>
#include <robotics/utils/linked_list_node.hpp>

#include "../address.hpp"
#include "../device/device.hpp"
#include "../types.hpp"

namespace robobus::network {
static robotics::logger::Logger intf_logger{"Interface", "robobus.interface"};

/// @brief 最低限のインタフェースの基底クラス
/// @details この基底クラスは CAN バスへデータ送信をする機能を有していない．
/// CAN バスへデータ送信をする場合は， @class Interface を継承すること．
class IInterface {
 public:
  virtual ~IInterface() = default;

  /// @brief Interface の ID を取得する
  [[nodiscard]] virtual uint8_t GetID() const = 0;

  /// @brief Interface の名前を取得する
  [[nodiscard]] virtual std::string GetName() const = 0;

  /// @brief メッセージを受信した際の処理
  virtual void HandleRx(Address const& from, CANDataType const& data) = 0;
};

/// @brief インタフェースに提供される CAN 送信機能
class InterfaceCANTx {
  std::shared_ptr<robotics::network::CANBase> can_;
  Device const& device_;
  uint8_t port_;

 public:
  explicit InterfaceCANTx(std::shared_ptr<robotics::network::CANBase> can,
                          Device const& device, uint8_t port)
      : can_(std::move(can)), device_(device), port_(port) {}

  /// @brief インタフェースクラスから CAN メッセージを送信するための関数
  void Send(Address dest, CANDataType const& data) const {
    auto msg_id = P2PMessageID::FromParts(device_.GetSelfId(), dest, port_);

    can_->Send(msg_id.Get(), data);
  }
};

class SyncRxMixin {
  struct Packet {
    Address from = Address(0);
    CANDataType data = {0};
  };

  struct MessageAwaiter : public internal::NonCopyable<MessageAwaiter> {
    std::optional<Packet> packet_ = std::nullopt;
    std::coroutine_handle<> handle_ = nullptr;

    void Invalidate() {
      packet_.reset();
      handle_ = nullptr;
    }

    void NotifyMessage(Packet const& packet) {
      this->packet_ = packet;

      if (handle_) {
        handle_.resume();
      } else {
        intf_logger.Error("NotifyMessage: Not available awaiter notified");
      }
    }

    bool await_ready() const { return packet_.has_value(); }

    void await_suspend(std::coroutine_handle<> handle) {
      this->handle_ = handle;
      packet_.reset();
    }

    auto await_resume() -> Packet& { return *packet_; }
  };

  struct WaitQueue : public robotics::utils::LinkedListNode<WaitQueue> {
    uint8_t opcode;
    MessageAwaiter awaiter;
  };

  robotics::utils::LinkedList<WaitQueue, 16> wait_queue_;

 protected:
  virtual void HandleRxAsync(Address const& from, CANDataType const& data) = 0;

  void HandleRxMixin(Address const& from, CANDataType const& data) {
    // intf_logger.Info("Received message from %d, op %02x", from.Get(), data[0]);

    for (auto node : wait_queue_) {
      //intf_logger.Info("Checking %p; %02x", node, node->opcode);
      if (node->opcode != data[0])
        continue;

      wait_queue_.Remove(node);

      // intf_logger.Info("Routed in sync handler");
      node->awaiter.NotifyMessage({.from = from, .data = data});

      return;
    }

    // intf_logger.Info("Routed in async handler");
    HandleRxAsync(from, data);
  }

  Coroutine<Packet> ReceiveAwait(uint8_t opcode) {
    // intf_logger.Info("ReceiveAwait %02x", opcode);
    auto node = wait_queue_.NewBack();
    if (!node) {
      intf_logger.Error("No more wait queue available");
      co_return Packet{};
    }

    node->in_use = true;
    node->opcode = opcode;
    node->awaiter.Invalidate();

    // intf_logger.Info("ReceiveAwait %02x waiting @%p", opcode, node);
    auto packet = co_await node->awaiter;

    wait_queue_.Remove(node);

    // intf_logger.Info("ReceiveAwait %02x done", opcode);

    co_return packet;
  }

 public:
  virtual ~SyncRxMixin() = default;
};

/// @brief CAN Tx の MixIn クラス
/// @note このクラスは単独で使用されることを意図していない．インタフェース実装時に継承することで利用すること．
class CANTxMixin {
  InterfaceCANTx can_tx_;

 protected:
  Device& device;

  /// @brief 指定したアドレスの同じインタフェースへデータを送信する
  void Send(Address dest, CANDataType const& data) const {
    can_tx_.Send(dest, data);
  }

 public:
  explicit CANTxMixin(InterfaceCANTx can_tx, Device& device_)
      : can_tx_(std::move(can_tx)), device(device_) {}
};
}  // namespace robobus::network