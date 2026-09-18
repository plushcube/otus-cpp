#include "map_reduce.h"
#include "support.h"

#include <gtest/gtest.h>

#include <string>

namespace {

// Строки датасета New York City Airbnb Open Data. Цена — 10-й столбец
// (индекс 9), последний столбец — availability_365.
const char *const kRow225 = "2595,Skylit Midtown Castle,2845,Jennifer,Manhattan,Midtown,40.75362,"
                            "-73.98377,Entire home/apt,225,1,45,2019-05-21,0.38,2,355";
const char *const kRow149 = "2539,Clean & quiet apt home by the park,2787,John,Brooklyn,Kensington,"
                            "40.64749,-73.97237,Private room,149,1,9,2018-10-19,0.21,6,365";
const char *const kHeader = "id,name,host_id,host_name,neighbourhood_group,neighbourhood,latitude,"
                            "longitude,room_type,price,minimum_nights,number_of_reviews,last_review,"
                            "reviews_per_month,calculated_host_listings_count,availability_365";

} // namespace

TEST(MapperTest, PrintsPriceOfEachRowInInputOrder) {
  const std::string input = std::string(kRow225) + "\n" + kRow149 + "\n";

  EXPECT_EQ(mr_test::run(map_reduce::map_price, input), "225\n149\n");
}

TEST(MapperTest, HandlesQuotedFieldsWithCommasAndEscapedQuotes) {
  const std::string row = "729306,\"Clean & Quiet BR in Sunset Park, BK\","
                          "3787686,\"Porfirio \"\"Firo\"\" & Maria\",Brooklyn,"
                          "Borough Park,40.64431,-74.00016,Private room,70,1,"
                          "139,2019-06-04,1.70,1,364";

  EXPECT_EQ(mr_test::run(map_reduce::map_price, row + "\n"), "70\n");
}

TEST(MapperTest, SkipsHeaderAndRowsWithBrokenColumnCount) {
  const std::string input =
      std::string(kHeader) + "\n" + kRow149 + "\n" + "1,Short row,2,Ann,Queens,Astoria,40.7,-73.9,Private room,99\n";

  EXPECT_EQ(mr_test::run(map_reduce::map_price, input), "149\n");
}

TEST(MapperTest, SkipsRowsWithNonNumericPrice) {
  const std::string row = "1,Some home,2,Ann,Queens,Astoria,40.7,-73.9,"
                          "Private room,ask,1,2,2019-01-01,0.5,1,10";

  EXPECT_EQ(mr_test::run(map_reduce::map_price, row + "\n"), "");
}
