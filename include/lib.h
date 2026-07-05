#pragma once

#include <map>
#include <stddef.h>
#include <vector>

template <typename T, T E> class Matrix {

  class Proxy {
  public:
    Proxy(Matrix &m, std::vector<size_t> idx) : m_matrix(m), m_indices(idx) {}

    Proxy operator[](size_t i) {
      m_indices.push_back(i);
      return Proxy(m_matrix, std::move(m_indices));
    }

    Proxy &operator=(const T &v) {
      if (m_indices.size() == m_matrix.m_dimensions) {
        if (v == E) {
          m_matrix.m_values.erase(m_indices);
        } else {
          m_matrix.m_values[m_indices] = v;
        }
      }
      return *this;
    }

    operator T() const {
      if (m_indices.size() == m_matrix.m_dimensions) {
        auto it = m_matrix.m_values.find(m_indices);
        if (it != m_matrix.m_values.cend()) {
          return it->second;
        }
      }
      return E;
    }

  private:
    Matrix &m_matrix;
    std::vector<size_t> m_indices;
  };

public:
  auto begin() { return m_values.begin(); }
  auto end() { return m_values.end(); }
  auto begin() const { return m_values.begin(); }
  auto end() const { return m_values.end(); }

  Matrix() = default;
  explicit Matrix(const size_t &d) : m_dimensions(d) {}

  size_t size() const noexcept { return m_values.size(); }

  Proxy operator[](const size_t &i) { return Proxy(*this, {i}); }

private:
  std::map<std::vector<size_t>, T> m_values{};
  size_t m_dimensions = 2;
};
