#pragma once

#include "command.h"

#include <chrono>

class CommandBuilder {
public:
  CommandBuilder() = delete;

  static Command make_command(const std::string &c) noexcept {
    auto now = std::chrono::system_clock::now();
    time_t ts = std::chrono::system_clock::to_time_t(now);

    if (c.size() == 0) {
      return {Command::Type::Command, ts, c};
    }

    switch (c[0]) {
    case '{':
      return {Command::Type::BlockStart, ts, c};
    case '}':
      return {Command::Type::BlockEnd, ts, c};
    default:
      return {Command::Type::Command, ts, c};
    }
  }
};
