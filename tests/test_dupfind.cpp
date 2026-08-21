#include <gtest/gtest.h>

#include <dupfind.h>

#include "utils/utils.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;
using namespace test_utils;

namespace {

void write_file(const fs::path &path, const std::string &content) {
  std::ofstream out(path);
  out << content;
}

// Множества путей всех групп результата, отсортированные для сравнения
// (порядок групп и файлов внутри группы не специфицирован).
std::vector<std::set<fs::path>> group_sets(const DupFinder::Result &result) {
  std::vector<std::set<fs::path>> groups;
  for (const auto &group : result) {
    groups.emplace_back(group.begin(), group.end());
  }
  std::sort(groups.begin(), groups.end());
  return groups;
}

// Конфиг для тестов блочной модели: заданный размер блока, хэш md5.
Config block_config(const std::string &dir, size_t block) {
  return Config{std::vector<std::string>{dir}, {}, {}, 0u, block, 1u, Config::Hash::md5};
}

std::string hash_name(Config::Hash hash) {
  switch (hash) {
  case Config::Hash::crc32:
    return "crc32";
  case Config::Hash::md5:
    return "md5";
  case Config::Hash::sha1:
    return "sha1";
  }
  return "unknown";
}

} // namespace

// Фикстура tests/fixtures/tree содержит ровно одну пару дубликатов:
// sub_a/dup.txt и sub_b/dup.txt. Остальные файлы уникальны (пустой
// .gitkeep отсекается min_size = 1).
TEST(DupFinderTest, FindsSingleDuplicateGroupInFixtureTree) {
  const Config cfg = make_config({fixture_dir().string()});
  const auto result = DupFinder{cfg}.run();

  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(group_sets(result)[0],
            (std::set<fs::path>{fixture_dir() / "sub_a" / "dup.txt",
                                fixture_dir() / "sub_b" / "dup.txt"}));
}

TEST(DupFinderTest, MissingDirProducesEmptyResult) {
  const Config cfg = make_config({"/nonexistent_dir_for_bayan_test"});
  EXPECT_NO_THROW(DupFinder{cfg}.run());
  EXPECT_TRUE(DupFinder{cfg}.run().empty());
}

// Несколько групп дубликатов в одном дереве находятся все и только они:
// без ложных срабатываний и без пропуска существующих дубликатов.
TEST(DupFinderTest, FindsAllDuplicateGroupsWithoutExtras) {
  TempDir tmp;
  write_file(tmp.path() / "g1a.txt", "group one content");
  write_file(tmp.path() / "g1b.txt", "group one content");
  write_file(tmp.path() / "g2a.txt", "group two content");
  write_file(tmp.path() / "g2b.txt", "group two content");
  write_file(tmp.path() / "unique.txt", "unique content");

  const Config cfg = make_config({tmp.path().string()});
  const auto result = DupFinder{cfg}.run();

  EXPECT_EQ(group_sets(result),
            (std::vector<std::set<fs::path>>{
                {tmp.path() / "g1a.txt", tmp.path() / "g1b.txt"},
                {tmp.path() / "g2a.txt", tmp.path() / "g2b.txt"}}));
}

// Дубликатов может быть больше двух: три одинаковых файла образуют одну
// группу, и ни один файл не попадает в вывод повторно.
TEST(DupFinderTest, MoreThanTwoDuplicatesFormOneGroup) {
  TempDir tmp;
  write_file(tmp.path() / "a.txt", "same content");
  write_file(tmp.path() / "b.txt", "same content");
  write_file(tmp.path() / "c.txt", "same content");

  const Config cfg = make_config({tmp.path().string()});
  const auto result = DupFinder{cfg}.run();

  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(group_sets(result)[0],
            (std::set<fs::path>{tmp.path() / "a.txt", tmp.path() / "b.txt",
                                tmp.path() / "c.txt"}));
}

// Файлы с разным числом блоков не могут быть дубликатами, даже если
// первый блок совпадает: полная последовательность хешей блоков должна
// совпасть. "Hello" — 1 блок, "Hello\nWorld!!!" — 3 блока при S = 5
// (в текущей реализации это обеспечивает проверка равенства размеров).
TEST(DupFinderTest, FilesWithDifferentBlockCountsAreNotDuplicates) {
  TempDir tmp;
  write_file(tmp.path() / "short.txt", "Hello");
  write_file(tmp.path() / "long.txt", "Hello\nWorld!!!");

  const Config cfg = block_config(tmp.path().string(), 5);
  EXPECT_TRUE(DupFinder{cfg}.run().empty());
}

