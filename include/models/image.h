#pragma once

#include "object.h"

#include <memory>
#include <set>
#include <vector>

class Image {
public:
  Image(const Size &sz) : m_size(sz) {}

  auto begin() { return m_objects.begin(); }
  auto end() { return m_objects.end(); }
  auto begin() const { return m_objects.begin(); }
  auto end() const { return m_objects.end(); }

  uint64_t add_shape(std::unique_ptr<Shape>, const Coord &);
  void del_shape(const uint64_t &);
  void draw(const std::set<uint64_t> &) const noexcept;

private:
  const Size m_size;
  std::vector<Object> m_objects{};
  uint64_t m_next_id{1};
};
