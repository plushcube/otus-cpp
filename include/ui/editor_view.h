#pragma once

#include "controllers/editor.h"
#include "models/rect.h"
#include "toolbar_view.h"

#include <memory>

class EditorView {
public:
  EditorView(std::unique_ptr<Editor>);

  void show() const noexcept;
  void quit() const noexcept;

  void send_action(const UI_Action &) const noexcept;
  void send_mouse_event(const UI_Mouse &, const Coord &) const noexcept;

  void show_new_image_dialog() const noexcept;
  void show_load_image_dialog() const noexcept;
  void show_save_image_dialog() const noexcept;

private:
  std::unique_ptr<Editor> p_editor;
  std::unique_ptr<ToolbarView> p_tools;
};
