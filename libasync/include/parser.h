#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <collector/collector.h>
#include <collector/dynamic_collector.h>
#include <collector/provider.h>
#include <collector/static_collector.h>
#include <commands/cmd_builder.h>
#include <commands/command.h>
#include <di/container.h>
#include <processor/processor.h>

class Parser {
public:
  explicit Parser(std::weak_ptr<DI_Container> di, const size_t &n) {
    auto locked = di.lock();
    if (!locked) {
      throw std::runtime_error("DI container expired!");
    }
    p_provider = locked->collector_provider(n);
  }

  void add_processor(std::shared_ptr<Processor> p) { p_processors.push_back(p); }

  void start() noexcept {}

  void process(const std::string &s) const noexcept {
    const Command cmd = CommandBuilder::make_command(s);

    switch (cmd.type) {
    case Command::Type::Command: {
      const auto collector = p_provider->collector();
      collector->collect(cmd);
      if (collector->is_full()) {
        flush();
      }
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

  void stop() const noexcept { flush(); }

private:
  std::shared_ptr<CollectorProvider> p_provider;
  std::vector<std::shared_ptr<Processor>> p_processors;

  void flush() const noexcept {
    const auto collector = p_provider->collector();
    const Collector::Bulk bulk = collector->flush();

    if (bulk.commands.empty()) {
      return;
    }

    for (const auto &p : p_processors) {
      p->process(bulk);
    }
  }
};
