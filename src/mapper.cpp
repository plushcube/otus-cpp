#include <iostream>
#include <string>

int main() {
  std::string line;
  while (std::getline(std::cin, line)) {
    std::cout << line << "\t" << "1" << std::endl;
  }

  return 0;
}
