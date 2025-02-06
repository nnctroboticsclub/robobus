#pragma once

#include "entry.hpp"
#include "entry_iterator.hpp"

namespace robobus::monitor {
class EntryIteratable {
  Entry* head_;

 public:
  EntryIteratable(Entry* head) : head_(head) {}

  auto begin() -> EntryIterator { return EntryIterator{head_}; }

  auto end() -> EntryIterator { return EntryIterator{nullptr}; }
};
}  // namespace robobus::monitor