#pragma once

#include <cstddef>
#include <expected>
#include <string>
#include <vector>

struct Config {
  enum class Hash { crc32, md5, sha1 };

  const std::vector<std::string> dirs;
  const std::vector<std::string> exclude;
  const std::vector<std::string> masks;

  const size_t depth;
  const size_t block;
  const size_t min_size;
  const Hash hash;

  static std::expected<Config, std::string> make_config(int, char **);
};
