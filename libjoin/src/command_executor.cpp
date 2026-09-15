#include "command_executor.h"

#include <utility>

void CommandExecutor::start(size_t workers) {
  std::lock_guard lk(m_mutex);
  for (size_t i = 0; i < workers; ++i) {
    m_threads.emplace_back([this] { worker_loop(); });
  }
}

void CommandExecutor::stop() {
  {
    std::lock_guard lk(m_mutex);
    m_stopped = true;
  }
  m_cv.notify_all();
  for (auto &thread : m_threads) {
    thread.join();
  }
  m_threads.clear();
}

void CommandExecutor::submit(std::function<void()> task) {
  {
    std::lock_guard lk(m_mutex);
    m_tasks.push(std::move(task));
  }
  m_cv.notify_one();
}

void CommandExecutor::worker_loop() {
  for (;;) {
    std::function<void()> task;
    {
      std::unique_lock lk(m_mutex);
      m_cv.wait(lk, [this] { return m_stopped || !m_tasks.empty(); });
      if (m_stopped && m_tasks.empty()) {
        return;
      }
      task = std::move(m_tasks.front());
      m_tasks.pop();
    }
    task();
  }
}
