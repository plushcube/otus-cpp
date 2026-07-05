#pragma once

#include <cstdint>
#include <memory>

#include "shapes/shape.h"

struct Object {
  uint64_t id;
  Coord center;
  std::unique_ptr<Shape> shape;

  Rect get_frame() const noexcept {
    Rect result = shape->get_bounds();
    result.origin = {center.x - result.size.width / 2.0f, center.y - result.size.height / 2.0f};
    return result;
  }
};
