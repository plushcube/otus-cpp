#pragma once

#include <sstream>
#include <string>
#include <string_view>

namespace mr_test {

// Прогоняет map/reduce-функцию над строкой входа и возвращает её вывод.
template <typename Fn> std::string run(Fn fn, const std::string &input) {
  std::ostringstream out;
  fn(std::string_view(input), out);
  return out.str();
}

// Читает единственное число из вывода reducer'а.
inline double scalar(const std::string &output) {
  std::istringstream in(output);
  double value = 0.0;
  in >> value;
  return value;
}

} // namespace mr_test
