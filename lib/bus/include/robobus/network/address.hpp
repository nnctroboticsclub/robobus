#pragma once

#include <cstdint>
namespace robobus::network {
/// @brief アドレスを表すクラス
/// @details いくつかの特殊なアドレスが定義されている．それらのアドレスについては特別に静的関数が用意されている．
class Address {
  uint8_t id_ = 0;

 public:
  Address() = default;
  explicit Address(uint8_t id) : id_(id) {}

  [[nodiscard]] auto Get() const -> uint8_t { return id_; }

  [[nodiscard]] auto IsInitialized() const -> bool { return id_ != 0; }

  bool operator==(Address const& other) const = default;

  static auto Broadcast() -> Address { return Address(255); }

  static auto Root() -> Address { return Address(1); }
};
}  // namespace robobus::network