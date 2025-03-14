#pragma once

/// @file robobus/coroutine/generic_awaiter.hpp
/// @note Dynamic Dispatch 用の Awaiter 純粋仮想クラス
/// @details
/// 　Dynamic Dispatch 用の Awaiter は，Awaiter のインタフェースを提供する．
/// これにより，Dynamic Dispatch において，Awaiter の型を指定せずに Awaiter を扱うことができる．
/// 　例えば，Runtime によらず任意の Awaiter を扱うことができるので，クラス側で Runtime 情報を持たずに
/// メゾッド側で任意の Runtime を扱うことができる．

#include <concepts>
#include <coroutine>
#include <memory>
#include <type_traits>
#include <utility>
namespace robobus::coroutine {
/// @brief Dynamic Dispatch 用の Awaiter 純粋仮想クラス
/// @tparam T Awaiter の戻り値の型
template <typename T>
class IAwaiter {
 public:
  virtual ~IAwaiter() = default;

  /// @brief Awaiter を Resume するための関数
  virtual void Resume(T value) = 0;

  //* 以下 C++ Awaiteable のインタフェース

  /// @brief Awaiter が実行済みかどうか．通常 Resume されたタイミングで await_ready() は true を返すようになる．
  virtual bool await_ready() const = 0;

  /// @brief Awaiter が co_await されたときに呼び出される関数． coroutine_handle はコピーして保持してもよい．
  virtual void await_suspend(std::coroutine_handle<> handle) = 0;

  /// @brief Awaiter が Resume された際に呼び出される関数．戻り値が co_await の結果となる．
  virtual T await_resume() = 0;
};

template <typename T>
using IAwaiterRef = std::unique_ptr<IAwaiter<T>>;

template <typename T, typename RetT>
concept AwaiterLike = requires(T t) {
  { t.await_ready() } -> std::same_as<bool>;
  {
    t.await_suspend(std::declval<std::coroutine_handle<>>())
    } -> std::same_as<void>;
  { t.await_resume() } -> std::same_as<RetT>;
};

static_assert(AwaiterLike<IAwaiter<int>, int>,
              "IAwaiter<int> must be AwaiterLike");
}  // namespace robobus::coroutine