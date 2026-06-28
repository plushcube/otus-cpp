#include <controllers/editor.h>
#include <services/svg_service.h>
#include <memory>

int main(int, char **) {
  std::shared_ptr<SVG_Service> svg = std::make_shared<SVG_Service>();
  std::unique_ptr<Editor> editor = std::make_unique<Editor>(svg, svg);
  editor->create_image();
  editor->get_canvas()->draw();
  editor->save_image("some_image.svg");
  editor->close_image();
  return 0;
}
