#include <iostream>

#include <config.h>

using namespace std;

int main(int ac, char **av) {
  const auto r = Config::make_config(ac, av);
  if (!r.has_value()) {
    cout << r.error() << endl;
    return 1;
  }

  const Config c = r.value();

  return 0;
}
