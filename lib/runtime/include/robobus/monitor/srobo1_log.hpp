#pragma once

#include <cstring>

#include <array>
#include <string>

#include <logger/log_sink.hpp>

namespace robobus::monitor::srobo1 {
class LogMonitor {
  using Line = struct {
    robotics::logger::core::Level level;
    std::string tag;
    std::string text;
  };
  using LineArray = std::array<Line, 12>;

  LineArray lines;

  class Iterator {
    LogMonitor* mon;
    Line* line;

   public:
    Iterator(LogMonitor& mon, Line* line) : mon(&mon), line(line) {}

    Line& operator*() { return *line; }
    Line* operator->() { return line; }

    Iterator& operator++() {
      if (line == &mon->lines[11]) {
        line = &mon->lines[0];
      } else {
        line++;
      }
      return *this;
    }

    bool operator!=(Iterator const& other) { return line != other.line; }
  };

  Iterator head{*this, &lines[0]};
  Iterator pointer{*this, &lines[11]};

  class Iteratable {
    LogMonitor* mon;

   public:
    Iteratable(LogMonitor* mon) : mon(mon) {}

    Iterator begin() { return mon->head; }
    Iterator end() { return mon->pointer; }
  };

 public:
  LogMonitor() {
    for (auto& line : lines) {
      line = Line{robotics::logger::core::Level::kInfo, "", "-- No log --"};
    }
  }

  void AddLine(Line const& line) {
    *pointer = line;

    pointer.operator++();
    head.operator++();
  }

  auto Lines() { return Iteratable(this); }

 public:
  class LoggerAdapter : public robotics::logger::LogSink {
    LogMonitor& monitor_;

   public:
    LoggerAdapter(LogMonitor& monitor) : monitor_(monitor) {}

    void Log(robotics::logger::core::Level level, const char* tag,
             const char* msg) override {
      std::string tag_str;
      tag_str.reserve(strlen(tag));
      for (size_t i = 0; tag[i] != '\0'; i++) {
        tag_str.push_back(tag[i]);
      }

      std::string msg_str;
      msg_str.reserve(strlen(msg));
      for (size_t i = 0; msg[i] != '\0'; i++) {
        msg_str.push_back(msg[i]);
      }

      monitor_.AddLine({level, tag_str, msg_str});
    }
  };
};
}  // namespace robobus::monitor::srobo1