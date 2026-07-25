#pragma once

#include <vector>

#include "collector.h"

class StaticCollector : public Collector {
public:
  StaticCollector(const size_t &n) : m_max(n) {}

  bool collect(const Command &c) noexcept override {
    if (is_full() || c.type != Command::Type::Command) {
      return false;
    }
    m_commands.push_back(c);
    return true;
  }

  bool is_full() const noexcept override { return m_commands.size() == m_max; }

  Collector::Bulk flush() noexcept override {
    if (m_commands.empty()) {
      return {};
    }

    Bulk r;
    r.start = m_commands[0].timestamp;
    for (const auto &c : m_commands) {
      r.commands.push_back(c.command);
    }
    m_commands.clear();
    return r;
  }

private:
  std::vector<Command> m_commands{};
  const size_t m_max;
};
