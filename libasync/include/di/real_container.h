#pragma once

#include <mutex>

#include <async/dispatcher.h>
#include <async/scheduler.h>

#include <collector/dynamic_collector.h>
#include <collector/provider.h>
#include <collector/static_collector.h>
#include <processor/saver.h>

#include "container.h"

class RealContainer : public DI_Container, public std::enable_shared_from_this<DI_Container> {
protected:
  using base_shared = std::enable_shared_from_this<DI_Container>;

public:
  std::shared_ptr<Scheduler> scheduler() noexcept override {
    std::call_once(m_scheduler_once,
                   [this] { p_scheduler = std::make_shared<Scheduler>(base_shared::shared_from_this()); });
    return p_scheduler;
  }

  std::shared_ptr<Dispatcher> dispatcher() noexcept override {
    std::call_once(m_dispatcher_once, [this] { p_dispatcher = std::make_shared<Dispatcher>(); });
    return p_dispatcher;
  }

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

private:
  std::shared_ptr<Scheduler> p_scheduler;
  std::shared_ptr<Dispatcher> p_dispatcher;
  std::once_flag m_scheduler_once;
  std::once_flag m_dispatcher_once;
};

class DI_Builder {
public:
  static std::shared_ptr<DI_Container> get_di() { return shared().p_di; }

private:
  DI_Builder() : p_di(std::make_shared<RealContainer>()) {}

  static DI_Builder &shared() {
    static auto o = DI_Builder();
    return o;
  }

  std::shared_ptr<DI_Container> p_di;
};
