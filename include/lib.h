#pragma once

#include <array>
#include <map>
#include <stddef.h>

template <typename T, T E, size_t N = 2> class Matrix {

  class Proxy {
  public:
    Proxy(Matrix &m, const size_t &f) : m_matrix(m), m_filled(1) { m_indices[0] = f; }
    Proxy(Matrix &m, std::array<size_t, N> idx, const size_t &filled) : m_matrix(m), m_indices(idx), m_filled(filled) {}

    Proxy operator[](size_t i) {
      m_indices[m_filled] = i;
      return Proxy(m_matrix, std::move(m_indices), m_filled + 1);
    }

    Proxy &operator=(const T &v) {
      if (m_filled == N) {
        if (v == E) {
          m_matrix.m_values.erase(m_indices);
        } else {
          m_matrix.m_values[m_indices] = v;
        }
      }
      return *this;
    }

    operator T() const {
      if (m_filled == N) {
        auto it = m_matrix.m_values.find(m_indices);
        if (it != m_matrix.m_values.cend()) {
          return it->second;
        }
      }
      return E;
    }

  private:
    Matrix &m_matrix;
    std::array<size_t, N> m_indices;
    size_t m_filled;
  };

public:
  auto begin() { return m_values.begin(); }
  auto end() { return m_values.end(); }
  auto begin() const { return m_values.begin(); }
  auto end() const { return m_values.end(); }

  Matrix() = default;

  size_t size() const noexcept { return m_values.size(); }

  Proxy operator[](const size_t &i) { return Proxy(*this, i); }

private:
  std::map<std::array<size_t, N>, T> m_values{};
};
