#include "map_reduce.h"
#include "support.h"

#include <gtest/gtest.h>

#include <string>

TEST(ReducerVarianceTest, ComputesPopulationVariance) {
  // Генеральная дисперсия {1, 2, 3, 4} = 1.25, выборочная была бы ≈ 1.67.
  const std::string output = mr_test::run(map_reduce::reduce_variance, "1\n2\n3\n4\n");

  EXPECT_NEAR(mr_test::scalar(output), 1.25, 0.01);
}

TEST(ReducerVarianceTest, SkipsBlankAndNonNumericLines) {
  const std::string output = mr_test::run(map_reduce::reduce_variance, "1\n\n2\nnot a price\n3\n4\n");

  EXPECT_NEAR(mr_test::scalar(output), 1.25, 0.01);
}

TEST(ReducerVarianceTest, ReturnsZeroForEqualValues) {
  const std::string output = mr_test::run(map_reduce::reduce_variance, "5\n5\n5\n");

  EXPECT_NEAR(mr_test::scalar(output), 0.0, 0.01);
}

TEST(ReducerVarianceTest, ReturnsZeroForSingleValue) {
  const std::string output = mr_test::run(map_reduce::reduce_variance, "42\n");

  EXPECT_NEAR(mr_test::scalar(output), 0.0, 0.01);
}

TEST(ReducerVarianceTest, HandlesSmallSpreadAroundLargeValues) {
  // {100000, 100001, 100002} → 2/3.
  const std::string output = mr_test::run(map_reduce::reduce_variance, "100000\n100001\n100002\n");

  EXPECT_NEAR(mr_test::scalar(output), 0.67, 0.01);
}

TEST(ReducerVarianceTest, PrintsNothingForEmptyInput) { EXPECT_EQ(mr_test::run(map_reduce::reduce_variance, ""), ""); }
