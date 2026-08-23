#pragma once

#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

#include "processor.h"

class Saver : public Processor {
public:
  explicit Saver() = default;

  void process(const Collector::Bulk &b) const noexcept override {
    const auto filename = make_filename(b.start);
    const auto content = make_content(b.commands);
    save_to_file(content, filename);
  }

private:
  std::string make_content(const std::vector<std::string> &v) const noexcept {
    std::string s = "bulk: ";
    for (const auto &c : v) {
      s += c;
      if (c != v.back()) {
        s += ", ";
      }
    }
    return s;
  }

  std::string make_filename(const time_t &t) const noexcept {
    return std::format("bulk{}.log", static_cast<uint64_t>(t));
  }

  void save_to_file(const std::string &c, const std::string &f) const {
    // Параллельные контексты могут получить один timestamp и писать в один
    // файл: open/append/close сериализуются, чтобы строки не перемешивались.
    static std::mutex m;
    std::lock_guard lk(m);
    std::filesystem::path path = f;
    std::ofstream out(path, std::ios::app);
    if (!out.is_open()) {
      throw std::runtime_error("Failed to open file: " + path.string());
    }
    out << c << std::endl;
    out.close();
  }
};
