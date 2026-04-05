#pragma once

#include <cstdio>
#include <vector>

#include <coroutine>

#include <Nano/linked_list.hpp>
#include <Nano/queue.hpp>
#include <logger/logger.hpp>
#include "robobus/internal/sematicses.hpp"
#include "robobus/runtime/runtime_impls.hpp"
#include "time_context.hpp"

#if !defined(NON_THREAD)
#include <chrono>
#include "NanoHW/thread.hpp"

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
  Nano::collection::Queue<ResumeRequest<Runtime>, 64> resume_list_lifo_;

 public:
  TimeContext<typename Runtime::Clock> time;
  bool task_finished_ = false;

 private:
  void ProcessResumeList() {
    const auto now = time.Now();

    if (resume_list_lifo_.Empty()) {
      task_finished_ = true;
    }

    std::vector<std::coroutine_handle<>> handles;

    const size_t it_max = resume_list_lifo_.Size();
    for (size_t i = 0; i < it_max && !resume_list_lifo_.Empty(); i++) {
      auto ptr = resume_list_lifo_.Pop();

      auto& c_time = ptr.time_point;
      auto& coro = ptr.coroutine;

      if (c_time > now) {
        resume_list_lifo_.Push(ptr);
        continue;
      }

      handles.emplace_back(coro);
    }

    for (auto handle : handles) {
      //printf("\x1b[41m \x1b[43m \x1b[0m%p __GRAPH__ %p --| Timer |--> %p\n",
      //       this, this, coro.address());
      handle.resume();
    }
  }

 public:
  Loop(Loop&& other) noexcept
      : resume_list_lifo_(std::move(other.resume_list_lifo_)),
        time(std::move(other.time)) {}

  Loop() {
    resume_list_lifo_.Clear();

    // printf("\x1b[41m \x1b[43m \x1b[0m%p __GRAPH__ %p[Loop]\n", this, this);
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

    if (resume_list_lifo_.Full()) {
      printf("\x1b[41m \x1b[43m \x1b[0m%p TimerResumeLIFO Full\n", this, this);
      return;
    }

    resume_list_lifo_.Push(node);
  }

  void ScheduleResumeInstantly(std::coroutine_handle<> coroutine) {
    if (resume_list_lifo_.Full()) {
      printf("\x1b[41m \x1b[43m \x1b[0m%p InstantResumeLIFO Full\n", this,
             this);
      return;
    }
    resume_list_lifo_.Push(ResumeRequest<Runtime>{
        .time_point = time.Now() - std::chrono::milliseconds(1),
        .coroutine = coroutine});
  }

  //* Root context
  void Run() {
    task_finished_ = false;
    while (not task_finished_) {
      time.Tick();
      ProcessResumeList();
    }
  }

#if !defined(NON_THREAD)
  void LaunchDebugThread() {
    using namespace std::chrono_literals;
    static robotics::logger::Logger logger{"debug.loop.robobus",
                                           "Loop\x1b[32mDebug\x1b[m"};

    nano_hw::thread::DynThread thread;
    thread.SetThreadName("Loop-Debug");
    thread.Start([this]() {
      while (true) {
        logger.Info("Elapsed: %d, Now: %d", time.ElapsedTime().count(),
                    time.Now().time_since_epoch().count());

        nano_hw::parallel::SleepForMS(1s);
      }
    });
  }
#endif
};
}  // namespace robobus::runtime