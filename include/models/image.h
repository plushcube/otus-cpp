#pragma once

#include "object.h"

#include <vector>

class Image {
public:
  Image() = default;

  auto begin() { return m_objects.begin(); }
  auto end() { return m_objects.end(); }
  auto begin() const { return m_objects.begin(); }
  auto end() const { return m_objects.end(); }

  uint64_t add_shape(const Shape &, const Coord &);
  void del_shape(const uint64_t &);
  void draw() const noexcept;

private:
  std::vector<Object> m_objects{};
};
