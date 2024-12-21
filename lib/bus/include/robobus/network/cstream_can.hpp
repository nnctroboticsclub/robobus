#pragma once

#include <stdint.h>
#include <logger/logger.hpp>
#include <memory>
#include <robotics/network/can_base.hpp>
#include <vector>

#include <robobus/internal/signal.hpp>
#include "./cstream.hpp"

namespace robobus::network::internal::cstream_can {
using ::robobus::internal::SignalRx;
using ::robobus::internal::SignalTx;
using ::robotics::logger::Logger;

using CANDataType = std::vector<uint8_t>;

/// @brief 制御ストリームに置ける制御データ
using ControlData = CANDataType;

/// @brief CAN 上での制御ストリーム
class ControlStreamOnCAN {
  static inline Logger logger{"can.cstream.nw", "ConSt@CAN"};

  std::shared_ptr<robotics::network::CANBase> can_;
  std::unique_ptr<ControlStream<CANDataType, ControlData>> st_ =
      std::make_unique<ControlStream<CANDataType, ControlData>>();

  uint32_t tx_ctrl_msg_id;
  uint32_t rx_ctrl_msg_id;
  uint32_t tx_data_msg_id;
  uint32_t rx_data_msg_id;

 public:
  SignalRx<ControlData> tx_ctrl{st_->tx_ctrl};
  SignalRx<CANDataType> tx_data{st_->tx_data};
  SignalRx<CANDataType> rx_data{st_->rx_data};

  SignalRx<int> rx_ok_{st_->rx_ok_};
  SignalRx<int> tx_ok_{st_->tx_ok_};

  struct Config {
    std::shared_ptr<robotics::network::CANBase> upper_can_;
    uint32_t tx_ctrl_msg_id;
    uint32_t rx_ctrl_msg_id;
    uint32_t tx_data_msg_id;
    uint32_t rx_data_msg_id;
  };

  explicit ControlStreamOnCAN(Config const& config)
      : can_(config.upper_can_),
        tx_ctrl_msg_id(config.tx_ctrl_msg_id),
        rx_ctrl_msg_id(config.rx_ctrl_msg_id),
        tx_data_msg_id(config.tx_data_msg_id),
        rx_data_msg_id(config.rx_data_msg_id) {
    logger.Info("ControlStreamOnCAN will initialized with following config:");
    logger.Info("  tx_ctrl_msg_id: %d", tx_ctrl_msg_id);
    logger.Info("  rx_ctrl_msg_id: %d", rx_ctrl_msg_id);
    logger.Info("  tx_data_msg_id: %d", tx_data_msg_id);
    logger.Info("  rx_data_msg_id: %d", rx_data_msg_id);
    st_->tx_ctrl.Connect(
        [this](auto const& data) { can_->Send(tx_ctrl_msg_id, data); });

    st_->tx_data.Connect(
        [this](auto const& data) { can_->Send(tx_data_msg_id, data); });

    can_->OnRx([this](uint32_t id, std::vector<uint8_t> const& data) {
      auto msg_id = uint32_t(id);
      if (msg_id == rx_ctrl_msg_id) {
        st_->LoadRxControlData(data);
      } else if (msg_id == rx_data_msg_id) {
        st_->FeedRxData(data);
      }
    });
  }

  inline void FeedTxData(CANDataType const& data) { st_->PutTxData(data); }

  inline void Tick(float delta_time_s) { st_->Tick(delta_time_s); }
};
}  // namespace robobus::network::internal::cstream_can

namespace robobus::network {
using CANDataType = internal::cstream_can::CANDataType;
using ControlStreamOnCAN = internal::cstream_can::ControlStreamOnCAN;
}  // namespace robobus::network