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

// Разбор конфига как это делает CLI (включая нормализацию масок).
Config parse_config(const std::vector<std::string> &args) {
  std::vector<char *> argv;
  argv.reserve(args.size());
  for (const auto &a : args) {
    argv.push_back(const_cast<char *>(a.c_str()));
  }
  auto res = Config::make_config(static_cast<int>(argv.size()), argv.data());
  if (!res) {
    throw std::runtime_error("parse failed: " + res.error());
  }
  return *res;
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

// По заданию маски имён файлов не зависят от регистра.
// NB: на macOS APFS по умолчанию регистронезависим, поэтому «File.TXT»
// и «file.txt» — один и тот же файл; проверяем направления по отдельности.
TEST(FileListTest, UppercaseFilenameMatchesLowercaseMask) {
  TempDir tmp;
  write_file(tmp.path() / "File.TXT", "abc");
  write_file(tmp.path() / "other.log", "abc");

  const Config cfg{std::vector<std::string>{tmp.path().string()}, {}, {"*.txt"},
                   0u, 1024u, 1u, Config::Hash::md5};
  EXPECT_EQ(file_names(cfg), (std::set<std::string>{"File.TXT"}));
}

// Маска в верхнем регистре должна матчить файл в нижнем. Нормализация
// масок происходит при разборе конфига (Config::make_config), поэтому
// конфиг строим тем же путём, что и CLI.
TEST(FileListTest, LowercaseFilenameMatchesUppercaseMask) {
  TempDir tmp;
  write_file(tmp.path() / "file.txt", "abc");
  write_file(tmp.path() / "other.log", "abc");

  // argv[0] — имя программы (пропускается парсером), директория — позиционный аргумент.
  const Config cfg = parse_config({"bayan", tmp.path().string(), "-M", "*.TXT", "-S", "1024", "-H", "md5"});
  EXPECT_EQ(file_names(cfg), (std::set<std::string>{"file.txt"}));
}

// Всё поддерево исключённой директории не попадает в обход.
TEST(FileListTest, ExcludedDirSubtreeIsSkipped) {
  TempDir tmp;
  write_file(tmp.path() / "keep.txt", "x");
  std::filesystem::create_directories(tmp.path() / "skip" / "nested");
  write_file(tmp.path() / "skip" / "nested" / "deep.txt", "y");
  write_file(tmp.path() / "skip" / "top.txt", "z");

  const Config cfg = make_config({tmp.path().string()},
                                 {(tmp.path() / "skip").string()});
  EXPECT_EQ(file_names(cfg), (std::set<std::string>{"keep.txt"}));
}

// По заданию depth = 0 означает «только указанная директория, без вложенных».
TEST(FileListTest, DepthZeroScansOnlyTopLevelDirectory) {
  TempDir tmp;
  write_file(tmp.path() / "top.txt", "x");
  std::filesystem::create_directories(tmp.path() / "sub");
  write_file(tmp.path() / "sub" / "nested.txt", "y");

  const Config cfg{std::vector<std::string>{tmp.path().string()}, {}, {},
                   0u, 1024u, 1u, Config::Hash::md5};
  EXPECT_EQ(file_names(cfg), (std::set<std::string>{"top.txt"}));
}
