#include <gtest/gtest.h>
#include <lib.h>
#include <list>
#include <sstream>
#include <tuple>
#include <vector>

TEST(lib_test, test_byte) {
  std::ostringstream result;
  print_ip(int8_t{-1}, result);
  ASSERT_EQ(result.str(), "255\n");
}

TEST(lib_test, test_word) {
  std::ostringstream result;
  print_ip(int16_t{0}, result);
  ASSERT_EQ(result.str(), "0.0\n");
}

TEST(lib_test, test_dword) {
  std::ostringstream result;
  print_ip(int32_t{2130706433}, result);
  ASSERT_EQ(result.str(), "127.0.0.1\n");
}

TEST(lib_test, test_qword) {
  std::ostringstream result;
  print_ip(int64_t{8875824491850138409}, result);
  ASSERT_EQ(result.str(), "123.45.67.89.101.112.131.41\n");
}

TEST(lib_test, test_string) {
  std::ostringstream result;
  print_ip(std::string{"Hello, World!"}, result);
  ASSERT_EQ(result.str(), "Hello, World!\n");
}

TEST(lib_test, test_vector) {
  std::ostringstream result;
  print_ip(std::vector<int>{100, 200, 300, 400}, result);
  ASSERT_EQ(result.str(), "100.200.300.400\n");
}

TEST(lib_test, test_list) {
  std::ostringstream result;
  print_ip(std::list<int>{400, 300, 200, 100}, result);
  ASSERT_EQ(result.str(), "400.300.200.100\n");
}

TEST(lib_test, test_tuple) {
  std::ostringstream result;
  print_ip(std::make_tuple(123, 456, 789, 0), result);
  ASSERT_EQ(result.str(), "123.456.789.0\n");
}

// TEST(lib_test, test_wrong_tuple) {
//   std::ostringstream result;
//   print_ip(std::make_tuple(123, 456, "foo", 0.0), result);
//   ASSERT_EQ(result.str(), "123.456.foo.0\n");
// }
