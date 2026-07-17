#pragma once

#include "collector.h"
#include "dynamic_collector.h"
#include "static_collector.h"

#include "../di/container.h"

#include <memory>

template <size_t N> class CollectorProvider {
public:
  explicit CollectorProvider(std::shared_ptr<DI_Container<N>> di)
      : p_dynamic(di->dynamic_collector()), p_static(di->static_collector()) {}

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
