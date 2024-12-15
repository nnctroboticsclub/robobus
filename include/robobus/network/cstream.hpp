#pragma once

#include <cstdint>
#include <logger/logger.hpp>
#include <memory>
#include <rd16.hpp>
#include "../internal/multi_updatable.hpp"
#include "../internal/signal.hpp"

namespace robobus::network::internal::cstream {
using robobus::internal::MultiUpdatable;
using robobus::internal::Signal;
using robobus::internal::SignalRx;
using robobus::internal::SignalTx;
using robotics::logger::Logger;
using robotics::network::RD16;

/// @brief 制御ストリームの状態符号
class ControlStreamState {
  uint32_t data_ = 0;

 public:
  explicit ControlStreamState(uint32_t data = 0) : data_(data) {}

  void operator=(uint32_t data) { data_ = data; }

  /// @brief データを混ぜ合わせる (32bit 幅)
  /// @param code 混ぜ合わせるコード
  void MixCode(uint32_t code) { data_ = (data_ * code + code) ^ (data_ >> 16); }

  /// @brief データを取得
  [[nodiscard]] uint32_t Get() const { return data_; }

  /// @brief データを設定
  void Set(uint32_t data) { data_ = data; }
};

/// @brief 入力側の制御ストリーム
template <typename DataType>
class ControlStream_Inbound {
  static inline Logger logger{"out.cstream.nw", "ConSt.\x1b[35mIn\x1b[m "};

  //* Caches
  ControlStreamState previous_state_v{0};
  ControlStreamState calculated_state_{0};
  bool is_ok_value_{false};
  bool has_new_data_{false};
  SignalTx<int> is_ok_signal_{std::make_shared<Signal<int>>()};
  SignalTx<DataType> data_signal_{std::make_shared<Signal<DataType>>()};

  void CheckIntegrity() {
    if (is_ok_value_) {
      return;
    }

    if (!has_new_data_) {
      // logger.Debug("RX Not Ready");
      return;
    }

    if (state_feedback->Get() == calculated_state_.Get()) {
      // logger.Debug("RX Same (%08X)", state_feedback->Get());
      return;
    }

    if (state_validate.Get() != calculated_state_.Get()) {
      // logger.Info("RX Ng (%08X != %08X)", state_validate.Get(),
      // calculated_state_.Get());
      return;
    }

    // logger.Info("RX Ok");

    /* logger.Debug("h_if: %08x => %08x", state_feedback->Get(),
                 calculated_state_.Get()); */

    state_feedback.GetOptional() = calculated_state_;
    state_feedback.Update();

    data_signal_.Fire(data);

    is_ok_signal_.Fire(0);
    is_ok_value_ = true;
    has_new_data_ = false;
  }

  DataType data;
  ControlStreamState state_validate{0};

 public:
  MultiUpdatable<ControlStreamState> state_feedback{ControlStreamState{0}};
  SignalRx<DataType> data_{data_signal_};
  SignalRx<int> is_ok_{is_ok_signal_};

 public:
  void FeedData(DataType const& data) {
    // logger.Trace("Feed data");
    has_new_data_ = true;
    is_ok_value_ = false;

    this->data = data;
    calculated_state_ = previous_state_v;
    calculated_state_.MixCode(RD16::FromData(data).Get());

    CheckIntegrity();
  }

  void SetStateValidate(ControlStreamState state) {
    // logger.Trace("Rx State Validate (%08X)", state.Get());
    if (state.Get() == state_validate.Get()) {
      // logger.Debug("State is same (Ignore)");
      return;
    }

    is_ok_value_ = false;
    previous_state_v = state_validate;
    state_validate = state;
    CheckIntegrity();
  }

  void Tick(float delta_time_s) { state_feedback.Tick(delta_time_s); }
};

/// @brief 出力側の制御ストリーム
template <typename DataType>
class ControlStream_Outbound {
  static inline Logger logger{"out.cstream.nw", "ConSt.\x1b[34mOut\x1b[m"};
  bool is_ok_value_ = false;
  SignalTx<int> is_ok_signal_{std::make_shared<Signal<int>>()};

