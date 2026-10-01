#pragma once

#include "config.h"

#include <ostream>
#include <variant>

namespace ArgParser {

enum class Command { Version, Help };
using Result = std::variant<Config, Command>;

[[nodiscard]] Result parse_args(const int, const char **);
void print_usage(std::ostream &);
void print_version(std::ostream &);

} // namespace ArgParser
