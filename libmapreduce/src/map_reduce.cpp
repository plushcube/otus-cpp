#include "map_reduce.h"

#include <cctype>
#include <charconv>
#include <cstdio>
#include <iomanip>
#include <ostream>

namespace {

// Схема датасета New York City Airbnb Open Data: 16 столбцов, цена — 10-й.
constexpr size_t kColumnCount = 16;
constexpr size_t kPriceColumn = 9;

// Границы поля цены и общее число полей строки CSV.
struct PriceLocation {
  size_t count = 0;
  size_t begin = 0;
  size_t end = 0;
};

// Запятые внутри кавычек не разделяют поля. Удвоенная кавычка внутри поля
// переключает состояние дважды, то есть на разметку полей не влияет.
PriceLocation locate_price(std::string_view line) {
  PriceLocation location;
  bool quoted = false;

  for (size_t i = 0; i < line.size(); ++i) {
    const char symbol = line[i];
    if (symbol == '"') {
      quoted = !quoted;
    } else if (symbol == ',' && !quoted) {
      ++location.count;
      if (location.count == kPriceColumn) {
        location.begin = i + 1;
      } else if (location.count == kPriceColumn + 1) {
        location.end = i;
      }
    }
  }

  ++location.count;
  return location;
}

// Строки могут заканчиваться \r, а числа — пробелами.
std::string_view trimmed(std::string_view text) {
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())) != 0) {
    text.remove_prefix(1);
  }
  while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())) != 0) {
    text.remove_suffix(1);
  }
  return text;
}

bool parse_number(std::string_view text, double &value) {
  if (text.empty()) {
    return false;
  }

  const char *begin = text.data();
  const char *end = begin + text.size();
  const std::from_chars_result result = std::from_chars(begin, end, value);

  return result.ec == std::errc() && result.ptr == end;
}

template <typename Fn> void for_each_line(std::string_view input, Fn fn) {
  size_t start = 0;
  while (start < input.size()) {
    const size_t end = input.find('\n', start);
    if (end == std::string_view::npos) {
      fn(input.substr(start));
      return;
    }

    fn(input.substr(start, end - start));
    start = end + 1;
  }
}

} // namespace

namespace map_reduce {

std::string read_stdin() {
  std::string input;
  char chunk[1 << 16];

  size_t got = 0;
  while ((got = std::fread(chunk, 1, sizeof(chunk), stdin)) > 0) {
    input.append(chunk, got);
  }
  return input;
}

void map_price(std::string_view input, std::ostream &out) {
  for_each_line(input, [&out](std::string_view line) {
    const PriceLocation location = locate_price(line);
    if (location.count != kColumnCount) {
      return;
    }

    long price = 0;
    const char *begin = line.data() + location.begin;
    const char *end = line.data() + location.end;
    const std::from_chars_result result = std::from_chars(begin, end, price);
    if (result.ec != std::errc() || result.ptr != end) {
      return;
    }

    out << price << '\n';
  });
}

void reduce_mean(std::string_view input, std::ostream &out) {
  size_t count = 0;
  double sum = 0.0;

  for_each_line(input, [&count, &sum](std::string_view line) {
    double value = 0.0;
    if (!parse_number(trimmed(line), value)) {
      return;
    }

    ++count;
    sum += value;
  });

  if (count == 0) {
    return;
  }
  out << std::fixed << std::setprecision(2) << sum / static_cast<double>(count) << '\n';
}

void reduce_variance(std::string_view input, std::ostream &out) {
  size_t count = 0;
  double mean = 0.0;
  double squared_deviations = 0.0;

  for_each_line(input, [&count, &mean, &squared_deviations](std::string_view line) {
    double value = 0.0;
    if (!parse_number(trimmed(line), value)) {
      return;
    }

    ++count;
    const double delta = value - mean;
    mean += delta / static_cast<double>(count);
    squared_deviations += delta * (value - mean);
  });

  if (count == 0) {
    return;
  }
  out << std::fixed << std::setprecision(2) << squared_deviations / static_cast<double>(count) << '\n';
}

} // namespace map_reduce
