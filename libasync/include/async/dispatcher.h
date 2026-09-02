#pragma once

#include <atomic>
#include <thread>

#include <async/queue.h>
#include <collector/collector.h>

class Dispatcher {
public:
  explicit Dispatcher() {};

  void start();
  void stop();
  void dispatch(const Collector::Bulk &);

private:
  std::atomic<int> m_context_count{0};
  std::atomic<bool> m_started{false};

  std::thread m_log_thread;
  std::thread m_file1_thread;
  std::thread m_file2_thread;

  BlockingQueue<Collector::Bulk> m_file_queue{};
  BlockingQueue<Collector::Bulk> m_log_queue{};

  bool set_started(const bool &);
  void worker_log();
  void worker_file();
};
