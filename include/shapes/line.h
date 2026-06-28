#pragma once

#include "shape.h"
#include "../models/rect.h"

class Line : public Shape {
public:
  Line(const Coord &s, const Coord &e) : m_start(s), m_end(e) {}

  void draw() const override;
  Rect get_bounds() const override;

private:
  Coord m_start;
  Coord m_end;
};
