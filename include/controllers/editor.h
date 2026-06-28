#pragma once

#include "../services/exporter.h"
#include "../services/importer.h"
#include "canvas.h"

#include <memory>
#include <string>

class Editor {
public:
  explicit Editor(std::shared_ptr<Exporter> e, std::shared_ptr<Importer> i) : p_exporter(e), p_importer(i) {}

  void create_image();
  void load_image(const std::string &);
  void save_image(const std::string &);
  void close_image();
  Canvas *get_canvas() const { return p_canvas.get(); }

private:
  std::unique_ptr<Canvas> p_canvas{nullptr};
  std::shared_ptr<Exporter> p_exporter;
  std::shared_ptr<Importer> p_importer;
};
