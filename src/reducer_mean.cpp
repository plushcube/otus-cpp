#include "map_reduce.h"

#include <iostream>

int main() {
  map_reduce::reduce_mean(map_reduce::read_stdin(), std::cout);

  return 0;
}
