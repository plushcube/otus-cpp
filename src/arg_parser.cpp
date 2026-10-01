#include "arg_parser.h"
#include "config.h"
#include "version.h"

using namespace ArgParser;
using namespace std;

namespace {

Config::Model get_model(const string_view &s) {
  if (s == "LR") {
    return Config::Model::LogReg;
  }
  if (s == "CB") {
    return Config::Model::CatBoost;
  }
  if (s == "MLP") {
    return Config::Model::MLP;
  }
  if (s == "CNN") {
    return Config::Model::CNN;
  }
  return Config::Model::Unknown;
}

} // namespace

void ArgParser::print_usage(ostream &out) {
  out << "fashio_mnist <filename> [LR|CB|MLP|CNN]\n"
      << "    filename    путь к csv-файлу с тестовыми данными\n"
      << "    LB          логистическая регрессия (не реализовано)\n"
      << "    CB          CatBoost (не реализовано)\n"
      << "    MLP         многослойный перцептрон\n"
      << "    CNN         сверточная нейронная сеть (не реализовано)\n"
      << "    --version   версия продукта\n"
      << "    --help      эта справка\n";
}

void ArgParser::print_version(std::ostream &out) {
  out << "fashio_mnist\n"
      << "Version: " << "0.0." << PROJECT_VERSION_PATCH << endl;
}

Result ArgParser::parse_args(const int argc, const char **argv) {
  Config config;
  for (int i = 1; i < argc; ++i) {
    const string_view arg = argv[i];

    if (arg == "--version") {
      return Result(Command::Version);
    }

    if (arg == "--help") {
      return Result(Command::Help);
    }

    const auto m = get_model(arg);
    if (m != Config::Model::Unknown) {
      config.model = m;
      continue;
    }

    config.filename = arg;
  }
  return Result(config);
}
