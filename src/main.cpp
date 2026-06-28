#include <controllers/editor.h>
#include <memory>
#include <services/svg_service.h>

int main(int, char **) {
  std::shared_ptr<SVG_Service> svg = std::make_shared<SVG_Service>();
  std::unique_ptr<Editor> editor = std::make_unique<Editor>(svg, svg);

  editor->create_image({200.0f, 200.0f});

  Canvas *canvas = editor->get_canvas();
  canvas->add_rectangle({50.0f, 50.0f}, {150.0f, 150.0f});
  canvas->add_circle({100.0f, 100.0f}, 50.0f);
  canvas->add_ellipse({100.0f, 100.0f}, 25.0f, 50.0f);
  canvas->add_line({50.0f, 50.0f}, {150.0f, 150.0f});

  auto id = canvas->get_shape_id({125.0f, 145.0f});
  if (id > 0) {
    canvas->del_shape(id);
  }

  editor->save_image("some_image.svg");
  editor->close_image();
  return 0;
}
