#include <models/image.h>

#include <iostream>

uint64_t Image::add_shape(std::unique_ptr<Shape> s, const Coord &c) {
  uint64_t id = m_next_id++;
  m_objects.push_back({id, c, std::move(s)});
  std::cout << "added new shape with id " << id << std::endl;
  return id;
}

void Image::del_shape(const uint64_t &id) {
  for (auto it = m_objects.begin(); it != m_objects.end(); ++it) {
    if (it->id == id) {
      m_objects.erase(it);
      std::cout << "shape with id " << id << " deleted" << std::endl;
    }
  }
}

void Image::draw() const noexcept {
  std::cout << "draw whole image:" << std::endl;
  for (const auto &o : m_objects) {
    o.shape->draw();
  }
}
