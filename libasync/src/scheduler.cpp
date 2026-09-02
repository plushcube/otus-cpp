#include <async/dispatcher.h>
#include <async/scheduler.h>

Scheduler::Task Scheduler::start_task(const size_t &sz) {
  p_gcd->start();

  auto p = std::make_shared<Parser>(p_di, sz);
  std::unique_lock lk(m_mutex);
  m_tasks[m_next_id] = p;
  return {m_next_id++, p};
}

void Scheduler::stop_task(const ID &id) {
  std::shared_ptr<Parser> p;
  {
    std::unique_lock lk(m_mutex);
    auto it = m_tasks.find(id);
    if (it == m_tasks.end()) {
      return;
    }
    p = it->second;
    m_tasks.erase(it);
  }
  p->stop();

  p_gcd->stop();
}

std::shared_ptr<Parser> Scheduler::get_value(const ID &id) {
  std::shared_lock lk(m_mutex);
  auto it = m_tasks.find(id);
  return it == m_tasks.end() ? nullptr : it->second;
}
