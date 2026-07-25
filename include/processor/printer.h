#include "processor.h"

#include <iostream>

class Printer : public Processor {
public:
  explicit Printer() = default;

  void process(const Collector::Bulk &b) const noexcept override { std::cout << make_content(b.commands) << std::endl; }

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
};
