#pragma once

#include "container.h"

class RealContainer : public DI_Container, public std::enable_shared_from_this<DI_Container> {
protected:
  using base_shared = std::enable_shared_from_this<DI_Container>;

public:
  std::shared_ptr<Saver> saver() noexcept override { return std::make_shared<Saver>(); }

  std::shared_ptr<DynamicCollector> dynamic_collector() noexcept override {
    return std::make_shared<DynamicCollector>();
  }

  std::shared_ptr<StaticCollector> static_collector(const size_t &n) noexcept override {
    return std::make_shared<StaticCollector>(n);
  }

  std::shared_ptr<CollectorProvider> collector_provider(const size_t &n) noexcept override {
    return std::make_shared<CollectorProvider>(base_shared::shared_from_this(), n);
  }
};
