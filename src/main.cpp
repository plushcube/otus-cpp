#include <controllers/editor.h>
#include <services/svg_service.h>
#include <ui/editor_view.h>

#include <memory>

int main(int, char **) {
  std::shared_ptr<SVG_Service> svg = std::make_shared<SVG_Service>();
  std::unique_ptr<Editor> editor = std::make_unique<Editor>(svg, svg);
  std::unique_ptr<EditorView> app_view = std::make_unique<EditorView>(std::move(editor));

  app_view->show();
  app_view->send_action(UI_Action::new_image);

  app_view->send_action(UI_Action::draw_rectangle);
  app_view->send_action(UI_Action::draw_circle);
  app_view->send_action(UI_Action::draw_ellipse);
  app_view->send_action(UI_Action::draw_line);

  app_view->send_mouse_event(UI_Mouse::left_btn_up, {125.0f, 145.0f});
  app_view->send_action(UI_Action::remove);

  app_view->send_action(UI_Action::save_image);
  app_view->send_action(UI_Action::quit);

  return 0;
}
