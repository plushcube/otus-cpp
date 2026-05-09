#include <gtest/gtest.h>
#include <lib.h>

TEST(parser_test, read_valid_ip) {
  std::string line = "192.168.1.1\t42\t42";
  ip4_t ip = read_ip(line);
  EXPECT_EQ(ip, ip4_t(192, 168, 1, 1));
}

TEST(parser_test, read_invalid_ip) {
  std::string line = "192.168\t\t";
  ip4_t ip = read_ip(line);
  EXPECT_EQ(ip, ip4_t(0, 0, 0, 0));
}

TEST(parser_test, read_empty_line) {
  std::string line = "";
  ip4_t ip = read_ip(line);
  EXPECT_EQ(ip, ip4_t(0, 0, 0, 0));
}

TEST(parser_test, read_incomplete_line) {
  std::string line = "192.168.1.1";
  ip4_t ip = read_ip(line);
  EXPECT_EQ(ip, ip4_t(192, 168, 1, 1));
}
