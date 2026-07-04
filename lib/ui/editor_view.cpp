#include <models/action.h>
#include <ui/editor_view.h>

#include <iostream>

EditorView::EditorView(std::unique_ptr<Editor> e) : p_editor(std::move(e)) {
  p_tools = std::make_unique<ToolbarView>();

  p_tools->add_button({UI_Action::new_image, "new", [&] {
                         show_new_image_dialog();
                       }});
  p_tools->add_button({UI_Action::load_image, "load", [&] {
                         show_load_image_dialog();
                       }});
  p_tools->add_button({UI_Action::save_image, "save", [&] {
                         show_save_image_dialog();
                       }});
  p_tools->add_button({UI_Action::quit, "quit", [&] {
                         quit();
                       }});

  const auto exec = [&](auto &&func) {
    auto c = p_editor->get_canvas();
    if (auto canvas = c.lock()) {
      func(canvas);
    } else {
      std::cerr << "no canvas!" << std::endl;
    }
  };

  p_tools->add_button({UI_Action::draw_rectangle, "rectangle", [exec] {
                         exec([](auto c) {
                           c->add_rectangle({50.0f, 50.0f}, {150.0f, 150.0f});
                         });
                       }});
  p_tools->add_button({UI_Action::draw_circle, "circle", [exec] {
                         exec([](auto c) {
                           c->add_circle({100.0f, 100.0f}, 50.0f);
                         });
                       }});
  p_tools->add_button({UI_Action::draw_ellipse, "ellipse", [exec] {
                         exec([](auto c) {
                           c->add_ellipse({100.0f, 100.0f}, 25.0f, 50.0f);
                         });
                       }});
  p_tools->add_button({UI_Action::draw_line, "line", [exec] {
                         exec([](auto c) {
                           c->add_line({50.0f, 50.0f}, {150.0f, 150.0f});
                         });
                       }});
  p_tools->add_button({UI_Action::remove, "delete", [exec] {
                         exec([](auto c) {
                           c->del_selected();
                         });
                       }});
}

void EditorView::show() const noexcept {
  std::cout << "show editor view" << std::endl;
  p_tools->show();
}

void EditorView::quit() const noexcept {
  p_editor->close_image();
  std::cout << "close editor view" << std::endl;
}

void EditorView::send_action(const UI_Action &a) const noexcept {
  for (size_t i = 0; i < p_tools->item_count(); ++i) {
    const auto b = p_tools->button_at(i);
    if (b.type == a) {
      b.action();
    }
  }
}

void EditorView::send_mouse_event(const UI_Mouse &e, const Coord &c) const noexcept {
  switch (e) {
  case UI_Mouse::left_btn_up:
    if (const auto canvas = p_editor->get_canvas().lock()) {
      std::cout << "try to toggle selection of shape at {" << c.x << ", " << c.y << "}" << std::endl;
      canvas->toggle_selected(c);
    }
    break;
  default:
    break;
  }
}

void EditorView::show_new_image_dialog() const noexcept {
  std::cout << "show new image dialog" << std::endl;
  p_editor->create_image({200.0f, 200.0f});
}

void EditorView::show_load_image_dialog() const noexcept {
  std::cout << "show load image dialog" << std::endl;
  p_editor->load_image("image.svg");
}

void EditorView::show_save_image_dialog() const noexcept {
  std::cout << "show save image dialog" << std::endl;
  p_editor->save_image("image.svg");
}
