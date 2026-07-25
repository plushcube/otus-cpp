#include <di/real_container.h>
#include <processor/printer.h>
#include <processor/saver.h>

#include <parser.h>

#include <iostream>

int main(int argc, char **argv) {
  size_t n = 3;
  if (argc > 1) {
    try {
      n = static_cast<size_t>(std::stoull(argv[1]));
    } catch (const std::invalid_argument &) {
      std::cerr << "Error: argument '" << argv[1] << "' is not a number." << std::endl;
      return 1;
    } catch (const std::out_of_range &) {
      std::cerr << "Error: number '" << argv[1] << "' is not valid." << std::endl;
      return 1;
    }
  }

  auto di = std::make_shared<RealContainer>();
  Parser p(di, n);
  p.add_processor(std::make_shared<Printer>());
  p.add_processor(std::make_shared<Saver>());

  std::string s;
  p.start();
  while (std::getline(std::cin, s)) {
    p.process(s);
  }
  p.stop();

  return 0;
}
