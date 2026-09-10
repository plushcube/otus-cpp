#pragma once

#include <string>

namespace Utils {

unsigned long parse_uint(const std::string &);
unsigned short parse_port(const std::string &);
bool is_reply_end(const std::string &);
void print_usage(const std::string &);

}; // namespace Utils
