#pragma once

#include <memory>
#include <stddef.h>

class Saver;
class DynamicCollector;
class StaticCollector;
class CollectorProvider;

class DI_Container {
public:
  virtual ~DI_Container() = default;

  virtual std::shared_ptr<Saver> saver() noexcept = 0;
  virtual std::shared_ptr<DynamicCollector> dynamic_collector() noexcept = 0;
  virtual std::shared_ptr<StaticCollector> static_collector(const size_t &) noexcept = 0;
  virtual std::shared_ptr<CollectorProvider> collector_provider(const size_t &) noexcept = 0;
};
