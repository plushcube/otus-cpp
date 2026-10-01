#pragma once

#include <string>

struct Config {
  enum class Model { LogReg, CatBoost, MLP, CNN, Unknown };

  std::string filename;
  Model model{Model::Unknown};

  bool is_valid() const { return !filename.empty() && model != Model::Unknown; }
};
