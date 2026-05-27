#include <allocator.h>
#include <container.h>

#include <iostream>
#include <map>

using namespace std;

int factorial(const int n) {
  int result = 1;
  for (int i = 2; i <= n; ++i) {
    result *= i;
  }
  return result;
}

int main(int, char **) {
  std::cout << "Standard std::map:" << std::endl;
  auto task1 = std::map<int, int>{};
  for (int i = 0; i < 10; ++i) {
    task1[i] = factorial(i);
  }
  for (const auto &pair : task1) {
    std::cout << pair.first << " " << pair.second << std::endl;
  }

  std::cout << "Standard std::map with custom allocator:" << std::endl;
  using mapValueType = std::map<int, int>::value_type;
  auto task2 = std::map<int, int, std::less<>, PlushAllocator<mapValueType>>{};
  for (int i = 0; i < 10; ++i) {
    task2[i] = factorial(i);
  }
  for (const auto &pair : task2) {
    std::cout << pair.first << " " << pair.second << std::endl;
  }

  std::cout << "Custom container:" << std::endl;
  PlushContainer<int> task3{};
  for (int i = 0; i < 10; ++i) {
    task3.push_back(i);
  }
  for (const auto &value : task3) {
    std::cout << value << " ";
  }
  std::cout << std::endl;

  std::cout << "Custom container with custom allocator:" << std::endl;
  PlushContainer<int, PlushAllocator<int>> task4{};
  for (int i = 0; i < 10; ++i) {
    task4.push_back(i);
  }
  for (const auto &value : task4) {
    std::cout << value << " ";
  }
  std::cout << std::endl;

  std::cout << "Done." << std::endl;
  return 0;
}
