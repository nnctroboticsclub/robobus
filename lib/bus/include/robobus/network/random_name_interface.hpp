#pragma once

#include <robotics/random/random.hpp>

#include "interface/interface.hpp"

namespace robobus::network {
/// @brief RandomName インタフェース
/// @details RandomName インタフェースはランダムな名前を持つ空のインタフェースである．
/// このインタフェースの目的はデバッグ目的であると同時に，新しくインタフェースを作成するときのテンプレートでもある．
class RandomName : public IInterface, public CANTxMixin {
  static inline robotics::logger::Logger logger{"test->robo-bus.random-name",
                                                "RandomName"};

  using CANTxMixin::Send;

  mutable std::string name_;

 public:
  using CANTxMixin::CANTxMixin;
  [[nodiscard]] uint8_t GetID() const final { return 0xFF; }
  [[nodiscard]] std::string GetName() const final {
    if (!name_.empty()) {
      return name_;
    }

    for (auto i = 0; i < 8; i++) {
      name_.push_back('A' + robotics::system::Random::GetByte() % 26);
    }

    return name_;
  }

  void HandleRx(Address const&, CANDataType const&) final {
    // RandomName インタフェースは受信したパケットをすべて放棄する．
  }
};
}  // namespace robobus::network