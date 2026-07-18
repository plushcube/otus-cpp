#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include <collector/collector.h>
#include <collector/dynamic_collector.h>
#include <collector/provider.h>
#include <collector/static_collector.h>
#include <commands/cmd_builder.h>
#include <commands/command.h>
#include <di/container.h>
#include <saver/saver.h>

template <size_t N> class Parser {
public:
  explicit Parser(std::weak_ptr<DI_Container<N>> di, const std::function<void(const std::vector<std::string> &)> &f)
      : m_callback(f) {
    auto locked = di.lock();
    if (!locked) {
      throw std::runtime_error("DI container expired!");
    }
    p_provider = locked->collector_provider();
  }

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

  void stop() noexcept { flush(); }

private:
  const std::function<void(const std::vector<std::string> &)> m_callback;
  std::shared_ptr<CollectorProvider<N>> p_provider;
  std::shared_ptr<Saver> p_saver;

  void flush() const noexcept {
    const auto collector = p_provider->collector();
    const Collector::Bulk bulk = collector->flush();

    if (bulk.commands.empty()) {
      return;
    }

    m_callback(bulk.commands);
    p_saver->save(bulk.start, bulk.commands);
  }
};
