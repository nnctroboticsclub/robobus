#pragma once

#include <cstring>

#include <korobo2025d/console/interpreter.hpp>

#include <robotics/utils/singleton.hpp>

#include "entry.hpp"
#include "entry_iteratable.hpp"
#include "srobo1_log.hpp"

namespace robobus::monitor {
class Bus : public robotics::utils::Singleton<Bus> {
  Entry* head_ = nullptr;
  Entry* tail_ = nullptr;

  srobo1::LogMonitor log_monitor_;

 public:
  void AddEntry(Entry* entry) {
    if (head_ == nullptr) {
      head_ = entry;
      tail_ = entry;
    } else {
      tail_->Next(*entry);
      tail_ = entry;
    }
  }

  auto Entries() { return EntryIteratable{head_}; }

  auto GetLogMonitor() { return log_monitor_; }

  void RemoveEntry(Entry* entry) {
    if (head_ == entry) {
      head_ = &entry->Next();
    } else {
      auto* prev = head_;
      while (&prev->Next() != entry) {
        prev = &prev->Next();
      }
      prev->Next() = entry->Next();
    }
  }

  void RegisterLogger() {
    auto* adapter = new srobo1::LogMonitor::LoggerAdapter{log_monitor_};

    robotics::logger::global_log_sink = adapter;
  }
};
}  // namespace robobus::monitor