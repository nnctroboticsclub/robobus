#pragma once

#include <cstdio>

#include <coroutine>

#include <logger/logger.hpp>
#include <robotics/utils/linked_list_node.hpp>
#include <robotics/utils/no_mutex_lifo.hpp>

#include "robobus/internal/sematicses.hpp"
#include "robobus/runtime/runtime_impls.hpp"
#include "time_context.hpp"

#if !defined(NON_THREAD)
#include <chrono>
#include <robotics/thread/thread.hpp>
#endif

namespace robobus::runtime {

template <RuntimeImpl Runtime>
struct ResumeRequest {
  typename Runtime::Clock::time_point time_point;
  std::coroutine_handle<> coroutine;
};

/// @brief コルーチンベースプログラムで用いるコンテキスト
template <RuntimeImpl Runtime>
class Loop : public internal::NonCopyable<Loop<Runtime>> {
  robotics::utils::NoMutexLIFO<ResumeRequest<Runtime>, 20> resume_list_lifo_;

 public:
  TimeContext<typename Runtime::Clock> time;

 private:
  void ProcessResumeList() {
    const auto now = time.Now();

    if (resume_list_lifo_.Empty()) {
      printf("There is no coroutine in waiting list\n");
    }

    const size_t it_max = resume_list_lifo_.Size();
    for (size_t i = 0; i < it_max && !resume_list_lifo_.Empty(); i++) {
      auto ptr = resume_list_lifo_.Pop();

      auto& c_time = ptr.time_point;
      auto& coro = ptr.coroutine;

      auto grace = c_time - now;

      if (c_time > now) {
        resume_list_lifo_.Push(ptr);
        continue;
      }

      coro.resume();
    }
  }

 public:
  Loop(Loop const&) = delete;
  Loop& operator=(Loop const&) = delete;

  Loop(Loop&& other) noexcept
      : resume_list_lifo_(std::move(other.resume_list_lifo_)),
        time(std::move(other.time)) {}

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

#if !defined(NON_THREAD)
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
#endif
};
}  // namespace robobus::runtime