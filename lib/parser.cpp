#include "lib.h"

using namespace std;

ip4_t parse_ip4(const string &ip) {
  auto start = ip.begin();
  auto end = ip.end();
  vector<uint8_t> result;
  while (start != end) {
    auto pos = find(start, end, '.');
    if (pos != end) {
      result.push_back(static_cast<uint8_t>(stoi(string(start, pos))));
      start = pos + 1;
    } else {
      result.push_back(static_cast<uint8_t>(stoi(string(start, end))));
      break;
    }
  }
  if (result.size() != 4) {
    return make_tuple(0, 0, 0, 0);
  } else {
    return make_tuple(result[0], result[1], result[2], result[3]);
  }
}

ip4_t read_ip(const string &line, const char delimeter) {
  auto pos = find(line.cbegin(), line.cend(), delimeter);
  if (pos != line.cend()) {
    return parse_ip4(string(line.cbegin(), pos));
  }
  return parse_ip4(string(line.cbegin(), line.cend()));
}
