#include <debug.h>

#include <config.h>
#include <iostream>

void print_config(const Config &c) {
  std::cout << "Config:\n";
  std::cout << "  dirs:  ";
  for (const auto &s : c.dirs) {
    std::cout << s << ", ";
  }
  std::cout << std::endl;
  std::cout << "  exc:   ";
  for (const auto &s : c.exclude) {
    std::cout << s << ", ";
  }
  std::cout << std::endl;
  std::cout << "  mask:  ";
  for (const auto &s : c.masks) {
    std::cout << s << ", ";
  }
  std::cout << std::endl;
  std::cout << "  depth: " << c.depth << std::endl;
  std::cout << "  block: " << c.block << std::endl;
  std::cout << "  file:  " << c.min_size << std::endl;
  std::cout << "  hash:  " << static_cast<int>(c.hash) << std::endl;
}
