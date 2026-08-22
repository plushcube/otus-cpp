#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <collector/collector.h>
#include <collector/provider.h>
#include <di/container.h>
#include <processor/processor.h>

class Parser {
public:
  explicit Parser(std::weak_ptr<DI_Container>, const size_t &);
  void add_processor(std::shared_ptr<Processor>);
  void start() noexcept;
  void process(const std::string &) const noexcept;
  void stop() const noexcept;

private:
  std::shared_ptr<CollectorProvider> p_provider;
  std::vector<std::shared_ptr<Processor>> p_processors;

  void flush() const noexcept;
};
