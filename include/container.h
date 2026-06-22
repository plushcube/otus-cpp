#pragma once

#include <memory>

template <typename T, class Allocator = std::allocator<T>>
class PlushContainer {
public:
  using value_type = T;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer =
      typename std::allocator_traits<Allocator>::const_pointer;

  class iterator;
  class const_iterator;

  PlushContainer() noexcept = default;
  explicit PlushContainer(const Allocator &alloc) : alloc_(alloc) {}
  explicit PlushContainer(size_type count, const T &value = T()) {
    data_ = alloc_traits::allocate(alloc_, count);
    for (size_type i = 0; i < count; ++i) {
      alloc_traits::construct(alloc_, data_ + i, value);
    }
    size_ = count;
    capacity_ = count;
  }

  PlushContainer(const PlushContainer &) = default;
  PlushContainer &operator=(const PlushContainer &) = default;
  PlushContainer(PlushContainer &&) noexcept = default;
  PlushContainer &operator=(PlushContainer &&) noexcept = default;

  ~PlushContainer() {
    clear();
    if (data_) {
      alloc_traits::deallocate(alloc_, data_, capacity_);
    }
  }

  size_type size() const noexcept { return size_; }
  size_type max_size() const noexcept { return alloc_traits::max_size(alloc_); }
  bool empty() const noexcept { return size_ == 0; }

  reference operator[](size_type n) { return data_[n]; }
  const_reference operator[](size_type n) const { return data_[n]; }

  reference front() { return data_[0]; }
  const_reference front() const { return data_[0]; }

  reference back() { return data_[size_ - 1]; }
  const_reference back() const { return data_[size_ - 1]; }

  iterator begin() noexcept;
  const_iterator begin() const noexcept;
  const_iterator cbegin() const noexcept;

  iterator end() noexcept;
  const_iterator end() const noexcept;
  const_iterator cend() const noexcept;

  void reserve(size_t new_capacity) {
    if (new_capacity <= capacity_) {
      return;
    }

    T *new_data = alloc_traits::allocate(alloc_, new_capacity);
    for (size_t i = 0; i < size_; ++i) {
      alloc_traits::construct(alloc_, new_data + i,
                              std::move_if_noexcept(data_[i]));
    }
    destroy_range(data_, data_ + size_);
    deallocate();

    data_ = new_data;
    capacity_ = new_capacity;
  }

  void push_back(const T &value) {
    if (size_ >= capacity_) {
      reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }
    alloc_traits::construct(alloc_, data_ + size_, value);
    ++size_;
  }

  void push_back(T &&value) {
    if (size_ >= capacity_) {
      reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }
    alloc_traits::construct(alloc_, data_ + size_, std::move(value));
    ++size_;
  }

  template <typename... Args> T &emplace_back(Args &&...args) {
    if (size_ >= capacity_) {
      reserve(capacity_ == 0 ? 1 : capacity_ * 2);
    }
    alloc_traits::construct(alloc_, data_ + size_, std::forward<Args>(args)...);
    ++size_;
    return data_[size_ - 1];
  }

  void pop_back() {
    if (size_ > 0) {
      --size_;
      // Только уничтожаем объект, память не освобождаем
      alloc_traits::destroy(alloc_, data_ + size_);
    }
  }

  void clear() noexcept {
    for (size_type i = 0; i < size_; ++i) {
      alloc_traits::destroy(alloc_, data_ + i);
    }
    size_ = 0;
  }

  class const_iterator {
  public:
    // Обязательные тайпдефы для std::iterator_traits
    using iterator_category = std::forward_iterator_tag;
    using value_type = PlushContainer::value_type;
    using difference_type = PlushContainer::difference_type;
    using pointer = PlushContainer::const_pointer;
    using reference = PlushContainer::const_reference;

    const_iterator() noexcept : ptr_(nullptr) {}

    reference operator*() const { return *ptr_; }
    pointer operator->() const { return ptr_; }

    const_iterator &operator++() {
      ++ptr_;
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator tmp = *this;
      ++ptr_;
      return tmp;
    }

    bool operator==(const const_iterator &other) const {
      return ptr_ == other.ptr_;
    }
    bool operator!=(const const_iterator &other) const {
      return ptr_ != other.ptr_;
    }

  protected:
    explicit const_iterator(pointer p) noexcept : ptr_(p) {}
    pointer ptr_;
    friend class PlushContainer;
  };

  class iterator : public const_iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = PlushContainer::value_type;
    using difference_type = PlushContainer::difference_type;
    using pointer = PlushContainer::pointer;
    using reference = PlushContainer::reference;

    // Конструктор по умолчанию
    iterator() noexcept : const_iterator() {}
    reference operator*() const {
      return const_cast<reference>(const_iterator::operator*());
    }
    pointer operator->() const {
      return const_cast<pointer>(const_iterator::operator->());
    }

    iterator &operator++() {
      const_iterator::operator++();
      return *this;
    }

    iterator operator++(int) {
      iterator tmp = *this;
      const_iterator::operator++();
      return tmp;
    }

  private:
    explicit iterator(PlushContainer::pointer p) noexcept : const_iterator(p) {}
    friend class PlushContainer;
  };

private:
  using alloc_traits = std::allocator_traits<Allocator>;

  Allocator alloc_; // Экземпляр аллокатора
  pointer data_ = nullptr;
  size_type size_ = 0;
  size_type capacity_ = 0;

  void destroy_range(T *start, T *end) noexcept {
    while (start != end) {
      alloc_traits::destroy(alloc_, start);
      ++start;
    }
  }

  void deallocate() {
    if (data_) {
      alloc_traits::deallocate(alloc_, data_, capacity_);
      data_ = nullptr;
      capacity_ = 0;
    }
  }
};

template <typename T, typename A>
typename PlushContainer<T, A>::iterator PlushContainer<T, A>::begin() noexcept {
  return iterator(data_);
}

template <typename T, typename A>
typename PlushContainer<T, A>::const_iterator PlushContainer<T, A>::begin() const noexcept {
  return const_iterator(data_);
}

template <typename T, typename A>
typename PlushContainer<T, A>::const_iterator PlushContainer<T, A>::cbegin() const noexcept {
  return const_iterator(data_);
}

template <typename T, typename A>
typename PlushContainer<T, A>::iterator PlushContainer<T, A>::end() noexcept {
  return iterator(data_ + size_);
}

template <typename T, typename A>
typename PlushContainer<T, A>::const_iterator PlushContainer<T, A>::end() const noexcept {
  return const_iterator(data_ + size_);
}

template <typename T, typename A>
typename PlushContainer<T, A>::const_iterator PlushContainer<T, A>::cend() const noexcept {
  return const_iterator(data_ + size_);
}
