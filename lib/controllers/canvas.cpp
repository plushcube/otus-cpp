#include <controllers/canvas.h>

#include <shapes/circle.h>
#include <shapes/ellipse.h>
#include <shapes/line.h>
#include <shapes/rectangle.h>

void Canvas::add_circle(const Coord &center, const float &r) const noexcept {
  p_image->add_shape(Circle{r}, center);
  draw();
}

void Canvas::add_ellipse(const Coord &center, const float &r1, const float &r2) const noexcept  {
  p_image->add_shape(Ellipse{r1, r2}, center);
  draw();
}

void Canvas::add_line(const Coord &start, const Coord &end) const noexcept {
  Coord bl {
    std::min(start.x, end.x),
    std::min(start.y, end.y),
  };
  Line line { start - bl, end - bl };
  Coord center = bl + line.get_bounds().get_center();
  p_image->add_shape(line, center);
  draw();
}

void Canvas::add_rectangle(const Coord &start, const Coord &end, const float &r) const noexcept {
  Coord bl {
    std::min(start.x, end.x),
    std::min(start.y, end.y),
  };
  Coord tr {
    std::max(start.x, end.x),
    std::max(start.y, end.y),
  };
  float w = std::abs(tr.x - bl.x);
  float h = std::abs(tr.y - bl.y);
  Rectangle rect { { w, h }, r };
  Coord center = bl + rect.get_bounds().get_center();
  p_image->add_shape(rect, center);
  draw();
}

uint64_t Canvas::shape_id(const Coord &p) const {
  for (const auto &o : *p_image) {
    if (o.get_frame().point_inside(p)) {
      return o.id;
    }
  }
  return 0;
}

void Canvas::del_shape(const uint64_t &id) const {
  p_image->del_shape(id);
  draw();
}

void Canvas::draw() const noexcept {
  p_image->draw();
}
