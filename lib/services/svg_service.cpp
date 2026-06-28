#include <services/svg_service.h>

#include <iostream>

std::unique_ptr<Image> SVG_Service::import_from_file(const std::string &) const {
  std::cout << "import svg" << std::endl;
  return nullptr;
}

void SVG_Service::export_to_file(const Image *, const std::string &) const {
  std::cout << "export to svg" << std::endl;
}
