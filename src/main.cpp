#include <async/async.h>
#include <iostream>

int main(int argc, char **argv) {
  size_t n = 3;
  if (argc > 1) {
    try {
      n = std::max(1, static_cast<int>(std::stoull(argv[1])));
    } catch (const std::invalid_argument &) {
      std::cerr << "Error: argument '" << argv[1] << "' is not a number." << std::endl;
      return 1;
    } catch (const std::out_of_range &) {
      std::cerr << "Error: number '" << argv[1] << "' is not valid." << std::endl;
      return 1;
    }
  }

  const auto p = connect(n);
  std::string s;
  while (std::getline(std::cin, s)) {
    receive(p, s);
  }
  disconnect(p);

  return 0;
}
