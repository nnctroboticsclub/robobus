#pragma once

#include <chrono>
#include <coroutine>
#include <list>

#include <logger/logger.hpp>
#include <robotics/thread/thread.hpp>

#include "robobus/runtime/runtime_impls.hpp"
#include "time_context.hpp"

namespace robobus::runtime {
/// @brief コルーチンベースプログラムで用いるコンテキスト
template <RuntimeImpl Runtime>
class Loop {
  std::list<std::coroutine_handle<>> coroutines_;

  std::list<
      std::pair<typename Runtime::Clock::time_point, std::coroutine_handle<>>>
      resume_list_;

 public:
  TimeContext<typename Runtime::Clock> time;

 private:
  void ProcessResumeList() {
    auto minimum_grace = Runtime::Clock::duration::max();
    const auto now = time.Now();

    for (auto&& [c_time, coro] : resume_list_) {
      auto grace = c_time - now;

      if (0 < grace.count() && grace < minimum_grace) {
        minimum_grace = grace;
      }

      if (c_time <= now) {
        coro.resume();

        continue;
      }
    }

    resume_list_.remove_if(
        [this, now](auto const& pair) { return pair.first <= now; });
  }

 public:
  Loop(Loop const&) = delete;
  Loop& operator=(Loop const&) = delete;

  Loop() = default;

  //* Loop traits
  void AddTask(std::coroutine_handle<> coroutine) {
    coroutines_.push_back(coroutine);
  }

  void RequestResumeAt(typename Runtime::Clock::time_point time_point,
                       std::coroutine_handle<> coroutine) {
    auto now = time.Now().time_since_epoch();
    auto delta = time_point - time.Started();

    resume_list_.push_back({time_point, coroutine});
  }

  //* Root context
  [[noreturn]] void Run() {
    while (true) {
      for (auto const& coroutine : coroutines_) {
        if (coroutine.done()) {
          coroutines_.remove(coroutine);
        }
      }

      time.Tick();
      ProcessResumeList();
    }
  }

  void LaunchDebugThread() {
    using namespace std::chrono_literals;
    static robotics::logger::Logger logger{"debug.loop.robobus",
                                           "Loop\x1b[32mDebug\x1b[m"};

    robotics::system::Thread thread;
    thread.SetThreadName("Loop-Debug");
    thread.Start([this]() {
      while (true) {
        logger.Info("Elapsed: %d, Now: %d, resume_list: %d entries",
                    time.ElapsedTime().count(),
                    time.Now().time_since_epoch().count(), resume_list_.size());

        robotics::system::SleepFor(1s);
      }
    });
  }
};
}  // namespace robobus::runtime