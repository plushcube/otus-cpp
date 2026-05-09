#include <iostream>
#include <lib.h>

using namespace std;

string format_ip4(const ip4_t &ip4) {
  return to_string(get<0>(ip4)) + "." +
         to_string(get<1>(ip4)) + "." +
         to_string(get<2>(ip4)) + "." +
         to_string(get<3>(ip4));
}

auto read_input() {
  vector<ip4_t> result;
  for (string line; getline(cin, line), !line.empty();) {
    result.push_back(read_ip(line));
  }
  return result;
}

void print_pool(const vector<ip4_t> &ip_pool) {
  for (const auto &ip4 : ip_pool) {
    cout << format_ip4(ip4) << endl;
  }
}

int main(int, char **) {
  auto ip_pool = read_input();

  sort_rev(ip_pool);
  print_pool(ip_pool);
  print_pool(filter(ip_pool, [](auto ip4) {
    return get<0>(ip4) == 1;
  }));
  print_pool(filter(ip_pool, [](auto ip4) {
    return get<0>(ip4) == 46 && get<1>(ip4) == 70;
  }));
  print_pool(filter(ip_pool, [](auto ip4) {
    return get<0>(ip4) == 46 || get<1>(ip4) == 46 || get<2>(ip4) == 46 || get<3>(ip4) == 46;
  }));

  return 0;
}
