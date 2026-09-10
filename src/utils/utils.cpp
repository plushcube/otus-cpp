#include "utils.h"

#include <climits>
#include <iostream>
#include <stdexcept>

unsigned long Utils::parse_uint(const std::string &s) {
  std::size_t pos = 0;
  const unsigned long v = std::stoul(s, &pos);
  if (pos != s.size()) {
    throw std::invalid_argument("not a number: " + s);
  }
  return v;
}

unsigned short Utils::parse_port(const std::string &s) {
  unsigned long port = Utils::parse_uint(s);
  if (port < 1000 || port > USHRT_MAX) {
    throw std::invalid_argument("invalid port number (1000...65535)");
  }
  return static_cast<unsigned short>(port);
}

bool Utils::is_reply_end(const std::string &line) { return line == "OK" || line.rfind("ERR", 0) == 0; }

void Utils::print_usage(const std::string &c) { std::cerr << "Usage: " << c << " <port>" << std::endl; }
