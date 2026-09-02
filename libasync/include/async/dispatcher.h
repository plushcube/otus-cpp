#pragma once

#include <mutex>
#include <thread>

#include <async/queue.h>
#include <collector/collector.h>
#include <processor/printer.h>
#include <processor/saver.h>

class Dispatcher {
public:
  void start();
  void stop();
  void dispatch(const Collector::Bulk &);

private:
  void worker_log();
  void worker_file(const Saver &saver);

  std::mutex m_lifecycle; // сериализует порождение/join воркеров
  int m_context_count{0}; // под m_lifecycle

  std::thread m_log_thread;
  std::thread m_file1_thread;
  std::thread m_file2_thread;

  BlockingQueue<Collector::Bulk> m_log_queue{};
  BlockingQueue<Collector::Bulk> m_file_queue{};

  Printer m_printer;
  Saver m_saver_file1{"1"};
  Saver m_saver_file2{"2"};
};
