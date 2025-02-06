#pragma once

#include <cstdio>

#include <string>
#include <vector>

namespace robobus::console {
template <typename T>
concept InterpreterHandler = requires {
  { T::HandleInputLine(std::declval<std::string>()) } -> std::same_as<void>;
};

template <InterpreterHandler Handler>
class Interpreter {
  char buf[256] = {0};
  int length = 0;
  int cursor_pos = 0;

  enum State {
    kNormal,
    kEscape,
    kEscapeBracket,
  };

  State state = kNormal;

  void HandleNormalInput(char ch) {
    switch (ch) {
      case '\n':
      case '\r':
        if (length == 0) {
          break;
        }
        Handler::HandleInputLine(std::string{buf, buf + length});
        length = 0;
        cursor_pos = 0;
        break;

      case 0x1b:
        state = kEscape;
        break;

      case 0x7f:  // Backspace
        if (cursor_pos == 0) {
          break;
        }

        // Shift characters 1 byte left
        for (int i = cursor_pos; i < length; i++) {
          buf[i - 1] = buf[i];
        }

        length--;
        cursor_pos--;
        break;

      case 0x7e:  // Delete
        if (cursor_pos == length) {
          break;
        }

        // Shift characters 1 byte left
        for (int i = cursor_pos + 1; i < length; i++) {
          buf[i - 1] = buf[i];
        }

        length--;
        break;

      default:
        // If trailing characters are available, shift them 1 byte right
        for (int i = length; i > cursor_pos; i--) {
          buf[i] = buf[i - 1];
        }

        // Insert character
        buf[cursor_pos] = ch;

        length++;
        cursor_pos++;
        break;
    }
  }

  void HandleEscapeInput(char ch) {
    switch (ch) {
      case '[':
        state = kEscapeBracket;
        break;
      default:
        state = kNormal;
        break;
    }
  }

  void HandleEscapeBracketInput(char ch) {
    switch (ch) {
      case 'D':
        cursor_pos = std::max(0, cursor_pos - 1);
        break;
      case 'C':
        cursor_pos = std::min(length, cursor_pos + 1);
        break;
      default:
        break;
    }
    state = kNormal;
  }

 public:
  void HandleInput(char ch) {
    switch (state) {
      case kNormal:
        HandleNormalInput(ch);
        break;
      case kEscape:
        HandleEscapeInput(ch);
        break;
      case kEscapeBracket:
        HandleEscapeBracketInput(ch);
        break;
    }
  }

  void Show() {
    printf("$ ");
    for (int i = 0; i < length; i++) {
      if (i == cursor_pos) {
        printf("\033[7m");
      }
      printf("%c", buf[i]);
      if (i == cursor_pos) {
        printf("\033[0m");
      }
    }

    if (cursor_pos == length) {
      printf("\033[7m");
      printf(" ");
      printf("\033[0m");
    }

    printf("\033[K\n");
  }

  std::vector<char> GetBuffer() { return std::vector<char>{buf, buf + length}; }
};
}  // namespace robobus::console