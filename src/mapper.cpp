#include "map_reduce.h"

#include <iostream>

int main() {
  map_reduce::map_price(map_reduce::read_stdin(), std::cout);

  return 0;
}
