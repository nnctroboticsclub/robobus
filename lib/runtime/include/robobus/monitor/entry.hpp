#pragma once

#include <robotics/utils/linked_list_node.hpp>
#include <string>

namespace robobus::monitor {
struct Entry : public robotics::utils::LinkedListNode<Entry> {
  std::string display_name;
  std::string text;
};
}  // namespace robobus::monitor