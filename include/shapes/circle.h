#pragma once

#include "shape.h"

class Circle : public Shape {
public:
  Circle(const float &r) : m_radius(r) {}

  void draw(const bool &) const override;
  Rect get_bounds() const override { return {{}, {m_radius * 2.0f, m_radius * 2.0f}}; }

private:
  float m_radius;
};
