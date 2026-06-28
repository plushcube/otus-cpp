#pragma once

#include "../models/image.h"

#include <memory>

class Canvas {
public:
  explicit Canvas(std::unique_ptr<Image> image) : p_image(std::move(image)) {}

  void add_circle(const Coord &center, const float &r) const noexcept;
  void add_ellipse(const Coord &center, const float &r1, const float &r2) const noexcept;
  void add_line(const Coord &start, const Coord &end) const noexcept;
  void add_rectangle(const Coord &start, const Coord &end, const float &r = 0.0) const noexcept;

  uint64_t shape_id(const Coord &p) const;
  void del_shape(const uint64_t &id) const;

  void draw() const noexcept;
  Image *raw_image() const noexcept { return p_image.get(); }

private:
  const std::unique_ptr<Image> p_image;
};
