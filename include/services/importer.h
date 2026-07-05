#pragma once

#include "models/image.h"
#include <memory>
#include <string>

class Importer {
public:
  virtual ~Importer() = default;

  virtual std::unique_ptr<Image> import_from_file(const std::string &) const = 0;
};
