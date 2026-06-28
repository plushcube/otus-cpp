#include <controllers/editor.h>
#include <iostream>

void Editor::create_image() {
  if (p_canvas) {
    close_image();
  }
  p_canvas = std::make_unique<Canvas>(
    std::make_unique<Image>()
  );
  std::cout << "new image created" << std::endl;
}

void Editor::load_image(const std::string &filepath) {
  if (p_canvas) {
    close_image();
  }
  p_canvas = std::make_unique<Canvas>(
    p_importer->import_from_file(filepath)
  );
  std::cout << "image loaded" << std::endl;
}

void Editor::save_image(const std::string &filepath) {
  if (!p_canvas) {
    return;
  }
  p_exporter->export_to_file(p_canvas->raw_image(), filepath);
  std::cout << "image saved" << std::endl;
}

void Editor::close_image() {
  p_canvas = nullptr;
  std::cout << "image closed" << std::endl;
}
