#include "lib.h"

#include <algorithm>

using namespace std;

void sort_rev(vector<ip4_t> &ip_pool) {
  sort(ip_pool.begin(), ip_pool.end(), [](const auto &lhs, const auto &rhs) {
    return lhs > rhs;
  });
}

vector<ip4_t> filter(const vector<ip4_t> &ip_pool, std::function<bool(const ip4_t &)> pred) {
  vector<ip4_t> result;
  for (const auto &ip : ip_pool) {
    if (pred(ip)) {
      result.push_back(ip);
    }
  }
  return result;
}
