#include <gtest/gtest.h>

#include <lib.h>

TEST(version_test, basic_assertion) {
  EXPECT_GT(version(), 0);
}
