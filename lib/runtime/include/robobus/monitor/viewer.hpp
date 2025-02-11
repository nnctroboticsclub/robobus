#pragma once

#include <robobus/context/context.hpp>
#include <robobus/coroutine/coroutine.hpp>
#include <robobus/internal/string_literal.hpp>
#include <robobus/runtime/runtime_impls.hpp>

#include "../console/interpreter.hpp"
#include "bus.hpp"

namespace robobus::monitor {

template <typename T>
concept ViewerHandler = requires {
  { T::ReadChar() } -> std::same_as<std::optional<char>>;
};

//! Occupies stdout
template <ViewerHandler vHandler, robobus::runtime::RuntimeImpl Runtime,
          internal::StringLiteral kPath>
coroutine::Coroutine<void> Viewer(context::Context<Runtime, kPath>&& ctx) {
  using namespace std::chrono_literals;

  static char command_buf[256] = {0};

  struct Handler {
    static void HandleInputLine(std::string line) {
      memset(command_buf, 0, sizeof(command_buf));
      memcpy(command_buf, line.c_str(), line.size());
    }
  };
  static robobus::console::Interpreter<Handler> input;

  auto& bus = Bus::GetInstance();

  // Clear screen, Move cursor to top-left, Hide cursor
  printf("\033[2J\033[;H\033[?25l");

  while (true) {
    printf("\033[;H");

    printf("Cmd: %s\x1b[K\n", command_buf);

    printf("\x1b[1;34m# Monitor\x1b[m\x1b[K\n");
    for (auto const& entry : bus.Entries()) {
      printf("%s: %s\x1b[K\n", entry.display_name.c_str(), entry.text.c_str());
    }

    printf("\x1b[1;34m# Logger\x1b[m\x1b[K\n");
    for (auto const& line : bus.GetLogMonitor().Lines()) {
      auto const& [level, tag, text] = line;

      using enum robotics::logger::core::Level;
      switch (level) {
        case kDebug:
          printf("D: ");
          break;
        case kInfo:
          printf("I: ");
          break;
        case kError:
          printf("E: ");
          break;
        case kVerbose:
          printf("V: ");
          break;
        case kTrace:
          printf("T: ");
          break;
        default:
          printf("?: ");
          break;
      }

      auto show_str = [](std::string const& str) {
        for (char i : str) {
          if (std::isprint(i)) {
            putchar(i);
          } else {
            printf("\\x%02x", i);
          }
        }
      };
      printf("\x1b[33m");
      show_str(tag);
      printf("\x1b[m: ");

      show_str(text);
      printf("\x1b[K\n");
    }

    input.Show();

    std::optional<char> r = vHandler::ReadChar();
    if (r.has_value()) {
      input.HandleInput(*r);
    }

    co_await ctx.Sleep(50ms);
  }
}
}  // namespace robobus::monitor