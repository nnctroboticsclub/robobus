#pragma once

#include <coroutine>
#include <functional>
#include <optional>
#include <vector>
#include "robobus/coroutine/coroutine.hpp"
#include "robobus/coroutine/coroutine_awaiter.hpp"
#include "robobus/coroutine/generic_awaiter.hpp"

namespace robobus::coroutine {
//* Forward declaration
template <typename ReturnType>
struct Coroutine;

template <typename ReturnType>
struct Promise;

//* BasePromise
template <typename ReturnType>
class BasePromise {
 protected:
  void Return() {
    if (is_finished) {
      printf("\x1b[41m \x1b[42m \x1b[0m%p Promise returned twice\n", this);
      while (1)
        ;
      return;
    }
    is_finished = true;
    for (auto& callback : on_return_callbacks) {
      callback();
    }
  }

 public:
  void AddOnReturnCallback(std::function<void()> callback) {
    on_return_callbacks.emplace_back(callback);
  }

  operator Promise<ReturnType>&() {
    return *static_cast<Promise<ReturnType>*>(this);
  }

  [[nodiscard]] auto IsFinished() const { return is_finished; }

 public:
  auto get_return_object() {
    return Coroutine<ReturnType>{
        std::coroutine_handle<Promise<ReturnType>>::from_promise(*this)};
  }

  auto initial_suspend() { return std::suspend_never{}; }

  auto final_suspend() noexcept { return std::suspend_never{}; }

  auto unhandled_exception() { std::terminate(); }

 private:
  bool is_finished = false;
  std::vector<std::function<void()>> on_return_callbacks;
};

//* Promise
template <std::copy_constructible ReturnType>
struct Promise<ReturnType> : public BasePromise<ReturnType> {
 public:
  auto get_return_value() { return value_; }

 public:
  auto return_value(ReturnType value) {
    // printf("\x1b[41m \x1b[42m \x1b[0m%p return_value (handle = %p)\n", this,
    //        std::coroutine_handle<Promise<ReturnType>>::from_promise(*this)
    //            .address());

    this->value_ = value;
    this->Return();

    return std::suspend_never{};
  }

 private:
  std::optional<ReturnType> value_;
};

template <>
struct Promise<void> : public BasePromise<void> {
 public:
  auto get_return_value() { return finished; }

 public:
  auto return_void() {
    // printf("\x1b[41m \x1b[42m \x1b[0m%p return_value (handle = %p)\n", this,
    //        std::coroutine_handle<Promise<void>>::from_promise(*this).address());
    finished = true;
    this->Return();

    return std::suspend_never{};
  }

 private:
  bool finished = false;
};

//* Concept

template <typename T, typename RetT>
constexpr const bool PromiseLike_v = requires(T t) {
  { t.initial_suspend() } -> AwaiterLike<RetT>;
  { t.final_suspend() } -> AwaiterLike<RetT>;
  t.unhandled_exception();

  { t.return_value() } -> AwaiterLike<RetT>;
};

template <typename T>
constexpr const bool PromiseLike_v<T, void> = requires(T t) {
  { t.initial_suspend() } -> AwaiterLike<void>;
  { t.final_suspend() } -> AwaiterLike<void>;
  t.unhandled_exception();

  { t.return_void() } -> AwaiterLike<void>;
};

template <typename T, typename RetT>
concept PromiseLike = PromiseLike_v<T, RetT>;

}  // namespace robobus::coroutine