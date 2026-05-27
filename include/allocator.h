#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>
#include <vector>

template <typename T, std::size_t PoolSize = 10> class PlushAllocator {
public:
  using value_type = T;
  using size_type = std::size_t;
  using propagate_on_container_move_assignment = std::true_type;

  template <typename U> struct rebind {
    using other = PlushAllocator<U, PoolSize>;
  };

  PlushAllocator() : m_count(0), m_capacity(0), p_occupied(nullptr) {
    expand_pool();
  }

  template <typename U> PlushAllocator(const PlushAllocator<U> &) {}

  T *allocate(size_type n) {
    T *p = find_gap(n);
    if (p) {
      return p;
    }

    if (n > std::size_t(-1) / sizeof(T)) {
      throw std::bad_alloc();
    }

    while (m_count + n > m_capacity) {
      expand_pool();
    }

    p = &(m_pool[m_count / pool_size][m_count % pool_size]);
    for (size_type i = 0; i < n; ++i) {
      p_occupied[m_count + i] = true;
    }
    m_count += n;
    return p;
  }

  void deallocate(T *p, size_type n) {
    for (size_type i = 0; i < m_count; ++i) {
      if (&m_pool[i / pool_size][i % pool_size] != p) {
        continue;
      }
      for (size_type k = i; k < i + n; ++k) {
        p_occupied[k] = false;
      }
      break;
    }
  }

  template <class U, class... Args> void construct(U *p, Args &&...args) {
    new (p) U(std::forward<Args>(args)...);
  }

  template <typename U> void destroy(U *p) { p->~U(); }

private:
  static constexpr size_type pool_size = PoolSize;

  size_type m_count;
  size_type m_capacity;
  std::vector<T *> m_pool;
  bool *p_occupied;

  T *find_gap(const size_type &n) {
    size_type start = 0;
    while (start < m_count) {
      if (p_occupied[start]) {
        ++start;
      } else {
        bool found = true;
        size_type gap = 1;
        for (; gap < n; ++gap) {
          if (p_occupied[start + gap]) {
            found = false;
            break;
          }
        }
        if (found) {
          return &m_pool[start / pool_size][start % pool_size];
        } else {
          start += gap + 1;
        }
      }
    }
    return nullptr;
  }

  void expand_pool() {
    m_pool.push_back(make_pool(pool_size));
    m_capacity += pool_size;
    if (p_occupied) {
      p_occupied = static_cast<bool *>(
          std::realloc(p_occupied, m_capacity * sizeof(bool)));
    } else {
      p_occupied = static_cast<bool *>(std::malloc(m_capacity * sizeof(bool)));
    }
  }

  static T *make_pool(const size_type &size) {
    T *pool = static_cast<T *>(std::malloc(size * sizeof(T)));
    if (!pool) {
      throw std::bad_alloc();
    }
    return pool;
  }
};

template <typename T, typename U, std::size_t S1, std::size_t S2>
bool operator==(const PlushAllocator<T, S1> &, const PlushAllocator<U, S2> &) {
  return S1 == S2; // аллокаторы равны только при одинаковом размере пула
}

template <typename T, typename U, std::size_t S1, std::size_t S2>
bool operator!=(const PlushAllocator<T, S1> &, const PlushAllocator<U, S2> &) {
  return S1 != S2;
}
