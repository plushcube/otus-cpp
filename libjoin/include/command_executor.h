#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class CommandExecutor {
public:
  void start(size_t workers);
  void stop();
  void submit(std::function<void()> task);

private:
  void worker_loop();

  std::mutex m_mutex;
  std::condition_variable m_cv;
  std::queue<std::function<void()>> m_tasks;
  bool m_stopped{false};
  std::vector<std::thread> m_threads;
};
