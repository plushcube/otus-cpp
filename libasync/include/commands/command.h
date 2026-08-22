#pragma once

#include <string>

struct Command {
  enum class Type {
    Command,
    BlockStart,
    BlockEnd,
  };

  Type type;
  time_t timestamp;
  std::string command;
};
