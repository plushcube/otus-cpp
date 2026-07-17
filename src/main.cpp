#include <di/real_container.h>
#include <parser.h>

#include <iostream>

void print_bulk(const std::vector<std::string> &v) {
  std::cout << "bulk:";
  for (const auto &s : v) {
    std::cout << " " << s << ",";
  }
  std::cout << "\b" << std::endl;
}

int main(int, char **) {
  constexpr size_t n = 3;

  auto di = std::make_shared<RealContainer<n>>();
  Parser<n> p(di, print_bulk);
  std::string s;

  p.start();
  while (std::getline(std::cin, s)) {
    p.process(s);
  }
  p.stop();

  return 0;
}
