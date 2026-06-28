#pragma once

#include "../models/image.h"
#include <string>

class Exporter {
public:
  virtual ~Exporter() = default;

  virtual void export_to_file(const Image *, const std::string &) const = 0;
};
