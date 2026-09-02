#pragma once

// Потокобезопасная очередь с блокирующим pop().
//
// Роли:
//   * производитель (Dispatcher::dispatch, вызывается из receive()/flush()) —
//     push(). Не блокируется, только будит одного ожидающего (notify_one).
//   * потребители (воркер-потоки log/file1/file2) — pop(). Блокируются на
//     cv_, пока не появится элемент, либо пока очередь не закроют.
//
// Жизненный цикл (управляется Dispatcher'ом):
//   * start():  reopen()  — очередь снова открыта, воркеры ждут элементы;
//   * stop():   close()   — «данных больше не будет»: воркеры дорабатывают
//                оставшиеся элементы (дренаж) и выходят из pop() с nullopt;
//                после этого диспетчер делает join() потоков;
//   * между close() и следующим reopen() push() вызываться не должен
//     (последний flush() происходит до close() — см. порядок в Dispatcher::stop).

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

template <typename T> class BlockingQueue {
public:
  explicit BlockingQueue() = default;

  BlockingQueue(const BlockingQueue &) = delete;
  BlockingQueue &operator=(const BlockingQueue &) = delete;

  void push(const T &v) {
    {
      std::lock_guard lk(m_lock);
      m_queue.push_back(v);
    }
    m_cv.notify_one();
  }

  void push(T &&v) {
    {
      std::lock_guard lk(m_lock);
      m_queue.push_back(std::move(v));
    }
    m_cv.notify_one();
  }

  std::optional<T> pop() {
    std::unique_lock lk(m_lock);
    m_cv.wait(lk, [this] { return !m_queue.empty() || m_closed; });
    if (m_queue.empty()) {
      return std::nullopt; // закрыта и пуста
    }
    T v = std::move(m_queue.front());
    m_queue.pop_front();
    return v;
  }

  void close() {
    {
      std::lock_guard lk(m_lock);
      m_closed = true;
    }
    m_cv.notify_all();
  }

  void reopen() {
    std::lock_guard lk(m_lock);
    m_closed = false;
  }

  bool closed() {
    std::lock_guard lk(m_lock);
    return m_closed;
  }

  std::size_t size() {
    std::lock_guard lk(m_lock);
    return m_queue.size();
  }

private:
  std::mutex m_lock;
  std::condition_variable m_cv;
  std::deque<T> m_queue;
  bool m_closed{false};
};
