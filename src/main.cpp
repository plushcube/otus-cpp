#include <iostream>

// #include <debug.h>
#include <dupfind.h>

using namespace std;

int main(int ac, char **av) {
  const auto r = Config::make_config(ac, av);
  if (!r.has_value()) {
    cout << r.error() << endl;
    return 1;
  }

  const Config c = r.value();
  // print_config(c);

  DupFinder f{c};
  for (const auto &block : f.run()) {
    for (const auto &file : block) {
      cout << file << "\n";
    }
    cout << endl;
  }

  return 0;
}
