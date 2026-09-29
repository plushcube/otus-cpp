#include "map_reduce.h"

#include <iostream>

int main() {
  map_reduce::reduce_variance(map_reduce::read_stdin(), std::cout);

  return 0;
}
