#pragma once

#include "models/rect.h"

class Shape {
public:
  virtual ~Shape() = default;
  virtual void draw(const bool &) const = 0;
  virtual Rect get_bounds() const = 0;
};
