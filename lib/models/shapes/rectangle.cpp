#include <models/shapes/rectangle.h>

#include <iostream>

void Rectangle::draw(const bool &s) const {
  // TODO: draw a line.
  std::cout << "  draw rectangle!" << (s ? " (selected)" : "") << std::endl;
}
