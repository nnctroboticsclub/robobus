#pragma once

#include <concepts>
#include <memory>

#include <logger/chained_log_sink.hpp>
#include <logger/logger.hpp>

#include <robobus/network/address.hpp>
#include <robobus/network/interface/interface.hpp>

namespace robobus::network::interface::irp {
/// @brief IRPInterface の上位層向けハンドラー
template <typename T>
concept IRPHandler = requires {
  { T::EnterBootloader() } -> std::same_as<void>;
  { T::EnterFastBootloader() } -> std::same_as<void>;
};

static robotics::logger::Logger logger{"test->robo-bus.irp", "IRP"};

template <IRPHandler Handler>
class IRPInterface : public robobus::network::IInterface,
                     public robobus::network::CANTxMixin {

  //! IRP インタフェースが生成される前の LogSink のバックアップ
  //! ChainedSink の Sink1 として使用される．
  robotics::logger::LogSink* stock_sink = nullptr;

  //! IRP インタフェースがバス USART に提供する USART Logsink
  robotics::logger::LogSink* ow_log_sink = nullptr;

  //! srobo1.logger に提供する LogSink
  std::unique_ptr<robotics::logger::ChainedLogger> logsink = nullptr;

 public:
  IRPInterface(InterfaceCANTx can_tx, Device& device_,
               robotics::logger::LogSink* debug_logsink)
      : CANTxMixin(can_tx, device_), ow_log_sink(debug_logsink) {
    stock_sink = robotics::logger::global_log_sink;

    logsink = std::make_unique<robotics::logger::ChainedLogger>(stock_sink,
                                                                ow_log_sink);
    robotics::logger::global_log_sink = logsink.get();

    logsink->Sink2Active(false);
  }
  using CANTxMixin::CANTxMixin;
  [[nodiscard]] uint8_t GetID() const final { return 0xE0; }
  [[nodiscard]] std::string GetName() const final { return "IRP"; }

  void HandleRx(Address const& from,
                robobus::network::CANDataType const& data) final {
    if (data.size() < 1) {
      logger.Error("Received empty data from %d", from.Get());
      return;
    }

    auto cmd = data[0];
    logger.Info("Received from %d: %02x", from.Get(), cmd);

    if (cmd == 0x00) {
      logger.Info("Entering bootloader");
      Handler::EnterBootloader();
    } else if (cmd == 0x01) {
      logger.Info("Disabling USART LogSink");
      logsink->Sink2Active(false);
      logger.Info("Disabled USART LogSink");
    } else if (cmd == 0x02) {
      logger.Info("Enabling USART LogSink");
      logsink->Sink2Active(true);
      logger.Info("Enabled USART LogSink");
    } else if (cmd == 0x03) {
      logger.Info("Launching fast bootloader");
      Handler::EnterFastBootloader();
    }
  }

  using CANTxMixin::Send;
};
}  // namespace robobus::network::interface::irp

namespace robobus::network {
using interface::irp::IRPHandler;
using interface::irp::IRPInterface;
}  // namespace robobus::network