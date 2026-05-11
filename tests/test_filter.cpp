#include <gtest/gtest.h>
#include <lib.h>

TEST(filter_test, reverse_sort) {
  std::vector<ip4_t> ips = {
    ip4_t(192, 168, 1, 1),
    ip4_t(10, 0, 0, 1),
    ip4_t(172, 16, 0, 1),
  };
  std::vector<ip4_t> sorted_ips = {
    ip4_t(192, 168, 1, 1),
    ip4_t(172, 16, 0, 1),
    ip4_t(10, 0, 0, 1),
  };
  sort_rev(ips);
  ASSERT_EQ(ips, sorted_ips);
}

TEST(filter_test, filter_first) {
  std::vector<ip4_t> ips = {
    ip4_t(192, 168, 1, 1),
    ip4_t(10, 0, 0, 1),
    ip4_t(172, 16, 0, 1),
  };
  std::vector<ip4_t> filtered_ips = {
    ip4_t(10, 0, 0, 1),
  };
  auto result = filter(ips, [](const ip4_t &ip) {
    return std::get<0>(ip) == 10;
  });
  ASSERT_EQ(result, filtered_ips);
}

TEST(filter_test, filter_two) {
  std::vector<ip4_t> ips = {
    ip4_t(192, 168, 1, 1),
    ip4_t(10, 0, 0, 1),
    ip4_t(172, 16, 0, 1),
  };
  std::vector<ip4_t> filtered_ips = {
    ip4_t(172, 16, 0, 1),
  };
  auto result = filter(ips, [](const ip4_t &ip) {
    return std::get<0>(ip) == 172 && std::get<1>(ip) == 16;
  });
  ASSERT_EQ(result, filtered_ips);
}
