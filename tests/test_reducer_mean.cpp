#include "map_reduce.h"
#include "support.h"

#include <gtest/gtest.h>

#include <string>

TEST(ReducerMeanTest, ComputesMeanOfUnsortedStream) {
  const std::string output = mr_test::run(map_reduce::reduce_mean, "300\n100\n200\n");

  EXPECT_NEAR(mr_test::scalar(output), 200.0, 0.01);
}

TEST(ReducerMeanTest, KeepsFractionalPart) {
  const std::string output = mr_test::run(map_reduce::reduce_mean, "1\n2\n");

  EXPECT_NEAR(mr_test::scalar(output), 1.5, 0.01);
}

TEST(ReducerMeanTest, ReturnsSingleValueAsIs) {
  const std::string output = mr_test::run(map_reduce::reduce_mean, "42\n");

  EXPECT_NEAR(mr_test::scalar(output), 42.0, 0.01);
}

TEST(ReducerMeanTest, SkipsBlankAndNonNumericLines) {
  const std::string output = mr_test::run(map_reduce::reduce_mean, "100\n\nnot a price\n200\n");

  EXPECT_NEAR(mr_test::scalar(output), 150.0, 0.01);
}

TEST(ReducerMeanTest, PrintsNothingForEmptyInput) { EXPECT_EQ(mr_test::run(map_reduce::reduce_mean, ""), ""); }
