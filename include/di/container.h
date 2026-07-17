#pragma once

#include <memory>
#include <stddef.h>

class DynamicCollector;
template <size_t N> class StaticCollector;
template <size_t N> class CollectorProvider;

template <size_t N> class DI_Container {
public:
  virtual ~DI_Container() = default;

  virtual std::shared_ptr<DynamicCollector> dynamic_collector() noexcept = 0;
  virtual std::shared_ptr<StaticCollector<N>> static_collector() noexcept = 0;
  virtual std::shared_ptr<CollectorProvider<N>> collector_provider() noexcept = 0;
};
