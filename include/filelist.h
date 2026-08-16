#pragma once

#include <config.h>

#include <filesystem>
#include <iterator>
#include <stack>

class FileList {
private:
  const Config m_config;

  static bool matches_glob(const char *str, const char *pat);

  class Iterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = std::filesystem::directory_entry;
    using difference_type = std::ptrdiff_t;
    using pointer = const std::filesystem::directory_entry *;
    using reference = const std::filesystem::directory_entry &;

    Iterator() = default;
    Iterator(const Config &);

    reference operator*() const { return m_current_value; }
    pointer operator->() const { return &m_current_value; }

    Iterator &operator++();
    Iterator operator++(int) {
      Iterator tmp(*this);
      ++(*this);
      return tmp;
    }

    bool operator==(const Iterator &other) const;
    bool operator!=(const Iterator &other) const { return !(*this == other); }

  private:
    void advance();

    std::stack<std::filesystem::recursive_directory_iterator> m_stack;
    std::filesystem::recursive_directory_iterator m_end_it;
    std::filesystem::directory_entry m_current_value;
    const Config *m_cfg = nullptr;
    bool m_is_end = true;
  };

public:
  using value_type = std::filesystem::directory_entry;
  using const_iterator = Iterator;

  explicit FileList(const Config &cfg) : m_config(cfg) {}

  const_iterator begin() const { return Iterator(m_config); }
  const_iterator end() const { return Iterator(); }
};
