#include <async/dispatcher.h>

void Dispatcher::start() {
  int old = m_context_count.fetch_add(1);
  if (old == 0 && !set_started(true)) {
    return;
  }
  // TODO: create threads
}

void Dispatcher::stop() {
  int old = m_context_count.fetch_sub(1);
  if (old == 1 && !set_started(false)) {
    return;
  }
  // TODO: destroy threads
}

void Dispatcher::dispatch(const Collector::Bulk &) {}

bool Dispatcher::set_started(const bool &s) {
  bool old = !s;
  return m_started.compare_exchange_strong(old, false);
}

void Dispatcher::worker_log() {}

void Dispatcher::worker_file() {}
