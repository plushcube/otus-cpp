#include <shapes/line.h>

#include <cstdlib>
#include <iostream>

void Line::draw(const bool &s) const {
  // TODO: draw a line.
  std::cout << "  draw line!" << (s ? " (selected)" : "") << std::endl;
}

Rect Line::get_bounds() const {
  float w = std::abs(m_end.x - m_start.x);
  float h = std::abs(m_end.y - m_start.y);
  return {{}, {w, h}};
}
