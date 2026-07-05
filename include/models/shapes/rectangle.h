#pragma once

#include "models/rect.h"
#include "shape.h"

class Rectangle : public Shape {
public:
  Rectangle(const Size &sz, const float &r = 0.0) : m_size(sz), m_radius(r) {}

  void draw(const bool &) const override;
  Rect get_bounds() const override { return {{}, m_size}; }

private:
  Size m_size;
  float m_radius;
};
