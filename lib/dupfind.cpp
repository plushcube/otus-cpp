#include <dupfind.h>

#include <filesystem>
#include <ranges>

using namespace std;

void DupFinder::run() {
  // TODO: fill me with your code
  for (const auto &d : m_config.dirs) {
    auto files =
        filesystem::recursive_directory_iterator(d) | views::filter([&](auto &e) { return is_match(m_config, e); });
  }
}

bool DupFinder::is_match(const Config &, const std::filesystem::directory_entry &) { return true; }
