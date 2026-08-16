#include <gtest/gtest.h>

#include <dupfind.h>

#include "utils/utils.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace test_utils;

namespace {

// Захват stdout на время жизни объекта.
class CoutCapture {
public:
  CoutCapture() : m_old(std::cout.rdbuf(m_buf.rdbuf())) {}
  ~CoutCapture() { std::cout.rdbuf(m_old); }

  std::string str() const { return m_buf.str(); }

private:
  std::ostringstream m_buf;
  std::streambuf *m_old;
};

std::vector<std::string> nonempty_lines(const std::string &s) {
  std::vector<std::string> lines;
  std::istringstream in(s);
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty()) {
      lines.push_back(line);
    }
  }
  return lines;
}

} // namespace

TEST(DupFinderTest, PrintsAllFixtureFiles) {
  const Config cfg = make_config({fixture_dir().string()});

  std::set<fs::path> actual;
  {
    CoutCapture cap;
    DupFinder{cfg}.run();
    for (const auto &line : nonempty_lines(cap.str())) {
      ASSERT_EQ(line.substr(0, 6), "file: ");
      std::string path_str = line.substr(6);
      // operator<< для directory_entry печатает путь в кавычках: "file: "/abs/path""
      if (path_str.size() >= 2 && path_str.front() == '"' && path_str.back() == '"') {
        path_str = path_str.substr(1, path_str.size() - 2);
      }
      actual.insert(fs::path(path_str));
    }
  }

  const auto expected_files = collect_regular_files(fixture_dir());
  const std::set<fs::path> expected(expected_files.begin(), expected_files.end());
  EXPECT_EQ(actual, expected);
}

TEST(DupFinderTest, MissingDirProducesNoOutput) {
  const Config cfg = make_config({"/nonexistent_dir_for_bayan_test"});

  CoutCapture cap;
  EXPECT_NO_THROW(DupFinder{cfg}.run());
  EXPECT_TRUE(nonempty_lines(cap.str()).empty());
}
