#pragma once

#include <cstdint>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "processor.h"

class Saver : public Processor {
public:
  explicit Saver(std::string postfix = {}) : m_postfix(std::move(postfix)) {}

  void process(const Collector::Bulk &b) const noexcept override {
    try {
      const auto content = make_content(b.commands);
      save_to_file(content, make_filename(b.start));
    } catch (const std::exception &e) {
      std::cerr << "Saver: " << e.what() << std::endl;
    }
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

  std::string make_filename(const time_t &t) const {
    const uint64_t ts = static_cast<uint64_t>(t);
    const std::string base =
        m_postfix.empty() ? std::format("bulk{}.log", ts) : std::format("bulk{}_{}.log", ts, m_postfix);
    if (base == m_last_base) {
      ++m_sequence; // ещё один блок за ту же секунду на этом же потоке
    } else {
      m_last_base = base;
      m_sequence = 0;
    }
    if (m_sequence == 0) {
      return base;
    }
    return m_postfix.empty() ? std::format("bulk{}_{}.log", ts, m_sequence)
                             : std::format("bulk{}_{}_{}.log", ts, m_postfix, m_sequence);
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

  std::string m_postfix;
  mutable std::string m_last_base;
  mutable unsigned m_sequence{0};
};
