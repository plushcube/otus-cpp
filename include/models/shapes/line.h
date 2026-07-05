#pragma once

#include "models/rect.h"
#include "shape.h"

class Line : public Shape {
public:
  Line(const Coord &s, const Coord &e) : m_start(s), m_end(e) {}

  void draw(const bool &) const override;
  Rect get_bounds() const override;

private:
  Coord m_start;
  Coord m_end;
};
