#include "join_storage.h"

#include <gtest/gtest.h>

#include <string>

namespace {

void insert_a(JoinStorage &s, int id, const std::string &name) {
  EXPECT_EQ(s.execute("INSERT A " + std::to_string(id) + " " + name), "OK\n");
}

void insert_b(JoinStorage &s, int id, const std::string &name) {
  EXPECT_EQ(s.execute("INSERT B " + std::to_string(id) + " " + name), "OK\n");
}

} // namespace

TEST(JoinStorageTest, BuildTablesFromTaskScenario) {
  JoinStorage s;
  insert_a(s, 0, "lean");
  insert_a(s, 1, "sweater");
  insert_a(s, 2, "frank");
  insert_a(s, 3, "violation");
  insert_a(s, 4, "quality");
  insert_a(s, 5, "precision");
  insert_b(s, 3, "proposal");
  insert_b(s, 4, "example");
  insert_b(s, 5, "lake");
  insert_b(s, 6, "flour");
  insert_b(s, 7, "wonder");
  insert_b(s, 8, "selection");
}

TEST(JoinStorageTest, DuplicateIdIsRejected) {
  JoinStorage s;
  insert_a(s, 0, "lean");
  EXPECT_EQ(s.execute("INSERT A 0 understand"), "ERR duplicate 0\n");
  EXPECT_EQ(s.execute("INSERT B 0 other"), "OK\n");
  EXPECT_EQ(s.execute("INSERT B 0 again"), "ERR duplicate 0\n");
}

TEST(JoinStorageTest, IntersectionMatchesTaskOutput) {
  JoinStorage s;
  insert_a(s, 0, "lean");
  insert_a(s, 1, "sweater");
  insert_a(s, 2, "frank");
  insert_a(s, 3, "violation");
  insert_a(s, 4, "quality");
  insert_a(s, 5, "precision");
  insert_b(s, 3, "proposal");
  insert_b(s, 4, "example");
  insert_b(s, 5, "lake");
  insert_b(s, 6, "flour");
  insert_b(s, 7, "wonder");
  insert_b(s, 8, "selection");

  const std::string expected =
      "3,violation,proposal\n4,quality,example\n5,precision,lake\nOK\n";
  EXPECT_EQ(s.execute("INTERSECTION"), expected);
}

TEST(JoinStorageTest, SymmetricDifferenceMatchesTaskOutput) {
  JoinStorage s;
  insert_a(s, 0, "lean");
  insert_a(s, 1, "sweater");
  insert_a(s, 2, "frank");
  insert_a(s, 3, "violation");
  insert_a(s, 4, "quality");
  insert_a(s, 5, "precision");
  insert_b(s, 3, "proposal");
  insert_b(s, 4, "example");
  insert_b(s, 5, "lake");
  insert_b(s, 6, "flour");
  insert_b(s, 7, "wonder");
  insert_b(s, 8, "selection");

  const std::string expected = "0,lean,\n1,sweater,\n2,frank,\n6,,flour\n7,,wonder\n8,,selection\nOK\n";
  EXPECT_EQ(s.execute("SYMMETRIC_DIFFERENCE"), expected);
}

TEST(JoinStorageTest, TruncateClearsTable) {
  JoinStorage s;
  insert_a(s, 1, "one");
  insert_b(s, 1, "uno");
  EXPECT_EQ(s.execute("TRUNCATE A"), "OK\n");
  EXPECT_EQ(s.execute("INTERSECTION"), "OK\n");
  EXPECT_EQ(s.execute("SYMMETRIC_DIFFERENCE"), "1,,uno\nOK\n");
  EXPECT_EQ(s.execute("TRUNCATE B"), "OK\n");
  EXPECT_EQ(s.execute("SYMMETRIC_DIFFERENCE"), "OK\n");
}

TEST(JoinStorageTest, EmptyTablesProduceNoRows) {
  JoinStorage s;
  EXPECT_EQ(s.execute("INTERSECTION"), "OK\n");
  EXPECT_EQ(s.execute("SYMMETRIC_DIFFERENCE"), "OK\n");
}

TEST(JoinStorageTest, RowsAreSortedById) {
  JoinStorage s;
  insert_a(s, 20, "twenty");
  insert_a(s, 2, "two");
  insert_a(s, 15, "fifteen");
  insert_b(s, 2, "dos");
  insert_b(s, 20, "veinte");
  EXPECT_EQ(s.execute("INTERSECTION"), "2,two,dos\n20,twenty,veinte\nOK\n");
  EXPECT_EQ(s.execute("SYMMETRIC_DIFFERENCE"), "15,fifteen,\nOK\n");
}

TEST(JoinStorageTest, MalformedCommandsReturnError) {
  JoinStorage s;
  EXPECT_EQ(s.execute(""), "OK\n");
  EXPECT_EQ(s.execute("INTERSECT"), "ERR unknown command\n");
  EXPECT_EQ(s.execute("INSERT"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("INSERT C 1 name"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("INSERT A x name"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("INSERT A 1"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("TRUNCATE"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("TRUNCATE C"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("INTERSECTION extra"), "ERR invalid arguments\n");
}

TEST(JoinStorageTest, NameMayContainSpaces) {
  JoinStorage s;
  EXPECT_EQ(s.execute("INSERT A 7 John Doe"), "OK\n");
  EXPECT_EQ(s.execute("INSERT B 7 Jane Roe"), "OK\n");
  EXPECT_EQ(s.execute("INSERT A 8 Single"), "OK\n");
  EXPECT_EQ(s.execute("INTERSECTION"), "7,John Doe,Jane Roe\nOK\n");
  EXPECT_EQ(s.execute("SYMMETRIC_DIFFERENCE"), "8,Single,\nOK\n");
}

TEST(JoinStorageTest, SeparatorIsStrictlyOneSpace) {
  JoinStorage s;
  EXPECT_EQ(s.execute("INSERT  A 1 x"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("INSERT A  1 x"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("INSERT A 1 "), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("TRUNCATE  A"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("TRUNCATE A "), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("INTERSECTION "), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("SYMMETRIC_DIFFERENCE extra"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("INSERT A\t1\tx"), "ERR invalid arguments\n");
  EXPECT_EQ(s.execute("INSERT\tA\t1\tx"), "ERR unknown command\n");
  EXPECT_EQ(s.execute("INSERT A 1 x y z"), "OK\n");
}
