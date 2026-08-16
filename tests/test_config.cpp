#include <gtest/gtest.h>

#include <config.h>

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
  EXPECT_EQ(cfg.depth, 0u);
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

TEST(ConfigTest, OutOfRangeValuesAreClamped) {
  const Config cfg = parse({"bayan", "dir", "--block-size=0", "-H", "crc32",
                            "--depth=-5", "--min-size=0"});
  EXPECT_EQ(cfg.block, 5u);   // <1 -> 5
  EXPECT_EQ(cfg.depth, 0u);   // <0 -> 0
  EXPECT_EQ(cfg.min_size, 1u); // <1 -> 1
  EXPECT_EQ(cfg.hash, Config::Hash::crc32);
}
