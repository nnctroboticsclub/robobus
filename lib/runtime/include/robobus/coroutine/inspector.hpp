#pragma once

#include <coroutine>

namespace robobus::coroutine {
struct CoroutineHandleInspector {
  std::coroutine_handle<> _handle;

  bool await_ready() const noexcept { return false; }
  bool await_suspend(std::coroutine_handle<> handle) noexcept {
    _handle = handle;
    return false;
  }
  std::coroutine_handle<> await_resume() noexcept { return _handle; }
};
}  // namespace robobus::coroutine