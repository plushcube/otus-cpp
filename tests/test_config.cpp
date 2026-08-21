#include <gtest/gtest.h>

#include <config.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

Config parse(const std::vector<std::string> &args) {
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

std::string parse_error(const std::vector<std::string> &args) {
  std::vector<char *> argv;
  argv.reserve(args.size());
  for (const auto &a : args) {
    argv.push_back(const_cast<char *>(a.c_str()));
  }
  auto res = Config::make_config(static_cast<int>(argv.size()), argv.data());
  if (res) {
    throw std::runtime_error("expected parse failure");
  }
  return res.error();
}

} // namespace

TEST(ConfigTest, NoDirsIsError) {
  EXPECT_EQ(parse_error({"bayan"}), "No input directories specified!");
}

TEST(ConfigTest, NoBlockSizeIsError) {
  EXPECT_EQ(parse_error({"bayan", "dir"}), "No block size specified!");
}

TEST(ConfigTest, NoHashIsError) {
  EXPECT_EQ(parse_error({"bayan", "dir", "-S", "1024"}),
            "No hash algorithm specified");
}

TEST(ConfigTest, UnknownHashIsError) {
  EXPECT_EQ(parse_error({"bayan", "dir", "-S", "1024", "-H", "xx"}),
            "Unknown hash function 'xx'!");
}

TEST(ConfigTest, HelpIsErrorWithOptionsText) {
  EXPECT_NE(parse_error({"bayan", "--help"}), "");
  // содержимое справки (список опций) попадает в текст ошибки
  EXPECT_NE(parse_error({"bayan", "--help"}).find("Allowed options"),
            std::string::npos);
}

TEST(ConfigTest, MinimalValidConfig) {
  const Config cfg = parse({"bayan", "dir", "-S", "1024", "-H", "md5"});
  EXPECT_EQ(cfg.dirs, (std::vector<std::string>{"dir"}));
  EXPECT_TRUE(cfg.exclude.empty());
  EXPECT_TRUE(cfg.masks.empty());
  // По умолчанию глубина не ограничена.
  EXPECT_EQ(cfg.depth, static_cast<size_t>(std::numeric_limits<int>::max()));
  EXPECT_EQ(cfg.block, 1024u);
  EXPECT_EQ(cfg.min_size, 1u);
  EXPECT_EQ(cfg.hash, Config::Hash::md5);
}

TEST(ConfigTest, AllOptionsParsed) {
  const Config cfg =
      parse({"bayan", "a", "b", "-E", "x", "-M", "*.cpp", "-D", "3", "-F", "10",
             "-S", "4096", "-H", "sha1"});
  EXPECT_EQ(cfg.dirs, (std::vector<std::string>{"a", "b"}));
  EXPECT_EQ(cfg.exclude, (std::vector<std::string>{"x"}));
  EXPECT_EQ(cfg.masks, (std::vector<std::string>{"*.cpp"}));
  EXPECT_EQ(cfg.depth, 3u);
  EXPECT_EQ(cfg.block, 4096u);
  EXPECT_EQ(cfg.min_size, 10u);
  EXPECT_EQ(cfg.hash, Config::Hash::sha1);
}

TEST(ConfigTest, BlockSizeBelowOneIsError) {
  EXPECT_EQ(parse_error({"bayan", "dir", "--block-size=0", "-H", "crc32"}),
            "Wrong block size specified!");
}

TEST(ConfigTest, NegativeDepthUnlimitedAndMinSizeClamped) {
  const Config cfg = parse({"bayan", "dir", "-S", "1024", "-H", "crc32",
                            "--depth=-5", "--min-size=0"});
  EXPECT_EQ(cfg.block, 1024u);
  // Отрицательная глубина трактуется как «без ограничения».
  EXPECT_EQ(cfg.depth, static_cast<size_t>(std::numeric_limits<int>::max()));
  EXPECT_EQ(cfg.min_size, 1u); // <1 -> 1
  EXPECT_EQ(cfg.hash, Config::Hash::crc32);
}

// Маски не зависят от регистра: при разборе конфига они приводятся
// к нижнему регистру.
TEST(ConfigTest, MasksAreLowercasedAtParseTime) {
  const Config cfg = parse({"bayan", "dir", "-S", "1024", "-H", "md5",
                            "-M", "*.TXT", "-M", "*.Cpp"});
  EXPECT_EQ(cfg.masks, (std::vector<std::string>{"*.txt", "*.cpp"}));
}

// Ошибки разбора (некорректное значение, неизвестная опция, повторное
// указание скалярной опции) возвращаются как ошибка конфига, а не
// роняют программу uncaught exception'ом от boost::program_options.
TEST(ConfigTest, NonNumericValueIsCommandLineError) {
  EXPECT_NE(parse_error({"bayan", "dir", "-S", "abc", "-H", "md5"})
                .find("Command line error"),
            std::string::npos);
}

TEST(ConfigTest, UnknownOptionIsCommandLineError) {
  EXPECT_NE(parse_error({"bayan", "dir", "-S", "1024", "-H", "md5", "--bogus"})
                .find("Command line error"),
            std::string::npos);
}

TEST(ConfigTest, RepeatedScalarOptionIsCommandLineError) {
  EXPECT_NE(parse_error({"bayan", "dir", "-S", "1024", "-S", "2048", "-H", "md5"})
                .find("Command line error"),
            std::string::npos);
}
