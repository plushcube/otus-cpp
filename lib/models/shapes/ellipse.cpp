#include <models/shapes/ellipse.h>

#include <iostream>

void Ellipse::draw(const bool &s) const {
  // TODO: draw a line.
  std::cout << "  draw ellipse!" << (s ? " (selected)" : "") << std::endl;
}
