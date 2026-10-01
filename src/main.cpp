#include <iostream>
#include <variant>

#include "arg_parser.h"
#include "runners/mlp_runner.h"

using namespace std;

int main(const int argc, const char **argv) {
  const auto r = ArgParser::parse_args(argc, argv);

  if (holds_alternative<ArgParser::Command>(r)) {
    const auto c = get<ArgParser::Command>(r);
    switch (c) {
    case ArgParser::Command::Version:
      ArgParser::print_version(cout);
      break;
    case ArgParser::Command::Help:
      ArgParser::print_usage(cout);
      break;
    }
    return 0;

  } else if (holds_alternative<Config>(r)) {
    const Config c = get<Config>(r);
    switch (c.model) {
    case Config::Model::LogReg:
      cerr << "Ошибка: В этой версии модель логистической регрессии не реализована" << endl;
      return 1;
    case Config::Model::CatBoost:
      cerr << "Ошибка: В этой версии модель CatBoost не реализована" << endl;
      return 1;
    case Config::Model::MLP:
      return MlpRunner::run(c.filename);
    case Config::Model::CNN:
      cerr << "Ошибка: В этой версии модель сверточной нейронной сети не реализована" << endl;
      return 1;
    case Config::Model::Unknown:
      cerr << "Ошибка: Неизвестная модель" << endl;
      return 1;
    }

  } else {
    ArgParser::print_usage(cerr);
    return 1;
  }
}
