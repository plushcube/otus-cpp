#include "utils.h"

#include <atomic>
#include <chrono>
#include <limits>
#include <utility>

#ifndef FIXTURES_DIR
#error "FIXTURES_DIR must be defined by CMake"
#endif

namespace test_utils {

Config make_config(std::vector<std::string> dirs, std::vector<std::string> exclude,
                   Config::Hash hash) {
  // Глубина обхода не ограничена (FileList кастует depth в int, поэтому
  // берём максимум int, а не size_t); тестам глубины задают depth явно.
  constexpr size_t unlimited_depth = static_cast<size_t>(std::numeric_limits<int>::max());
  return Config{std::move(dirs), std::move(exclude), {}, unlimited_depth, 1024, 1, hash};
}

std::vector<std::filesystem::path> collect_files(const Config &cfg) {
  std::vector<std::filesystem::path> files;
  for (const auto &entry : FileList{cfg}) {
    files.push_back(entry.path());
  }
  return files;
}

std::set<std::string> file_names(const Config &cfg) {
  std::set<std::string> names;
  for (const auto &entry : FileList{cfg}) {
    names.insert(entry.path().filename().string());
  }
  return names;
}

std::vector<std::filesystem::path> collect_regular_files(const std::filesystem::path &root,
                                                         size_t min_size) {
  std::vector<std::filesystem::path> files;
  std::error_code ec;
  for (std::filesystem::recursive_directory_iterator it(root, ec), end; it != end && !ec;
       it.increment(ec)) {
    if (it->is_regular_file() && it->file_size() >= min_size) {
      files.push_back(it->path());
    }
  }
  return files;
}

std::filesystem::path fixture_dir(const std::string &name) {
  return std::filesystem::path(FIXTURES_DIR) / name;
}

namespace {

std::string unique_dir_name() {
  static std::atomic<size_t> counter{0};
  const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  return "bayan_test_" + std::to_string(static_cast<size_t>(now)) + "_" +
         std::to_string(counter.fetch_add(1));
}

} // namespace

TempDir::TempDir() : m_path(std::filesystem::temp_directory_path() / unique_dir_name()) {
  std::filesystem::create_directories(m_path);
}

TempDir::~TempDir() { std::filesystem::remove_all(m_path); }

} // namespace test_utils
