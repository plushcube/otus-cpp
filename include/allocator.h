#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <new>

template <typename T, std::size_t PoolSize = 10> class PlushAllocator {
public:
  using value_type = T;
  using size_type = std::size_t;

  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap_assignment = std::true_type;

  template <typename U> struct rebind {
    using other = PlushAllocator<U, PoolSize>;
  };

  PlushAllocator() noexcept = default;
  template <typename U> PlushAllocator(const PlushAllocator<U> &) noexcept {}
  ~PlushAllocator() {
    while (pool) {
      Pool *next = pool->next_pool;
      delete pool;
      pool = next;
    }
  }

  T *allocate(const size_type &n) {
    if (pool == nullptr) {
      pool = new Pool(n);
    }
    Pool *p = pool;
    while (p->next_pool) {
      p = p->next_pool;
    }

    T *result = p->get_next_free(n);
    if (result) {
      return result;
    } else {
      p->next_pool = new Pool(n);
      return p->next_pool->get_next_free(n);
    }
  }

  void deallocate(T *p, const size_type &) {
    Pool *prev = nullptr;
    Pool *curr = pool;
    while (curr) {
      if (curr->contains(p)) {
        curr->free(p);
        if (curr->empty()) {
          if (prev) {
            prev->next_pool = curr->next_pool;
          } else {
            pool = curr->next_pool;
          }
          delete curr;
        }
        break;
      } else {
        prev = curr;
        curr = curr->next_pool;
      }
    }
  }

  template <class U, class... Args> void construct(U *p, Args &&...args) { new (p) U(std::forward<Args>(args)...); }

  template <typename U> void destroy(U *p) { p->~U(); }

private:
  class Pool {
  public:
    struct alignas(std::max(alignof(T), alignof(void *))) Node {
      Node *next;
      alignas(T) T data;

      T *get_data() { return reinterpret_cast<T *>(data); }
    };

    Pool() = delete;
    Pool(const size_type &n) : capacity(n) {
      data_head = static_cast<Node *>(::operator new(n * sizeof(Node)));
      if (!data_head) {
        throw std::bad_alloc();
      }
      for (size_t i = 0; i < n - 1; ++i) {
        data_head[i].next = &data_head[i + 1];
      }
      data_head[n - 1].next = nullptr;
      next_free = data_head;
    }
    ~Pool() { ::operator delete(static_cast<void *>(data_head)); }

    bool contains(const T *const p) const noexcept {
      const char *pool_start = reinterpret_cast<const char *>(data_head);
      const char *pool_end = pool_start + capacity * sizeof(Node);
      const char *ptr = reinterpret_cast<const char *>(p);
      return ptr >= pool_start && ptr < pool_end;
    }

    T *get_next_free(size_type n) noexcept {
      if (count + n > capacity) {
        return nullptr;
      }
      count += n;
      T *result = &(next_free->data);
      for (size_t i = 0; i < n; ++i) {
        next_free = next_free->next;
      }
      return result;
    }

    void free(T *p) noexcept {
      Node *node = reinterpret_cast<Node *>(p);
      node->next = next_free;
      next_free = node;
      --count;
    }

    bool empty() const noexcept { return count == 0; }

  private:
    Node *data_head{};
    Node *next_free{};
    size_type count{};
    size_type capacity{};
    Pool *next_pool{};

    friend class PlushAllocator;
  };

  Pool *pool{};
};

template <typename T, typename U, std::size_t S1, std::size_t S2>
bool operator==(const PlushAllocator<T, S1> &, const PlushAllocator<U, S2> &) {
  return false;
}
