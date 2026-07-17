#pragma once

#include <array>

#include "collector.h"

template <size_t N> class StaticCollector : public Collector {
public:
  bool collect(const Command &c) noexcept override {
    if (is_full() || c.type != Command::Type::Command) {
      return false;
    }
    m_commands[m_size++] = c;
    return true;
  }

  bool is_full() const noexcept override { return m_size == N; }

  Collector::Bulk flush() noexcept override {
    if (m_size == 0) {
      return {};
    }

    Bulk r;
    r.start = m_commands[0].timestamp;
    for (size_t i = 0; i < m_size; ++i) {
      r.commands.push_back(m_commands[i].command);
    }
    m_size = 0;
    return r;
  }

private:
  std::array<Command, N> m_commands{};
  size_t m_size{0};
};
