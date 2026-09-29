#include "map_reduce.h"
#include "support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

namespace {

// Эталонные значения посчитаны независимо (python, csv.reader) по столбцу
// price датасета: 48880 корректных строк, 15 строк с нарушенным числом
// столбцов отброшены.
constexpr double kReferenceMean = 152.72;
constexpr double kReferenceVariance = 57687.93;
constexpr double kTolerance = 0.01;
constexpr size_t kReferenceRowCount = 48880;

std::string mappedPrices() {
  std::ifstream input(MR_DATASET_PATH, std::ios::binary);
  EXPECT_TRUE(input.is_open());

  std::ostringstream buffer;
  buffer << input.rdbuf();

  std::ostringstream out;
  map_reduce::map_price(buffer.str(), out);
  return out.str();
}

} // namespace

TEST(PipelineTest, MapsOnlyWellFormedRowsOfDataset) {
  const std::string prices = mappedPrices();

  EXPECT_EQ(std::count(prices.begin(), prices.end(), '\n'), static_cast<ptrdiff_t>(kReferenceRowCount));
}

TEST(PipelineTest, MatchesReferenceStatisticsOfDataset) {
  const std::string prices = mappedPrices();

  EXPECT_NEAR(mr_test::scalar(mr_test::run(map_reduce::reduce_mean, prices)), kReferenceMean, kTolerance);
  EXPECT_NEAR(mr_test::scalar(mr_test::run(map_reduce::reduce_variance, prices)), kReferenceVariance, kTolerance);
}
