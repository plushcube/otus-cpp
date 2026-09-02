#include <async/dispatcher.h>

#if defined(__APPLE__) || defined(__linux__)
#include <pthread.h>
#endif

namespace {

// Имена потоков для отладки. API на Apple/Linux различается; на остальных
// платформах — no-op.
void set_thread_name(const char *name) noexcept {
#if defined(__APPLE__)
  pthread_setname_np(name);
#elif defined(__linux__)
  pthread_setname_np(pthread_self(), name);
#else
  (void)name;
#endif
}

} // namespace

void Dispatcher::start() {
  std::lock_guard lk(m_lifecycle);
  if (m_context_count++ != 0) {
    return;
  }

  m_log_queue.reopen();
  m_file_queue.reopen();

  m_log_thread = std::thread([this] {
    set_thread_name("bulk-log");
    worker_log();
  });
  m_file1_thread = std::thread([this] {
    set_thread_name("bulk-file1");
    worker_file(m_saver_file1);
  });
  m_file2_thread = std::thread([this] {
    set_thread_name("bulk-file2");
    worker_file(m_saver_file2);
  });
}

void Dispatcher::stop() {
  std::lock_guard lk(m_lifecycle);
  if (--m_context_count != 0) {
    return;
  }

  m_log_queue.close();
  m_file_queue.close();
  m_log_thread.join();
  m_file1_thread.join();
  m_file2_thread.join();
}

void Dispatcher::dispatch(const Collector::Bulk &bulk) {
  m_log_queue.push(bulk);
  m_file_queue.push(bulk);
}

void Dispatcher::worker_log() {
  while (auto bulk = m_log_queue.pop()) {
    m_printer.process(*bulk);
  }
}

void Dispatcher::worker_file(const Saver &saver) {
  while (auto bulk = m_file_queue.pop()) {
    saver.process(*bulk);
  }
}