// Хвостовой неполный блок дополняется нулями: файлы одинакового размера,
// но с разным последним байтом, получают разные хеши последних блоков:
// "Hello\n" -> ["Hello", "\n\0\0\0\0"], "Hello\0" -> ["Hello", "\0\0\0\0\0"].
// Файлы разного размера дубликатами не считаются (проверка размера).
TEST(DupFinderTest, LastPartialBlockIsZeroPadded) {
  TempDir tmp;
  write_file(tmp.path() / "nl.txt", "Hello\n");
  write_file(tmp.path() / "nul.txt", std::string("Hello\0", 6));
  write_file(tmp.path() / "copy.txt", "Hello\n");

  const Config cfg = block_config(tmp.path().string(), 5);
  const auto result = DupFinder{cfg}.run();

  // Дубликаты — только два файла с одинаковым содержимым "Hello\n".
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(group_sets(result)[0],
            (std::set<fs::path>{tmp.path() / "nl.txt", tmp.path() / "copy.txt"}));
}

// Файлы разного размера дубликатами не считаются: проверяется размер
// файла, а не число блоков. "Hello\n" (6 байт) и "Hello\n\0\0\0\0"
// (10 байт) при S = 5 дают одинаковую последовательность блоков
// ["Hello", "\n\0\0\0\0"] (хвостовой неполный блок дополняется нулями),
// но разный размер — не пара.
TEST(DupFinderTest, DifferentSizesAreNotDuplicates) {
  TempDir tmp;
  write_file(tmp.path() / "six.txt", "Hello\n");
  write_file(tmp.path() / "ten.txt", std::string("Hello\n\0\0\0\0", 10));

  const Config cfg = block_config(tmp.path().string(), 5);
  EXPECT_TRUE(DupFinder{cfg}.run().empty());
}

// Если файл не удалось прочитать (например, нет прав), он не должен
// считаться дубликатом другого файла: сбой чтения прерывает сравнение.
// NB: под root права снять не удастся — тест пропускается.
TEST(DupFinderTest, UnreadableFilesAreNotDuplicates) {
  TempDir tmp;
  write_file(tmp.path() / "n1.txt", "content one");
  write_file(tmp.path() / "n2.txt", "content two");

  std::error_code ec;
  fs::permissions(tmp.path() / "n1.txt", fs::perms::none, ec);
  fs::permissions(tmp.path() / "n2.txt", fs::perms::none, ec);
  if (ec) {
    GTEST_SKIP() << "cannot remove file permissions";
  }
  {
    std::ifstream probe(tmp.path() / "n1.txt", std::ios::binary);
    if (probe.is_open()) {
      GTEST_SKIP() << "running as root: files remain readable";
    }
  }

  const Config cfg = make_config({tmp.path().string()});
  EXPECT_TRUE(DupFinder{cfg}.run().empty());
}

// В выводе должны быть полные пути, даже если директория задана
// относительным путём.
TEST(DupFinderTest, ResultPathsAreAbsolute) {
  TempDir tmp;
  write_file(tmp.path() / "a.txt", "same");
  write_file(tmp.path() / "b.txt", "same");

  std::error_code ec;
  const fs::path rel = fs::relative(tmp.path(), fs::current_path(), ec);
  const std::string root = ec ? tmp.path().string() : rel.string();

  const Config cfg = make_config({root});
  const auto result = DupFinder{cfg}.run();

  ASSERT_EQ(result.size(), 1u);
  for (const auto &path : result[0]) {
    EXPECT_TRUE(fs::path(path).is_absolute());
  }
}

// Работа утилиты с разными алгоритмами хэширования: каждый из
// поддерживаемых хэшей должен находить дубликаты и не давать
// ложных срабатываний на различных файлах.
class DupFinderHashTest : public ::testing::TestWithParam<Config::Hash> {};

INSTANTIATE_TEST_SUITE_P(
    HashAlgorithms, DupFinderHashTest,
    ::testing::Values(Config::Hash::crc32, Config::Hash::md5, Config::Hash::sha1),
    [](const ::testing::TestParamInfo<Config::Hash> &info) {
      return hash_name(info.param);
    });

TEST_P(DupFinderHashTest, IdenticalFilesAreFound) {
  TempDir tmp;
  const std::string content = "content for hash tests";
  write_file(tmp.path() / "a.dat", content);
  write_file(tmp.path() / "b.dat", content);
  write_file(tmp.path() / "c.dat", "different content");

  const Config cfg = make_config({tmp.path().string()}, {}, GetParam());
  const auto result = DupFinder{cfg}.run();

  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(group_sets(result)[0],
            (std::set<fs::path>{tmp.path() / "a.dat", tmp.path() / "b.dat"}));
}

TEST_P(DupFinderHashTest, DistinctFilesAreNotDuplicates) {
  TempDir tmp;
  write_file(tmp.path() / "a.dat", "AAAA");
  write_file(tmp.path() / "b.dat", "BBBB");

  const Config cfg = make_config({tmp.path().string()}, {}, GetParam());
  EXPECT_TRUE(DupFinder{cfg}.run().empty());
}
