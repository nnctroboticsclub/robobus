#pragma once

#include <bitset>
#include <cstdint>

namespace robobus::network::device {

/// @brief デバイスの特徴をあらわす列挙型
enum class Feature : uint8_t {
  kContainer,
  kDevice,

  kMax,
};

static_assert(static_cast<int>(Feature::kMax) <= 8, "Too many features");

/// @brief デバイスの特徴を保持するクラス
class Features {
  std::bitset<8> features_ = 0;

 public:
  void Add(Feature feature) { features_.set(static_cast<int>(feature)); }

  void Remove(Feature feature) { features_.reset(static_cast<int>(feature)); }

  [[nodiscard]] bool Contains(Feature feature) const {
    return features_.test(static_cast<int>(feature));
  }
};
}  // namespace robobus::network::device