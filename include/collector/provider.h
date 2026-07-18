#pragma once

#include "collector.h"
#include "dynamic_collector.h"
#include "static_collector.h"

#include "../di/container.h"

#include <memory>

template <size_t N> class CollectorProvider {
public:
  explicit CollectorProvider(std::weak_ptr<DI_Container<N>> di) {
    auto locked = di.lock();
    if (!locked) {
      throw std::runtime_error("DI container expired!");
    }
    p_dynamic = locked->dynamic_collector();
    p_static = locked->static_collector();
  }

  bool is_dynamic() const noexcept { return m_dynamic; }
  void set_dynamic(const bool v) noexcept { m_dynamic = v; }

  std::shared_ptr<Collector> collector() const noexcept {
    if (m_dynamic) {
      return p_dynamic;
    } else {
      return p_static;
    }
  }

private:
  bool m_dynamic{false};
  std::shared_ptr<DynamicCollector> p_dynamic;
  std::shared_ptr<StaticCollector<N>> p_static;
};
