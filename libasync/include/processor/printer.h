#include "processor.h"

#include <iostream>
#include <mutex>

class Printer : public Processor {
public:
  explicit Printer() = default;

  void process(const Collector::Bulk &b) const noexcept override {
    // Общий std::cout: запись сериализуется (без этого libc++ лениво
    // инициализирует fill-символ cout с data race, а строки могут рваться).
    // Сериализуется только сам вывод, не обработка команд.
    static std::mutex m;
    std::lock_guard lk(m);
    std::cout << make_content(b.commands) << std::endl;
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
};