  void CheckIntegrity() {
    if (is_ok_value_) {
      return;
    }

    if (!state_validate_.GetOptional().has_value()) {
      // logger.Debug("TX Not Ready");
      return;
    }

    if (state_validate_->Get() != state_feedback_.Get()) {
      // logger.Debug("TX Mismatch (%08X != %08X)", state_validate_->Get(),
      //              state_feedback_.Get());
      return;
    }

    // logger.Info("TX Ok");

    data_.Reset();
    is_ok_signal_.Fire(0);
    is_ok_value_ = true;
  }

  ControlStreamState state_feedback_{};

 public:
  MultiUpdatable<DataType> data_;
  MultiUpdatable<ControlStreamState> state_validate_{ControlStreamState{0}};
  SignalRx<int> is_ok_{is_ok_signal_};

 public:
  void SetData(DataType data) {
    // logger.Info("Feed data");
    is_ok_value_ = false;
    data_.GetOptional().emplace(data);
    data_.Update();

    state_validate_->MixCode(RD16::FromData(data).Get());
    // logger.Info("  +- h_ov: %08X", state_validate_->Get());
    state_validate_.Update();
  }

  void SetStateFeedback(ControlStreamState state) {
    // logger.Info("Tx State Feedback (%08X)", state.Get());
    is_ok_value_ = false;
    state_feedback_ = state;

    CheckIntegrity();
  }

  void Tick(float delta_time_s) {
    data_.Tick(delta_time_s);
    state_validate_.Tick(delta_time_s);
  }
};

/// @brief 制御ストリーム
template <typename DataType, typename CtrlDataTYpe>
class ControlStream {
  static inline Logger logger{"cstream.nw", "ConSt    "};

  ControlStream_Inbound<DataType> inbound_;
  ControlStream_Outbound<DataType> outbound_;

  SignalTx<CtrlDataTYpe> tx_ctrl_signal_{std::make_shared<Signal<DataType>>()};

  void ProcessTxControlData() {
    CtrlDataTYpe ret;
    auto h_ov = outbound_.state_validate_->Get();
    auto h_if = inbound_.state_feedback->Get();
    // logger.Debug("Tx Control Data: %08X %08X", h_ov, h_if);

    ret.resize(8);
    ret[0] = (h_ov >> 24) & 0xff;
    ret[1] = (h_ov >> 16) & 0xff;
    ret[2] = (h_ov >> 8) & 0xff;
    ret[3] = (h_ov >> 0) & 0xff;
    ret[4] = (h_if >> 24) & 0xff;
    ret[5] = (h_if >> 16) & 0xff;
    ret[6] = (h_if >> 8) & 0xff;
    ret[7] = (h_if >> 0) & 0xff;

    tx_ctrl_signal_.Fire(ret);
  }

 public:
  SignalRx<CtrlDataTYpe> tx_ctrl{tx_ctrl_signal_};
  SignalRx<DataType> tx_data{outbound_.data_.updated};
  SignalRx<DataType> rx_data{inbound_.data_};

  SignalRx<int> rx_ok_{inbound_.is_ok_};
  SignalRx<int> tx_ok_{outbound_.is_ok_};

  ControlStream() {
    outbound_.state_validate_.updated.Connect(
        [this](ControlStreamState const&) { ProcessTxControlData(); });
    inbound_.state_feedback.updated.Connect(
        [this](ControlStreamState const&) { ProcessTxControlData(); });
  }

  void LoadRxControlData(CtrlDataTYpe data) {
    auto h_iv = (uint32_t(data[0]) << 24) | (uint32_t(data[1]) << 16) |
                (uint32_t(data[2]) << 8) | (uint32_t(data[3]));

    auto h_of = (uint32_t(data[4]) << 24) | (uint32_t(data[5]) << 16) |
                (uint32_t(data[6]) << 8) | (uint32_t)(data[7]);

    inbound_.SetStateValidate(ControlStreamState{h_iv});
    outbound_.SetStateFeedback(ControlStreamState{h_of});
  }

  void PutTxData(DataType const& data) { outbound_.SetData(data); }

  void FeedRxData(DataType const& data) { inbound_.FeedData(data); }

  void Tick(float delta_time_s) {
    inbound_.Tick(delta_time_s);
    outbound_.Tick(delta_time_s);
  }
};

}  // namespace robobus::network::internal::cstream

namespace robobus::network {
using internal::cstream::ControlStream;
}  // namespace robobus::network