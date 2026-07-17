#pragma once

#include "commands/command.h"
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <collector/collector.h>
#include <collector/dynamic_collector.h>
#include <collector/provider.h>
#include <collector/static_collector.h>
#include <commands/cmd_builder.h>
#include <di/container.h>

template <size_t N> class Parser {
public:
  explicit Parser(std::shared_ptr<DI_Container<N>> di, const std::function<void(const std::vector<std::string> &)> &f)
      : m_callback(f), p_provider(di->collector_provider()) {}

  void start() noexcept {}

  void process(const std::string &s) const noexcept {
    const Command cmd = CommandBuilder::make_command(s);

    switch (cmd.type) {
    case Command::Type::Command: {
      const auto collector = p_provider->collector();
      if (collector->is_full()) {
        flush();
      }
      collector->collect(cmd);
      break;
    }

    case Command::Type::BlockStart: {
      if (!p_provider->is_dynamic()) {
        flush();
        p_provider->set_dynamic(true);
      }
      const auto collector = p_provider->collector();
      collector->collect(cmd);
      break;
    }

    case Command::Type::BlockEnd: {
      const auto collector = p_provider->collector();
      collector->collect(cmd);
      if (collector->is_full()) {
        flush();
        p_provider->set_dynamic(false);
      }
      break;
    }
    }
  }

  void stop() noexcept { flush(); }

private:
  const std::function<void(const std::vector<std::string> &)> m_callback;
  const std::shared_ptr<CollectorProvider<N>> p_provider;

  void flush() const noexcept {
    const auto collector = p_provider->collector();
    const Collector::Bulk bulk = collector->flush();
    // TODO: save bulk to file
    m_callback(bulk.commands);
  }
};
