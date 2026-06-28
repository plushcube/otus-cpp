#include <models/image.h>

uint64_t Image::add_shape(const Shape &, const Coord &) {
  return 0;
}

void Image::del_shape(const uint64_t &id) {
  for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
    if (it->id == id) {
      m_objects.erase(it);
    }
  }
}

void Image::draw() const noexcept {
  for (const auto &o : m_objects) {
    o.shape->draw();
  }
}
