#include <lib.h>

#include <cassert>
#include <iostream>

int main(int, char **) {
  Matrix<int, 0> matrix;

  for (int i = 0; i < 10; ++i) {
    matrix[i][i] = i;
    matrix[i][9 - i] = 9 - i;
  }

  for (int i = 1; i < 9; ++i) {
    for (int j = 1; j < 9; ++j) {
      std::cout << matrix[i][j] << " ";
    }
    std::cout << std::endl;
  }

  std::cout << matrix.size() << std::endl;

  for (const auto &v : matrix) {
    for (const auto &i : v.first) {
      std::cout << "[" << i << "]";
    }
    std::cout << " " << v.second << std::endl;
  }

  ((matrix[100][100] = 314) = 0) = 217;
  // std::cout << matrix[100][100] << std::endl;

  return 0;
}
