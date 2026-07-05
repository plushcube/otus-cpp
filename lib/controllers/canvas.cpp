#include <controllers/canvas.h>

#include <models/shapes/circle.h>
#include <models/shapes/ellipse.h>
#include <models/shapes/line.h>
#include <models/shapes/rectangle.h>

void Canvas::add_circle(const Coord &center, const float &r) const noexcept {
  p_image->add_shape(std::make_unique<Circle>(r), center);
  draw();
}

void Canvas::add_ellipse(const Coord &center, const float &r1, const float &r2) const noexcept {
  p_image->add_shape(std::make_unique<Ellipse>(r1, r2), center);
  draw();
}

void Canvas::add_line(const Coord &start, const Coord &end) const noexcept {
  Coord bl{
      std::min(start.x, end.x),
      std::min(start.y, end.y),
  };
  Line line{start - bl, end - bl};
  Coord center = bl + line.get_bounds().get_center();
  p_image->add_shape(std::make_unique<Line>(line), center);
  draw();
}

void Canvas::add_rectangle(const Coord &start, const Coord &end, const float &r) const noexcept {
  Coord bl{
      std::min(start.x, end.x),
      std::min(start.y, end.y),
  };
  Coord tr{
      std::max(start.x, end.x),
      std::max(start.y, end.y),
  };
  float w = std::abs(tr.x - bl.x);
  float h = std::abs(tr.y - bl.y);
  Rectangle rect{{w, h}, r};
  Coord center = bl + rect.get_bounds().get_center();
  p_image->add_shape(std::make_unique<Rectangle>(rect), center);
  draw();
}

bool Canvas::toggle_selected(const Coord &p) noexcept {
  for (const auto &o : *p_image) {
    if (o.get_frame().point_inside(p)) {
      if (m_selected.contains(o.id)) {
        m_selected.erase(o.id);
      } else {
        m_selected.insert(o.id);
      }
      draw();
      return true;
    }
  }
  return false;
}

void Canvas::del_selected() noexcept {
  for (const auto &id : m_selected) {
    p_image->del_shape(id);
  }
  m_selected.clear();
  draw();
}

void Canvas::draw() const noexcept { p_image->draw(m_selected); }
