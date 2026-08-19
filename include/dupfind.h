#pragma once

#include <config.h>
#include <hasher.h>

#include <filesystem>
#include <fstream>

class DupFinder {
public:
  using Doubles = std::vector<std::string>;
  using Result = std::vector<Doubles>;

  explicit DupFinder(const Config &cfg) : m_config(cfg), m_hasher(Hasher::make_hasher(cfg)) {}

  Result run();

private:
  using Buffer = std::vector<char>;

  struct File {
    std::filesystem::directory_entry entry;
    std::vector<Hasher::Hash> blocks;
    std::ifstream stream = {};
    size_t hashed = 0;
    bool is_grouped = false;
  };

  const Config m_config;
  std::vector<File> m_files;
  std::unique_ptr<Hasher> m_hasher;

  bool match(File &, File &, Buffer &);
  bool read_next_block(File &, Buffer &);
};
