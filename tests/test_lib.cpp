#include <gtest/gtest.h>
#include <lib.h>

TEST(lib_test, test_create) {
  Matrix<int, -1> matrix;
  ASSERT_EQ(matrix.size(), 0);
}

TEST(lib_test, test_default_value) {
  Matrix<int, -1> matrix;
  auto a = matrix[0][0];
  ASSERT_EQ(a, -1);
  ASSERT_EQ(matrix.size(), 0);
}

TEST(lib_test, test_set_value) {
  Matrix<int, -1> matrix;
  matrix[100][100] = 314;
  auto a = matrix[100][100];
  ASSERT_EQ(a, 314);
  ASSERT_EQ(matrix.size(), 1);
}
