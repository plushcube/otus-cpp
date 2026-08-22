#pragma once

#include <string>
#include <vector>

#include "../commands/command.h"

class Collector {
public:
  struct Bulk {
    time_t start;
    std::vector<std::string> commands;
  };

  virtual ~Collector() = default;

  virtual bool collect(const Command &) noexcept = 0;
  virtual bool is_full() const noexcept = 0;
  virtual Bulk flush() noexcept = 0;
};
