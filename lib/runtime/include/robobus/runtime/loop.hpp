#pragma once

#include <chrono>
#include <coroutine>
#include <list>

#include <logger/logger.hpp>
#include <robotics/thread/thread.hpp>
#include <robotics/utils/linked_list_node.hpp>
#include <robotics/utils/no_mutex_lifo.hpp>

#include "robobus/runtime/runtime_impls.hpp"
#include "time_context.hpp"

namespace robobus::runtime {

template <RuntimeImpl Runtime>
struct ResumeRequest {
  typename Runtime::Clock::time_point time_point;
  std::coroutine_handle<> coroutine;
};

/// @brief コルーチンベースプログラムで用いるコンテキスト
template <RuntimeImpl Runtime>
class Loop {
  robotics::utils::NoMutexLIFO<ResumeRequest<Runtime>, 5> resume_list_lifo_;

 public:
  TimeContext<typename Runtime::Clock> time;

 private:
  void ProcessResumeList() {
    auto minimum_grace = Runtime::Clock::duration::max();
    const auto now = time.Now();

    const size_t it_max = resume_list_lifo_.Size();
    for (size_t i = 0; i < it_max && !resume_list_lifo_.Empty(); i++) {
      auto ptr = resume_list_lifo_.Pop();

      auto& c_time = ptr.time_point;
      auto& coro = ptr.coroutine;

      auto grace = c_time - now;

      if (0 < grace.count() && grace < minimum_grace) {
        minimum_grace = grace;
      }

      if (c_time <= now) {
        coro.resume();

        continue;
      } else {
        resume_list_lifo_.Push(ptr);
      }
    }
  }

 public:
  Loop(Loop const&) = delete;
  Loop& operator=(Loop const&) = delete;

  Loop() {
    while (!resume_list_lifo_.Full()) {
      resume_list_lifo_.Push(ResumeRequest<Runtime>{});
    }

    while (!resume_list_lifo_.Empty()) {
      resume_list_lifo_.Pop();
    }
  }

  //* Loop traits

  void RequestResumeAt(typename Runtime::Clock::time_point time_point,
                       std::coroutine_handle<> coroutine) {
    auto now = time.Now().time_since_epoch();
    auto delta = time_point - time.Started();

    auto node = ResumeRequest<Runtime>{
        .time_point = time_point,
        .coroutine = coroutine,
    };

    resume_list_lifo_.Push(node);
  }

  //* Root context
  [[noreturn]] void Run() {
    while (true) {
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
        logger.Info("Elapsed: %d, Now: %d", time.ElapsedTime().count(),
                    time.Now().time_since_epoch().count());

        robotics::system::SleepFor(1s);
      }
    });
  }
};
}  // namespace robobus::runtime