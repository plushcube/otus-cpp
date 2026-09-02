#pragma once

#include <deque>
#include <mutex>

template <typename T> class BlockingQueue {
public:
  explicit BlockingQueue() = default;

  void push(const T &) noexcept;
  T pop();

private:
  std::mutex m_lock;
  std::deque<T> m_deque;
};
