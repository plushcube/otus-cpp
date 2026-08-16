#include <gtest/gtest.h>

#include "utils/utils.h"

#include <filesystem>
#include <fstream>
#include <set>

namespace fs = std::filesystem;
using namespace test_utils;

namespace {

void write_file(const fs::path &path, const std::string &content) {
  std::ofstream out(path);
  out << content;
}

} // namespace

TEST(FileListTest, MissingDirInDirsIsIgnored) {
  const Config cfg = make_config({"/nonexistent_dir_for_bayan_test"});
  EXPECT_TRUE(collect_files(cfg).empty());
}

TEST(FileListTest, MissingDirAlongsideExistingOneIsSkipped) {
  TempDir tmp;
  write_file(tmp.path() / "a.txt", "hi");
  write_file(tmp.path() / "b.bin", "\x01\x02\x03");

  const Config cfg = make_config({"/nonexistent_dir_for_bayan_test", tmp.path().string()});
  EXPECT_EQ(file_names(cfg), (std::set<std::string>{"a.txt", "b.bin"}));
}

TEST(FileListTest, MissingDirInExcludeIsIgnored) {
  TempDir tmp;
  write_file(tmp.path() / "a.txt", "hi");

  const Config cfg = make_config({tmp.path().string()}, {"/nonexistent_dir_for_bayan_test"});
  EXPECT_EQ(file_names(cfg), (std::set<std::string>{"a.txt"}));
}

TEST(FileListTest, FixtureTreeIsFullyScanned) {
  const fs::path root = fixture_dir();
  const Config cfg = make_config({root.string()});

  const auto actual = collect_files(cfg);
  const auto expected = collect_regular_files(root);

  EXPECT_EQ(std::set<fs::path>(actual.begin(), actual.end()),
            std::set<fs::path>(expected.begin(), expected.end()));
}
