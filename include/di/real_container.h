#pragma once

#include "container.h"

template <size_t N> class RealContainer : public DI_Container<N>, public std::enable_shared_from_this<DI_Container<N>> {
protected:
  using base_shared = std::enable_shared_from_this<DI_Container<N>>;

public:
  std::shared_ptr<Saver> saver() noexcept override { return std::make_shared<Saver>(); }

  std::shared_ptr<DynamicCollector> dynamic_collector() noexcept override {
    return std::make_shared<DynamicCollector>();
  }

  std::shared_ptr<StaticCollector<N>> static_collector() noexcept override {
    return std::make_shared<StaticCollector<N>>();
  }

  std::shared_ptr<CollectorProvider<N>> collector_provider() noexcept override {
    return std::make_shared<CollectorProvider<N>>(base_shared::shared_from_this());
  }
};
