#pragma once

#include "exporter.h"
#include "importer.h"

class SVG_Service: public Exporter, public Importer {
public:
  SVG_Service() = default;

  std::unique_ptr<Image> import_from_file(const std::string &) const override;
  void export_to_file(const Image *, const std::string &) const override;
};
