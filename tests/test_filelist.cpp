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

// Пересекающиеся корни (/data и /data/subdir): поддерево /data/subdir
// обходится один раз — через родительский корень. Иначе путь попал бы в
// список дважды, и файл мог бы быть объявлен дубликатом самого себя.
TEST(FileListTest, NestedRootIsNotScannedTwice) {
  TempDir tmp;
  std::filesystem::create_directories(tmp.path() / "data" / "subdir");
  write_file(tmp.path() / "data" / "top.txt", "x");
  write_file(tmp.path() / "data" / "subdir" / "deep.txt", "y");

  const Config cfg = make_config({(tmp.path() / "data").string(),
                                  (tmp.path() / "data" / "subdir").string()});
  const auto files = collect_files(cfg);

  ASSERT_EQ(files.size(), 2u);
  EXPECT_EQ(std::set<fs::path>(files.begin(), files.end()),
            (std::set<fs::path>{tmp.path() / "data" / "top.txt",
                                tmp.path() / "data" / "subdir" / "deep.txt"}));
}

// Тот же случай, но корни перечислены в обратном порядке: результат
// не зависит от порядка.
TEST(FileListTest, NestedRootsInReverseOrder) {
  TempDir tmp;
  std::filesystem::create_directories(tmp.path() / "data" / "subdir");
  write_file(tmp.path() / "data" / "top.txt", "x");
  write_file(tmp.path() / "data" / "subdir" / "deep.txt", "y");

  const Config cfg = make_config({(tmp.path() / "data" / "subdir").string(),
                                  (tmp.path() / "data").string()});
  const auto files = collect_files(cfg);

  ASSERT_EQ(files.size(), 2u);
  EXPECT_EQ(std::set<fs::path>(files.begin(), files.end()),
            (std::set<fs::path>{tmp.path() / "data" / "top.txt",
                                tmp.path() / "data" / "subdir" / "deep.txt"}));
}

// Один и тот же корень указан дважды — обходится один раз.
TEST(FileListTest, DuplicateRootIsScannedOnce) {
  TempDir tmp;
  write_file(tmp.path() / "a.txt", "x");

  const Config cfg = make_config({tmp.path().string(), tmp.path().string()});
  const auto files = collect_files(cfg);

  ASSERT_EQ(files.size(), 1u);
  EXPECT_EQ(files[0], tmp.path() / "a.txt");
}

// Разные лексические формы одного пути (слеш на конце, «./», «..»)
// схлопываются нормализацией: корень один, обход один.
TEST(FileListTest, LexicallyDifferentFormsOfSameRoot) {
  TempDir tmp;
  std::filesystem::create_directories(tmp.path() / "data" / "subdir");
  write_file(tmp.path() / "data" / "a.txt", "x");

  const std::string data = (tmp.path() / "data").string();
  const Config cfg = make_config({data + "/", data + "/./subdir/.."});
  const auto files = collect_files(cfg);

  ASSERT_EQ(files.size(), 1u);
  EXPECT_EQ(files[0], tmp.path() / "data" / "a.txt");
}

// Непересекающиеся корни оба обходятся; общих путей нет.
TEST(FileListTest, DisjointRootsAreBothScanned) {
  TempDir tmp;
  std::filesystem::create_directories(tmp.path() / "a");
  std::filesystem::create_directories(tmp.path() / "b");
  write_file(tmp.path() / "a" / "1.txt", "x");
  write_file(tmp.path() / "b" / "2.txt", "y");

  const Config cfg = make_config({(tmp.path() / "a").string(),
                                  (tmp.path() / "b").string()});
  EXPECT_EQ(file_names(cfg), (std::set<std::string>{"1.txt", "2.txt"}));
}
