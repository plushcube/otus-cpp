#include <models/shapes/circle.h>

#include <iostream>

void Circle::draw(const bool &s) const {
  // TODO: draw a line.
  std::cout << "  draw circle!" << (s ? " (selected)" : "") << std::endl;
}
