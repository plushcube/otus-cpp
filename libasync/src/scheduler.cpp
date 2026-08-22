#include <scheduler.h>

Scheduler &Scheduler::shared() noexcept {
  static Scheduler scheduler{};
  return scheduler;
}

Scheduler::Task Scheduler::start_task(const size_t &sz) noexcept {
  auto p = std::make_shared<Parser>(p_di, sz);
  m_tasks[m_next_id] = p;
  return {m_next_id++, p};
}

void Scheduler::stop_task(const size_t &id) noexcept {
  if (!m_tasks.contains(id)) {
    return;
  }
  const auto p = m_tasks[id];
  p->stop();
  m_tasks.erase(id);
}
