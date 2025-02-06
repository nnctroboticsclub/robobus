#pragma once

#include "./entry.hpp"

namespace robobus::monitor {
class EntryIterator {
  Entry* entry_;

 public:
  EntryIterator(Entry* entry) : entry_(entry) {}

  auto is_end() -> bool { return entry_ == nullptr; }

  auto operator!=(EntryIterator const& other) const -> bool {
    return entry_ != other.entry_;
  }

  auto operator++() -> EntryIterator& {
    entry_ = &entry_->Next();
    return *this;
  }

  auto operator*() -> Entry& { return *entry_; }
};
}  // namespace robobus::monitor