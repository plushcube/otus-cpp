#pragma once

#include "collector.h"

class DynamicCollector : public Collector {
public:
  bool collect(const Command &c) noexcept override {
    switch (c.type) {
    case Command::Type::BlockStart:
      ++m_depth;
      break;
    case Command::Type::BlockEnd:
      --m_depth;
      break;
    case Command::Type::Command:
      m_commands.push_back(c);
      break;
    }
    return true;
  }

  bool is_full() const noexcept override { return m_depth == 0 && !m_commands.empty(); }

  Bulk flush() noexcept override {
    if (!is_full()) {
      return {};
    }

    Bulk r;
    r.start = m_commands[0].timestamp;
    for (const auto &c : m_commands) {
      r.commands.push_back(c.command);
    }
    m_commands.clear();
    m_depth = 0;
    return r;
  }

private:
  std::vector<Command> m_commands;
  size_t m_depth{0};
};
