#pragma once

#include "shape.h"

class Ellipse : public Shape {
public:
  Ellipse(const float &rv, const float &rh) : m_rv(rv), m_rh(rh) {}

  void draw() const override;
  Rect get_bounds() const override { return { {}, { m_rh * 2.0f, m_rv * 2.0f } }; }

private:
  float m_rv;
  float m_rh;
};
