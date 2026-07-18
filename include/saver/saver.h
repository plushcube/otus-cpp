#pragma once

#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>

class Saver {
public:
  explicit Saver() = default;

  void save(const time_t &t, const std::vector<std::string> &v) const {
    const auto filename = make_filename(t);
    const auto content = make_content(v);
    save_to_file(content, filename);
  }

private:
  std::string make_content(const std::vector<std::string> &v) const noexcept {
    std::string s;
    for (const auto &c : v) {
      s += c;
      if (c != v.back()) {
        s += ", ";
      }
    }
    return s;
  }

  std::string make_filename(const time_t &t) const noexcept {
    std::tm tm;
    localtime_r(&t, &tm);
    int h = tm.tm_hour;
    int m = tm.tm_min;
    int s = tm.tm_sec;
    return std::format("{:02d}{:02d}{:02d}.log", h, m, s);
  }

  void save_to_file(const std::string &c, const std::string &f) const {
    std::filesystem::path path = f;
    std::ofstream out(path, std::ios::app);
    if (!out.is_open()) {
      throw std::runtime_error("Failed to open file: " + path.string());
    }
    out << c << std::endl;
    out.close();
  }
};
